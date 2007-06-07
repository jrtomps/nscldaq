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

#include <config.h>
#include "AssemblerApplication.h
#include "AssemblerCommand.h"

#include <TCLInterpreter.h>

/*!
   This function is called during initialization.
   We need to create the extensions for our interpreter.
*/
int
AssemblerApplication::operator()()
{
    CTCLInterpreter* pInterp   = getInterpreter();
    AssemblerCommand* pCommand = new Assembler(*pInterp);
    return TCL_OK;
}

AssemblerApplication myApp;
CTCLApplication*     pApplication(&myApp);

