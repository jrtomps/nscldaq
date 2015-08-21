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
# @file   TclServicesPackage.cpp
# @brief  Package initialization for the TCL bindings to libServices
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CTCLServiceApiCommand.h"

extern "C" {
    int Tclservices_Init(Tcl_Interp* interp)
    {
        CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
        
        // Register package and namespace
        
        Tcl_PkgProvide(interp, "daqservices", "1.0");
        Tcl_CreateNamespace(interp, "::nscldaq", 0, 0);
        
        // Register the commands:
        
        new CTCLServiceApiCommand(*pInterp, "::nscldaq::services");
        
        return TCL_OK;
    }
}


void* gpTCLApplication(0);