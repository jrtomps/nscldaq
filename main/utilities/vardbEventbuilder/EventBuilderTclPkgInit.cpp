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
# @file   EventBuilderTclPkgInit.tcl
# @brief  Package initialization for the vardbEventBuilder package.
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CTCLVarDbEventBuilder.h"



const char* version="1.0";

extern "C" {
    int  Tclvardbeventbuilder_Init(Tcl_Interp* pI)
    {
                
#ifdef USE_TCL_STUBS
        Tcl_InitStubs(pInterp, "8.4", 0);
#endif
        int status = Tcl_PkgProvide(pI, "vardbEventBuilder", version);
        if (status != TCL_OK) return status;
        
        if( Tcl_CreateNamespace(pI, "::nscldaq::", NULL, NULL) == NULL) {
            return TCL_ERROR;
        }
        // Create the command ensemble:
        
        CTCLInterpreter* pInterp = new CTCLInterpreter(pI);
        new CTCLVarDbEventBuilder(pInterp, "::nscldaq::evb");
        
        return TCL_OK;
    }
}


void* gpTCLApplication(0);

