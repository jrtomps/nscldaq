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
        } else if (subCommand == "setStandalone") {
            setStandalone(interp, objv);
        } else if (subCommand == "setNoStandalone") {
            setNoStandalone(interp, objv);
        } else if (subCommand == "isStandalone") {
            isStandalone(interp, objv);
        } else if (subCommand == "listPrograms") {
            listPrograms(interp, objv);
        } else if (subCommand == "listEnabledPrograms") {
            listEnabledPrograms(interp, objv);
        } else if (subCommand == "listStandalonePrograms") {
            listStandalonePrograms(interp, objv);
        } else if (subCommand == "listInactivePrograms") {
            listInactivePrograms(interp, objv);
        } else if (subCommand == "listActivePrograms") {
            listActivePrograms(interp, objv);
        } else if (subCommand == "deleteProgram") {
            deleteProgram(interp, objv);
        } else if (subCommand == "setGlobalState") {
            setGlobalState(interp, objv);
        } else if (subCommand == "getGlobalState") {
            getGlobalState(interp, objv);
        } else if (subCommand == "getParticipantStates") {
            getParticipantStates(interp, objv);
        } else if (subCommand == "title") {
            title(interp, objv);
        } else if (subCommand == "timeout") {
            timeout(interp, objv);
        } else if (subCommand == "recording"){
            recording(interp, objv);
        } else if (subCommand == "runNumber") {
            runNumber(interp, objv);
        } else if (subCommand == "waitTransition") {
            waitTransition(interp, objv);
        } else if (subCommand == "processMessages") {
            processMessages(interp, objv);
        } else if (subCommand == "isActive"){
            isActive(interp, objv);
        } else if (subCommand == "setProgramState") {
            setProgramState(interp, objv);
        } else if (subCommand == "getProgramState") {
            getProgramState(interp, objv);
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
/**
 * setProgramState
 *    Set the state of an individual program  This is normally used if
 *    the program is standalone or if the program has died and its state machine
 *    needs to be set to a known condition.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - Command words.
 */
void
CTCLStateManagerInstanceCommand::setProgramState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "setProgramState needs program name and new state");
    std::string name(objv[2]);
    std::string state(objv[3]);
    
    m_pApi->setProgramState(name.c_str(), state.c_str());
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

    interp.setResult(
        m_pApi->isProgramEnabled(std::string(objv[2]).c_str()) ? "1" : "0"
    );
    
 }
/**
 * setStandalone
 *    Set a program into the standalone state.
 *
 * @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
void
CTCLStateManagerInstanceCommand::setStandalone(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "setStandalone requires exactly the program name");
    std::string program = objv[2];
    
    m_pApi->setProgramStandalone(program.c_str());
    
}
/**
 * setNoStandalone
 *    Turn off the standalone flag.
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
void
CTCLStateManagerInstanceCommand::setNoStandalone(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "setNoStandalone requires exactly the program name");
    std::string program = objv[2];
    
    m_pApi->setProgramNoStandalone(program.c_str());
}
/**
 * isStandalone
 *   Set result to 1 if so 0 if not.
 *  @param interp - interpreter decoding the map.
 *  @param dict   - CTCLObject& of the map.
 */
void
CTCLStateManagerInstanceCommand::isStandalone(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
   requireExactly(objv, 3, "isStandalone needs exactly a program name");
   
   std::string program = objv[2];
   interp.setResult(m_pApi->isProgramStandalone(program.c_str()) ? "1" : "0");
}
/**
 * listPrograms.
 *    Returns a list of the defined programs.  No filtering is done.
 * @param interp - interpreter running the command.
 * @param objv   - vector of commnd words.
 */
void
CTCLStateManagerInstanceCommand::listPrograms(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listPrograms takes no additional parameters");
    
    std::vector<std::string> progs = m_pApi->listPrograms();
    vectorToResult(interp, progs);
}
/**
 * listEnabledPrograms
 *    List the programs with the enable flag true.
 * @param interp - interpreter the command is executing on.
 * @param objv   - command words.
 */
void
CTCLStateManagerInstanceCommand::listEnabledPrograms(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listEnabledPrograms needs no other parameters");
    
    std::vector<std::string> progs = m_pApi->listEnabledPrograms();
    vectorToResult(interp, progs);

}
/**
 * listStanalonePrograms
 *    List the set of programs that have their standalone flag 'true'.
 *
 * @param interp - Interpreter executing the command.
 * @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::listStandalonePrograms(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listStandalonePrograms needs no other parameter");
    
    std::vector<std::string> progs = m_pApi->listStandalonePrograms();
    vectorToResult(interp, progs);
}
/**
 * listInactivePrograms
 *    list programs that either are disabled or standalone.
 *  @param interp - interpreter running the command.
 *  @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::listInactivePrograms(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listInactivePrograms needs no other parameter");
    
    std::vector<std::string> progs = m_pApi->listInactivePrograms();
    vectorToResult(interp, progs);
}
/**
 * listActivePrograms
 *    List the programs that are not inactive.
 * @param interp - interpreter running the command.
 * @param objv   - words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::listActivePrograms(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listActivePrograms needs no other parameters");
    
    std::vector<std::string> progs = m_pApi->listActivePrograms();
    vectorToResult(interp, progs);
}

/**
 * deleteProgram
 *    Delete an existing program.
 *
 * @param interp  - interpreter running the command.
 * @param objv    - words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::deleteProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "deleteProgram needs only a program name");
    
    std::string program = objv[2];
    m_pApi->deleteProgram(program.c_str());
}

/**
 * setGlobalState
 *    Set the global state
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::setGlobalState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "setGlobalState requires only the next state");
    std::string state = objv[2];
    m_pApi->setGlobalState(state.c_str());
}
/**
 * getGlobalState
 *    Return the current value for the gflobal state.
 *
 * @param interp - the interpreter running the command.
 * @param objv   - The words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::getGlobalState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "getGlobal state requires no parameters");
    
    interp.setResult(m_pApi->getGlobalState());
}
/**
 * getParticipantStates
 *    Return a list of pairs.  The first element of each pair is a
 *    participant in global state changes. The second element is the
 *    state of that participant.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateManagerInstanceCommand::getParticipantStates(
   CTCLInterpreter& interp, std::vector<CTCLObject>& objv 
)
{
    requireExactly(objv, 2, "getParticipantStates requires no parameters");
    
    std::vector<std::pair<std::string, std::string> > progStates =
        m_pApi->getParticipantStates();
        
    // Marshall the pairs into a list of pairs.
    
    CTCLObject result; result.Bind(interp);
    for (int i = 0; i < progStates.size(); i++) {
        CTCLObject progName;  progName.Bind(interp);
        CTCLObject progState; progState.Bind(interp);
        CTCLObject element;   element.Bind(interp);
        
        progName = progStates[i].first;
        progState= progStates[i].second;
        
        element += progName;
        element += progState;
        result  += element;
    }
    
    interp.setResult(result);
}
/**
 * title
 *   Set or get the current title.  If there's an extra parameter it is
 *   the new title.  If not, the title is returned.
 *
 * @param interp - interpreter in which the command is runnig.
 * @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::title(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "title can have at most a new title");
    
    if (objv.size() == 2) {
        interp.setResult(m_pApi->title());
    } else {
        std::string title = objv[2];
        m_pApi->title(title.c_str());
    }
}
/**
 * timeout
 *    set/get the current state transition timeout.  If there's an extra
 *    parameter it must be an integer new timeout value.
 *
 * @param interp - interpreter on which the command is running.
 * @param objv   - Command words.
 */
void
CTCLStateManagerInstanceCommand::timeout(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "timeout can have at most a new timeout value");
    
    if (objv.size() == 2) {
        CTCLObject result;
        result.Bind(interp);
        result = int(m_pApi->timeout());
        
        interp.setResult(result);
    } else {
        int newTimeout = objv[2];
        m_pApi->timeout(newTimeout);
    }
}
/**
 * recording
 *   Set/get the value of the recording flag.  If an extra parameter is
 *   provided, it must be a legal Tcl boolean and becomes the new
 *   value of the recording flag.
 *
 *  @param interp - interpreter on which the command is running.
 *  @param objv   - command words.
 */
void
CTCLStateManagerInstanceCommand::recording(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "recording can have a most a new recording value");
    
    if (objv.size() == 2) {
        interp.setResult(m_pApi->recording() ? "1" : "0");  // EIAS
    } else {
        std::string valueString = objv[2];
        int valueInt;
        int status = Tcl_GetBoolean(
            interp.getInterpreter(), valueString.c_str(), &valueInt
        );
        
        if (status != TCL_OK) {
            throw std::invalid_argument("recording - invalid boolean parameter");
        }
        
        m_pApi->recording(bool(valueInt));
        
    }
}
/**
 * runNumber
 *    get/set the run number.  If an extra integer parameter is provided,
 *    it becomes the new run number.
 *
 *  @param interp  - interpreter on which the command is running.
 *  @param objv    - Command words.
 */
void
CTCLStateManagerInstanceCommand::runNumber(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "runNumber takes at most a new run number parameter");
    
    if (objv.size() == 2) {
        CTCLObject result;
        result.Bind(interp);
        result = int(m_pApi->runNumber());
        interp.setResult(result);
    } else {
        int newRun = objv[2];
        if (newRun < 0) {
            throw std::domain_error("Run numbers must be positive");
        }
        m_pApi->runNumber(newRun);
    }
}
/**
 * waitTransition
 *   Wait for a transition to either complete or to time out.
 *   If there is an additional parameter, it is a callback script that
 *   is given program names and their state transitions.  Note that this
 *   method is not waiting in the Tcl event loop so  you may need to
 *   either spin this off into a thread or have your callback invoke
 *   update idletasks.
 *
 * @param interp - Interpreter in which the command is being run.
 * @param objv   - The words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::waitTransition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "waitTransition takes at most a callback script");
    
    try {
        CStateManager::TransitionCallback cb(0);
        void*                              cd(0);
        CallbackInfo                       cinfo;
        if(objv.size() == 3) {
            cb = dispatchTransitionScript;
            cd = &cinfo;
            cinfo.s_pInterp = &interp;
            cinfo.s_scriptBase = std::string(objv[2]);
        }
        m_pApi->waitTransition(cb, cd);
        interp.setResult("1");
    } catch(...) {
        interp.setResult("0");
    }
}
/**
 * processMessages
 *    Empty the message queue, potentially invoking a script callback
 *    for each message in the queue.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::processMessages(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "processMessages at most takes a call back script");
    
    CStateManager::BacklogCallback  cb(0);
    void*                           cd(0);
    CallbackInfo                     info;
    
    if (objv.size() == 3) {
        cb = &dispatchMessageScript;
        cd = &info;
        info.s_pInterp    = &interp;
        info.s_scriptBase = std::string(objv[2]);
    }
    
    m_pApi->processMessages(cb, cd);
    
    
}

/**
 * isActive
 *    Sets the result to a true value(1) if the specified program is
 *    active and a false value (0) if not.
 * @param interp - interpreter running the command.
 * @param objv   - Words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::isActive(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "isActive requires (only) a program name");
    std::string programName(objv[2]);
    interp.setResult(m_pApi->isActive(programName.c_str()) ? "1" : "0");
}

/**
 * getProgramState
 *    Set the interpreter result with the state of a program.
 *
 *  @param interp - interpreter running the command
 *  @param objv   - command words that make up the command.
 */
void
CTCLStateManagerInstanceCommand::getProgramState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "getProgramState requires exactly the program name");
    
    std::string name(objv[2]);
    interp.setResult(m_pApi->getProgramState(name.c_str()));
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
/**
 * vectorToResult
 *   Marshall a vector of strings into a TCL list that is set as an
 *   interpreter result.
 *
 * @param interp - the interpreter who's result will be set.
 * @param vec    - std::vector<std::string> to marshall.
 */
void
CTCLStateManagerInstanceCommand::vectorToResult(
    CTCLInterpreter& interp, std::vector<std::string> vec
)
{
CTCLObject result;
    result.Bind(interp);
    for (int i =0; i < vec.size(); i++) {
        CTCLObject item;
        item.Bind(interp);
        
        item = vec[i];
        result += item;
    }
    
    interp.setResult(result);    
}
/**
 * DictPutString
 *   Puts a string key/value pair into a dict.
 * @param pInt  - interpreter doing the processing.
 * @param pDict - Target dict.
 * @param key   - Key to put.
 * @param value - Value to associate with the key.
 */
void
CTCLStateManagerInstanceCommand::DictPutString(
    Tcl_Interp* pInt, Tcl_Obj* pDict, std::string key, std::string value
)
{
    Tcl_Obj* pKey   = Tcl_NewStringObj(key.c_str(), -1);
    Tcl_Obj* pValue = Tcl_NewStringObj(value.c_str(), -1);
    
    Tcl_DictObjPut(pInt, pDict, pKey, pValue);
}
/** 
*   dispatchTransitionScript
*     Dispatches the state transtion callback script from waitTransition.
*
*     @param manager - state manager object pointer.
*     @param program  - Name of program making transition.
*     @param state    - New state that program is in.
*     @param cd       - Actually  a pCallbackInfo struct.
*
* @note this is static.
*/
void
CTCLStateManagerInstanceCommand::dispatchTransitionScript(
    CStateManager& manager, std::string program, std::string state,
    void* cd
)
{
    pCallbackInfo pInfo = static_cast<pCallbackInfo>(cd);
    
    // Construct the command:
    
    std::string command = pInfo->s_scriptBase;
    command += " {";
    command += program;
    command += "} {";
    command += state;
    command += "}";
    
    pInfo->s_pInterp->GlobalEval(command);
}

/**
 * dispatchMessageCallback
 *   Called from processMessages to dispatch to the callback script set up
 *   by the user.
 *
 *  @param mgr - State manager object.
 *  @param msg - Notification message.
 *  @param cd  - Pointer to what is actually a CallbackInfo struct.
 *  
 */
void
CTCLStateManagerInstanceCommand::dispatchMessageScript(
    CStateManager& mgr, CStateTransitionMonitor::Notification msg,
    void* cd
)
{
    pCallbackInfo pInfo = static_cast<pCallbackInfo>(cd);
    CTCLInterpreter& Interp(*(pInfo->s_pInterp));
    Tcl_Interp*      pInt = Interp.getInterpreter();
    
    // Create the parameter dict from the message.
    
    Tcl_Obj* pDict = Tcl_NewDictObj();
    
    switch (msg.s_type) {
    case CStateTransitionMonitor::GlobalStateChange:
        
        DictPutString(pInt, pDict, "type", "GlobalStateChange");
        DictPutString(pInt, pDict, "state", msg.s_state);
        break;
    case CStateTransitionMonitor::ProgramStateChange:
        DictPutString(pInt, pDict, "type", "ProgramStateChange");
        DictPutString(pInt, pDict, "state", msg.s_state);
        DictPutString(pInt, pDict, "program", msg.s_program);
        break;
    case CStateTransitionMonitor::ProgramJoins:
        DictPutString(pInt, pDict, "type", "ProgramJoins");
        DictPutString(pInt, pDict, "program", msg.s_program);
        break;
    case CStateTransitionMonitor::ProgramLeaves:
        DictPutString(pInt, pDict, "type", "ProgramLeaves");
        DictPutString(pInt, pDict, "program", msg.s_program);
        break;
    default:
        throw std::runtime_error("Unrecognized message type in processMessages callback");
    }
    
    std::string command = pInfo->s_scriptBase;
    command += " {";
    command += Tcl_GetString(pDict);
    command += "}";
    
    Interp.GlobalEval(command);
}

