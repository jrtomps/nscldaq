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
# @file   TCLStateClientPackage.cpp
# @brief  Package initialization for the stateclient Tcl package.
# @author <fox@nscl.msu.edu>
*/
#include <tcl.h>
#include <TCLInterpreter.h>
#include "CTCLStateClientCommand.h"
#include "CTCLStateManagerCommand.h"

const char* version = "1.0";
const char* packageName = "stateclient";

extern "C" {
    
    int Tclstateclient_Init(Tcl_Interp* pInterp)
    {

        if (
            Tcl_PkgProvide(pInterp, packageName, version) !=
            TCL_OK
        ) {
            return TCL_ERROR;
        }
        
        // Make the enclosing namespace
        
        Tcl_CreateNamespace(
            pInterp, "::nscldaq", nullptr, nullptr
        );
        
        // Create the command:
        
        CTCLInterpreter* pInterpObj =
            new CTCLInterpreter(pInterp);
            
        new CTCLStateClientCommand(
            *pInterpObj, "::nscldaq::stateclient"
        );
        new CTCLStateManagerCommand(
            *pInterpObj, "::nscldaq::statemanager"
        );
        return TCL_OK;
    }
}

void* gpTCLApplication(0);