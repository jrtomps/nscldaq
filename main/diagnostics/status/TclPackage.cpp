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
# @file   TclPackage.cpp
# @brief  Initialization of Tcl Package for messaging package.
# @author <fox@nscl.msu.edu>
*/

#include <tcl.h>
#include "TCLInterpreter.h"
#include <zmq.hpp>
#include "CTCLRingStatistics.h"
#include "CTCLReadoutStatistics.h"
#include "CTCLLogMessage.h"
#include "CTCLStateChangeMessage.h"
#include "CTCLSubscriptions.h"
#include "CTCLDecodeMessage.h"
#include "CTCLAggregate.h"


static const char* packageName = "statusMessage";
static const char* packageVers = "1.0";


/**
 * tclStatusMessages_init
 *    Initializes the package.  Note that this must have C bindings for
 *    the Tcl loader to find it:
 *
 *  @param pInterp - Tcl_Interp* pointer.
 *  @return int    - TCL_OK - on ok load else TCL_ERROR.
 */

extern "C" {
    int Tclstatusmessages_Init(Tcl_Interp* pInterp)
    {

        int status = Tcl_PkgProvide(pInterp, packageName, packageVers);
        if (status != TCL_OK) {
            return status;
        }
    
        // Create a CTCLInterpreter on which we'll define the package's
        // command:
        
        CTCLInterpreter* interp  = new CTCLInterpreter(pInterp);
        
        // Add the commands:
        
        new CTCLRingStatistics(*interp);
        new CTCLReadoutStatistics(*interp);
        new CTCLLogMessage(*interp);
        new CTCLStateChangeMessage(*interp);
        new CTCLSubscription(*interp);
        new CTCLDecodeMessage(*interp);
        new CTCLAggregate(*interp);
        return TCL_OK;
    }
    // needed by tcl++
    
    void* gpTCLApplication(0);

}


