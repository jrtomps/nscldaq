//
// Test program for the CTCLInterpreterStartup.
//   We start up a TCL interpreter in its own thread
//   and register the Echo command extension as a synchronized
//   extension.
//   The program should also understand the sync command.
//

#include <spectrodaq.h>
#include <iostream.h>
#include "TCLExtension.h"
#include <CTCLInterpreterStartup.h>

class MyInterpThread : public CTCLInterpreterStartup
{
  TCLExtension* m_pCommand;
protected:
  //! Add the echo command and whatever CTCLInterpreterStartup adds.
  virtual void  RegisterExtensions () {
    cerr << "Registering extensions\n";
    cerr.flush();
    m_pCommand = new TCLExtension(getInterpreter());
    m_pCommand->Register();
    cerr << "Registering base class extensions\n";
    cerr.flush();
    CTCLInterpreterStartup::RegisterExtensions();
    cerr << "Returning to TCL Main loop\n";
    cerr.flush();
  }
};

class App : public DAQROCNode
{
  virtual int  operator() (int argc, char **argv) {
    MyInterpThread Tcl;
    DAQThreadId tid = daq_dispatcher.Dispatch(Tcl);

    DAQStatus stat;
    Join(tid, &stat);
    cerr << "Tcl interpreter exited" << stat.GetStatusCode() << endl;
    return 0;
  }

};

App theApp;
