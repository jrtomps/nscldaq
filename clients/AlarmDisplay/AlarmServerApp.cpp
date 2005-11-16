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


static const char* Copyright = "(C) Copyright Michigan State University 1944, All rights reserved";
#include <config.h>
#include <Iostream.h>
#include <CAlarmServer.h>
// #include <TCLApplication.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>
#include <SpectroFramework.h>

class AlarmServerApp : public DAQROCNode
{
  int operator() (int argc, char** argv) {
    CAlarmServer server;
    try { 
      while(server()) { 
	cout << "Restarting server (SIGHUP to kill)." << endl;
      }
    }
    catch(CException& ace) {
      cerr << "Caught exception while attempting to listen for requests\n";
      cerr << "Reason was: " << ace.ReasonText() << endl;
      cerr << ace.WasDoing() << endl;

    }
    cout << "Server exited normally" << endl;
  }
};

AlarmServerApp myServerApp;


CTCLApplication* gpTCLApplication = NULL;

// If spectrodaq main is separable, then I need to define main
// here to ensure that TCL++'s main is not pulled in by mistake.
//

#ifdef HAVE_SPECTRODAQ_MAIN
int
main(int argc, char** argv, char** envp) 
{
  return spectrodaq_main(argc, argv, envp);
}
#endif
