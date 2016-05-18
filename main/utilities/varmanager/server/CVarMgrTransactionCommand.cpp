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
# @file   CVarMgrTransactionCommand.cpp
# @brief  Implementation of varmgr::transaction.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrTransactionCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>

#include <tcl.h>                          // Want to directly look at the status.

#include <stdexcept>
#include <memory>
#include <string>

/**
 * constructor
 *    @param interp - Interpreter on which the command is being registered.
 *    @param name   - Name of the command being registered
 */
CVarMgrTransactionCommand::CVarMgrTransactionCommand(
    CTCLInterpreter& interp, const char* name
) :
    CTCLObjectProcessor(interp, name, true)
{}

/**
 * destructor
 *    let the base class handle this
 */
CVarMgrTransactionCommand::~CVarMgrTransactionCommand()
{}

/**
 * operator()
 *    process the command itself.
 *    -  Ensure there are exactly two operands.
 *    -  Ensure the first is a handle and get it.
 *    -  Start a transaction on the handle.
 *    -  Execute the script
 *    -  Take the appropriate action depending on the result of the script.
 *
 *  @param interp -- interpreter executing the command.
 *  @param objv   -- Vector of objects that make up the command words.
 */
int
CVarMgrTransactionCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    int status = TCL_OK;
    try {
        requireExactly(objv, 3, "::varmgr::transaction requires only a handle and a script");
        
        std::string handle = objv[1];
        std::string script = objv[2];
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if(!pApi) {
            throw std::runtime_error("Invalid variable database handle");
        }
        std::unique_ptr<CVarMgrApi::Transaction> t(pApi->transaction());
        
        status = Tcl_Eval(interp.getInterpreter(), script.c_str());
        
        // handle the cases where we need and map other cases to
        // normal return.
        
        switch (status) {
            case TCL_BREAK:
                status = TCL_OK;                    // Rollback but ok.
            case TCL_ERROR:
                t->scheduleRollback();               // Rollback.
                break;
            default:
                status = TCL_OK;                   // Successful and commit.
        }
        
    }                                             // Commit or rollback here.
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch(CVarMgrApi::Unimplemented& e) {
        interp.setResult("Underlying database transport does not support transactions");
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string s) {
        interp.setResult(s);
        return TCL_ERROR;
    }
    catch (const char* s) {
        interp.setResult(s);
        return TCL_ERROR;
    }
    catch(...) {
        interp.setResult("varmgr::transaction caught an unanticpated exception");
        return TCL_ERROR;
    }
    
    return status;
    
}
