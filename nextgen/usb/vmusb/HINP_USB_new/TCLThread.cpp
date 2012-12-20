/*!
   Implements the TCLTK class:

*/
#include "TCLThread.h"
#include <assert.h>
#include <tk.h>
extern "C" {
#include <tk.h>
}


// The interpreter is a static member.

Tcl_Interp* TCLTK::m_pInterp(0);

// external function prototypes
extern int Chipcont_SafeInit(Tcl_Interp *interp);

/*! 
   Constructor... just creates the Tcl/Tk interpreter for now..
   the user will need to register commands on the interpreter
   before starting it up.  The assumption is that 
   there will only be one interpreter.
*/



TCLTK::TCLTK()
{
  
}
/*!
   Start the interpreter:
   Schedule *this with the daq_dispatcher:
*/
void TCLTK::Start(int argc, char** argv)
{
  this->operator()(argc, argv);

}
/*!
   Register a command with the interpreter... this
   will die if the interpreter has not yet been instantiated..
*/
int TCLTK::Register(const char* pCommand,
		    Tcl_CmdProc* pProcessor,
		    ClientData  pUserData,
		    Tcl_CmdDeleteProc* pDeleteProcessor)
{
  assert(m_pInterp);		// Die if no interp to register on.

  if (Tcl_CreateCommand(m_pInterp, (char*)pCommand,
			pProcessor, pUserData, pDeleteProcessor) != NULL)
    return TCL_OK;
  else
    return TCL_ERROR;

}
/*!
   Thread entry point... we just call Tk_Main and
   specify TCLTK::AppInit as the initializer... there,
   we'll do the real work of initializing stuff.
*/
int TCLTK::operator()(int argc, char** argv)
{
  Tk_Main(argc, argv, TCLTK::AppInit);
  return 0;			// thread exit status.
}
/*!
   Application initialization: NOTE TO Jon:
   you could add command registration here just like
   any other Tcl program.  We are already running in tcl's
   thread context at this point.
*/
int TCLTK::AppInit(Tcl_Interp* pInterp)
{
  int i;

  if(Tcl_Init(pInterp) == TCL_ERROR) // Start Tcl...
    return TCL_ERROR;

  if(Tk_Init(pInterp) == TCL_ERROR) // Start tk...
    return TCL_ERROR;
  Tcl_StaticPackage(pInterp, "Tk", Tk_Init, Tk_SafeInit);

  m_pInterp = pInterp;		// Enable Register.

  i=Chipcont_SafeInit(pInterp);
  // Now start up tkcon. 

  
  return TCL_OK;
}

