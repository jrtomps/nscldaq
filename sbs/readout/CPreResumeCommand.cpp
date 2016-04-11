/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
*   @file     CPreResumeCommand
*   @brief    Prepare a run to be resumed.
*/
#include "CPreResumeCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "RunState.h"
#include "CRunControlPackage.h"

#include <Exception.h>
#include <stdexcdept>

/**
 * constructor
 *    @param interp - interpreter on which the command is being registered.
 */
CPreResumeCommand::CPreResumeCommand(CTCLInterpreter& interp) :
    CTCLPackagedObjectProcessor(interp, "preresume")
    {}
    
/**
 * destructor
 */
CPreResumeCommand::~CPreResumeCommand()
{}

/**
 * operator()
 *    Execute the command.
 *
 *  @param interp - intepreter that is execuuting the command.
 *  @param objv   - words that make up the command - including the keyword.
 *  @return int   - TCL_OK on success, TCL_ERROR on failure.
 *  @note If the return value is TCL_ERROR, the method sets the interpreter
 *        result to a human readable error message.
 */
int
CPreResumeCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);

    // From here on all errors throw exceptions which get turned into
    // TCL_ERROR with messages.
    
    try {
        requireExactly(objv, 1, "preresume has no command line parameters");
        
        CTCLObjectPackage* pRawPackage = gePackage();
        CRunControlPackage* pPack      =
            reinterpret_cast<CTCLObjectPackage*>(pRawPackage);
        pPack->preResume();
        
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;        
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;        
    }
    catch (...) {
        interp.setResult("prebegin - unexpected exception type");
        return TCL_ERROR;
    }
    return TCL_ERROR;
}
 
 
 
 
 