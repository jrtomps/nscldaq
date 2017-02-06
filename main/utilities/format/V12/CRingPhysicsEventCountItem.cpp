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
#include "V12/CRingPhysicsEventCountItem.h"
#include <V12/CRawRingItem.h>
#include <Deserializer.h>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace DAQ {
  namespace V12 {

///////////////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals.

/*!
   Default constructor has a unix timestamp of now, a time offset of 0, an event
   count of 0, event timestamp of NULL_TIMESTAMP, source id of 0.
*/
CRingPhysicsEventCountItem::CRingPhysicsEventCountItem()
    : CRingPhysicsEventCountItem(V12::NULL_TIMESTAMP, 0, 0, 0, 0, 1)
{
}

/*!
   Creates an event count item timestamped to now (i.e. unix timestamp) with a specified
   number of events and run offset. The source id is 0 and the event timestamp is NULL_TIMESTAMP.

   \param count       - number of events.
   \param timeOffset  - Seconds into the active run time at which this item
                        was produced.
*/
CRingPhysicsEventCountItem::CRingPhysicsEventCountItem(uint64_t count,
                               uint32_t timeOffset)
    : CRingPhysicsEventCountItem(V12::NULL_TIMESTAMP, 0, count, timeOffset, 0, 1)
{

}

/*!
  Creates an event count item with an explicit count, unix timestamp, and offset.
 The source id and event timestamp are 0 and NULL_TIMESTAMP, respectively.
   \param count       - number of events.
   \param timeOffset  - Seconds into the active run time at which this item
   \param stamp       - Timestamp at which the event was produced.
*/
CRingPhysicsEventCountItem::CRingPhysicsEventCountItem(uint64_t count,
                               uint32_t timeOffset,
                               time_t   stamp)
 : CRingPhysicsEventCountItem(V12::NULL_TIMESTAMP, 0, count, timeOffset, stamp, 1)
{
}

/**
 * \brief Constructor with all elements fully specified.*
 *
 * @param timestamp - Event timestamp value.
 * @param source    - Id of the data source.
 * @param count     - Number of physics events.
 * @param timeoffset  - How long into the run we are.
 * @param stamp      - Unix timestamp.
 * @param divisor    - timeoffset/divisor = seconds.
 */
CRingPhysicsEventCountItem::CRingPhysicsEventCountItem(
    uint64_t timestamp, uint32_t source,
    uint64_t count, uint32_t timeoffset, time_t stamp,
    int divisor) :
    m_evtTimestamp(timestamp),
    m_sourceId(source),
    m_timeOffset(timeoffset),
    m_offsetDivisor(divisor),
    m_timestamp(stamp),
    m_eventCount(count)
{}

 
/*!
  Construction from a raw ring item.

  The raw ring item body is parsed into local data. If the data of the raw ring item
  was swapped, it is handled accordingly. After construction, all data stored by the
  object is a in native byte order.

  \param rhs - a raw ring item.

  \throw std::bad_cast if rhs is not a PHYSICS_EVENT_COUNT item.
*/
CRingPhysicsEventCountItem::CRingPhysicsEventCountItem(const CRawRingItem& rhs)
{
  if (rhs.type() != PHYSICS_EVENT_COUNT) {
    throw bad_cast();
  }

  m_evtTimestamp = rhs.getEventTimestamp();
  m_sourceId = rhs.getSourceId();

  Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(rhs.getBody(), rhs.mustSwap());

  uint32_t temp;
  stream >> m_timeOffset;
  stream >> temp; m_timestamp = temp;
  stream >> m_offsetDivisor;
  stream >> m_eventCount;
}

/*! \brief Destructor */
CRingPhysicsEventCountItem::~CRingPhysicsEventCountItem()
{}

/*!
   \brief Equality comparison operator

   \param rhs - item being compared to *this.


   \return bool
   \retval true  - Equal (evt timestamp, source id, count, offset, divisor, and timestamp are equal)
   \retval false  - otherwise
*/
bool
CRingPhysicsEventCountItem::operator==(const CRingItem& rhs) const
{
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    const auto pItem = dynamic_cast<const CRingPhysicsEventCountItem*>(&rhs);
    if (!pItem) return false;
    if (m_timeOffset != pItem->getTimeOffset()) return false;
    if (m_timestamp != pItem->getTimestamp()) return false;
    if (m_offsetDivisor != pItem->getTimeDivisor()) return false;
    if (m_eventCount != pItem->getEventCount()) return false;
    return true;
}
/*!
  Inequality comparison operator

  \param rhs - item being compared to *this.

  \return bool
  \retval !(operator==(rhs)).
*/
bool
CRingPhysicsEventCountItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}


/*! \return V12::PHYSICS_EVENT_COUNT */
uint32_t CRingPhysicsEventCountItem::type() const
{
    return PHYSICS_EVENT_COUNT;
}


/*!
 * \brief CRingPhysicsEventCountItem::setType
 *
 * \param type  the new type (must be V12::PHYSICS_EVENT_COUNT)
 *
 * \throws std::invalid_argument if type is not V12::PHYSICS_EVENT_COUNT
 */
void CRingPhysicsEventCountItem::setType(uint32_t type)
{
    if (type != V12::PHYSICS_EVENT_COUNT) {
        std::string errmsg("CRingPhysicsEventCountItem::setType() only ");
        errmsg += "accepts V12::PHYSICS_EVENT_COUNT.";
        throw std::invalid_argument(errmsg);
    }
}

/*!
 * \return 40 (all physics event count items are the same number of bytes)
 */
uint32_t CRingPhysicsEventCountItem::size() const
{
    return 40;
}


uint32_t CRingPhysicsEventCountItem:: getSourceId() const
{
    return m_sourceId;
}

void CRingPhysicsEventCountItem::setSourceId(uint32_t id)
{
    m_sourceId = id;
}

uint64_t CRingPhysicsEventCountItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CRingPhysicsEventCountItem::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}


/*!
 * \return false
 *
 * All CRingPhysicsEventCountItem data is stored in native byte order.
 */
bool CRingPhysicsEventCountItem::mustSwap() const
{
    return false;
}

/*!
 * \return false
 *
 * All CRingPhysicsEventCountItem objects are leaf types.
 */
bool CRingPhysicsEventCountItem::isComposite() const
{
    return false;
}


/*!
 * \brief Serialize content to a raw ring item
 *
 * \param item  the raw ring item to fill
 *
 * If item has a nonzero body content prior to this call, that content is
 * erased.
 */
void CRingPhysicsEventCountItem::toRawRingItem(CRawRingItem& item) const
{
    item.setType(type());
    item.setSourceId(m_sourceId);
    item.setEventTimestamp(m_evtTimestamp);

    auto& body = item.getBody();
    body.clear();

    body << m_timeOffset;
    body << uint32_t(m_timestamp);
    body << m_offsetDivisor;
    body << m_eventCount;
}

/*!
    \return uint32_t
    \retval The curren value of the time offset.
*/
uint32_t
CRingPhysicsEventCountItem::getTimeOffset() const
{
  return m_timeOffset;
}
/**
 * computeTimeOffset
 *
 * Get the floating point seconds into the run using the time offset and
 * divisor fields:
 *
 * @return float
 */
float
CRingPhysicsEventCountItem::computeElapsedTime() const
{
    float timeOffset = m_timeOffset;
    float divisor    = m_offsetDivisor;
    
    return timeOffset/divisor;
}
/**
 * getTimeDivisor
 *   Return the divisor used to compute the offset.
 *
 * @return uint32_t
 */
uint32_t
CRingPhysicsEventCountItem::getTimeDivisor() const
{
  return m_offsetDivisor;
}
/*!
   set the time offset.
   \param offset - new value of time offset.
*/
void
CRingPhysicsEventCountItem::setTimeOffset(uint32_t offset)
{
  m_timeOffset = offset;
}

/*!
  \return time_t
  \retval current value of the timestamp.
*/
time_t
CRingPhysicsEventCountItem::getTimestamp() const
{
  return m_timestamp;
}
/*!
  \param stamp - New value for the timestamp.
*/
void
CRingPhysicsEventCountItem::setTimestamp(time_t stamp)
{
  m_timestamp = stamp;
}

/*!
  \return uint64_t
  \retval number of events submitted  to the ring this run.
*/
uint64_t
CRingPhysicsEventCountItem::getEventCount() const
{
  return m_eventCount;
}
/*!
  \param count - new value for number of events submitted.
*/
void
CRingPhysicsEventCountItem::setEventCount(uint64_t count)
{
  m_eventCount = count;
}
//////////////////////////////////////////////////////////
//
// Virtual method overrides.


/**
 * typeName
 * 
 *   Provides the type name for this ring item.
 *
 * @return std::string type name.
 */
std::string
CRingPhysicsEventCountItem::typeName() const
{
  return std::string("Trigger Count");
}
/**
 * toString
 *
 * Returns a stringified version of the item.
 *
 * Note that if the elapsed time if zero, there will be no attempt to compute
 * an average rate.
 *
 * @return item - the string.
 */
std::string
CRingPhysicsEventCountItem::toString() const
{
  std::ostringstream out;
  out.precision(1);
  out.setf(ios::fixed);

  time_t time = getTimestamp();
  string   timeString   = std::ctime(&time);
  timeString = string(timeString.begin(), timeString.end()-1);
  uint32_t offset = getTimeOffset();
  uint64_t events = getEventCount();
  double elapsedTime = computeElapsedTime();

  out << headerToString(*this);
  out << "Unix Tstamp  : " << timeString << endl;
  out << "Event Count  : " << events << endl;
  out << "Elapsed Time : " << elapsedTime << " seconds" << endl;
  if (elapsedTime > 0) {
      out << "Average Rate : " << events/elapsedTime <<  " evts/sec" << endl;
  }

  return out.str();
}


} // end of V12 namespace
} // end DAQ
