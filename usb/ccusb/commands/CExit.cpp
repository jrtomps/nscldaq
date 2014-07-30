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
# @file   CExit.cpp
# @brief  Implement a replacement for the Tcl exit command.
# @author <fox@nscl.msu.edu>
*/

#include "CExit.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <Globals.h>
#include <TclServer.h>

/**
 * constructor
 *   Create the 'exit' command which should now override the Tcl exit
 *   command:
 *   @param interp - interpreter on which this will be registered.
 */
CExit::CExit(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "exit", true)
    {}
/**
 * destructor
 *   Just lets this chain to the parent destructor.
 */
CExit::~CExit() {}

/**
 * operator()
 *   Executes the command
 *   *  Ensure there's at most one extra argument.
 *   *  If there is an argument that must be an integer and it replaces the
 *      default exit status (0).
 *   *  Send an exit event to the Tcl Server
 *   *  Join with the tcl server.
 *   *  Invoke Tcl_Exit to exit the application.
 *
 *  @param interp - Reference to the encapsulated interpreter that is running
 *                  the command.
 *  @param objv   - Array of CTCLObjects that represent the command words.
 *  @return int   - Though actually this function does not return unless
 *                  there is an error, in which case it returns TCL_ERROR.
 */
int
CExit::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireAtMost(objv, 2, "exit takes at most the status parameters");
        
        // Default the exit status and override it if there is a param:
        
        int status = 0;
        if (objv.size() == 2) {
            status = objv[1];                // Throws if not integer.
        }
        // Send an event to the TclServer proces's interpreter asking it to exit.
        //
        
        TclServer* pServer = ::Globals::pTclServer;
        pServer->scheduleExit();
        
        
        // Join the TclServer's thread.
        
        pServer->join();
        
        // Exit the program:
        
        Tcl_Exit(status);
    }
    catch (std::string msg) {
        interp.setResult(msg);
    }
    
    return TCL_ERROR;
}