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
#include "V12/CRingTextItem.h"
#include <V12/CRawRingItem.h>
#include <Deserializer.h>
#include <sstream>
#include <ctime>
#include <algorithm>
using namespace std;

namespace DAQ {
  namespace V12 {

///////////////////////////////////////////////////////////////////////////////////
//
//   Constructors and other canonical member functions.
//

/*!
   Construct a ring item that contains text strings.
   The item will have a timestamp of 'now' and an offset time of 0.

   The event timestamp and source id are set to NULL_TIMESTAMP and 0, respectively.

   \param type       - the ring item type. This must be
                        PACKET_TYPES or MONITORED_VARIABLES.
   \param theStrings - the set of strings in the ring buffer item.

   \throws std::invalid_argument if type is not PACKET_TYPES or MONITORED_VARIABLES
*/
CRingTextItem::CRingTextItem(uint16_t type, vector<string> theStrings)
 : CRingTextItem(type, V12::NULL_TIMESTAMP, 0, theStrings, 0, time(nullptr), 1)
{
}
/*!
  Construct a ring buffer, but this time provide actual values for the
  time offset and time stamp. The source id and event timestamp are set to
  0 and NULL_TIMESTAMP, respectively.

  \param type    - Type of ring item... see the previous constructor.
  \param strings - The strings to put in the buffer.
  \param offsetTime - The time in to the run at which this is being inserted.
  \param timestamp  - The absolute time when this is being created.

  \throws std::invalid_argument if type is not PACKET_TYPES or MONITORED_VARIABLES
*/
CRingTextItem::CRingTextItem(uint16_t       type,
                 vector<string> strings,
                 uint32_t       offsetTime,
                 time_t         timestamp)
    : CRingTextItem(type, V12::NULL_TIMESTAMP, 0, strings, offsetTime, timestamp)
{
}
/**
 * constructor
 *
 * Construct a text item that has all data fields explicitly specified.
 *
 * \param type           - Type of ring item.
 * \param eventTimestamp - Event clock time at which this occured.
 * \param source         - Id of the data source.
  \param strings - The strings to put in the buffer.
  \param offsetTime - The time in to the run at which this is being inserted.
  \param timestamp  - The absolute time when this is being created.
  \param divisor    - offsetTime/divisor = seconds into the run (default: 1)
 */
CRingTextItem::CRingTextItem(
    uint16_t type, uint64_t eventTimestamp, uint32_t source,
    std::vector<std::string> theStrings, uint32_t offsetTime, time_t timestamp,
    int divisor) :
    m_type(type),
    m_evtTimestamp(eventTimestamp),
    m_sourceId(source),
    m_timeOffset(offsetTime),
    m_timestamp(timestamp),
    m_offsetDivisor(divisor),
    m_strings(theStrings)
{
    if (type != PACKET_TYPES && type != MONITORED_VARIABLES ) {
        std::string errmsg = "CRingTextItem may only take a text type ";
        errmsg += "(i.e. PACKET_TYPES, MONITORED_VARIABLES)";
        throw std::invalid_argument(errmsg);
    }
}

/*!
   Construct from a raw ring item

   \param rhs  - The raw ring item from which this is constructed.
   
   \throw bad_cast - if rhs is not a text ring item.

   It will parse the body until all of the strings have been found or
   until the body is fully parsed.
*/
CRingTextItem::CRingTextItem(const CRawRingItem& rhs)
{
  if (!validType(rhs)) throw bad_cast();

  m_sourceId     = rhs.getSourceId();
  m_evtTimestamp = rhs.getEventTimestamp();

  auto& body = rhs.getBody();
  Buffer::ContainerDeserializer<Buffer::ByteBuffer> rhsBody(body, rhs.mustSwap());

  uint32_t stringCount, temp;
  rhsBody >> m_timeOffset;
  rhsBody >> temp; m_timestamp = temp;
  rhsBody >> stringCount;
  rhsBody >> m_offsetDivisor;

  m_strings.reserve(stringCount);

  auto iter = rhsBody.pos();
  auto next = iter;
  while (iter != body.end()) {
    next = std::find(iter, body.end(), '\0');

    if (next != body.end()) {
        m_strings.emplace_back(iter, next);
        next++;
        iter = next;
    }
  }
}

/*! Destructor */
CRingTextItem::~CRingTextItem()
{}

/*!
  \brief Equality comparison operator

  No real point in comparing the item pointers.

  \param rhs - the item being compared to *this
  \return bool
  \retval false  either the evt tstamp, source id, type, time offset, divisor, timestamp, or the strings are different
  \retval true   otherwise
*/
bool
CRingTextItem::operator==(const CRingItem& rhs) const
{
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    const auto pItem = dynamic_cast<const CRingTextItem*>(&rhs);
    if (m_type != pItem->type()) return false;
    if (m_timeOffset != pItem->getTimeOffset()) return false;
    if (m_offsetDivisor != pItem->getTimeDivisor()) return false;
    if (m_timestamp != pItem->getTimestamp()) return false;
    if (m_strings != pItem->getStrings()) return false;
    return true;
}

/*!
 Inequality comparison operator

  \param rhs      - the item being compared to *this.

  \retval false   - Items are equal
  \retval true    - items are inequal

*/
bool
CRingTextItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}


///////////////////////////////////////////////////////////////////////////////////////
//
// accessors (both selectors and mutators).
//

uint32_t CRingTextItem::type() const
{
    return m_type;
}

/*!
 * \brief Set the type
 *
 * \param type  new type (must be either MONITORED_VARIABLES or PACKET_TYPES)
 *
 * \throws std::invalid_argument if type is not V12::MONITORED_VARIABLES or PACKET_TYPES
 */
void CRingTextItem::setType(uint32_t type)
{
    if (type != MONITORED_VARIABLES && type != PACKET_TYPES) {
        std::string errmsg ="V12::CRingTextItem ";
        errmsg += "Text items may only take two types (V12::MONITORED_VARIABLES";
        errmsg += " or V12::PACKET_TYPES)";
        throw std::invalid_argument(errmsg);
    }
    m_type = type;
}

/*!
 * \brief CRingTextItem::size
 * \return computed ring item size
 *
 * The size is computed from all of the strings. There is a single \0 character counted
 * for each of the strings.
 */
uint32_t CRingTextItem::size() const
{
    size_t cumulativeLength = 0;

    for (auto& s : m_strings ) {
        cumulativeLength += s.size();
    }

    return 9*sizeof(uint32_t) + m_strings.size() + cumulativeLength;
}


uint64_t CRingTextItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CRingTextItem::setEventTimestamp(uint64_t stamp)
{
    m_evtTimestamp = stamp;
}

uint32_t CRingTextItem::getSourceId() const
{
    return m_sourceId;
}

void CRingTextItem::setSourceId(uint32_t id)
{
    m_sourceId = id;
}

/*!
 * \return false (CRingTextItems are always leafs)
 */
bool CRingTextItem::isComposite() const
{
    return false;
}

/*!
 * \return  false (data is always stored in native byte order)
 */
bool CRingTextItem::mustSwap() const
{
    return false;
}


/*!
 * \brief Serialize data into a raw ring item
 *
 * \param buffer    the ring item to fill with data
 *
 * The body of the raw ring item is cleared before being filled.
 */
void CRingTextItem::toRawRingItem(CRawRingItem& buffer) const
{
    buffer.setType(m_type);
    buffer.setEventTimestamp(m_evtTimestamp);
    buffer.setSourceId(m_sourceId);

    // clear the body
    auto& body = buffer.getBody();
    body.clear();
    body.reserve(size());

    body << m_timeOffset;
    body << uint32_t(m_timestamp);
    body << uint32_t(m_strings.size());
    body << m_offsetDivisor;

    for (auto& s : m_strings) {
        body.insert(body.end(), s.begin(), s.end());
        body.push_back(0); // null terminate
    }
}


/*!
    \return vector<string>
    \retval The strings that were put in the item unpacked into elements of the vector.
*/
vector<string>
CRingTextItem::getStrings() const
{
   return m_strings;
}

vector<string>&
CRingTextItem::getStrings()
{
   return m_strings;
}

uint32_t
CRingTextItem::getStringCount() const {
    return m_strings.size();
}
/*!
   Modify the buffered value of the run time offset.  This may be done if you use the
   simplified constuctor and only later figure out what the run time offset actually is.
   \param offset
*/
void
CRingTextItem::setTimeOffset(uint32_t offset)
{
    m_timeOffset = offset;
}
/*!
   \return uint32_t
   \retval the time offset value.
*/
uint32_t
CRingTextItem::getTimeOffset() const
{
  return m_timeOffset;
}
/**
 * computeElapsedTime
 *
 * Determine the floating point seconds into the run using the
 * time offset and the divisor. The elapsed time is just computed
 * by dividing the offset by the divisor (i.e. elapsed time = offset / divisor).
 *
 * @return float
 */
float
CRingTextItem::computeElapsedTime() const
{
    float time   = m_timeOffset;
    float divisor= m_offsetDivisor;
    
    return time/divisor;
}
/**
 * @return the time offset divisor
 */
uint32_t
CRingTextItem::getTimeDivisor() const
{
  return m_offsetDivisor;
}

void
CRingTextItem::setTimeDivisor(uint32_t divisor)
{
    m_offsetDivisor = divisor;
}

/*!
   Set a new value for the timestamp of the item.
*/
void
CRingTextItem::setTimestamp(time_t stamp)
{
    m_timestamp = stamp;
}
/*!
   \return time_t
   \retval absolute timestamp from the buffer.

*/
time_t
CRingTextItem::getTimestamp() const
{
  return m_timestamp;
}

///////////////////////////////////////////////////////////
//
// Virtual method implementations.

/**
 * typeName
 *
 *  return the stringified item type name.
 *
 * @return std::string - name of item-type.
 */
std::string
CRingTextItem::typeName() const
{
  if(type() == PACKET_TYPES) {
    return std::string("Packet types");
  } else if (type() == MONITORED_VARIABLES) {
    return std::string("Monitored Variables");
  } else {
    throw std::string("CRingTextItem::typeName - Invalid type!");
  }
}
/**
 * toString
 *
 * Returns a stringified version of the item that can
 * be read by humans.
 *
 * @return std::string - stringified output.
 */
std::string
CRingTextItem::toString() const
{
  std::ostringstream out;
  out.precision(1);
  out.setf(std::ios::fixed);

  uint32_t elapsed  = getTimeOffset();
  time_t time = getTimestamp();
  string   timeString = std::ctime(&time);
  vector<string> strings = getStrings();

  out << headerToString(*this);
  out << "Elapsed Time : " << computeElapsedTime() << " seconds" << std::endl;
  out << "Unix Tstamp  : " << timeString;
  out << "# of Strings : " << m_strings.size() << std::endl;
  for (int i = 0; i < strings.size(); i++) {
    out << i << ": \"" << strings[i] << "\"" << endl;
  }

  return out.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//

/*
**  Returns true if the type of the current item is a valid text item.
**
*/
bool
CRingTextItem::validType(const CRingItem& item) const
{
  return ((item.type() == PACKET_TYPES) || (item.type() == MONITORED_VARIABLES));
}

  } // end of V12 namespace
} // end DAQ
