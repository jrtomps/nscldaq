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
   \param type       - the ring item type. This should be 
                        PACKET_TYPES or MONITORED_VARIABLES.
   \param theStrings - the set of strings in the ring buffer item.

*/
CRingTextItem::CRingTextItem(uint16_t type, vector<string> theStrings)
 : CRingTextItem(type, V12::NULL_TIMESTAMP, 0, theStrings, 0, time(nullptr), 1)
{
}
/*!
  Construct a ring buffer, but this time provide actual values for the
  time offset and time stamp.
  \param type    - Type of ring item... see the previous constructor.
  \param strings - The strings to put in the buffer.
  \param offsetTime - The time in to the run at which this is being inserted.
  \param timestamp  - The absolute time when this is being created.

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
 * Construct a text item that has a body header as well as the strings and
 * timestamp.
 *
 * @param type           - Type of ring item.
 * @param eventTimestamp - Event clock time at which this occured.
 * @param source         - Id of the data source.
  \param strings - The strings to put in the buffer.
  \param offsetTime - The time in to the run at which this is being inserted.
  \param timestamp  - The absolute time when this is being created.
  @param divisor    - offsetTime/divisor = seconds into the run (default: 1)
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
   Construct from an undifferentiated ring item.
   If the ring item does not have a type that is consistent with
   a text item type that is a strong error.
   \param rhs  - The ring item from which this is constructed.
   
   \throw bad_cast - if rhs is not a text ring item.
*/
CRingTextItem::CRingTextItem(const CRawRingItem& rhs)
{
  if (!validType(rhs)) throw bad_cast();

  m_sourceId     = rhs.getSourceId();
  m_evtTimestamp = rhs.getEventTimestamp();

  auto& body = rhs.getBody();
  Buffer::Deserializer<Buffer::ByteBuffer> rhsBody(body, rhs.mustSwap());

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

/*!
  Destructor just chains to base class.
*/
CRingTextItem::~CRingTextItem()
{}

///*!
//  Assignment
//  \param rhs - the item being assigned to this.
//  \return CRingTextItem&
//  \retval *this
//*/
//CRingTextItem&
//CRingTextItem::operator=(const CRingTextItem& rhs)
//{
//  if (this != &rhs) {
//    CRingItem::operator=(rhs);
//    init();
//  }
//  return *this;
//}

/*!
  Comparison for equality.  No real point in comparing the item pointers.
  unless this == &rhs they will always differ.
  \param rhs - the item being compared to *this
  \return int
  \retval 0        - Not equal
  \retval nonzero  - equal.
*/
int
CRingTextItem::operator==(const CRingTextItem& rhs) const
{
    if (m_type != rhs.m_type) return 0;
    if (m_evtTimestamp != rhs.m_evtTimestamp) return 0;
    if (m_sourceId != rhs.m_sourceId) return 0;
    if (m_timeOffset != rhs.m_timeOffset) return 0;
    if (m_offsetDivisor != rhs.m_offsetDivisor) return 0;
    if (m_timestamp != rhs.m_timestamp) return 0;
    if (m_strings != rhs.m_strings) return 0;
    return 1;
}
/*!
  Comparison for inequality.
  \param rhs      - the item being compared to *this.
  \retval 0       - Items are not inequal
  \retval nonzero - items are inequal

  \note My stilted English is because C++ allows perverse cases where
  a == b  does not necesarily imply !(a != b) and vica versa. In fact, these
  operators can be defined in  such a way that they have nothing whatever to do
  with comparison (just as ostream::operator<< has nothing to do with
  shifting). however  my definition is sensible in that a == b is the logical converse of
  a != b, and vicaversa, and these operators really do compare.
*/
int
CRingTextItem::operator!=(const CRingTextItem& rhs) const
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

uint32_t CRingTextItem::size() const
{
    size_t cumulativeLength = 0;

    for (auto& s : m_strings ) {
        cumulativeLength += s.size();
    }

    return 7*sizeof(uint32_t) + m_strings.size() + cumulativeLength;
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

bool CRingTextItem::isComposite() const
{
    return false;
}

bool CRingTextItem::mustSwap() const
{
    return false;
}

void CRingTextItem::toRawRingItem(CRawRingItem& buffer) const
{

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
 * Determin the floating point seconds into the run using the
 * time offset and the divisor.
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
 * @return the time offset divisor offset/divisor in float gives seconds.
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

  uint32_t elapsed  = getTimeOffset();
  time_t time = getTimestamp();
  string   timeString = std::ctime(&time);
  vector<string> strings = getStrings();

  out << timeString << " : Documentation item ";
  out << typeName();
  out << headerToString(*this);

  out << elapsed << " seconds in to the run\n";
  for (int i = 0; i < strings.size(); i++) {
    out << strings[i] << endl;
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
