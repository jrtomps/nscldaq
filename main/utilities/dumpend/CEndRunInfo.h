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
# @file   CEndRunInfo.h
# @brief  Obtain the end of run information from a run file.
# @author <fox@nscl.msu.edu>
*/
#ifndef CENDRUNINFO_H
#define CENDRUNINFO_H
#include <stdint.h>
#include <string>
#include <time.h>


/**
 * @class CEndRunInfo
 *   Abstract base class for pulling end run information out of a run file.
 *   See CEndRunInfoFactory for a class that generates the correct subclass
 *   given a filename.
 */
class CEndRunInfo
{
protected:
    int m_nFd;                              // File descriptor open on the file.
public:
    CEndRunInfo(int fd);
    virtual ~CEndRunInfo();
    
    // exported interfaces:
    
public:
    virtual unsigned numEnds()                        const = 0;
    
    virtual bool     hasBodyHeader(int which = 0)     const = 0;
    virtual uint64_t getEventTimestamp(int which = 0) const = 0;
    virtual uint32_t getSourceId(int which = 0)       const = 0;
    virtual uint32_t getBarrierType(int which = 0)    const = 0;
    
    virtual uint32_t getRunNumber(int which = 0)      const = 0;
    virtual float    getElapsedTime(int which=0)      const = 0;
    virtual std::string getTitle(int which=0)         const = 0;
    virtual time_t   getTod(int which = 0)            const = 0;    
};

#endif
