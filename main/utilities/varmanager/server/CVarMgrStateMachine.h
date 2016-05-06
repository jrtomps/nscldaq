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
# @file   CVarMgrStateMachine.h
# @brief  Defines class to implement varmgr statemachine command.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRSTATEMACHINE_H
#define CVARMGRSTATEMACHINE_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class  CVarMgrStateMachine
 *    This class implements the statemachine class.  The form of the command:
 *
 *  \verbatim
 *     statemachine $handle type-name transition-dict
 *  \endverbatim
 *
 *  The transition dict is a dict whose keys are states and whose values are
 *  lists of valid target states from that state.
 */
class CVarMgrStateMachine : public CTCLObjectProcessor
{
public:
    CVarMgrStateMachine(CTCLInterpreter& interp, const char* command);
    virtual ~CVarMgrStateMachine();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif