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


class CChannel;

/**
 * @class CRun
 *    Encapsulates all of the information for a run.  This includes;
 *    -   Run Number
 *    -   Scaler sums for that run (across all data sources).
 */
class CRun {
private:
    unsigned                                  m_runNumber;
    
    // A bit of explanation about this map.
    // the outer map is keyed by source id.
    // The inner map is keyed by channel number within the source id.
    // the CChannel actual type depends on the type of scaler.
    
    std::map<unsigned, std::map<unsigned, CChannel*> > m_scalerSums;  
    
public:
    CRun(unsigned runNumber);
    virtual ~CRun();
    
public:
    
    void update(
        unsigned src, unsigned channel, unsigned value, bool incremental,
        unsigned width = 32);
    
    std::vector<unsigned> sources();
    std::vector<uint64_t> sums(unsigned srcId);
    unsigned              getRun() const;

private:
    CChannel* getChan(unsigned src, unsigned ch, bool incremental);
};

#endif