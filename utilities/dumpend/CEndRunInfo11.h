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
# @file   CEndRunInfo11.h
# @brief  Class that provides end run information for version 11.x
# @author <fox@nscl.msu.edu>
*/

#ifndef CENDRUNINFO11_H
#define CENDRUNINFO11_H
#include "CEndRunInfo.h"
#include <vector>
#include <memory>

class CRingStateChangeItem;


/**
 * @class CEndRunInfo11
 *     Scan the end run items from a data source and provide information about
 *     them.  Note that in 11.0 several end run items might appear in a run file
 *     (think event builder).
 */
class CEndRunInfo11 : public CEndRunInfo
{
private:
    std::vector<std::unique_ptr<CRingStateChangeItem> > m_endRuns;
    
public:
    CEndRunInfo11(int fd);
    virtual ~CEndRunInfo11();
    
public:
    virtual unsigned numEnds()                        const ;
    
    // TODO: which should be unsigned.
    
    virtual bool     hasBodyHeader(int which = 0)     const ;
    virtual uint64_t getEventTimestamp(int which = 0) const ;
    virtual uint32_t getSourceId(int which = 0)       const ;
    virtual uint32_t getBarrierType(int which = 0)    const ;
    
    virtual uint32_t getRunNumber(int which = 0)      const ;
    virtual float    getElapsedTime(int which=0)      const ;
    virtual std::string getTitle(int which=0)         const ;
    virtual time_t   getTod(int which = 0)            const ;

    // utilities:
    
private:
    void loadEndRuns();
    void throwIfNoSuch(int which) const ;
};

#endif
