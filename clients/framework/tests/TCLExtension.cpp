//
// This file contains a TCL Extension derived
// from CDAQTCLProcessor.
//   It implements the "Echo" command.
// This command just echoes its parameters
// into the result string.
//   It is used by both TCLTest and TKTest
// as the extension registered into the 
// command interpreters they start up.
//
// Associated header:
//    TCLExtension.h

#include "TCLExtension.h"

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
