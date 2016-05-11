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
# @file   CRun.cpp
# @brief  Implement class that has scaler sums for a run.
# @author <fox@nscl.msu.edu>
*/
#include "CRun.h"
#include "CIncrementalChannel.h"
#include "CCumulativeChannel.h"

/**
 * constructor
 *    Initialize a new object.
 *  @param runNumber - the run number for which we're accumulating sums.
 */
CRun::CRun(unsigned runNumber) :
    m_runNumber(runNumber)
{}

/**
 * destructor
 */
CRun::~CRun() {
    
    // Need to delete the channel objects:
    
    for (auto pSrc = m_scalerSums.begin(); pSrc != m_scalerSums.end(); pSrc++) {
        std::map<unsigned, CChannel*>& inner(pSrc->second);
        for(auto pChan = inner.begin(); pChan != inner.end(); pChan++) {
            delete pChan->second;
        }
    }
    // The map itself gets cleaned up properly.
    
}


/**
 * update
 *    Update a single channel for this run.
 *
 *  @param src  - id of the source the channel comes from.
 *  @param chan - Channel number for the data.
 *  @param value - Scaler raw value.
 *  @param incremental - True if the scaler is incremental false if cumulative.
 *  @param width - Number of bits of scaler width.
 *
 * @note - if the channel has not come into existence first it is created
 *         and addeed to the m_scalerSums map.
 */
void
CRun::update(
    unsigned src, unsigned chan, unsigned value, bool incremental,
    unsigned width
)
{
    CChannel* pChan = getChan(src,chan, incremental);
    pChan->update(value, width);
}

/**
 * sources
 *  @return std::vector<unsigned> returns the ids of the sources in this run.
 *          this can be sparse.
 */
std::vector<unsigned>
CRun::sources()
{
    std::vector<unsigned> result;
    
    for(auto p = m_scalerSums.begin(); p != m_scalerSums.end(); p++) {
        result.push_back(p->first);
    }
    return result;
}
/**
 * sums
 *    Returns the susm from a specified data source.
 *    While theoretically, given how processing works, this could be
 *    a sparse vector, in fact, given the format of a scaler ring item
 *    it will not be.  Furthermore we can rely on the facts that:
 *    
 *    - Traversal of a map via interators is sorted by key.
 *    - Referencing a nonexistent data source will give an empty map and hence
 *      an empty vector.
 *
 * @param src  - source id we want the sums for.
 * @return std::vector<uint64_t> - vector (possibily empty) of scaler sums.
 */
std::vector<uint64_t>
CRun::sums(unsigned srcid)
{
    std::vector<uint64_t> result;
    
    std::map<unsigned, CChannel*>& srcData(m_scalerSums[srcid]);
    for(auto p = srcData.begin(); p != srcData.end(); p++) {
        
        // This inner loop is wher we rely on iterator ordering and dense packing.
        
        result.push_back(*(p->second));
    }
    return result;
}
/**
 * getRun
 *   @return the run number.
 */
unsigned
CRun::getRun() const
{
    return m_runNumber;
}

/*-----------------------------------------------------------------------------
 *
 *  private methods:
 */

/**
 * getChan
 *   Get the channel for the specified src/channel no.  If necessary the
 *   correct type of channel is created and inserted in the map.
 *
 * @param src  - Source id.
 * @param chan - Channel number.
 * @param incremental - boolean, true if this is an incremental channel.
 */
CChannel*
CRun::getChan(unsigned src, unsigned chan, bool incremental)
{
    CChannel* pResult = m_scalerSums[src][chan];
    
    // If there isn't a channel in that slot make one.
    
    if(!pResult) {
        if (incremental) {
            pResult = new CIncrementalChannel;
        } else {
            pResult = new CCumulativeChannel;
        }
        m_scalerSums[src][chan] = pResult;
    }
    return pResult;
}


