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
# @file   CTCLStateChangeMessage.h
# @brief  TCL Encapsulation of CStatusDefinitions::StateChange
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSTATECHANGEMESSAGE_H
#define CTCLSTATECHANGEMESSAGE_H

#include <TCLObjectProcessor.h>
#include "CStatusMessage.h"
#include <zmq.hpp>
#include <map>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CTCLStateChangeMessage
 *     This outer class provides a Tcl command ensemble for wrapping
 *     instances of the CStatusDefinitions::StateChange class in a Tcl command.
 *     The inner class: TclStateChangeMessage wraps the actual instances and
 *     is what is created/destroyed by this outer class.
 */
class CTCLStateChangeMessage : public CTCLObjectProcessor
{
private:
    class TCLStateChangeMessage;
    typedef std::map<std::string, TCLStateChangeMessage*> Registry;
private:
    static unsigned m_instanceNumber;
    Registry        m_registry;
    bool            m_testing;

public:
    CTCLStateChangeMessage(CTCLInterpreter& interp, const char* cmd="StateChange");
    virtual ~CTCLStateChangeMessage();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void enableTesting()  { m_testing = true; }
    void disableTesting() { m_testing = true; } 

private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
private:
    // Nested class that does the actual wrapping.
    
    class TCLStateChangeMessage : public CTCLObjectProcessor
    {
    private:
        CStatusDefinitions::StateChange*   m_pObject;
        zmq::socket_t*                     m_pSocket;
    public:
        TCLStateChangeMessage(
            CTCLInterpreter& interp, const char* cmd,
            CStatusDefinitions::StateChange* pObject, zmq::socket_t* pSocket
        );
        virtual ~TCLStateChangeMessage();
        
    public:
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void logChange(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
};
#endif