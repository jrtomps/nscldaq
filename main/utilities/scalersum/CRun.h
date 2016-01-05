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
# @file   CRun.h
# @brief  Class to encapsulate the scalers from a run.
# @author <fox@nscl.msu.edu>
*/

#ifndef CRUN_H
#define CRUN_H

#include <vector>
#include <map>
#include <cstdint>

/**
 * @class CRun
 *    Encapsulates all of the information for a run.  This includes;
 *    -   Run Number
 *    -   Scaler sums for that run (across all data sources).
 */
class CRun {
private:
    unsigned                                  m_runNumber;
    std::map<unsigned, std::vector<uint64_t>> m_scalerSums;  // key is sourceid.
    
public:
    CRun(unsigned runNumber);
    virtual ~CRun();
    
public:
    
    void                  add(unsigned srcId, std::vector<uint32_t> scalers);
    std::vector<unsigned> sources();
    std::vector<uint64_t> sums(unsigned srcId);
    unsigned              getRun() const;
};

#endif