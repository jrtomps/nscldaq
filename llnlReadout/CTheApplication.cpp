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

#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//   Configuration constants:

static const int tclServerPort(27000);
static const string daqConfigBasename("daqconfig.tcl");
static const string ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32); // Number of buffers that can be inflight.
static const uint32_t bufferSize(13*1024*sizeof(uint16_t)); // 13kword buffers...+pad


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
  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 


  cerr << "VM-USB scriptable readout version " << versionString << endl;

  try {				// Last chance exception catching...
    
    createUsbController();
    setConfigFiles();
    initializeBufferPool();
    startOutputThread();
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
*/
void
CTheApplication::startOutputThread()
{
  COutputThread* router = new COutputThread;
  daq_dispatcher.Dispatch(*router);

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

/*
   Create the USB controller.  The usb controller will be the first
   one available (should be the only one).  It is a failable error for
   there not to be any controllers.
*/
void
CTheApplication::createUsbController()
{
  vector<struct usb_device*> controllers = CVMUSB::enumerate();
  if (controllers.size() == 0) {
    cerr << "There appear to be no VMUSB controllers so I can't run\n";
    exit(EX_CONFIG);
  }
  Globals::pUSBController = new CVMUSB(controllers[0]);

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
  CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
  new CBeginRun(*pInterp);
  new CEndRun(*pInterp);
  new CPauseRun(*pInterp);
  new CResumeRun(*pInterp);


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

// Create the application instance so that Spectrodaq can get us going.k

static CTheApplication theApp;


void* gpTCLApplication(0);

int main(int argc, char** argv, char** env)
{
  return spectrodaq_main(argc, argv, env);
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
