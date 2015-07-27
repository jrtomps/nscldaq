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
# @brief  Header for the ring command (select output ring).
# @author <fox@nscl.msu.edu>
*/
#ifndef CRINGCOMMAND_H
#define CRINGCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;
class COutputThread;


/**
 * CRingCommand
 *    This class provides a 'ring' command that allows the output ring to be
 *    modified at run time.  This is necessary to support the PUB/USB state
 *    manager as that will get the ring buffer from the variable database rather
 *    then from the command line.
 */
class CRingCommand : public CTCLObjectProcessor
{
private:
    COutputThread*  m_pRouter;

public:
    CRingCommand(CTCLInterpreter& interp, const char* name, COutputThread* pRouter);
    virtual ~CRingCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif