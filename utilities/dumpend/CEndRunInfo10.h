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
# @file   CEndRunInfo10.h
# @brief  Define a class that can extract end runs from an nscldaq10.x file.
# @author <fox@nscl.msu.edu>
*/
#ifndef CENDRUNINFO10_H
#define CENDRUNINFO10_H

#include "CEndRunInfo.h"
#include <vector>
#include <DataFormat10.h>
#include <memory>
/*
 * @class CEndRunInfo10
 *    Pull information out of end run records from nscldaq 10.x note that this
 *    version of nscldaq does not have body headers so
 *    -   hasBodyHeader() hardwires to false and
 *    -   the functions that pull stuff out of body headers hardwire throws of
 *        std::strings.
 */
class CEndRunInfo10 : public CEndRunInfo
{
  std::vector<std::unique_ptr<NSCLDAQ10::StateChangeItem> > m_endRuns;
public:
    CEndRunInfo10(int fd);
    virtual ~CEndRunInfo10();
    
    // Overrides:
    
public:
    virtual unsigned numEnds()                        const ;
    
    virtual bool     hasBodyHeader(int which = 0)     const ;
    virtual uint64_t getEventTimestamp(int which = 0) const ;
    virtual uint32_t getSourceId(int which = 0)       const ;
    virtual uint32_t getBarrierType(int which = 0)    const ;
    
    virtual uint32_t getRunNumber(int which = 0)      const ;
    virtual float    getElapsedTime(int which=0)      const ;
    virtual std::string getTitle(int which=0)         const ;
    virtual time_t   getTod(int which = 0)            const ;    
    
    // Utilities:
    
private:
    void loadEndRuns();
    void throwIfBadIndex(int which) const;
};
#endif
