//
// Test program for the CTCLInterpreterStartup.
//   We start up a TCL interpreter in its own thread
//   and register the Echo command extension as a synchronized
//   extension.
//   The program should also understand the sync command.
//

#include <spectrodaq.h>
#include <iostream.h>
#include <CTKInterpreterStartup.h>


#include <CDAQTCLProcessor.h>



class TCLExtension : public CDAQTCLProcessor
{

public:
  TCLExtension(CTCLInterpreter* pInterp);
  virtual int  operator() (CTCLInterpreter &rInterpreter, CTCLResult &rResult, 
			   int nArguments, char *pArguments[]);

};


/*! 
  Constructors  The constructor sets the command name to "Echo"
  and passes the interpreter on to the base class constructor:
  */
TCLExtension::TCLExtension(CTCLInterpreter* pInterp) :
  CDAQTCLProcessor("Echo", pInterp)
{
}

/*!
  Executes the Echo Command. All arguments, from pArguments[1] on are
  put into the result string as list elements.  This ensures that
  they will be appropriately formatted for further use.
  */
int
TCLExtension::operator() (CTCLInterpreter &rInterpreter, CTCLResult &rResult, 
			  int nArguments, char *pArguments[])
{
  for(int i = 1; i < nArguments; i++) {
    rResult.AppendElement(pArguments[i]);
  }
  return TCL_OK;
}




class MyInterpThread : public CTKInterpreterStartup
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
    CTKInterpreterStartup::RegisterExtensions();
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
