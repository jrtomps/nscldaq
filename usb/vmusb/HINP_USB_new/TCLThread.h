#ifndef __TCLTHREAD_H
#define __TCLTHREAD_H


#include <tk.h>
extern "C"
{
#include <tcl.h>
}
/*!
   Class to add a tcl/tk interpreter in a thread 
   of a Spectrodaq process.   To use:
   #include <TCLThread.h>
...

   TCLTK interpreter;

   interpreter.Start(argc,argv);
   interpreter.Register("newcommand",
                        CommandProcessor,
			UserParameter
			[, DeleteProcessor]);



   where argc/argv are same as you might pass to a
   main().
   the current version assums that tkcon.tcl is in the 
   current directory, sources it in and ensures that
   wish won't read from stdin.  Note that all commands
   execute in Tcl/Tk's thread and it's the command
   processor's responsibility to syncrhonize with
   any other threads in the process.

   this implementation is limited to a single interpreter!!!!

*/


class TCLTK
{
private:
  static Tcl_Interp* m_pInterp;
public:
  TCLTK();			// Constructor.
private:
  TCLTK(const TCLTK&);		// Illegal to copy.
  TCLTK& operator=(const TCLTK&); // Illegal to assign too.
public:
  void Start(int argc, char** argv);
  Tcl_Interp* getInterp() {return m_pInterp;} // In case you need it.
  int Register(const char* pCommand,
		Tcl_CmdProc* Processor,
		ClientData  pUserData,
		Tcl_CmdDeleteProc* pDeleteProcessor = 0);
protected:
  int operator()(int argc, char** argv); // Thread entry.
  static int AppInit(Tcl_Interp* pInterp); // Tcl_AppInit.
  
};

#endif
