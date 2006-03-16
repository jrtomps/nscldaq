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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Class: CVMEMapCommand

//////////////////////////.cpp file/////////////////////////////////////////////////////

#include <config.h>
#include "TCLResult.h"
#include "VMEMapCommand.h"    				
#include "VMECommand.h"
#include <sys/ioctl.h>
#include <CVMEInterface.h>


#include <string>
#include <sys/mman.h>
#include <Iostream.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Subcommand strings:

static const string aSubCommands[] = {
  string("set"),
  string("get")
};
static vector <string> vSubCommands(aSubCommands, &aSubCommands[2]);
#define SC_SET  0
#define SC_GET  1
#define SC_NOMATCH -1

// size switches:
// 
static const string aSizeSwitches[] = {
  string("-l"),
  string("-w"),
  string("-b")
};
static vector<string> vSizeSwitches(aSizeSwitches, &aSizeSwitches[3]);
#define SZ_LONG  0
#define SZ_WORD  1
#define SZ_BYTE  2
#define SZ_NOMATCH -1


/** Construct the 'map'  In this implementation we really are not mapping
 *  at all but abstracting the access behind a CVmeModule object.
 *  That class understands the dirty details of how to access the VME bus
 * regardless of the characteristics of the underlying VME interface module.
 *  @param   Name:
 *       Name of the address space we are creating.
 *  @param   pInterp
 *      The TCL interpreter on which we register.
 *  @param   Base
 *      Base address of the VME map.
 *  @param   nSize
 *      Size of the VME map.
 *  @param   nCrate
 *       Crate in which the map is done.
 *  @param   space
 *       Addresss space selector.  See VmeModule.h for information
 *       about this parameter.
 *      
 */
CVMEMapCommand::CVMEMapCommand( string           Name, 
				CTCLInterpreter* pInterp,
				ULong_t          Base, 
				UInt_t           nSize,
				UInt_t           nCrate,
				CVmeModule::Space space) :
  CTCLProcessor(Name, pInterp),
  m_nPhysBase(Base),
  m_nSize(nSize),
  m_nCrate(nCrate),
  m_pSpace(new CVmeModule(space, Base, nSize, nCrate)),
  m_Name(Name)
{
  Register();
}


//
// Destructor must unmap the window... presumably base class unregisters us
//
CVMEMapCommand::~CVMEMapCommand()
{
  OnDelete();

}

// Functions for class CVMEMapCommand
//  Function: 	operator()
//  Purpose: 	
// Called to dispatch to appropriate Get/Set members
// based on the first command line parameter.
//  Parameters: CTCLInterpreter& rInterp, CTCLResult& rResult, 
//              int nArgs, char** pArgs
//  Comments: 	
int CVMEMapCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult,
			       int nArgs, char** pArgs)  
{
  // Valid syntaxes are:
  //    get [-l|w|b] offset
  //    set [-l|w|b] offset value
  //
  // We'll attempt to process the size switches as well as the
  // subcommand.

  nArgs--;
  pArgs++;
  // Need at least one more argument:
  //
  if(nArgs <= 0) {
    return Usage(rResult);
  }

  int size = SZ_LONG;		// Assume the size is a long.

  // Decode the keyword:

  string subcommand(*pArgs);
  pArgs++;
  nArgs--;
  int SubCommand = MatchKeyword(vSubCommands, subcommand, SC_NOMATCH);
  if(nArgs) {
    int PossibleSize = MatchKeyword(vSizeSwitches, string(*pArgs), SZ_NOMATCH);
    if(PossibleSize != SZ_NOMATCH) {
      size = PossibleSize;
      pArgs++;
      nArgs--;
    }
  }
  // Now we have a keyword and a size:  dispatch to get or set or error:

  try {
    switch(SubCommand) {
    case SC_GET:
      return Get(rInterp, rResult, size, nArgs, pArgs);
      break;
    case SC_SET:
      return Set(rInterp, rResult, size, nArgs, pArgs);
      break;
    case SC_NOMATCH:
    default:
      return Usage(rResult);
    }
  }
  catch (string msg) {
    rResult += msg;
    return TCL_ERROR;
  }
  catch (...) {
    rResult += "Unexpected exception type caugh in CVMEMapCommand::operator()";
    return TCL_ERROR;
  }
  
}
//  Function: 	Get
//  Purpose: 	
// Get (read) from VME bus:
//
//  syntax:   
//    spacename get [-l|w|b] offset
//
//  Parameters: CTCLInterpreter& rInterp, CTCLResult& rResult,
//              int nArgs, char** pArgs
//  Comments: 	
int CVMEMapCommand::Get(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			int nSize,
			int nArgs, char** pArgs)  
{
  // The next parameter should be the offset and must exist, the
  // caller has taken care of the possible size qualifier.
  // We must:
  //    Turn the offset into a virtual address,
  //    Range check the address to be sure that it fits in the window.
  //    Do an appropriately sized read from the va. and convert the
  //    data gotten to a result string.
  //
  if(nArgs != 1) return Usage(rResult);
  Long_t nOffset;
  try {
    nOffset = CVmeCommand::TextToULong(*pArgs);
  }
  catch (...) {
    rResult += "  Expecting an integer for the offset";
    return TCL_ERROR;
  }

  // Make sure the offset is in the address window represented
  // by this command.

  if(nOffset >= m_nSize) {
    rResult += *pArgs;
    rResult += " is outside the mapped address window";
    return TCL_ERROR;
  }
  // Everything's fine now:  do the read:

  CVMEInterface::Lock();

  ULong_t nValue;
  switch(nSize) {
  case SZ_LONG:
    nValue = m_pSpace->peekl(nOffset/sizeof(long));
    break;
  case SZ_WORD:
    nValue = m_pSpace->peekw(nOffset/sizeof(short));
    break;
  case SZ_BYTE:
    nValue = m_pSpace->peekb(nOffset);
    break;
  default:
    CVMEInterface::Unlock();
    return Usage(rResult);
  }
  CVMEInterface::Unlock();
  // nValue must be turned into a result string:
  //

  char sValue[100];
  sprintf(sValue, "%d", nValue);
  rResult += sValue;
  return TCL_OK;
}
//  Function: 	Set
//  Purpose: 	
// Called to process a Set (write) to the
//  vme bus.
//     spacename set [-l|w|b] offset value
//
//  Parameters: CTCLInterpreter& rInterp, CTCLResult& rResult, 
//              int  nArgs, char** pArgs
//  Comments: 	
int CVMEMapCommand::Set(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			int nSize,
			int  nArgs, char** pArgs)  
{
  // The caller has taken are of the size flag.  We should have 2 
  // more parameters on the command line; the offset and the value to
  // poke.
  //
  if(nArgs != 2) return Usage(rResult);

  Long_t nOffset;
  Long_t nValue;
  Char_t* pOffset = *pArgs;

  try {
    nOffset = CVmeCommand::TextToULong(*pArgs);
    nArgs--; 
    pArgs++;
  }
  catch(...) {
    rResult += " Expecting an integer for offset";
    return TCL_ERROR;
  }

  try {
    nValue = CVmeCommand::TextToULong(*pArgs);
    nArgs--;
    pArgs++;
  }
  catch(...) {
    rResult += " Expecting an integer for the value";
    return TCL_ERROR;
  }

  // Make sure the offset represents a value that's inside
  // the address window represented by this command:

  if(nOffset >= m_nSize) {
    rResult += pOffset;
    rResult +=" is outside the address window";
    return TCL_ERROR;
  }

  CVMEInterface::Lock();
  switch(nSize) {
  case SZ_LONG:
    m_pSpace->pokel(nValue, nOffset/sizeof(long));
    break;
  case SZ_WORD:
    m_pSpace->pokew(nValue, nOffset/sizeof(UShort_t));
    break;
  case SZ_BYTE:
    m_pSpace->pokeb(nValue, nOffset);
    break;
  default:
    CVMEInterface::Unlock();
    return Usage(rResult);
  }
  CVMEInterface::Unlock();
  return TCL_OK;
  

}
// Function: Usage:
// Purpose:
//   Put out command usage information to result.
//
int
CVMEMapCommand::Usage(CTCLResult& rResult)
{
  rResult += "Usage:\n";
  rResult += "  mapname set [-l|w|b] offset value\n";
  rResult += "  mapname get [-l|w|b] offset\n";
  rResult += "Offset and value must both be unsigned.\n";
  return TCL_ERROR;
}
//
// Function:
//   OnDelete
// Purpose:
//   Delete resources associated with themap.
//
void
CVMEMapCommand::OnDelete()
{
  delete m_pSpace;
  m_pSpace = 0;
}
