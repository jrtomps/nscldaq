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
# @file   CVarMgrEnumCommand.cpp
# @brief  Implement the enum operation.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrEnumCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"


#include <stdexcept>

/**
 * constructor
 *
 * @param interp - references the interpreter on which the command will be
 *                 registered.
 * @param command- The command name string that will activate our command.
 */
CVarMgrEnumCommand::CVarMgrEnumCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true) {}
    
/**
 * destructor
 */
CVarMgrEnumCommand::~CVarMgrEnumCommand() {}

/**
 * operator()
 *    Executes the enum command
 *    -  Require the correct parameter count.
 *    -  Extract/translate the handle.
 *    -  Extract the name
 *    -  Marshall the list of values to an EnumValues vector.
 *    -  Make the enum.
 *
 * @param interp  - interpreter running the command.
 * @param objv    - Vector of encapsulated command word objects.
 * @return integer - TCL_OK on success, TCL_ERROR on failure.
 */
int
CVarMgrEnumCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    sprintf(usage, "Usage\n   %s handle type-name values",
            std::string(objv[0]).c_str());
    try {
        requireExactly(objv, 4, usage);
        
        std::string handle = objv[1];
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        
        std::string typeName = objv[2];
        
        CVarMgrApi::EnumValues values;
        for (int i =0; i < objv[3].llength(); i++) {
            CTCLObject value;
            value.Bind(interp);
            value = objv[3].lindex(i);
            values.push_back(std::string(value));
        }
        
        pApi->defineEnum(typeName.c_str(), values);
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}