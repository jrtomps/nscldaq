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
# @file   CTCLAggregate.cpp
# @brief  Implement Tcl binding to start a multi-node status aggregation thread.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLAggregate.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <Exception.h>
#include <thread>
#include <CMultiAggregator.h>

/**
 *  constrution
 *     @param interp   - the interpreter the command will be registered on.
 *     @param command  - The command that will be registered for this object.
 */
CTCLAggregate::CTCLAggregate(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true),
    m_running(false)
{}

/**
 * destructor
 *    Note the destruction of this command does not stop any aggregation
 *    thread.
 */
CTCLAggregate::~CTCLAggregate() {}

/**
 * operator()
 *    Start the thread.  It's an error to start it twice (the second time
 *    will fail anyway since the address will already be in use)
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - Command words (should be no new ones).
 *  @return int  - TCL_OK - success, TCL_ERROR - failure.
 */
int
CTCLAggregate::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "No extra command parameters are allowded");
        if(m_running) {
            throw std::logic_error("An aggregator thread is already running");
        }
        CMultiAggregator* pAggregator = new CMultiAggregator("StatusPublisher", 10);
        std::string uri = pAggregator->getPublisherURI();
        new std::thread(CTCLAggregate::aggregatorThread, std::ref(*pAggregator));
        interp.setResult(uri);
        m_running = true;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * aggregatorThread
 *    Statis method that is the entry point of the thread that runs the multi
 *    aggregator.  A very simple method:
 *
 * @param a - references the aggregator object.
 */
void
CTCLAggregate::aggregatorThread(CMultiAggregator& a) {
    a();
}
