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


#include <config.h>
#include "V12/CRingStateChangeItem.h"
#include <V12/CRawRingItem.h>
#include <V12/DataFormat.h>
#include <Deserializer.h>
#include <sstream>
#include <string.h>
#include <ctime>

using namespace std;

namespace DAQ {
  namespace V12 {

////////////////////////////////////////////////////////////////////////////
//
// Constructors and canonicals.
//

/*!
   Construct a bare bone state change buffer.  The run number, time offset
   are both set to zero.  The timestamp is set to 'now'.
   The title is set to an emtpy string.
   \param reason - Reason for the state change.  This defaults to BEGIN_RUN.
*/
CRingStateChangeItem::CRingStateChangeItem(uint16_t reason)
    :
    CRingStateChangeItem(V12::NULL_TIMESTAMP, 0, reason,
                         0, 0, time(NULL), "",1)
{
}
/*!
   Fully specified construction the initial values of the various
   fields are specified by the constructor parameters.

   \param reason     - Why the state change buffer is being emitted (the item type).
   \param runNumber  - Number of the run that is undegoing transitino.
   \param timeOffset - Number of seconds into the run at which this is being emitted.
   \param timestamp  - Absolute time to be recorded in the buffer.. tyically
                       this should be time(NULL).
   \param title      - Title string.  The length of this string must be at most
                       TITLE_MAXSIZE.

   \throw CRangeError - If the title string can't fit in s_title.
*/
CRingStateChangeItem::CRingStateChangeItem(uint16_t reason,
                       uint32_t runNumber,
                       uint32_t timeOffset,
                       time_t   timestamp,
                       std::string title)
    :
    CRingStateChangeItem(V12::NULL_TIMESTAMP, 0, reason,
                         runNumber, timeOffset, timestamp, title,1)
{}

/**
 * constructor - for timetamped item.
 *
 * @param eventTimestamp  - Event timestamp
 * @param sourceId   - Source id of the event.
   \param reason     - Why the state change buffer is being emitted (the item type).
   \param runNumber  - Number of the run that is undegoing transitino.
   \param timeOffset - Number of seconds into the run at which this is being emitted.
   \param timestamp  - Absolute time to be recorded in the buffer.. typically
                       this should be time(NULL).
   \param title      - Title string.
   @param offsetDivisor - What timeOffset needs to be divided by to get seconds.

 */
CRingStateChangeItem::CRingStateChangeItem(
        uint64_t eventTimestamp, uint32_t sourceId, uint16_t reason,
        uint32_t runNumber, uint32_t timeOffset, time_t   timestamp,
        std::string title, uint32_t offsetDivisor)
    : m_type(reason),
      m_evtTimestamp(eventTimestamp),
      m_sourceId(sourceId),
      m_runNumber(runNumber),
      m_timeOffset(timeOffset),
      m_timestamp(timestamp),
      m_offsetDivisor(offsetDivisor),
      m_title(title)
{}

/*!
   Constructor that copies/converts an existing ring item into a state change
   item. This is often used to wrap a state change object around a ring item just
   fetched from the ring, and determined by the caller to be a state change item.

   \param item - The source item.
   \throw bad_cast - the item is not a state change item.
*/
CRingStateChangeItem::CRingStateChangeItem(const CRawRingItem& rhs)
{

  if (rhs.type() != V12::BEGIN_RUN &&
          rhs.type() != V12::END_RUN &&
          rhs.type() != V12::PAUSE_RUN &&
          rhs.type() != V12::RESUME_RUN) {
    throw std::bad_cast();
  }

  m_type = rhs.type();
  m_sourceId     = rhs.getSourceId();
  m_evtTimestamp = rhs.getEventTimestamp();

  Buffer::Deserializer<Buffer::ByteBuffer> rhsBody(rhs.getBody(), rhs.mustSwap());

  uint32_t titleLength, temp;
  rhsBody >> m_runNumber;
  rhsBody >> m_timeOffset;
  rhsBody >> temp; m_timestamp = temp;
  rhsBody >> m_offsetDivisor;
  rhsBody >> titleLength;

  char * pTitle = new char[titleLength];
  rhsBody.extract(pTitle, pTitle + titleLength);
  m_title = std::string(pTitle, pTitle+titleLength);
  delete [] pTitle;
}

/*!
  Destruction just chains to the base class.

*/
CRingStateChangeItem::~CRingStateChangeItem()
{
}


/*!
  Comparison is just the base class compare as comparing the item pointers is a bad
  idea since there could be two very different objects that have equal items.
 
  \param rhs - Right hand side object of the == operator.
  \return int
  \retval 0 not equal
  \retval 1 equal.
*/
bool
CRingStateChangeItem::operator==(const CRingItem& rhs) const
{
  if (m_type != rhs.type()) return false;
  if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
  if (m_sourceId != rhs.getSourceId()) return false;

  const CRingStateChangeItem* pItem = dynamic_cast<const CRingStateChangeItem*>(&rhs);

  if (!pItem) return false;
  if (m_runNumber != pItem->getRunNumber()) return false;
  if (m_timeOffset != pItem->getElapsedTime()) return false;
  if (m_timestamp != pItem->getTimestamp()) return false;
  if (m_offsetDivisor != pItem->getOffsetDivisor()) return false;
  if (m_title != pItem->getTitle()) return false;

  return true;
}

/*!
   Inequality is just the logical inverse of equality.
*/
bool
CRingStateChangeItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}

/////////////////////////////////////////////////////////////////////////////
// Accessor methods.


uint32_t CRingStateChangeItem::type() const
{
    return m_type;
}

void CRingStateChangeItem::setType(uint32_t type)
{
    m_type = type;
}

uint32_t CRingStateChangeItem::size() const
{
    return 20 + 20 + m_title.size();
}

uint64_t CRingStateChangeItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CRingStateChangeItem::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}

uint32_t CRingStateChangeItem::getSourceId() const
{
    return m_sourceId;
}

void CRingStateChangeItem::setSourceId(uint32_t id)
{
    m_sourceId = id;
}


bool CRingStateChangeItem::isComposite() const
{
    return false;
}

bool CRingStateChangeItem::mustSwap() const
{
    return false;
}


void CRingStateChangeItem::toRawRingItem(CRawRingItem& item) const
{
    item.setType(type());
    item.setEventTimestamp(getEventTimestamp());
    item.setSourceId(getSourceId());

    Buffer::ByteBuffer& body = item.getBody();

    body << m_runNumber;
    body << m_timeOffset;
    body << uint32_t(m_timestamp);
    body << m_offsetDivisor;
    body << uint32_t(m_title.size());
    body.insert(body.end(), m_title.begin(), m_title.end());

}



/*!
   Set the run number for the item.
   \param run - new run number.
*/
void
CRingStateChangeItem::setRunNumber(uint32_t run)
{
        m_runNumber = run;
}
/*!
  \return uint32_t
  \retval the item's current run number.
*/
uint32_t
CRingStateChangeItem::getRunNumber() const
{
    return m_runNumber;
}

/*!
   Set the elapsed run time field for the item.
   \param offset - seconds into the run.
*/
void
CRingStateChangeItem::setElapsedTime(uint32_t offset)
{
    m_timeOffset = offset;
}
/*!
  \return uint32_t
  \retval time offset field of the item.
*/
uint32_t
CRingStateChangeItem::getElapsedTime() const
{
    return m_timeOffset;
}
/**
 * getElapsedTime
 *
 * @return float - Elapsed time taking into account the divisor.
 */
float
CRingStateChangeItem::computeElapsedTime() const
{
    float offset = m_timeOffset;
    float divisor = m_offsetDivisor;
    return offset/divisor;
}
/*!
  Set the title string.
  \param title - new title string.
  \throw CRangeError - if the title string is too long to fit.
*/
void
CRingStateChangeItem::setTitle(string title)
{
    m_title = title;
}

/*!
    \return std::string
    \retval the title string of the item.
*/
string
CRingStateChangeItem::getTitle() const
{
    return m_title;
}
/*!
   Set the timestamp
   \param stamp  - new timestamp.
*/
void
CRingStateChangeItem::setTimestamp(time_t stamp)
{
    m_timestamp = stamp;
}
/*!
    \return time_t
    \retval timestamp giving absolute time of the item.
*/
time_t
CRingStateChangeItem::getTimestamp() const
{
    return m_timestamp;
}

void
CRingStateChangeItem::setOffsetDivisor(uint32_t divisor)
{
    m_offsetDivisor = divisor;
}



uint32_t
CRingStateChangeItem::getOffsetDivisor() const
{
 return m_offsetDivisor;
}


////////////////////////////////////////////////////////////////////////////////
//
// Virtual method overrides:

/**
 * typeName
 *
 *  Stringify the type of the item.
 *
 * @return std::string - the item type
 */
std::string
CRingStateChangeItem::typeName() const
{
  switch (type()) {
  case V12::BEGIN_RUN:
    return "Begin Run";
  case V12::END_RUN:
    return "End Run";
  case V12::PAUSE_RUN:
    return "Pause Run";
  case V12::RESUME_RUN:
    return "Resume Run";
  }
}
/**
 * toString
 *
 * Returns a string that is the ascified body of the item.
 *
 * @return std::string - ascified version of the item.
 */
std::string
CRingStateChangeItem::toString() const
{
  std::ostringstream out;		//  Build up via outputting to this psuedo stream.

  uint32_t run       = getRunNumber();
  uint32_t elapsed   = getElapsedTime();
  string   title     = getTitle();
  time_t   wall_time = getTimestamp();
  string   timestamp = std::ctime(&wall_time);
  timestamp = std::string(timestamp.begin(), timestamp.end()-1);

  out <<  timestamp << " : Run State Change : " << typeName();
  out << " at " << elapsed << " seconds into the run\n";
  out << headerToString(*this);
  out << "Title     : " << title << std::endl;
  out << "Run Number: " << run   << endl;


  return out.str();
}
    
///////////////////////////////////////////////////////////////////////////////
// Private utilities.


/*
 * Evaluate wheter or the type of the current ring item is 
 * a state change or not.
 */
bool
CRingStateChangeItem::isStateChange()
{
  int t = type();
  return (
      (t == V12::BEGIN_RUN )              ||
      (t == V12::END_RUN)                 ||
      (t == V12::PAUSE_RUN)               ||
      (t == V12::RESUME_RUN));
}


} // end of V12 namespace
} // end DAQ
