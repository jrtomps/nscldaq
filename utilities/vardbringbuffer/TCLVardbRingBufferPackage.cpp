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
# @file   TCLVardbRingBufferPackage.cpp
# @brief  Initialization for the vardbringbuffer package.
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CTCLVardbRingBuffer.h"


extern "C" {
    int Tclvardbringbuffer_Init(Tcl_Interp* pInterp)
    {
        // Make the ::nscldaq namespace if appropriate.
        
        Tcl_CreateNamespace(pInterp, "::nscldaq", NULL, NULL);
        
        // Register our package
        
        Tcl_PkgProvide(pInterp, "vardbringbuffer", "1.0");
        
        // Now wrap the interpreter in CLTCObject
        // Instantiante commands and keep them safe.
        
        CTCLInterpreter* interp = new CTCLInterpreter(pInterp);
        new CTCLVardbRingBuffer(*interp, "::nscldaq::vardbringbuffer");
        
        return TCL_OK;
    }
}
void* gpTCLApplication(0);