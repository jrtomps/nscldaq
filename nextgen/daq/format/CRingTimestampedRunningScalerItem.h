/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/** 
 * @file CRingTimestampedRunningScalerItem.h
 * @brief ring item class definition for time stamped non-incremental scaler data
 * @author Ron Fox (fox@nscl.msu.edu)
 */

#ifndef __CRINGTIMESTAMPEDRUNNINGSCALERITEM_H
#define __CRINGTIMESTAMPEDRUNNINGSCALERITEM_H

#ifndef __CRINGITEM_H
#include "CRingItem.h"
#endif

#ifndef __DATAFORMAT_H
#include "DataFormat.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CPPRTL_TYPEINFO
#include <typeinfo>
#ifndef __CPPRTL_TYPEINFO
#define __CPPRTL_TYPEINFO
#endif
#endif



/**
 * @class CRingItemTimestampedRunningScalerItem
 *
 *  Encapsulates scaler data that has a timestamp synchronized to the
 * event data and is not incremental.
 */
class CRingTimestampedRunningScalerItem : public CRingItem
{
private:
  pNonIncrTimestampedScaler m_pItem;

  // Canonical operations:

  CRingTimestampedRunningScalerItem(uint64_t eventTimestamp,
				    uint32_t startTime,
				    uint32_t stopTime,
				    uint32_t intervalDivisor,
				    time_t   unixTimestamp,
				    std::vector <uint32_t> scalers);
  CRingTimestampedRunningScalerItem(const CRingItem& rhs) throw(std::bad_cast);
  CRingTimestampedRunningScalerItem(const CRingTimestampedRunningScalerItem& rhs);
  virtual ~CRingTimestampedRunningScalerItem();
  CRingTimestampedRunningScalerItem& operator=(const CRingTimestampedRunningScalerItem& rhs);
  int operator==(const CRingTimestampedRunningScalerItem& rhs) const;
  int operator!=(const CRingTimestampedRunningScalerItem& rhs) const;

  // Accessors:

  uint64_t getTimestamp() const;
  float    getStartTime() const;
  float    getStopTime() const;
  time_t   getCalendarTime() const;
  std::vector<uint32_t> getScalers() const;


  // Utility methods:

private:
  void init(uint32_t nScalers);
  size_t bodySize(uint32_t nScalers);
  
};


#endif
