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


//
// VMEPackage.cpp:
//   Contains the package initialization for the Vme package.
//   That package can be loaded via the Tcl 
//                   load Vme 
//   command.
//   We create and bind a CVMECommand object on the current interpreter.
//

#include <config.h>
#include <tcl.h>
#include <TCLInterpreter.h>
#include "VMECommand.h"
#include <CopyrightNotice.h>
#include <iostream>

using namespace std;

static const char* pInstdir=HOME;
static const char* pScriptSubdir="/Scripts"; // Where scripts may live.

static char* version="2.0";
extern "C" {
int Sbsvme_Init(Tcl_Interp* pInterp)
{
  // Identify ourselves.

  CopyrightNotice::Notice(cerr, "Vmetcl", version, "2002");
  CopyrightNotice::AuthorCredit(cerr, "Vmetcl", "Ron Fox", NULL);

  // Indicate we provide the package.

  Tcl_PkgProvide(pInterp, "Vme", version);


// Wrap the interpreter in an object.

  CTCLInterpreter &rInterp(*(new CTCLInterpreter(pInterp))); 

  // Register the map making command.  

  CVmeCommand* p= new CVmeCommand(&rInterp);
  p->Register();

  
  // Add our script directory to the package path.
  //
  
  string AddPath("lappend auto_path ");
  AddPath += pInstdir;
  AddPath += pScriptSubdir;
  rInterp.Eval(AddPath);


  return TCL_OK;
}
void* gpTCLApplication(0);
}
