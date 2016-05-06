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
# @file   CEndRunInfo11.cpp
# @brief  Implementation of CEndRunInfo11
# @author <fox@nscl.msu.edu>
*/
#include "CEndRunInfo11.h"
#include <CFileDataSource.h>
#include <DataFormat.h>
#include <CRingStateChangeItem.h>
#include <stdexcept>


/**
 * constructor
 *    In addition to base class construction, we load the m_endRuns with all
 *    end runs found in the file.  
 *
 *    @param fd  - File descriptor open on the file.
 */
CEndRunInfo11::CEndRunInfo11(int fd) :
    CEndRunInfo(fd)
{
    loadEndRuns();        
}

/**
 * Destructor - by using unique_ptr's the destruction of the vector will
 * also destroy the ring items.
 */
CEndRunInfo11::~CEndRunInfo11()
{
    
}


/**
 * numEnds
 *    Return the number of end run items the file has.
 * @return unsigned
 */
unsigned
CEndRunInfo11::numEnds() const
{
    return m_endRuns.size();
}
/**
 * hasBodyHeader
 *    @param which - Which end run to ask about.
 *    @return bool - true if that end run has a body header false otherwise.
 *    @throw std::range_error if which is out of range.
*/
bool
CEndRunInfo11::hasBodyHeader(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->hasBodyHeader();
}
/**
 * getEventTimestamp
 *    @param which - which end run to ask about.
 *    @return uint64_t - the timestamp for the end run.
 *    @throw std::range_error - no such end of run.
 *    @throw std::string- The item has no body header.
 */
uint64_t
CEndRunInfo11::getEventTimestamp(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getEventTimestamp();
}
/*
 * getSourceId
 *    @param which - which end run to ask about.
 *    @return uint32_t - Id of source that emitted this end run.
 *    @throw std::range_error - no such end of run.
 *    @throw std::string  - no body header in the specified item.
 */
uint32_t
CEndRunInfo11::getSourceId(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getSourceId();
}
/**
 * getBarrierType
 *   @param which - selects the end run record to get info from.
 *   @return uint32_t - barrier type id (note 0 means not a barrier).
 *   @throw std::range_error - no such end of run.
 *   @throw std::string - no body header in the specified item.
 */
uint32_t
CEndRunInfo11::getBarrierType(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getBarrierType();
}
/**
 *  getRunNumber
 *     @param which - selects the end run record to get info from.
 *     @return uint32_t - run number in the selected record.
 *     @throw std::range_error - if the selected end run record does not exist.
 */
uint32_t
CEndRunInfo11::getRunNumber(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getRunNumber();
}
/**
 * getElapsedTime
 *    Return the total number of seconds the run lasted according to an end run
 *    record.  This may have a finer resolution than seconds if the readout
 *    uses a timing divisor.
 *
 *    @param which   - which end run record to interrogate.
 *    @return float
 *     @throw std::range_error - if the selected end run record does not exist.
 */
float
CEndRunInfo11::getElapsedTime(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->computeElapsedTime();
}
/**
 * getTitle
 *   @param which  - Which end run item to select
 *   @return std::string - the title.
 *   @throw std::range_error - if the selected end run record does not exist.
 */
std::string
CEndRunInfo11::getTitle(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getTitle();
}
/**
 * getTod
 *    Return the time of day (and date) at which an end run record was generated.
 *    @param which - which end run record to select.
 *    @return time_t - unix timestamp of the end of run time.
 *    @throw std::range_error - if the selected end run record does not exist.
 */
time_t
CEndRunInfo11::getTod(int which) const
{
    throwIfNoSuch(which);
    return m_endRuns[which]->getTimestamp();
}
/*-----------------------------------------------------------------------------
 * Utilities
 */


/**
 * loadEndRuns
 *
 *   Private member function that runs over the ring items in the file and
 *   saves pointers to all ring items in m_endRuns
 */
void
CEndRunInfo11::loadEndRuns()
{
    std::vector<uint16_t> filter;
    CFileDataSource src(m_nFd, filter);
    
    CRingItem* pItem;
    while ((pItem = src.getItem())) {
        if (pItem->type() == END_RUN) {
            m_endRuns.push_back(
                std::unique_ptr<CRingStateChangeItem>(
                    new CRingStateChangeItem(*pItem)
                )
            );
        }
        delete pItem;
    }
    
}
/**
 * throwIfNoSuch
 *    throws an std::range_error if the selected end run record does not
 *    exist.
 *
 *    @param which - end run record being looked for.
 */
void
CEndRunInfo11::throwIfNoSuch(int which) const
{
    unsigned w = which;
    if (w >= m_endRuns.size()) {
        throw std::range_error("Selected end run record does not exist.");
    }
}