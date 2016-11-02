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
# @file   CTCLLogMessage.h
# @brief  Tcl wrappers for the CStatusDefinitions::LogMessage class.
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLLOGMESSAGE_H
#define CTCLLOGMESSAGE_H
#include <TCLObjectProcessor.h>
#include "CStatusMessage.h"
#include <map>
#include <zmq.hpp>


class CTCLInterpreter;
class CTCLObject;


/**
 * @class CTCLLogMessage
 *     Provides a Tcl wrapping of the CStatusDefinitions::LogMessage class.
 *     This command provides a constructor and destructor for instances
 *     of wrappers of instances of CStatusDefinitions::LogMessage.
 *     The wrapper itself is implemnted as an inner class in
 *     CTCLLogMessage::TCLLogMessage.
 */
class CTCLLogMessage : public CTCLObjectProcessor
{
    // Type/forward definitions
    
private:
        class TCLLogMessage;
        typedef std::map<std::string, TCLLogMessage*>  Registry;
        
    // Class/instance data:
    
private:
        static unsigned m_instanceNumber;
        Registry        m_registry;
        bool            m_testing;
    
    // Canonicals:
public:   
    CTCLLogMessage(CTCLInterpreter& interp, const char* command= "LogMessage");
    virtual ~CTCLLogMessage();
    
    // operations:
public:    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void enableTesting()  { m_testing = true;}
    void disableTesting() { m_testing = false;}
    
    // Sub command execution methods:
    
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void test(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    /**
     * Inner class that wraps an actual object.
     */
private:
    class TCLLogMessage : public CTCLObjectProcessor
    {
    private:
        CStatusDefinitions::LogMessage* m_pObject;
        zmq::socket_t*                  m_pSocket;
    public:
        TCLLogMessage(
            CTCLInterpreter& interp, const char* command,
            CStatusDefinitions::LogMessage* pObject, zmq::socket_t* pSocket
        );
        virtual ~TCLLogMessage();
        
    public:
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void log(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
};
#endif
