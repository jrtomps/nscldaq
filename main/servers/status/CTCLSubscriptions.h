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
# @file   CTCLSubscription.h
# @brief  Defines classes for Tcl bindings to CStatusSubscription
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLSUBSCRIPTION_H
#define CTCLSUBSCRIPTION_H

#include <TCLObjectProcessor.h>
#include <map>
#include <zmq.hpp>

class CTCLInterpreter;
class CTCLObject;
class CStatusSubscription;

/**
 *  @class CTCLSubscription
 *    Provides Tcl bindings to the C++ CStatusSubscription class.  See
 *    the internal NSCL link:
 *
 *    https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Subscription_API#TclBindings
 *
 * For a description of how this all works.
 */
class CTCLSubscription : public CTCLObjectProcessor
{
private:
    class SubscriptionInstance;
    typedef std::map<std::string, SubscriptionInstance*> Registry;
private:
    Registry        m_registry;
    static unsigned m_sequence;
    
public:
    CTCLSubscription(
        CTCLInterpreter& interp, const char* cmd = "statusSubscription"
    );
    virtual ~CTCLSubscription();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Nested instance class:
private:
    class SubscriptionInstance : public CTCLObjectProcessor
    {
    public:
        SubscriptionInstance(
            CTCLInterpreter& interp, const char* cmd, zmq::socket_t& sock,
            CStatusSubscription& subs
        );
        virtual ~SubscriptionInstance();
    public:
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    };
};

#endif
