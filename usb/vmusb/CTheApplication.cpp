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

static const char* versionString = "V5.0";

#include <config.h>
#include "CTheApplication.h"
#include "Globals.h"

#include <COutputThread.h>
#include <TclServer.h>
#include <CVMUSB.h>

#include <TCLInterpreter.h>
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <Exception.h>
#include <tcl.h>
#include <DataBuffer.h>

#include <vector>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include "cmdline.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:

static const string   daqConfigBasename("daqconfig.tcl");
static const string   ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32);                      // Number of buffers that can be inflight.
static const uint32_t bufferSize(13*1024*sizeof(uint16_t)); // 13kword buffers...+pad
static       int      tclServerPort(27000);		    // Default value.


// Static member variables and initialization.

bool CTheApplication::m_Exists(false);

/*!
   Construct ourselves.. Note that if m_Exists is true,
   we BUGCHECK.
*/
CTheApplication::CTheApplication()
{
  if (m_Exists) {
    cerr << "Attempted to create more than one instance of the application\n";
    exit(EX_SOFTWARE);
  }
  m_Exists = true;
  m_pInterpreter = static_cast<CTCLInterpreter*>(NULL);
  Globals::running = false;
  
}
/*!
   Destruction is a no-op since it happens at program exit.
*/
CTheApplication::~CTheApplication()
{
}

/*!
   Thread entry point.  We don't care that much about our command line parameters.
   Note that the configuration files are as follows:
   $HOME/config/daqconfig.tcl     - Data acquisition configuration.
   $HOME/config/controlconfig.tcl - Controllable electronics configuration.
   We will not be returned to after calling startInterpreter().
   startInterpreter will eventually return control to the Tcl event loop which
   will exit directly rather than returning up the call chain.

   \param argc : int
      Number of command line parameters (ignored).
   \param argv : char**
      The command line parameters (ignored).

*/
int CTheApplication::operator()(int argc, char** argv)
{
  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 

  // Process them via gengetopt:

  struct gengetop_args_info parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);	// Actually fails if the commandline is incorrect.

  cerr << "VM-USB scriptable readout version " << versionString << endl;

  // If --enumerate was given, just enumerate the VM-USB modules connected to the
  // system on stdout and exit.

  if (parsedArgs.enumerate_given) {
    enumerateVMUSB();
    exit(EXIT_SUCCESS);
  }

  try {				// Last chance exception catching...
    
    createUsbController(parsedArgs.serialno_given ? parsedArgs.serialno_arg : 
			reinterpret_cast<const char*>(NULL));
    
    // Set default configuration file names and then override with the ones supplied on
    // the command line (if any).
    
    setConfigFiles();
    if (parsedArgs.daqconfig_given) {
      Globals::configurationFilename = std::string(parsedArgs.daqconfig_arg);
    }
    if (parsedArgs.ctlconfig_given) {
      Globals::controlConfigFilename = std::string(parsedArgs.ctclconfig_arg);
    }
    
    initializeBufferPool();
    startOutputThread(destinationRing(parsedArgs.ring_given ? parsedArgs.ring_arg :
				      reinterpret_cast<const char*>(NULL)));

    // Replace the default server port if the user supplied one and start the Tcl server.

    if (parsedArgs.port_given) {
      tclServerPort = parsedArgs.port_arg;
    }

    startTclServer();
    startInterpreter();
  }
  catch (string msg) {
    cerr << "CTheApplication caught a string exception: " << msg << endl;
    throw;
  }
  catch (const char* msg) {
    cerr << "CTheApplication caught a char* excpetion " << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "CTheApplication caught an NCLDAQ exception: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "CTheApplication thread caught an excpetion of unknown type\n";
    throw;
  }
    return EX_SOFTWARE; // keep compiler happy, startInterpreter should not return.
}


/*
   Start the output thread.  This thread is responsible for 
   reformatting and transferring buffers of data from the VM-USB to 
   spectrodaq.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.

   @param ring - std::ring that contains the name of the ring buffer into which data should be put.

*/
void
CTheApplication::startOutputThread()
{
  COutputThread* router = new COutputThread(ring);
  router.start();
  usleep(500);

}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.
*/
void
CTheApplication::startTclServer()
{
  TclServer* pServer = new TclServer;
  pServer->start(tclServerPort, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);
  Globals::pTclServer = pServer; // Save for readout.
  usleep(500);
}
/*
    Start the Tcl interpreter, we use the static AppInit as a trampoline into the
    interpreter configuration and back to the interpreter event loop so the
    default Tcl event loop can be used.
*/
void
CTheApplication::startInterpreter()
{
  Tcl_Main(m_Argc, m_Argv, CTheApplication::AppInit);
}

/*!
   Create the USB controller.  The usb controller will be the first
   one available (should be the only one).  It is a failable error for
   there not to be any controllers.

   @param pSerialNo - Serial numberr of the controller we want to
                      to use.  If this is NULL we just attach to the first
		      controller enumerated.
*/
void
CTheApplication::createUsbController(const char* pSerialNo)
{
  vector<struct usb_device*> controllers = CVMUSB::enumerate();
  usb_device*                pSelectedDevice(0);
  if (controllers.size() == 0) {
    cerr << "There appear to be no VMUSB controllers so I can't run\n";
    exit(EX_CONFIG);
  }
  if (pSerialNo) {
    for (int i = 0; i < controllers.size(); i++) {
      if (pSerialNo == CVMUSB::serialNo(controllers[i])) {
	pSelectedDevice = controllers[i];
	break;
      }
    }
  } else {
    pSelectedDevice = controllers[0];
  }


  // Exit if we don't have a matching device:
  // Note that this can only happen if pSerialNo is not null:

  if (!pSelectedDevice) {
    cerr << "USB enumeration does not show " pSerialNo << " as present/available\n";
    exit(EX_CONFIG);
  }

  Globals::pUSBController = new CVMUSB(pSelectedDevice);

  cerr << "Found a controller, firmware: " << hex << Globals::pUSBController->readFirmwareID()
       << dec << endl;

}
/**
 * Enumerate the set of VM-USB serial numgers that are currently active in the system.
 * The enumeration goes to cout.;
 *
 */
void
CTheApplication::enumerateVMUSB()
{
  vector<struct usb_device*> controllers = CVMUSB::enumerate();
  for (int i = 0; i < controllers.size(); i++) {
    std::string serial = CVMUSB::serialNo(controllers[i]);
    std::cout << "[" << i << "] : " << serial << std::endl;
  }
  cout.flush();
}
/* 
  Set the configuration files to the global storage
*/
void
CTheApplication::setConfigFiles()
{
  Globals::configurationFilename = makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = makeConfigFile(ctlConfigBasename);

}



/*
   Initialize the interpreter.  This invoves:
   - Wrapping the interpreter into a CTCLInterpreter Object.
   - Creating the commands that extend the interpreter.
   - Returning TCL_OK so that the interpreter will start running the main loop.

*/
int
CTheApplication::AppInit(Tcl_Interp* interp)
{
  Tcl_Init(interp);		// Get all the paths etc. setup.
  CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
  new CBeginRun(*pInterp);
  new CEndRun(*pInterp);
  new CPauseRun(*pInterp);
  new CResumeRun(*pInterp);


  return TCL_OK;
}
/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(bufferSize);
    gFreeBuffers.queue(p);
  }
}

/*
   Make a configuration filename:  This is done by taking a basename
   and prepending the home directory and config subdir to its path:

*/
string
CTheApplication::makeConfigFile(string baseName)
{
  string home(getenv("HOME"));
  string pathsep("/");
  string config("config");
  string dir;
  
  // The user can define a CONFIGDIR env variable to move the configuration dir.

  if (getenv("CONFIGDIR")) {
    dir =getenv("CONFIGDIR");
  } else {
    dir = home + pathsep + config;
  }


  string result = dir +  pathsep + baseName;
  return result;

}

/**
 * Determine the output ring.  If one is specified that one is used.
 * if not, a ring named after the current logged in user is used instead.
 * getpwuid_r is used because it is thread safe.
 * 
 * @param pRingName - If not null, this is the name of the ring and overrides the default ring.
 *                    If null a default ring name is constructed and returned.
 * 
 * @return std::string
 * @retval Name of ring to which we should connnect.
 *
 * @throws std::string - If the ring name is defaulted but  we can't figure out what it should be
 *                       due to system call failures.
 */
std::string
CTheApplication::destinationRing(const char* pRingName)
{
  if (pRingName) {
    return std::string(pRingName);
  } else {
    uid_t uid = getuid();	// Currently running user.
    struct passwd  Entry;
    struct passwd* pEntry;
    char   dataStorage[1024];	// Storage used by getpwuid_r(3).

    if (getpwuid_r(uid, &Entry, dataStorage, sizeof(dataStorage), &pEntry)) {
      int errorCode = errno;
      std::string errorMessage = 
	"Unable to determine the current username in CTheApplication::destinationRing: ");
    errorMessage += strerror(errorCode);
    throw errorMessage;

    }
    return string(Entry.pw_name);
  }
}

/*-------------------------------------------------------------------------------------------*/


/*
  Initialize the application.  All we need to do is instantiate a CTheApplication object
  and pass it the argc/argv unmolested.

  @param argc - The count of command line words.
  @param argv - pointer to an array of command word pointers.

  

*/

int
main(int argc, char** argv)
{
  CTheApplication app;
  
  return app(argc, argv);
}

void* gpTCLApplication(0);	// Needed by the TclPlus library
