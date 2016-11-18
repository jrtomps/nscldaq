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
# @file   CZmqContext.cpp
# @brief  Provides an application singleton for a ZMQ context.
# @author <fox@nscl.msu.edu>
*/
#include <CStatusMessage.h>


static const unsigned ZMQ_THREADS(5);                 // Some arbitrary number.

zmq::context_t*  CStatusDefinitions::ZmqContext::m_context(0);

/**
 * getInstance
 *    Return a reference to an instance of the singleton context.
 *    If necessary the context is new-d into being.
 */
zmq::context_t&
CStatusDefinitions::ZmqContext::getInstance()
{
    if (!m_context) {
        m_context = new zmq::context_t(ZMQ_THREADS);
    }
    return *m_context;
}
/**
 * reset
 *    used in test applications to destroy the context between tests.
 */
void
CStatusDefinitions::ZmqContext::reset()
{
    delete m_context;            // No op if there's no context at this time.
    m_context = 0;
}