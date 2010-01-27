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

#include <config.h>
#include <CScriptedSegment.h>

#include <TCLInterpreter.h>
#include <RangeError.h>
#include <ErrnoException.h>
#include <TCLException.h>
#include "CDigitizerDictionary.h"
#include "CModuleCommand.h"
#include "CReadOrder.h"
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


// Standard creators:


#include "CCAENV775Creator.h"
#include "CCAENV785Creator.h"
#include "CCAENV792Creator.h"
#include "CCAENV830Creator.h"
#include "CPacketCreator.h"
#include "CSIS3300Creator.h"
#include "CV1x90Creator.h"

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif


using namespace std;
///////////////////////////////////////////////////////////////
//
// Implementation of the public interface
//
//////////////////////////////////////////////////////////////
/*!
    Constructor initializes all the member data so that it is clear
    we don't have any read orders. 

*/
CScriptedSegment::CScriptedSegment() :
  m_pInterp(0),
  m_pDictionary(0),
  m_pReadOrder(0),
  m_pModuleCommand(0)
{}

/*!
   Initialization in our case involves
   - Deleting the old infrastructure and creating new.
   - Adding the supported modules to the new infrastructure.
   - Process the configuration file.
   - Getting and processing the configuration file.

*/
void
CScriptedSegment::initialize()
{
  // Remove the old infrastructure

  delete m_pModuleCommand;
  delete m_pReadOrder;
  delete m_pDictionary;
  delete m_pInterp;

  // Create the new infrastructure:


  m_pInterp        = new CTCLInterpreter();
  m_pDictionary    = new CDigitizerDictionary();
  m_pReadOrder     = new CReadOrder(m_pInterp, m_pDictionary);
  m_pModuleCommand = new CModuleCommand(m_pInterp, m_pDictionary);


  // Add standard and user written module creators.

  addStandardCreators();
  addUserWrittenCreators();

  // figure out which file is the configuration file and process it.

  try {
    string configFile = getConfigurationFile();
    processHardwareFile(configFile.c_str(), *m_pInterp);
    
    // Initialize the read order that was created:
    
    m_pReadOrder->Initialize();	// One-time intialization.
    clear();			// Clear everything for good measure
    m_pReadOrder->Prepare();	// Get ready for the first event.
  }
  catch(CErrnoException& e) {
    string error = "Configuration file processing failed\n";
    error        += "Unable to open configuration file\n";
    error        += e.ReasonText();
    throw error;
  }
  catch(string msg) {
    string error = "Configuration file processing failed:\n";
    error       += msg;
    error       += "\nAt: \n";
    error       += CTCLException::getTraceback(*m_pInterp);
    throw error;
  }
  catch(const char* msg) {
    string error = "Configuration file processing failed:\n";
    error       += msg;
    error       += "\nAt: \n";
    error       += CTCLException::getTraceback(*m_pInterp);
    throw error;
  }
  catch(CTCLException& tclfail) {
    string error = "Configuration file processing failed\n";
    error       +=tclfail.ReasonText();
    error       += "\nTraceback:\n";
    error       += tclfail.getTraceback();
    throw error;
  }
  catch (...) {
    throw "Configuration file processing failed";
  }
  
}
/*!
   After reading an event, the digitizers must be cleared and
   made ready for the next event:
*/
void
CScriptedSegment::clear()
{
  m_pReadOrder->Clear();	// Clear and...
  m_pReadOrder->Prepare();	// Prepare for the next event.
}
/*!
   There's actually no way to disable in the CReadOrder..so this is a no-op
*/
void
CScriptedSegment::disable()
{
}
/*!
   To read we are just going to delegate to the read order:
   @param pBuffer  - Pointer tothe buffer into which the data will be put.
   @param maxwords - Maximum number of words we're allowed to put in the buffer.

   @return size_t
   @retval number of words actually read.

   @throws CRangeError (if we live long enough) if the number of words read 
                        exceeds maxwords.. though in that case it's very possible
                        we'll SEGFAULT first.
*/
size_t
CScriptedSegment::read(void* pBuffer, size_t maxwords)
{
  int nWords = m_pReadOrder->Read(pBuffer);
  if (nWords > maxwords) {
    throw CRangeError(0, maxwords, nWords,
		      "Overflowed the event buffer in an event read");
  }
  return nWords;
}
///////////////////////////////////////////////////////////////////////
//
// Implementation of the protected interface.
// Note that some entries are documented for doxygen for those who
// may extend this class.
//


/*!
   Returns the name of the configuration file as an STL std::string.
   in the default implementation, this is the first of the following
   to have a readable file:
   HARDWARE_FILE            environment variable
   ~/config/hardware.tcl    where ~ comes from the HOME env name.
   ./hardware.tcl           where . is the cwd.

   @return std::string
   @throws CErrnoException for ENOENTRY if there is no a matching file.


*/
string
CScriptedSegment::getConfigurationFile()
{

  // HARDWARE_FILE:

  const char* path = getenv("HARDWARE_FILE");
  if (path && (access(path, R_OK) == 0) ) {

    return string(path);
  }

  // ~/config/hardware.tcl

  const char* home = getenv("HOME");
  if (home) {
    string path(home);
    path += string("/config/hardware.tcl");
    if (access(path.c_str(), R_OK) == 0) {
      return path;
    }

  }

  //  cwd/hardware.tcl

  char here[PATH_MAX];
  char* pResult = getcwd(here, PATH_MAX);
  if (pResult) {
    string path(here);
    path += "/hardware.tcl";
    if (access(path.c_str(), R_OK) == 0) {
      return path;
    }
  }
  errno = ENOENT;
  throw CErrnoException("Locating a configuration file");

  

}

/*!
   Add a new module creator this is the mechanism a user written addUserWrittenCreators
   would use to add creators it instantiated into the set supported by the module
   command (For that matter, addStandardCrators uses it too).
   @param creator - reference to the module creator to add.

*/
void
CScriptedSegment::addCreator(CModuleCreator& creator)
{
  m_pModuleCommand->AddCreator(&creator);
}

/*!
   Stub for addUserWrittenCreators... in order to extend the set of
   creators supported by the module command, simply derive a class from this
   class, override this addUserWrittenCreators to add support for the new modules,
   and in Skeleton.cpp instantiate your new class and register it as the
   event segment to read out.
*/
void
CScriptedSegment::addUserWrittenCreators()
{
}

/*!

  Add the standard creators to the module command (via addCreator):
  - CCAENV775Creator  - Creates CAEN V775 TDC modules.
  - CCAENV785Creator  - Creates CAEN V785 ADC modules.
  - CCAENV792Creator  - Creates CAEN V792 QDC modules (works for the 892)
  - CCAENV830Creator  - Creates CAEN V830 scaler modules.
  - CSIS3300Creator   - Creates SIS 3300 scaler mdoules.
  - CV1x90Creator     - Creates a CAEN V1190/1290 multihit tdc module.
  - CPacketCreator    - Creates a packet ..which holds other modules.

*/
void
CScriptedSegment::addStandardCreators()
{
  // All creators are dynamically created ..presumably destruction of the
  // CModuleCommand will delete them ??!?

  addCreator(*(new CCAENV775Creator()));
  addCreator(*(new CCAENV785Creator()));
  addCreator(*(new CCAENV792Creator()));
}
/*
  Process the hardware file.  This results in the creation of the elements of
  this event segment that actually do the work of managing the specified hardware.
  Parameters:
      pFilename    - Pointer to the full path to the configuration file.
      rInterp      - Refers to the Tcl interpreter that will be used to
                     process the configuration file.
  Throws: 
     Various exceptions can be thrown from the configuration processors.
     These are passed through without catch here.  Most likely exceptions
     include CTCLException, const char*, and std::string in no particular order.
*/
void
CScriptedSegment::processHardwareFile(const char*      pFilename, 
				      CTCLInterpreter& rInterp)
{
  rInterp.EvalFile(pFilename);
}
