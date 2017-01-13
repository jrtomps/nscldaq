#ifndef NSCLDAQ12_CRINGSCALERITEM_H
#define NSCLDAQ12_CRINGSCALERITEM_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#include "V12/CRingItem.h"
#include "V12/DataFormat.h"

#include <vector>
#include <typeinfo>
#include <RangeError.h>
#include <string>

namespace DAQ {
  namespace V12 {

  class CRawRingItem;

/*!
   This class derived from CRingItem and represents a set of scalers that have been 
   formatted as a ring item.  
*/
class CRingScalerItem : public CRingItem
{
public:
  uint32_t        m_sourceId;
  uint64_t        m_evtTimestamp;
  uint32_t        m_intervalStartOffset;
  uint32_t        m_intervalEndOffset;
  uint32_t        m_timestamp;
  uint32_t        m_intervalDivisor;  /* 11.0 sub second time intervals */
  bool            m_isIncremental;    /* 11.0 non-incremental scaler flag */
  uint32_t        m_scalerWidth;
  std::vector<uint32_t>       m_scalers;

  // Constructors and canonicals.:

public:
  CRingScalerItem(size_t numScalers);
  CRingScalerItem(uint32_t startTime,
                  uint32_t stopTime,
                  time_t   timestamp,
                  std::vector<uint32_t> scalers,
                  bool                  isIncremental = true,
                  uint32_t              timeOffsetDivisor = 1,
                  uint32_t width=32);
  CRingScalerItem(uint64_t eventTimestamp, uint32_t source,
                  uint32_t startTime,
                  uint32_t stopTime,
                  time_t   timestamp,
                  std::vector<uint32_t> scalers,
                  uint32_t timeDivisor = 1, bool incremental=true,
                  uint32_t scalerWidth=32);

  CRingScalerItem(const CRawRingItem& rhs);
  CRingScalerItem(const CRingScalerItem& rhs);
  
  virtual ~CRingScalerItem();

//  CRingScalerItem& operator=(const CRingScalerItem& rhs);
  int operator==(const CRingScalerItem& rhs) const;
  int operator!=(const CRingScalerItem& rhs) const;

  // Accessor member functions.
  uint32_t  type() const { return PERIODIC_SCALERS; }
  void      setType(uint32_t type);
  uint32_t  size() const;
  uint64_t  getEventTimestamp() const;
  void      setEventTimestamp(uint64_t tstamp);
  uint32_t  getSourceId() const;
  void      setSourceId(uint32_t sid);
  bool      isComposite() const;
  bool      mustSwap() const;
  void      toRawRingItem(CRawRingItem& item) const;

  void     setStartTime(uint32_t startTime);
  uint32_t getStartTime() const;
  float    computeStartTime() const;
  void     setEndTime(uint32_t endTime);
  uint32_t getEndTime() const;
  float    computeEndTime() const;
  void     setTimeDivisor(uint32_t value);
  uint32_t getTimeDivisor() const;
  void     setTimestamp(time_t stamp);
  time_t   getTimestamp() const;
  
  void setIncremental(bool enable);
  bool isIncremental() const;

  void     setScaler(uint32_t channel, uint32_t value);
  uint32_t getScaler(uint32_t channel) const;
  std::vector<uint32_t> getScalers() const;
  void setScalers(const std::vector<uint32_t>& values);

  uint32_t getScalerCount() const;

  uint32_t getScalerWidth() const;
  void setScalerWidth(uint32_t width);

  // Virtual methods overriddent:

  virtual std::string typeName() const;
  virtual std::string toString() const;

  // utility.

private:
  size_t bodySize() const;
  void throwIfInvalidChannel(uint32_t channel,
                 const char* message) const;
  std::string headerToString() const;
};
   
} // end of V12 namespace
} // end DAQ
#endif
