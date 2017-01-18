#ifndef DAQ_V12_CRINGPHYSICSEVENTCOUNTERITEM_H
#define DAQ_V12_CRINGPHYSICSEVENTCOUNTERITEM_H


/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "V12/CRingItem.h"
#include "V12/DataFormat.h"

#include <time.h>
#include <typeinfo>
#include <string>
#include <ctime>

namespace DAQ {
  namespace V12 {

/*!
   The physics event count item provides periodic informatino about how
   many events have been produced by the DAQ system.  This is provided
   so that applications that sample event data can determine what fraction
   of the data they've managed to keep up with.
   Applications can count the physics events they've seen,
   They can then divide this number by the number of events that
   have actually been generated to get an analysis fraction.

*/
class CRingPhysicsEventCountItem : public CRingItem
{

private:
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    uint32_t m_timeOffset;
    uint32_t m_offsetDivisor;
    time_t m_timestamp;
    uint64_t m_eventCount;

  // constructors and other canonicals:
public:
  CRingPhysicsEventCountItem();
  CRingPhysicsEventCountItem(uint64_t count,
                             uint32_t timeOffset);
  CRingPhysicsEventCountItem(uint64_t count,
                 uint32_t timeoffset,
                 time_t stamp);
  CRingPhysicsEventCountItem(
    uint64_t timestamp, uint32_t source,
    uint64_t count, uint32_t timeoffset, time_t stamp,
    int divisor=1);
  
  CRingPhysicsEventCountItem(const CRawRingItem& rhs);
  CRingPhysicsEventCountItem(const CRingPhysicsEventCountItem& rhs) = default;

  virtual ~CRingPhysicsEventCountItem();

//  CRingPhysicsEventCountItem& operator=(const CRingPhysicsEventCountItem& rhs);
  int operator==(const CRingPhysicsEventCountItem& rhs) const;
  int operator!=(const CRingPhysicsEventCountItem& rhs) const;


  // public interface:
public:
  uint32_t type() const;
  void setType(uint32_t type);

  uint32_t size() const;

  uint32_t getSourceId() const;
  void setSourceId(uint32_t sourceId);

  uint64_t getEventTimestamp() const;
  void setEventTimestamp(uint64_t tstamp);

  bool mustSwap() const;
  bool isComposite() const;

  void toRawRingItem(CRawRingItem& item) const;


  uint32_t getTimeOffset() const;
  void     setTimeOffset(uint32_t offset);
  float    computeElapsedTime() const;
  uint32_t getTimeDivisor() const;

  time_t   getTimestamp() const;
  void     setTimestamp(time_t stamp);

  uint64_t getEventCount() const;
  void     setEventCount(uint64_t count);

  // Virtual method overrides:


  virtual std::string typeName() const;	// Textual type of item.
  virtual std::string toString() const; // Provide string dump of the item.


private:
  void init();
};


  } // end of V11 namespace
} // end of DAQ
#endif
