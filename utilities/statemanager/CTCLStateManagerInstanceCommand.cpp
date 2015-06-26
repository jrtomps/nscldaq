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
# @file   CTCLStateManagerInstanceCommand.cpp
# @brief  command ensemble that encapsulates a state manager instance.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStateManagerInstanceCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CStateManager.h"
#include <stdexcept>
#include <iostream>
/**
 * constructor:
 *   @param interp - interpreter on which the command is registered.
 *   @param name   - The command to register.
 *   @param requrl - URI for the request port of the server.
 *   @param suburl - URI of the subscriptionp port of the server.
 */
CTCLStateManagerInstanceCommand::CTCLStateManagerInstanceCommand(
    CTCLInterpreter& interp, std::string name,
    std::string requrl, std::string suburl
) : CTCLObjectProcessor(interp, name.c_str(), true),
    m_pApi(0)
{
    m_pApi = new CStateManager(requrl.c_str(), suburl.c_str());
}
/**
 * destructor:
 */
CTCLStateManagerInstanceCommand::~CTCLStateManagerInstanceCommand()
{
    delete m_pApi;
}

/**
 * operator()
 *    Dispatch the command ensemble based on the second command word.
 * @param interp - Interpreter executing the command.
 * @param objv   - Command words.
 * @return int : TCL_OK on successful command execution, TCL_ERROR if not.
 */
int
CTCLStateManagerInstanceCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command reuires a subcommand");
        std::string subCommand = objv[1];
        
        if (subCommand == "programParentDir") {
            programParentDir(interp, objv);
        } else if (subCommand == "addProgram") {
            addProgram(interp, objv);
        } else if (subCommand == "getProgram") {
            getProgram(interp, objv);
        } else if (subCommand == "modifyProgram") {
            modifyProgram(interp, objv);
        } else if (subCommand == "enableProgram") {
            enableProgram(interp, objv);
        } else if (subCommand == "disableProgram") {
            disableProgram(interp, objv);
        } else if (subCommand == "isProgramEnabled") {
            isProgramEnabled(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand keyword");
        }
    }
    catch (std::exception & e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught");
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*-----------------------------------------------------------------------
 *  Subcommand execution.
 */

/**
 * programParentDir
 *    Set/get program parent directory.
 *
 * @param interp - Interpreter executing the command.
 * @param objv   - Command words.
 */
void
CTCLStateManagerInstanceCommand::programParentDir(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    if (objv.size() == 2) {
        interp.setResult(m_pApi->getProgramParentDir());
    } else if (objv.size() == 3) {
        std::string newdir = objv[2];
        m_pApi->setProgramParentDir(newdir.c_str());
    } else {
        throw std::invalid_argument("Too many parameters for programParentDir");
    }
}
/**
 * addProgram
 *   Adds a new program to the system.
 *
 * @param interp - Interpreter executing the command.
 * @param objv   - Command words.
 */
void
CTCLStateManagerInstanceCommand::addProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "addProgram name dict - mssing or too many params");
    
    // Pull out the parameters.
    
    std::string name = objv[2];
    std::map<std::string, std::string> programDefMap =
        dictToMap(interp, objv[3]);
    
    // Merge in defaults:
    
    std::map<std::string, std::string> defaults;
    defaults["enabled"]  = "true";
    defaults["standalone"] = "false";
    defaults["outring"]    = "";
    defaults["inring"]     = "";
    
    programDefMap.insert(defaults.begin(), defaults.end());
    
    // Enforce existence of mandatory parameters:
    
    if ((programDefMap.find("path") == programDefMap.end()) ||
        (programDefMap.find("host") == programDefMap.end())) {
        throw std::invalid_argument(
            "'host' and 'program' are required one or both arem missing"
        );
    }
    
    // Map -> program def struct:
    
    CStateManager::ProgramDefinition programDef;
    programDef.s_enabled    = programDefMap["enabled"] == "true" ? true : false;
    programDef.s_standalone =
        programDefMap["standalone"] == "true" ? true : false;
    programDef.s_path    = programDefMap["path"];
    programDef.s_host    = programDefMap["host"];
    programDef.s_outRing = programDefMap["outring"];
    programDef.s_inRing  = programDefMap["inring"];
    
    m_pApi->addProgram(name.c_str(), &programDef);
}

/**
 * getProgram
 *    Set the result with a dict that decribes the specified program.
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
void
CTCLStateManagerInstanceCommand::getProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "getProgram needs program name (and only that)");
    
    CStateManager::ProgramDefinition def = m_pApi->getProgramDefinition(
        std::string(objv[2]).c_str()
    );
    
    // Turn the dict into a struct:
    
    CTCLObject key;  key.Bind(interp);
    CTCLObject val;  val.Bind(interp);
    
    Tcl_Obj* pResult = Tcl_NewDictObj();
    Tcl_Interp* pInterp = interp.getInterpreter();
    
    key = std::string("enabled");
    val = std::string(def.s_enabled ? "true" : "false");
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
    
    key = std::string("standalone");
    val = std::string(def.s_standalone ? "true" : "false");
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
    
    key = std::string("path");
    val = def.s_path;
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
    
    key = std::string("host");
    val = def.s_host;
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
    
    key = std::string("outring");
    val = def.s_outRing;
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
    
    key = std::string("inring");
    val = def.s_inRing;
    Tcl_DictObjPut(pInterp, pResult, key.getObject(), val.getObject());
        
    interp.setResult(pResult);
}
/**
 * modifyProgram
 *    Modifies the specified program.
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
void
CTCLStateManagerInstanceCommand::modifyProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "modifyProgram needs a program name and dict only");
    
    std::string name = objv[2];
    std::map<std::string, std::string> modDef = dictToMap(interp, objv[3]);
    
    CStateManager::ProgramDefinition desc = m_pApi->getProgramDefinition(
        name.c_str()
    );
    
    // For each key we care about in the map, modify the struct:
    
    std::map<std::string, std::string>::iterator p;
    
    if ((p = modDef.find("enabled")) != modDef.end()) {
        desc.s_enabled = p->second == "true" ? true : false;
    }
    
    if ((p = modDef.find("standalone")) != modDef.end()) {
        desc.s_standalone = p->second == "true" ? true : false;
    }
    
    if ((p = modDef.find("path")) != modDef.end()) {
        desc.s_path = p->second;
    }
    
    if ((p = modDef.find("host")) != modDef.end()) {
        desc.s_host = p->second;
    }
    
    if ((p = modDef.find("inring")) != modDef.end()) {
        desc.s_inRing = p->second;
    }
    
    if ((p = modDef.find("outring")) != modDef.end()) {
        desc.s_outRing = p->second;
    }
    
    // Update the program (no attempt is made to optimize for an
    // empty dict).
    
    m_pApi->modifyProgram(name.c_str(), &desc);
}
/**
 * enableProgram
 *
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
 void
 CTCLStateManagerInstanceCommand::enableProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
 )
 {
    requireExactly(objv, 3, "enableProgram needs exactly the program name");
    
    std::string name = std::string(objv[2]);
    m_pApi->enableProgram(name.c_str());
 }
 /**
  * disableProgram
  * 
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
 void
 CTCLStateManagerInstanceCommand::disableProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
 )
 {
    requireExactly(objv, 3, "disableProgram needs exactly the program name");
    
    std::string name = std::string(objv[2]);
    m_pApi->disableProgram(name.c_str());
 }
 
 /**
  * isProgramEnabled
  *  Sets the interpreter result to 1 if the program is enabled, 0 otherwise.
  * @param interp - interpreter decoding the map.
  * @param dict   - CTCLObject& of the map.
 */
 void
 CTCLStateManagerInstanceCommand::isProgramEnabled(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
 )
 {
    requireExactly(
        objv, 3, "isProgramEnabled requires exactly the program name"
    );

    std::cerr << "Seting result\n";
    interp.setResult(
        m_pApi->isProgramEnabled(std::string(objv[2]).c_str()) ? "1" : "0"
    );
    
 }
/*------------------------------------------------------------------
 * utilities:
 */

/**
 * dictToMap
 *    Converts a dict to an std::map.
 *
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 *  @return std::string<std::string, std::string> map whose keys/values
 *                  are the same as that of the dict.
 */
std::map<std::string, std::string>
CTCLStateManagerInstanceCommand::dictToMap(CTCLInterpreter& interp, CTCLObject& obj)
{
    Tcl_Interp* pInterp = interp.getInterpreter();
    Tcl_Obj*    pObj    = obj.getObject();
    
    Tcl_DictSearch searchPtr;
    Tcl_Obj*        key;
    Tcl_Obj*        value;
    int             done;
    
    int status = Tcl_DictObjFirst(
        pInterp, pObj, &searchPtr, &key, &value, &done
    );
    
    
    if (status == TCL_ERROR) {
        throw std::invalid_argument("Parameter must be a dict");
    }
    
    
    std::map<std::string, std::string> result;
    while(!done) {
        result[Tcl_GetString(key)] = Tcl_GetString(value);
        Tcl_DictObjNext(&searchPtr, &key, &value, &done);
    }
    
    return result;
}
