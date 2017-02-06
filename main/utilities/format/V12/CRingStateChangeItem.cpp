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


#include <config.h>
#include "V12/CRingStateChangeItem.h"
#include <V12/CRawRingItem.h>
#include <Deserializer.h>
#include <sstream>
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
   are both set to zero.  The timestamp is set to 'now'. The event timestamp
   is NULL_TIMESTAMP and the source id is 0.

   The title is set to an empty string.

   \param reason - Reason for the state change.  This defaults to BEGIN_RUN.
*/
CRingStateChangeItem::CRingStateChangeItem(uint16_t reason)
    :
    CRingStateChangeItem(V12::NULL_TIMESTAMP, 0, reason,
                         0, 0, time(NULL), "",1)
{
}

/*!
   Construction with all state change specific values specified by
   the constructor parameters. The event timestamp and source id are
   NULL_TIMESTAMP and 0, respectively.

   \param reason     - Why the state change buffer is being emitted (the item type).
   \param runNumber  - Number of the run that is undergoing transition.
   \param timeOffset - Number of seconds into the run at which this is being emitted.
   \param timestamp  - Absolute wall time to be recorded in the buffer.. tyically
                       this should be time(NULL).
   \param title      - Title string.


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
 * Completely specified constructor
 *
 * @param eventTimestamp  - Event timestamp
 * @param sourceId   - Source id of the event.
   \param reason     - Why the state change buffer is being emitted (the item type).
   \param runNumber  - Number of the run that is undegoing transition.
   \param timeOffset - Number of seconds into the run at which this is being emitted.
   \param timestamp  - Absolute wall time to be recorded in the buffer.. typically
                       this should be time(NULL).
   \param title      - Title string.
   @param offsetDivisor - What timeOffset needs to be divided by to get seconds.

   \throws std::invalid_argument if reason is not a valid state change type
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
{

    if ( ! isStateChange(type()) ) {
        throw std::invalid_argument("CRingStateChangeItem() constructed with non-state change type id.");
    }
}

/*!
   Construct a state change item from a raw ring item. The byte order is handled appropriately
   when parsing the raw ring item body.

   \param item - The raw ring item.

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

  Buffer::ContainerDeserializer<Buffer::ByteBuffer> rhsBody(rhs.getBody(), rhs.mustSwap());

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

/*! Destruction */
CRingStateChangeItem::~CRingStateChangeItem()
{
}


/*! \brief Equality comparison operator

  \param rhs - item to comparse *this to

  \return boolean
  \retval false (either type, source id, event tstamp, run number, time offset, tstamp, divisor, or title is different)
  \retval true otherwise
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


/*! \return false (always a leaf item) */
bool CRingStateChangeItem::isComposite() const
{
    return false;
}

/*! \return false (always stored in native byte order) */
bool CRingStateChangeItem::mustSwap() const
{
    return false;
}

/*!
 * \brief Serialize data fields into the body of a raw ring item.
 *
 * \param item  the raw ring item to fill
 *
 * If the body of the raw ring item is nonempty prior to being passed
 * as an argument, the existing contents are discarded.
 *
 */
void CRingStateChangeItem::toRawRingItem(CRawRingItem& item) const
{
    item.setType(type());
    item.setEventTimestamp(getEventTimestamp());
    item.setSourceId(getSourceId());

    Buffer::ByteBuffer& body = item.getBody();
    body.clear();

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
 * The actual elapsedTime is computed as offset/divisor.
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
    \retval timestamp giving absolute wall time of the item.
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
 * Returns a string that is a human-readable, string representation of the item
 *
 * @return std::string - ascified version of the item.
 */
std::string
CRingStateChangeItem::toString() const
{
  std::ostringstream out;		//  Build up via outputting to this psuedo stream.
  out.precision(1);
  out.setf(std::ios::fixed);

  uint32_t run       = getRunNumber();
  uint32_t elapsed   = getElapsedTime();
  string   title     = getTitle();
  time_t   wall_time = getTimestamp();
  string   timestamp = std::ctime(&wall_time);
  timestamp = std::string(timestamp.begin(), timestamp.end()-1);

  out << headerToString(*this);
  out << "Run Number   : " << run << endl;
  out << "Unix Tstamp  : " << timestamp << endl;
  out << "Elapsed Time : " << computeElapsedTime() << " seconds" << std::endl;
  out << "Title        : " << title << std::endl;

  return out.str();
}
    
///////////////////////////////////////////////////////////////////////////////
// Private utilities.


/*
 * Evaluate wheter or the type of the current ring item is 
 * a state change or not.
 *
 * \retval true (type is BEGIN_RUN, END_RUN, RESUME_RUN, or PAUSE_RUN)
 * \retval false otherwise
 */
bool
CRingStateChangeItem::isStateChange(uint32_t t) const
{
  return (
      (t == V12::BEGIN_RUN )              ||
      (t == V12::END_RUN)                 ||
      (t == V12::PAUSE_RUN)               ||
      (t == V12::RESUME_RUN));
}


} // end of V12 namespace
} // end DAQ
