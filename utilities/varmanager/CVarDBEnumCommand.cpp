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
# @file   CVarDBEnumCommand.cpp
# @brief  Class that implements the enum command for the Vardb Tcl extension.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDBEnumCommand.h"
#include "CVarDbOpenCommand.h"
#include "CEnumeration.h"

#include "TCLInterpreter.h"
#include "TCLObject.h"
#include <exception>


/**
 * constructor
 *    Creates the command.
 *
 * @param interp - the interpreter on which the command is registered.
 * @param command - the command verb string.
 */
CVarDbEnumCommand::CVarDbEnumCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarDbEnumCommand::~CVarDbEnumCommand() {}


/**
 * opertor()
 *    Given control when the command is executed.
 *  @param interp -interpreter executing the command.
 *  @param objv   - Command words.
 *  @return int - TCL_OK correct completion, TCL_ERROR failure.
 *
 *  @note we use exceptions to make error processing simpler.
 *  
 */
int CVarDbEnumCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 4);
        std::string handle = objv[1];
        std::string typeName = objv[2];
        CTCLObject& valueList(objv[3]);
        
        // Get the database handle figured out:
        
        CVarDbOpenCommand::HandleState hState =
            CVarDbOpenCommand::translateHandle(handle);
        if (hState.s_db == NULL) {
            throw std::string("enum - invalid database handle");
        }
        // Marshall the value list into a vector of strings:
        
        std::vector<std::string> values;
        for (int i = 0; i < valueList.llength(); i++) {
            CTCLObject aValue = valueList.lindex(i);
            aValue.Bind(interp);
            std::string sValue = aValue;
            values.push_back(aValue);
        }
        
        CEnumeration::create(*(hState.s_db), typeName.c_str(), values);
    }
    catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught");
        return TCL_ERROR;
        
    }
    return TCL_OK;
}
