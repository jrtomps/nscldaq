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
# @file   CTCLAggregate.h
# @brief  Define Tcl command to start an aggregator thread.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLAGGREGATE_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLOjbect;
class CMultiAggregator;

/**
 * @class CTCLAggregate
 *    This class starts the multi-node aggregator as a thread within the
 *    Tcl script.  The aggregator is a thread that discovers nodes in the
 *    dataflow and aggregates status messages from all of them allowing a
 *    single point of subscription for an application.
 *    Typical patterns are therefore to start the aggregator and then to
 *    connect to it making status subscriptions that will then be global to
 *    all the status messages from all nodes that participate in the data flow.
 *    Usage:
 * \verbatim
 *     set uri [statusaggregator]
 *  \endverbatim
 *
 *  Where uri is the URI the application can now connect/subscribe to.
 */
class CTCLAggregate : public CTCLObjectProcessor
{
public:
    CTCLAggregate(CTCLInterpreter& interp, const char* command = "statusaggregator");
    virtual ~CTCLAggregate();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
static:
    void aggregatorThread(CMultiAggregator& a);
};