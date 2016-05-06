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
# @file   CRingCommand.cpp
# @brief  Implement the ring command
# @author <fox@nscl.msu.edu>
*/

#include "CRingCommand.h"
#include "CExperiment.h"
#include "RunState.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>



/**
 * construtor
 *  @param interp - interpreter that is running the command.
 *  @param name   - Name of the command
 *  @param pExperiment - points to the experiment.
 */
CRingCommand::CRingCommand(
    CTCLInterpreter& interp, const char* name, CExperiment* pExperiment
) : CTCLObjectProcessor(interp, name, true), m_pExperiment(pExperiment)
{}


/**
 * destructor
 */
CRingCommand::~CRingCommand()
{}


/**
 * operator()
 *    - Ensure there's a ring.
 *    - Ensure the run is halted.
 *    - Invoke the setRing method of the experiment object.
 *    
 * @param interp - interpreter running the command
 * @param objv   - vector of words that make up the command.
 * @return int   - TCL_OK - success TCL_ERROR failed with message in the result.
 */
int
CRingCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 2, "ring command needs only a ring name");
        std::string ringName = objv[1];
        RunState* pState = RunState::getInstance();
        
        if(pState->stateName() == "Inactive") {
            m_pExperiment->setRing(ringName);
        } else {
            throw std::string("Run must be halted to use the 'ring' command");
        }
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
    return TCL_OK;
}