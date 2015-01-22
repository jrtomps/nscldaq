/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Class: CVmeCommand

//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "VMECommand.h"    				
#include "TCLString.h"
#include "TCLInterpreter.h"
#include "TCLResult.h"
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

static const string aSubCommands[] = {
  string("create"),
  string("list"),
  string("delete")
};

static vector<string> vSubCommands(aSubCommands, &aSubCommands[3]);
#define SC_CREATE 0
#define SC_LIST   1
#define SC_DELETE 2
#define SC_NOMATCH -1





// The definitions below create a mapping table between
//  address space selectors.
//  For compatibility with old scripts, we need to support the old 
// NSCL/Bit3 names directly as well as new style device independent
// names.
//

typedef struct _MapEntry  {
  const string me_sLogicalDev;	// Name passed in by user on cmd.
  const 
  CVmeModule::Space  me_eAPIDev; // Name expected by the api.
} MapEntry;

static const MapEntry kaDeviceMap[] = {
  // Short I/O space (A16):

  {"shortio"      , CVmeModule::a16d16   },
  {"shortio16"    , CVmeModule::a16d16   },
  {"vme16"        , CVmeModule::a16d16   },
  {"vme16d16"     , CVmeModule::a16d16   },
  {"/dev/vme16d16", CVmeModule::a16d16   },

  // Standard addressing space (A24):

  {"standard"     , CVmeModule::a24d32   },
  {"standard16"   , CVmeModule::a24d32   },
  {"vme24"        , CVmeModule::a24d32   },
  {"vme2416"      , CVmeModule::a24d32   },
  {"/dev/vme24d32", CVmeModule::a24d32   },
  {"/dev/vme24d16", CVmeModule::a24d32   },

  // extended address space (A32):

  {"extended"     , CVmeModule::a32d32   },
  {"extended16"   , CVmeModule::a32d32   },
  {"vme32"        , CVmeModule::a32d32   },
  {"vme32d16"     , CVmeModule::a32d32   },
  {"/dev/vme32d32", CVmeModule::a32d32   },
  {"/dev/vme32d16", CVmeModule::a32d32   },

  // Geographical addressing... only A24/D32 is supported:

  {"geographical" , CVmeModule::geo      },
  {"geo" ,          CVmeModule::geo      }



};
const unsigned int knDeviceMapSize = sizeof(kaDeviceMap) / 
                                     sizeof(MapEntry);



// Functions for class CVmeCommand

//  Function: 	operator()
//  Purpose: 	
// Called by the tcl vme command.
// Dispatches based on the subcommand to Add, Listmaps or DeleteMaps.
//  Parameters: CTCLInterpreter& rInterpreter, 
//              CTCLResult& rResult, int nArgs, char** pArgs
//
// COmmand syntax:
//
//  vme create spacename [-device name] base size
//      Creates a new vme map named spacename.  If present, -device's
//      parameter indicates which VME device to open that in turn determines
//      the address modifiers.  base Is the offset into the VME address
//      space for the base of the map.  size is the number of bytes in the
//      map. The spacename is created as a new command whose subcommands
//      allow you to read/write the vme bus.
// vme list
//      Lists the set of address spaces defined.
// vme delete spacename
//      Deletes the specified map.
//
int CVmeCommand::operator()(CTCLInterpreter& rInterpreter, 
			    CTCLResult& rResult, int nArgs, char** pArgs)  
{
  try {
    nArgs--;
    pArgs++;
    if(nArgs <= 0) return Usage(rResult);
    
    string sSubCommand(*pArgs);	// SubCommand string
    nArgs--;
    pArgs++;
    int nSubCommand = MatchKeyword(vSubCommands, sSubCommand, SC_NOMATCH);
    switch(nSubCommand) {
    case SC_CREATE:
      return AddMap(rInterpreter, rResult, nArgs, pArgs);
      break;
    case SC_LIST:
      return ListMaps(rInterpreter, rResult, nArgs, pArgs);
      break;
    case SC_DELETE:
      return DeleteMap(rInterpreter, rResult, nArgs, pArgs);
      break;
    case SC_NOMATCH:
    default:
      return Usage(rResult);
    }
  }
  catch( const string& rError) {
    
    rResult = rError;
    rResult += "\n";
    return Usage(rResult);
  }
}
//  Function: 	AddMap
//  Purpose: 	
// Adds a vme space map syntax:
//    vme create mapname [-device devname] base size
//                mapname       - name of a new TCL command (CVMEMapCommand) 
//                                 which manages the map.
//                -device devname - optional vme device specification.
//                                  default is to the A24/D32 device.
//                -crate  cno   - Optional vme crate number.
//                base          - Offset into the VME space
//                size          - # bytes requested.  This is increased to a 
//                                multiple of the system
//                                pagesize.
//
//  Parameters: CTCLInterpreter& rInterp, CTCLResult& rResult, 
//              int nArgs, char** pArgs
//  Comments: 	
int CVmeCommand::AddMap(CTCLInterpreter& rInterp, 
			CTCLResult& rResult, int nArgs, char** pArgs)  
{
  // Add the map:  We must pick out the various parameters:
  // There must be either 3 parameters or 5:
  //
  if((nArgs < 3) ) {
    return Usage(rResult);
  }
  string mapname(*pArgs);	// Pull out the map name.
  nArgs--; pArgs++;

  // The map must not be a duplicate:
  //

  map<string,CVMEMapCommand*>::iterator p = m_Spaces.find(mapname);
  if(p != m_Spaces.end()) {
    rResult = mapname;
    rResult += " already exists you must first delete it";
    return TCL_ERROR;
  }

  string devname("standard"); // Default map is A24 D32
  long    nCrate(0);		// Default crate is 0
  unsigned long    nBase;	// VME base address of map.
  unsigned long    nSize;	// Size of vme map in bytes.


  // Process all switches:

  while(nArgs) {
    if(nArgs < 2) {		// Everything comes in twos:
      return Usage(rResult);
    }
    Switches sw = MatchSwitch(*pArgs);
    switch(sw) {
    case device:		// -device; next param is name.
      nArgs--; pArgs++;
      devname = *pArgs;
      break;
    case crate:			// -crate; next param is exp for crate#
      nArgs--; pArgs++;
      try {
	nCrate = rInterp.ExprLong(*pArgs);
	
      }
      catch(...) {
	rResult += " Failed to convert crate as integer expression\n";
	return Usage(rResult);
      }
      break;
    case unknown:		// This had better be the base:
      try {
	nBase = TextToULong(*pArgs);
      }
      catch (...) {
	rResult += "Failed to convert base address to integer\n";
	return Usage(rResult);
      }
      break;
    default:		// This indicates a coding error.
      assert(0);
    }
    if(sw == unknown) break;
    nArgs--;
    pArgs++;
  }
  nArgs--; pArgs++;
  if(nArgs != 1) {		// Need a size as well as a base
    return Usage(rResult);
  }

  // Decode the area size:

  try {
    nSize = (unsigned long)TextToULong(*pArgs);
  }
  catch(...) {
    rResult += "Failed to convert map size to integer";
    return TCL_ERROR;
  }
  if(nSize < 0) {
    rResult += "Map size must be positive";
    return TCL_ERROR;
  }
  // Now create the map:

  CVmeModule::Space space = UserToLogical(devname);

  try {
    CVMEMapCommand* pMap = new CVMEMapCommand(mapname,
					      &rInterp,
					      nBase, nSize, nCrate,
					      space);
    m_Spaces[mapname] = pMap;
    rResult = mapname;
    
  }
  catch (CMmapError& error) {
    rResult = error.ReasonText();
    return TCL_ERROR;
  }
  catch (string& msg) {
    rResult = msg;
    return TCL_ERROR;
  }
  catch (char* pmsg) {
    rResult = pmsg;
    return TCL_ERROR;
  }
  catch (...) {
    rResult = "Unexpected exception caught while mapping";
    return TCL_ERROR;
  }

  return TCL_OK;
}
//  Function: 	ListMaps
//  Purpose: 	
// Lists the set of known maps.
// Syntax:
//     vme list
//  Parameters: CTCLInterpreter& rInterp, 
//              CTCLResult& rResult, int nArgs, char** pArgs
//  Comments: 	
int CVmeCommand::ListMaps(CTCLInterpreter& rInterp, 
			  CTCLResult& rResult, int nArgs, char** pArgs)  
{
  if(nArgs != 0) return Usage(rResult);

  // List the maps from the iterator.  Each map creates a list entry:
  //   {mapname base}
  //
  CTCLString ResultString;
  map<string,CVMEMapCommand*>::iterator p;
  for(p = m_Spaces.begin(); p != m_Spaces.end(); p++) {
    char start[100];
    CVMEMapCommand* pMap = p->second;

    ResultString.StartSublist();
    ResultString.AppendElement(p->first);
    sprintf(start, "0x%lx", pMap->getBase());
    ResultString.AppendElement(start);
    ResultString.EndSublist();
  }
  rResult = (const char*)ResultString;
  return TCL_OK;
}
//  Function: 	DeleteMap
//  Purpose: 	
// Deletes a map.
//   Syntax:
//      vme delete mapname
//
//  Parameters: CTCLInterpreter& rInterp, 
//              CTCLResult& rResult, int nArgs, char** pArgs
//  Comments: 	
int CVmeCommand::DeleteMap(CTCLInterpreter& rInterp, 
			   CTCLResult& rResult, int nArgs, char** pArgs)  
{
  if(nArgs != 1) return Usage(rResult);
  string MapName(*pArgs);

  map<string,CVMEMapCommand*>::iterator p = m_Spaces.find(MapName);
  if(p == m_Spaces.end()) {
    rResult += "No such  map: ";
    rResult += MapName;
    return TCL_ERROR;
  }
  //
  //   We destroy the CVMEMapCommand pointed to by the iterator and erase the 
  //   element from the map:
  //

  CVMEMapCommand* pMap = p->second;
  m_Spaces.erase(p);
  delete pMap;

  return TCL_OK;
}
// Function: Usage
// Purpose:
//   Local member function to append command usage information to the Result.
//
// Parameters:
//    CTCLResult& rResult
//
int
CVmeCommand::Usage(CTCLResult& rResult) 
{
  rResult += "Usage:\n";
  rResult += "  vme create spacename [-device name] [-crate number]  base size\n";
  rResult += "  vme list\n";
  rResult += "  vme delete mapname\n"; 
  return TCL_ERROR;
}





//
// Function:
//   ~CVmeCommand
// Operation Type:
//   Destructor:
//
CVmeCommand::~CVmeCommand()
{
  // Delete all of the maps which we maintain:

  map<string,CVMEMapCommand*>::iterator p = m_Spaces.begin();
  for(; p != m_Spaces.end(); p++) {
    delete p->second;
  }
}

/*!
   Convert a user VME address space selector to a logical device
   name.  In order to keep device and API independence, adress
   space to device translation is now a two step operation.
   - Convert the selector (preferrably e.g. standard) to an
     API device name.
   - Convert the API device name to a device suitable to be
     passed to the API's open function.

 Parameters:
 \param rsUser - const string& [in] The address space selector.
              preferrabley one of the following choices (although
	      legacy Bit3/NSCL device names are supported):
	      - shortio - A16D32 address space.  If suffixed with
	                  16 then A16D16 is selected.
	      - standard - A24D32 address space. If suffixed with
	                  16 then A24D16 is selected.
	      - extended - A32D32 address space.  If suffixed with 
	                  16 then A32D16 is selected.
			  
	      Note that some drivers/hardware may not support
              mapping 32 bit transfers to two 16 bit transfers.
	      In that case, the translation will usually give
	      the 32 bit device.  It is therefore best to
	      use 16 bit transfers to 16 bit devices.

 \return string - Name of the logical device.  If the requested
             device is recognized but not supported by the API
	     driver  or hardware, the string "unsupported"
	     is returned.  If the API name is not selected,
	     see below under throws:
	     
  \throw string exception error message if the rsUser selector
      is not matched in the lookup table.

\note
The number of entries in the table is small as is the 
lookup frequency so we just do a linear search.

*/
CVmeModule::Space
CVmeCommand::UserToLogical(const string& rsUser)
{
  for(int i=0; i < knDeviceMapSize; i++) {
    if(rsUser == kaDeviceMap[i].me_sLogicalDev) {
      return kaDeviceMap[i].me_eAPIDev;
    }
  }
  throw string("VME Address space selector is invalid");
}


/*!
  Matches a string against the set of switches recognized by
  the program.:
\param pSwitch (const char* [in])  The string to match.


\return Switch value.
*/
CVmeCommand::Switches
CVmeCommand::MatchSwitch(const char* pSwitch)
{
  string Switch(pSwitch);

  if(Switch == string("-device")) return CVmeCommand::device;
  if(Switch == string("-crate")) return CVmeCommand::crate;
  return unknown;
}
/*!
   Convert a text string to an unsigned long.  If there's a problem,
   a string exception is thrown.

   @param text
      The text to convert.
   \return unsigned long
   \retval    The value of the conversion.
   \throw  string - the stringified version of the errno.
*/
unsigned long
CVmeCommand::TextToULong(const char* text)
{
  errno = 0;
  unsigned long result = strtoul(text, NULL, 0);
  if(errno) {
    string message = strerror(errno);
    throw message;
  }
  return result;
}
