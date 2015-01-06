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
# @file   VarDbTclPackage.cpp
# @brief  Package load and registration for Tcl support of the variable database.
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include "TCLInterpreter.h"
#include "CVarDbCreateCommand.h"
#include "CVarDbOpenCommand.h"
#include "CVarDbCloseCommand.h"
#include "CVarDbMkdirCommand.h"
#include "CVarDbCdCommand.h"
#include "CVarDbGetwdCommand.h"
#include "CVarDbLsCommand.h"
#include "CVarDbRmdirCommand.h"
#include "CVarDbVariableCommand.h"
#include "CVarDBEnumCommand.h"

static const char* version="1.0";

extern "C" {
    /**
     *    vardb_Init
     *    Initialize the vardb package:
     *    -  Initialize the stubs interface if using them.
     *    -  Create the ::vardb namespace
     *    -  Declare the vardb package
     *    - Create the command processors.
     *
     * @param interp - Tcl_Interp* that is loading the package.
     * @return int   - TCL_OK on success, TCL_ERROR on failure.
     */
    int Vardbtcl_Init(Tcl_Interp* interp)
    {
        int status;
#ifdef USE_TCL_STUBS
        Tcl_InitStubs(interp, "8.4", 0);
#endif
        if((Tcl_CreateNamespace(interp, "::vardb", NULL, NULL)) == NULL) {
            return TCL_ERROR;
        }

        if ((status = Tcl_PkgProvide(interp, "vardb", version)) != TCL_OK) {
            return status;
        }
    
        
        CTCLInterpreter& Interp(*(new CTCLInterpreter(interp))); // wrap in object.
        
        new CVarDbCreateCommand(Interp, "::vardb::create");                        
        new CVarDbOpenCommand(Interp, "::vardb::open");                        
        new CVarDbCloseCommand(Interp, "::vardb::close");
        new CVarDbMkdirCommand(Interp, "::vardb::mkdir");
        new CVarDbCdCommand(Interp, "::vardb::cd");
        new CVarDbGetwdCommand(Interp, "::vardb::getwd");
        new CVarDbLsCommand(Interp, "::vardb::ls");
        new CVarDbRmdirCommand(Interp, "::vardb::rmdir");
        new CVarDbVariableCommand(Interp, "::vardb::var");
        new CVarDbEnumCommand(Interp, "::vardb::enum");
        
        return TCL_OK;                
                                
    }

}

int gpTCLApplication = 0;