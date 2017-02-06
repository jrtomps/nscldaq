#ifndef DAQ_V12_CRINGSTATECHANGEITEM_H
#define DAQ_V12_CRINGSTATECHANGEITEM_H
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


#include <V12/CRingItem.h>
#include <V12/DataFormat.h>

#include <string>
#include <typeinfo>
#include <RangeError.h>

namespace DAQ {
  namespace V12 {

/*!
  This class represents a state change item.
  State change items are items that indicate a change in the state of
  data taking.  The set of state changes recognized by this class are:

  - BEGIN_RUN  - the run has become active after being stopped.
  - END_RUN    - The run has become inactive after being ended.
  - PAUSE_RUN  - The run has become inactive after being temporarily paused.
  - RESUME_RUN - The run has become active after being resumed while paused.

  This object is suitable for use by both producers (who must create state change
  items), and consumers (who must inspect the contents of such items after getting
  them from a ring).

  A CRingStateChangeItem is always a leaf type and stores its data in native
  byte order.

*/
class CRingStateChangeItem : public CRingItem
{
private:
    uint32_t    m_type;
    uint64_t    m_evtTimestamp;
    uint32_t    m_sourceId;
    uint32_t    m_runNumber;
    uint32_t    m_timeOffset;
    time_t      m_timestamp;
    uint32_t    m_offsetDivisor;
    std::string m_title;

  // construction and other canonicals
public:
  CRingStateChangeItem(uint16_t reason = BEGIN_RUN);
  CRingStateChangeItem(uint16_t reason,
                       uint32_t runNumber,
                       uint32_t timeOffset,
                       time_t   timestamp,
                       std::string title);

  CRingStateChangeItem(uint64_t eventTimestamp, uint32_t sourceId,
                       uint16_t reason,
                       uint32_t runNumber,
                       uint32_t timeOffset,
                       time_t   timestamp,
                       std::string title,
                       uint32_t offsetDivisor = 1);

  CRingStateChangeItem(const CRawRingItem& item);
  CRingStateChangeItem(const CRingStateChangeItem& rhs) = default;
  virtual ~CRingStateChangeItem();

  CRingStateChangeItem& operator=(const CRingStateChangeItem& rhs) = default;

  virtual uint32_t type() const;
  virtual void setType(uint32_t type);

  virtual uint32_t size() const;

  virtual uint64_t getEventTimestamp() const;
  virtual void setEventTimestamp(uint64_t tstamp);

  virtual uint32_t getSourceId() const;
  virtual void setSourceId(uint32_t id);

  virtual bool isComposite() const;

  virtual bool mustSwap() const;

  virtual void toRawRingItem(CRawRingItem& item) const;

  virtual bool operator==(const CRingItem& rhs) const;
  virtual bool operator!=(const CRingItem& rhs) const;

  // Accessors for elements of the item (selectors and mutators both).

  void setRunNumber(uint32_t run);
  uint32_t getRunNumber() const;

  void     setElapsedTime(uint32_t offset);
  uint32_t getElapsedTime() const;
  float    computeElapsedTime() const;

  void setTitle(std::string title);
  std::string getTitle() const;

  void setTimestamp(time_t stamp);
  time_t getTimestamp() const;

  void setOffsetDivisor(uint32_t divisor);
  uint32_t getOffsetDivisor() const;

  virtual std::string typeName() const;
  virtual std::string toString() const;

  // Utility functions..
private:
  bool isStateChange(uint32_t type) const;
};

  } // end of V11 namespace
} // end DAQ
#endif

