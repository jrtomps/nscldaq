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
# @file   CTCLEvbInstance.h
# @brief  Instance command for event builder manipulation on a specific
#         db.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLEVBINSTANCE_H
#define CTCLEVBINSTANCE_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;
class CVardbEventBuilder;

/**
 * @class CTCLEvbInstance
 *    Command for instances of connections that manipulate the event builder
 *    schema of the variable database.
 *    This is a command ensemble whose subcommands closely match the method
 *    names for the CVardbEventBuilder class.
 */
class CTCLEvbInstance : public CTCLObjectProcessor
{
private:
    CVardbEventBuilder* m_pApi;

public:
    CTCLEvbInstance(
        CTCLInterpreter& interp, const char* cmd, CVardbEventBuilder* pApi
    );
    virtual ~CTCLEvbInstance();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif
