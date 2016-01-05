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
CRun::~CRun() {}


/**
 * add
 *    Add scalers to the sums for this run.
 *    -   If necessary a new source id is created.
 *    -   If necessary the vector is expanded zero filled to match the
 *        size of the input vector
 *    -   Values in the input vector are added to corresponding value in the
 *        vector for the source id.
 *  @param srcId  - Id of the source for which this scaler has been supplied.
 *  @param scalers - The scalers for this period.
 */
void
CRun::add(unsigned srcId, std::vector<uint32_t> scalers)
{
    std::vector<uint64_t>& sums(m_scalerSums[srcId]);  // Creates empty vector if needed.
    
    // If needed expand the sum vector:
    
    while (sums.size() < scalers.size()) {
        sums.push_back(0);                      // Expand zero filled.
    }
    
    // This loop works because we've ensured the sums vector is at least as
    // long as the scalers vector:
    
    auto o = sums.begin();
    for (auto i = scalers.begin(); i != scalers.end(); i++, o++) {
        *o += *i;
    }
}
/**
 * sources
 *    Returns a vector containing all the data source ids seen so far for this
 *    run.
 *
 * @return std::vector<unsigned>
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
 *    Returns the sums for a source id.
 *
 *  @param srcId - the source we want the sums for.
 *  @return std::vector<uint64_t>
 *  @note - if there are no sums for this srcid an empty vector is returned:
 */
std::vector<uint64_t>
CRun::sums(unsigned srcId)
{
    return m_scalerSums[srcId];
}

/**
 * getRun
 *    Return the run number
 *
 *  @return unsigned
 */
unsigned
CRun::getRun() const
{
    return m_runNumber;
}