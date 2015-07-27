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
#include <COutputThread.h>
#include <CRunState.h>


#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>



/**
 * constructor
 *   @param interp - interpreter on which the command is registering.
 *   @param name   - name of the command.
 *   @param pRouter - Pointer to the output thread.
 */
CRingCommand::CRingCommand(
    CTCLInterpreter& interp, const char* name, COutputThread* pRouter
) : CTCLObjectProcessor(interp, name, true),
    m_pRouter(pRouter)
{}

/**
 * destructor
 */
CRingCommand::~CRingCommand() {}

/**
 * operator()
 *    Ensure we have a ring name.
 *    Ensure the run is halted.
 *    Invoke the COutputThread::setRing method.
 * @param interp - interpreter running the command
 * @param objv   - Command words.
 * @return int   TCL_OK - success, TCL_ERROR failed with message in result.
 */
int
CRingCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 2, "ring command needs ring name only");
        std::string ringName = objv[1];
        
        CRunState* pState = CRunState::getInstance();
        if (pState->getState() == CRunState::Idle) {
            m_pRouter->setRing(ringName.c_str());
        } else {
            throw std::string("ring - run must be halted to issue this cvommand");
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