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
# @file   CTCLVardbRingBuffer.cpp
# @brief  Implement Tcl command to manage instances of API objects.s
# @author <fox@nscl.msu.edu>
*/

#include "CTCLVardbRingBuffer.h"
#include "CVardbRingBuffer.h"
#include "CTCLVardbRingBufferInstance.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <exception>
#include <stdexcept>
#include <Exception.h>

/**
 * constructor
 *    @param interp - interpreter on which the command will be registered.
 *    @param name   - Command initial keyword.
 */
CTCLVardbRingBuffer::CTCLVardbRingBuffer(CTCLInterpreter& interp, const char* name) :
    CTCLObjectProcessor(interp, name, true)
{}

/**
 * destructor
 *    Destroy all the dynamically allocated api objects:
 */
CTCLVardbRingBuffer::~CTCLVardbRingBuffer()
{
    for (auto p = m_Instances.begin(); p != m_Instances.end(); p++) {
        delete p->second;
        p->second = 0;
    }
}

/**
 * operator()
 *    Command processor.  Recieves control when the command is executed in
 *    its registered interpreter.  The command has 'create' and 'destroy'
 *    subcommand.  This method therefore:
 *    -  Sets up the try/catch pattern of error handling.
 *    -  Binds the objv elements to the interpreter.
 *    -  Requires a command keyword.
 *    -  Dispatches to the correct subcommand handler.
 *
 *  @param interp - The interpreter that is running the command.
 *  @param objv   - Vector of command line words.
 *  @return int   - TCL_OK on correct execution else TCL_ERROR.
 *
 *  @note The interpreter result contents depend on the subcommand and result.
 *        In general, however errors are converted into TCL_ERROR with the
 *        text associated with the exception installed in the result.
 */
int
CTCLVardbRingBuffer::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    int result = TCL_OK;
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Missing subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw std::runtime_error("Invalid subcommand");
        }
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        result = TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        result = TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        result = TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        result = TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught report this as a bug");
        result = TCL_ERROR;
    }
    
    return result;
}

/**
 * create
 *    Create a new api instance.
 *
 *  @param interp - The interpreter that is running the command.
 *  @param objv   - Vector of command line words.
 */
void
CTCLVardbRingBuffer::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Require an api name and a URI
    
    requireExactly(objv, 4, "Incorrect parameter count");
    std::string name = objv[2];
    std::string uri  = objv[3];
    
    auto p = m_Instances.find(name);
    if (p == m_Instances.end()) {
        m_Instances[name] =
            new CTCLVardbRingBufferInstance(interp, name.c_str(), uri.c_str());   
        
    } else {
        throw std::runtime_error("An api with this name has already been created.");
    }
    
}

/**
 * destroy
 *    Destroy an existing api instance.
 *
 *  @param interp - The interpreter that is running the command.
 *  @param objv   - Vector of command line words.
 */
void
CTCLVardbRingBuffer::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Require an api name to destroy:
    
    requireExactly(objv, 3, "Incorrect number of command parameters");
    std::string name = objv[2];
    auto p = m_Instances.find(name);
    if (p == m_Instances.end()) {
        throw std::runtime_error("No api by that name");
    } else {
        delete p->second;
        m_Instances.erase(p);
    }
    
}