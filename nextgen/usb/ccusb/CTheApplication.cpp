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

static const char* versionString = "V2.0";

#include <config.h>
#include "CTheApplication.h"
#include "Globals.h"

#include <COutputThread.h>
#include <CCCUSB.h>

#include <TCLInterpreter.h>
#include <TCLException.h>
#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <Exception.h>
#include <tcl.h>
#include <DataBuffer.h>
#include <TclServer.h>

#include <vector>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "cmdline.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:

static const int    tclServerPort(27000); // Default Tcl server port.
static  string daqConfigBasename("daqconfig.tcl");
static  string ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32); // Number of buffers that can be inflight.


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
  struct gengetopt_args_info arg_struct; // parsed command line args.

  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 


  // Parse the command line parameters.  This exits on failure:

  cmdline_parser(argc, argv, &arg_struct);



  cerr << "CC-USB scriptable readout version " << versionString << endl;

  // If we were just asked to enumerate the interfaces do so and exit:

  if(arg_struct.enumerate_given) {
    enumerate();
    exit(EXIT_SUCCESS);
  }

  try {				// Last chance exception catching...
    
    createUsbController(arg_struct.serialno_given ? arg_struct.serialno_arg : NULL);
    setConfigFiles(arg_struct.daqconfig_given ? arg_struct.daqconfig_arg : NULL,
    		   arg_struct.ctlconfig_given ? arg_struct.ctlconfig_arg : NULL);
    initializeBufferPool();
    startOutputThread(destinationRing(arg_struct.ring_given ? arg_struct.ring_arg : NULL)); 
    startTclServer(arg_struct.port_given ? arg_struct.port_arg : tclServerPort);

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
   reformatting and transferring buffers of data from the CC-USB to 
   a ring.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.

   @param ring - Name of the ring COutputThread will use as its output

*/
void
CTheApplication::startOutputThread(std::string ring)
{
  COutputThread* router = new COutputThread(ring.c_str());
  router->start();

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
   Create the USB controller.  
   
   @param  pSerialNo - If not null, the serial number of the CC-USB to
                       use. Otherwise, the first one in the enumeration is used.

*/
void
CTheApplication::createUsbController(const char* pSerialNo)
{
  vector<struct usb_device*> controllers = CCCUSB::enumerate();
  struct usb_device* pMyController(0);

  if (controllers.size() == 0) {
    cerr << "There appear to be no CC-USB controllers so I can't run\n";
    exit(EX_CONFIG);
  }
  // If necessary locate the correct device:

  if (pSerialNo) {
    for (int i = 0; i < controllers.size(); i++) {
      if (CCCUSB::serialNo(controllers[i]) == pSerialNo) {
	pMyController = controllers[i];
	break;
      }
    }
  } else {
    pMyController = controllers[0];
  }
  // This only fails if the serial number was provided

  if (!pMyController) {
    std::string msg = "Unable to find a CC-USB with the serial number: ";
    msg += pSerialNo;
    throw msg;
  }

  Globals::pUSBController = new CCCUSB(pMyController);

}

/**
 * Enumerate the controller serial numbers to stdout:
 */
void 
CTheApplication::enumerate()
{
  try {
    std::vector<struct usb_device*> ccusbs = CCCUSB::enumerate();
    for (int i = 0; i < ccusbs.size(); i++) {
      std::cout << "[" << i << "] : " << CCCUSB::serialNo(ccusbs[i]) << std::endl;
    }
  }
  catch (string msg) {
    std::cerr << "Unable to enumerate CC-USB modules: " << msg << std::endl;
  }
}
/* 
  Set the configuration files to the global storage

  @param pDaqConfig - if not null a pointer to the to the path to the daq configuration
                      file.  If null a default is used.
  @param pCtlConfig - if not null a pointer to the path to the contrl configuration.
                      if null a default is used.


*/
void
CTheApplication::setConfigFiles(const char* pDaqConfig, const char* pCtlConfig)
{
  Globals::configurationFilename = pDaqConfig ? pDaqConfig : makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = pCtlConfig ? pCtlConfig : makeConfigFile(ctlConfigBasename);

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
  Tcl_Init(interp);
  CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
  new CBeginRun(*pInterp);
  new CEndRun(*pInterp);
  new CPauseRun(*pInterp);
  new CResumeRun(*pInterp);


  // Look for readoutRC.tcl in the config directory.  If it exists, run it.

  string initScript = makeConfigFile(string("readoutRC.tcl"));
  try {
    if (access(initScript.c_str(), R_OK) == 0) {
      pInterp->EvalFile(initScript.c_str());
    }
  }
  catch (CTCLException except) {
    cerr << "Failed to run initialization file.\n";
    cerr << except.ReasonText() << endl;
  }

  return TCL_OK;
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


void* gpTCLApplication(0);	// Need this for tclplus.


/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - Globals::bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(Globals::bufferSize);
    gFreeBuffers.queue(p);
  }
}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.

   @param port - the port on which the tcl server will listen.
*/
void
CTheApplication::startTclServer(int port)
{
  TclServer* pServer = new TclServer;
  pServer->start(port, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);
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
	"Unable to determine the current username in CTheApplication::destinationRing: ";
    errorMessage += strerror(errorCode);
    throw errorMessage;

    }
    return string(Entry.pw_name);
  }
}


/**
 * Create the application object and transfer control to it.
 *
 * @param argc - number of command line words.
 * @param argv - array of pointers to comandline words argv[0] points to the
 *              name of the program as typed on the command line.
 *
 * @return int
 *
 */
int 
main(int argc, char** argv)
{
  CTheApplication app;
  return app(argc, argv);
}
