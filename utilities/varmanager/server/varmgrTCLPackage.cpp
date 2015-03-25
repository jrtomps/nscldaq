/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   varmgrTCLPackage.cpp
# @brief  package installation for the varmgr Tcl package.
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CVarMgrCreateCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrCloseCommand.h"
#include "CVarMgrMkdirCommand.h"


static const char* version="1.0";


extern "C" {
    int Varmgrtcl_Init(Tcl_Interp* pInterp)
    {
        // If needed initialize the stubs interfae.
        
#ifdef USE_TCL_STUBS
        Tcl_InitStubs(pInterp, "8.4", 0);
#endif
        // register the package:
        
        int status;
        status = Tcl_PkgProvide(pInterp, "varmgr", version);
        if (status != TCL_OK) {
            return status;
        }
        // Register the namespace.
        
        if((Tcl_CreateNamespace(pInterp, "::varmgr", NULL, NULL)) == NULL) {
            return TCL_ERROR;
        }
        
        // Wrap the interpreter and create the command objects:
        
        CTCLInterpreter* pWrappedInterp(new CTCLInterpreter(pInterp));
        new CVarMgrCreateCommand(*pWrappedInterp, "::varmgr::create");
        new CVarMgrOpenCommand(*pWrappedInterp, "::varmgr::open");
        new CVarMgrCloseCommand(*pWrappedInterp, "::varmgr::close");
        new CVarMgrMkdirCommand(*pWrappedInterp, "::varmgr::mkdir");
        // Success.
        
        return TCL_OK;
    }
}


void* gpTCLApplication(0);