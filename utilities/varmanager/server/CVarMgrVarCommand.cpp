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
# @file   CVarMgrVarCommand.cpp
# @brief  Implement the var command.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrVarCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>



/**
 * constructor
 *
 * @param interp     - Interpreter on which the command will be registered.
 * @param pCommand   - String that will activate this command.
 */
CVarMgrVarCommand::CVarMgrVarCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrVarCommand::~CVarMgrVarCommand() {}

/**
 * operator()
 *    Command processor - dispatches the subcommand or complains about an error.
 *
 *  @param interp   - Interpreter executing the command.
 *  @param objv     - Objects that make up the command words.
 *  @return int TCL_OK if successful, TCL_ERROR if not.
 */
int
CVarMgrVarCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    sprintf(usage, "Usage:\n  %s subcommand handle ...", std::string(objv[0]).c_str());
    
    try {
        // We  need to have at least a subcommand and a handle (3 command words)
        
        requireAtLeast(objv, 3, usage);
        std::string subCommand = objv[1];
        std::string handle     = objv[2];
        CVarMgrApi* pApi       = CVarMgrOpenCommand::handleToApi(handle.c_str());
        
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        
        
        
        if(subCommand == "create") {
            create(pApi, objv);
        } else if (subCommand == "get") {
            interp.setResult(get(pApi, objv));
        } else if (subCommand == "set") {
            set(pApi, objv);
        } else if (subCommand == "ls") {
            ls(interp, pApi, objv);
        } else if (subCommand == "destroy") {
            rmvar(pApi, objv);
        } else {
            throw std::runtime_error("Invalid subcommand");
        }
        
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

/**
 * create
 *    Create a new variable.
 *    - Ensure we have the right number of parameters: (5, or 6).
 *    - Extract the path.
 *    - Extract the type.
 *    - If available extract the initial value.
 *    - Create the variable using the api.
 *    
 * @param pApi - Pointer to the API object.
 * @param objv - Bound interpreter objects that are the command.
 * @throw std::runtime_error - errors we detect.
 * @throw std::string - errors detected a bit deeper donwn (e.g. requirexxx).
 */
void
CVarMgrVarCommand::create(CVarMgrApi* pApi, std::vector<CTCLObject>& objv)
{
    char usage[100];
    sprintf(
        usage, "Usage\n  %s %s name type ?initial-value",
        std::string(objv[0]).c_str(),  std::string(objv[1]).c_str()
    );
    
    requireAtLeast(objv, 5 ,usage);
    requireAtMost(objv, 6, usage);
    
    std::string name = objv[3];
    std::string type = objv[4];
    std::string initial;
    const char* szInitial(0);
    
    if (objv.size() == 6) {
        initial = std::string(objv[5]);
        szInitial = initial.c_str();
    }
    pApi->declare(name.c_str(), type.c_str(), szInitial);
}
/**
 * get
 *   Retrieve the value of a variable.
 *   - Require the right number of variable.s
 *   - Pull out the path.
 *   - Return the variable's value.
 *   
 * @param pApi - Pointer to the api.
 * @param objv - Bount objects that make up the command line.
 * @return std::string - Value of the variable.
 * @throw std::runtime_error - errors we directly detect.
 * @throw std::string -errors detected  by e..g requirexxx functions.
 */
std::string
CVarMgrVarCommand::get(CVarMgrApi* pApi, std::vector<CTCLObject>& objv)
{
    char usage[100];
    sprintf(
        usage, "Usage\n  %s %s variable-path",
        std::string(objv[0]).c_str(),  std::string(objv[1]).c_str()
    );
    requireExactly(objv, 4, usage);
    
    std::string path = objv[3];
    return pApi->get(path.c_str());
}

/**
 * set
 *   Set a new value for the variable.
 *   - Ensure we have the right number of parameters.
 *   - Extract the path and new value
 *   - Use the API to set the value.
 *
 * @param pApi - Pointer to the API object.
 * @param objv - Vector of bound objects that are the command words.
 */
void
CVarMgrVarCommand::set(CVarMgrApi* pApi, std::vector<CTCLObject>& objv)
{
    char usage[100];
    sprintf(
        usage, "Usage\n   %s %s variable-path new-value",
        std::string(objv[0]).c_str(), std::string(objv[1]).c_str()
    );
    requireExactly(objv, 5, usage);
    
    std::string path = objv[3];
    std::string value= objv[4];
    
    pApi->set(path.c_str(), value.c_str());
    
}
/**
 * ls
 *     List the variables in a directory.  This is compatible with
 *     vardb::ls however the variable id, the directory id,  and
 *     the type id are all -1 signifying that they are meaningless.
 *
 *  @param interp - The interpreter running the command.
 *  @param pApi   - Pointer to an APi object
 *  @param objv   - Bound objects make up the command words.
 *  @note the result is a list of five element lists consisting of:
 *   -1,name,type,-1,-1,
 */
void
CVarMgrVarCommand::ls(
    CTCLInterpreter& interp, CVarMgrApi* pApi, std::vector<CTCLObject>& objv
)
{
    char usage[100];
    sprintf(
        usage, "Usage\n  %s  %s handle ?path?",
        std::string(objv[0]).c_str(), std::string(objv[1]).c_str()
    );
    requireAtLeast(objv, 3, usage);
    requireAtMost(objv, 4, usage);
    
    const char* pPath(0);
    std::string path;
    if (objv.size() == 4) {
        path = std::string(objv[3]);
        pPath = path.c_str();
    }
    
    std::vector<CVarMgrApi::VarInfo> info = pApi->lsvar(path.c_str());
    CTCLObject result; result.Bind(interp);
    for(int i =0; i < info.size(); i ++) {
        CTCLObject item; item.Bind(interp);
        CTCLObject element; element.Bind(interp);
        element = -1                 ; item += element;
        element = info[i].s_name     ; item += element;
        element = info[i].s_typeName ; item += element;
        element = -1;                ; item += element; item += element;
        result += item;
        
    }
    
    interp.setResult(result);
}
/**
 * rmvar
 *    Destroy a variable.  This requires a handle and a path.
 * @param pApi - pointer to the api object.
 * @param objv - Vector of bound objects making up the command
 */
void
CVarMgrVarCommand::rmvar(CVarMgrApi* pApi, std::vector<CTCLObject>& objv)
{
    char usage[100];
    sprintf(usage, "Usage\n %s %s handle path",
            std::string(objv[0]).c_str(), std::string(objv[1]).c_str());
    requireExactly(objv, 4, usage);
    
    std::string path   = objv[3];
    pApi->rmvar(path.c_str());
    
    
}