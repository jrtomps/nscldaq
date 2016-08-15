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
# @file   COutputThread.h
# @brief  Implements COutputThread - see the header for description/rationale.
# @author <fox@nscl.msu.edu>
*/

#include "COutputThread.h"
#include <stdexcept>
#include <algorithm>

/**
 * Constructor
 *   For now this is null, just setting the thread name in the base class
 *   constructor.
 */
COutputThread::COutputThread() :
Thread(std::string("OutputThread"))
{
    
}

/**
 * destructor
 *   Null just giving the base class a chance to run:
 */
COutputThread::~COutputThread() {}



/*---------------------------------------------------------------------------
 * Methods called by other threads:
 */

/**
 *  addObserver
 *     Add a new observer to the end of the list
 *
 *  @param o - pointer to an objecte derived from CFragmentHandler::Observer.
 */
void
COutputThread::addObserver(CFragmentHandler::Observer* o)
{
    CriticalSection c(m_observerGuard);
    m_observers.push_back(o);
    
}
/**
 *  removeObserver
 *    Remove exsting observer from the list:
 *
 *  @param o - observer to remove.,
 *  @throw std::invalid_argument if o is not an observer.
 */
void
COutputThread::removeObserver(CFragmentHandler::Observer* o)
{
    CriticalSection c(m_observerGuard);
    std::list<CFragmentHandler::Observer*>::iterator p =
        std::find(m_observers.begin(), m_observers.end(), o);
    if (p == m_observers.end()) {
        throw std::invalid_argument("COutputThread::removeObserber - observer does not exist");
    }
    m_observers.erase(p);
}

/**
 * queueFragments
 *    Queue a vector of fragments to the thread.  The run() method will notice
 *    these eventually, invoke the observers on them and clean up.
 *
 *  @param pFrags - pointer to a vector of fragment pointers we queue for
 *                  processing.
 */
void
COutputThread::queueFragments(std::vector<EVB::pFragment>* pFrags)
{
    m_inputQueue.queue(pFrags);
}