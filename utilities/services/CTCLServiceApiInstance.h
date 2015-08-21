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
# @file   CTCLServiceApiInstance.h
# @brief  Service API instance command.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSERVICEAPIINSTANCE_H
#define CTCLSERVICEAPIINSTANCE_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;
class CServiceApi;

/**
* @class CTCLServiceApiInstance
*    Represents an instance of an service API object connected to a database
*    via a URI.
*/
class CTCLServiceApiInstance : public CTCLObjectProcessor
{
private:
    CServiceApi*   m_pApi;
public:
    CTCLServiceApiInstance(
        CTCLInterpreter& interp, const char* command, std::string uri
    );
    virtual ~CTCLServiceApiInstance();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
protected:
    void createProg(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setHost(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void remove(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listAll(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif