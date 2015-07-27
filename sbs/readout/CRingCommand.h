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
# @file   CRingCommand.h
# @brief  Header for the ring command executor.
# @author <fox@nscl.msu.edu>
*/
#ifndef CRINGCOMMAND_H
#define CRINGCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLOjbect;
class CExperiment;

/**
 *  CRingCommand
 *     Implements the tcl command that extablishes a new ring.
 */
class CRingCommand : public CTCLObjectProcessor
{
private:
    CExperiment* m_pExperiment;
    
public:
    CRingCommand(CTCLInterpreter& interp, const char* name, CExperiment* pExperiment);
    virtual ~CRingCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif