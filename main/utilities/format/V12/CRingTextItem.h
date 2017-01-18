#ifndef DAQ_V12_CRINGTEXTITEM_H
#define DAQ_V12_CRINGTEXTITEM_H

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

#include <RangeError.h>
#include <stdint.h>
#include <time.h>
#include <string>
#include <vector>
#include <typeinfo>

namespace DAQ {
  namespace V12 {

  class CRawRingItem;

/*!
  The text ring item provides a mechanism to put an item in/take an item out of 
  a ring buffer that consists of null terminated text strings.  
*/
class CRingTextItem : public CRingItem
{
  // Private data:
private:
    uint32_t m_type;
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    uint32_t m_timeOffset;
    time_t   m_timestamp;
    uint32_t m_offsetDivisor;
    std::vector<std::string> m_strings;

public:
  // Constructors and other canonicals:

  CRingTextItem(uint16_t type,
		std::vector<std::string> theStrings);
  CRingTextItem(uint16_t type,
        std::vector<std::string> theStrings,
        uint32_t                 offsetTime,
        time_t                   timestamp) ;
  CRingTextItem(
    uint16_t type, uint64_t eventTimestamp, uint32_t source,
    std::vector<std::string> theStrings, uint32_t offsetTime, time_t timestamp,
    int offsetDivisor = 1
  );
  CRingTextItem(const CRawRingItem& rhs);
  CRingTextItem(const CRingTextItem& rhs) = default;
  CRingTextItem& operator=(const CRingTextItem& rhs) = default;

  virtual ~CRingTextItem();


  int operator==(const CRingTextItem& rhs) const;
  int operator!=(const CRingTextItem& rhs) const;

  // Public interface:
public:
  uint32_t type() const;
  void setType(uint32_t type);

  uint32_t size() const;

  uint64_t getEventTimestamp() const;
  void setEventTimestamp(uint64_t stamp);

  uint32_t getSourceId() const;
  void setSourceId(uint32_t id);

  bool isComposite() const;
  bool mustSwap() const;
  void toRawRingItem(CRawRingItem& buffer) const;


  std::vector<std::string>  getStrings() const;
  std::vector<std::string>& getStrings();
  uint32_t getStringCount() const;

  void     setTimeOffset(uint32_t offset);
  uint32_t getTimeOffset() const;
  float    computeElapsedTime() const;
  uint32_t getTimeDivisor() const;
  void     setTimeDivisor(uint32_t divisor);
  void     setTimestamp(time_t stamp);
  time_t   getTimestamp() const;

  // Virtual methods all ring overrides.

  virtual std::string typeName() const;
  virtual std::string toString() const;

  //private utilities:
private:
  bool   validType(const CRingItem& item) const;
};

} // end of V11 namespace
} // end DAQ

#endif
