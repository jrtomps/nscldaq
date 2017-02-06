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
#include "V12/CRingScalerItem.h"
#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <Deserializer.h>

#include <ctime>
#include <stdexcept>

#include <time.h>
#include <string.h>
#include <sstream>
#include <stdio.h>

using namespace std;

namespace DAQ {
  namespace V12 {

///////////////////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals.
//

/*!
  Construct a scaler item with the scalers all zeroed and the times not set...
  except for the absolute timestamp which is set to now. The source id and
  event timestamp are set to 0 and NULL_TIMESTAMP, respectively.

  \param numScalers - Number of scalers that will be put in the item.
*/
CRingScalerItem::CRingScalerItem(size_t numScalers)
  : CRingScalerItem(NULL_TIMESTAMP, 0, 0, 0, time(NULL), std::vector<uint32_t>(numScalers,0), true, 1, 32) {}

/*!
  Construct a scaler item with all the info but the header specified

  The event timestamp and source id are set to NULL_TIMESTAMP and 0, respectively.

  \param startTime - incremental scaler interval start time.
  \param endTime   - incremental scaler interval end time
  \param timestamp - Absolute system time at which the item was created.
  \param scalers   - Vector of scaler values.
  \param isIncremental - value of the incremental flag (defaults to true).
  \param timestampDivisor - Value of the timestamp divisor - defaults to 1.


*/
CRingScalerItem::CRingScalerItem(uint32_t startTime,
    uint32_t stopTime,
    time_t   timestamp,
    std::vector<uint32_t> scalers,
    bool isIncremental,
    uint32_t timeOffsetDivisor,
    uint32_t scalerWidth) :
    CRingScalerItem(NULL_TIMESTAMP, 0, startTime, stopTime, timestamp,
                    scalers, timeOffsetDivisor, isIncremental, scalerWidth)
{}

/**
 * Constructor with all data fields made explicit.
 *
 * Construcs a scaler ring item that has a body header.
 *
 * \param eventTimestamp - The event timestamp for the body header.
 * \param source         - id of the source that created this item.
 * \param barrier        - barrier type or 0 if this was not a barrier.
 * \param startTime - incremental scaler interval start time.
 * \param endTime   - incremental scaler interval end time
 * \param timestamp - Absolute system time at which the item was created.
 * \param scalers   - Vector of scaler values.
 * \param timeDivisor - The divisor for the start/end times that yields seconds.
 *                      defaults to 1.
 * \param incremental - True (default) if the scaler item is incremental.
 */
CRingScalerItem::CRingScalerItem(
    uint64_t eventTimestamp, uint32_t source, uint32_t startTime,
    uint32_t stopTime, time_t   timestamp, std::vector<uint32_t> scalers,
    uint32_t timeDivisor, bool incremental, uint32_t scalerWidth)
    : m_sourceId(source),
      m_evtTimestamp(eventTimestamp),
      m_intervalStartOffset(startTime),
      m_intervalEndOffset(stopTime),
      m_timestamp(timestamp),
      m_intervalDivisor(timeDivisor),
      m_isIncremental(incremental),
      m_scalerWidth(scalerWidth),
      m_scalers(scalers)
{
}

/*!
 * \brief Construct from a raw ring item
 *
 * \param rhs - the raw ring item containing the raw data to parse
 *
 * \throws std::bad_cast if rhs.type() does not return V12::PERIODIC_SCALERS
 * \throws std::runtime_error if isIncremental field is neither 0 or 1
 */
CRingScalerItem::CRingScalerItem(const CRawRingItem& rhs)
{
  if (rhs.type() != PERIODIC_SCALERS) {
    throw std::bad_cast();
  }
  m_sourceId     = rhs.getSourceId();
  m_evtTimestamp = rhs.getEventTimestamp();

  Buffer::ContainerDeserializer<Buffer::ByteBuffer> rhsBody(rhs.getBody(), rhs.mustSwap());

  uint32_t scalerCount, temp;
  rhsBody >> m_intervalStartOffset;
  rhsBody >> m_intervalEndOffset;
  rhsBody >> m_timestamp;
  rhsBody >> m_intervalDivisor;
  rhsBody >> scalerCount;
  rhsBody >> temp;
  if (temp == 0) {
    m_isIncremental = false;
  } else if (temp == 1) {
    m_isIncremental = true;
  } else {
    throw std::runtime_error("V12::CRingScalerItem(const CRawRingItem&) Bad value for is incremental field.");
  }
  rhsBody >> m_scalerWidth;

  m_scalers.reserve(scalerCount);
  for (int i=0; i<scalerCount; ++i) {
    rhsBody >> temp;
    m_scalers.push_back(temp);
  }
  
}

/*!
  Destruction just delegates.
*/
CRingScalerItem::~CRingScalerItem()
{}

/*!
   \brief Equality comparison operator

    \param rhs - Item being compared to *this.
    \return bool

    \retval true if all data members are equal
    \retval false otherwise
*/
bool
CRingScalerItem::operator==(const CRingItem& rhs) const
{
  if (m_sourceId != rhs.getSourceId()) return false;
  if (m_evtTimestamp != rhs.getEventTimestamp()) return false;

  const CRingScalerItem* pItem = dynamic_cast<const CRingScalerItem*>(&rhs);
  if (!pItem) return false;
  if (m_intervalStartOffset != pItem->getStartTime()) return false;
  if (m_intervalEndOffset != pItem->getEndTime()) return false;
  if (m_timestamp != pItem->getTimestamp()) return false;
  if (m_intervalDivisor != pItem->getTimeDivisor()) return false;
  if (m_isIncremental != pItem->isIncremental()) return false;
  if (m_scalerWidth != pItem->getScalerWidth()) return false;
  if (m_scalers != pItem->getScalers()) return false;

  return true;
}
/*!
   Inequality comparison operator

   \rhs - Item being compared to *thisl
   \return bool
   \retval false - items are equal
   \retval true - items are not equal
*/
bool
CRingScalerItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}

/*!
 * \brief CRingScalerItem::setType
 *
 * \param type  the new type (must be PERIODIC_SCALERS)
 *
 * \throws std::runtime_error if type is not PERIODIC_SCALERS
 */
void      CRingScalerItem::setType(uint32_t type) {
    if (type != PERIODIC_SCALERS) {
        throw std::runtime_error("DAQ::V12::CRingScalerItem::setType() Type must remain PERIODIC_SCALERS.");
    }
}

/*!
 * \return the number bytes in the item
 */
uint32_t  CRingScalerItem::size() const
{
  return 20 + bodySize();
}

uint64_t  CRingScalerItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void      CRingScalerItem::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}

uint32_t  CRingScalerItem::getSourceId() const
{
    return m_sourceId;
}
void      CRingScalerItem::setSourceId(uint32_t sid)
{
    m_sourceId = sid;
}

/*!
 * \return false (CRingScalerItems are always leaf types)
 */
bool      CRingScalerItem::isComposite() const
{
    return false;
}

/*!
 * \return false (all data is stored in native byte order)
 */
bool      CRingScalerItem::mustSwap() const
{
    return false;
}

/*!
 * \brief Serialize the data to a raw ring item
 *
 * \param item  the raw ring item that will receive the data
 *
 * The body of item is cleared prior to adding new data
 */
void CRingScalerItem::toRawRingItem(DAQ::V12::CRawRingItem& item) const
{
  item.setType(type());
  item.setEventTimestamp(m_evtTimestamp);
  item.setSourceId(m_sourceId);

  auto& body = item.getBody();
     
  body.clear();
  body << m_intervalStartOffset;
  body << m_intervalEndOffset;
  body << m_timestamp;
  body << m_intervalDivisor;
  body << uint32_t(m_scalers.size());
  body << (m_isIncremental ? uint32_t(1) : uint32_t(0));
  body << m_scalerWidth;
  for (auto& scaler : m_scalers) {
    body << scaler;
  }
  item.setMustSwap(false);
}



/*!
   Set the scaler interval start time.

   \param startTime - the new interval start time.
*/
void
CRingScalerItem::setStartTime(uint32_t startTime)
{
    m_intervalStartOffset = startTime;
}
/*!
   \return uint32_t
   \retval the current interval start time.
*/
uint32_t
CRingScalerItem::getStartTime() const
{
    return m_intervalStartOffset;
}
/**
 * computeStartTime
 *
 * Determine the floating point start time in seconds using the time offset and
 * divisor values. The time is properly computed as (start_time / divisor).
 *
 * @return float
 */
float
CRingScalerItem::computeStartTime() const
{
     float start   = m_intervalStartOffset;
     float divisor = m_intervalDivisor;
     return start/divisor;
}

/*!
   Set the interval ent time
   \pram endTime - the new interval end time.
*/
void
CRingScalerItem::setEndTime(uint32_t endTime) 
{
    m_intervalEndOffset = endTime;
}

/*!
   \return uint32_t
   \retval The current interval end time.
*/
uint32_t
CRingScalerItem::getEndTime() const
{
    return m_intervalEndOffset;
}
/**
 * computeEndTime
 *
 * Compute the interval end time using both the end time and the divisor.
 * The end time is properly computed as (end_time) / divisor.
 *
 * @return float - time in seconds.
 */
float
CRingScalerItem::computeEndTime() const
{
    float end   = m_intervalEndOffset;
    float divisor = m_intervalDivisor;
    return end/divisor;   
}

/*!
 * \brief CRingScalerItem::setTimeDivisor
 *
 * \param value     the divisor value
 */
void
CRingScalerItem::setTimeDivisor(uint32_t value)
{
    m_intervalDivisor = value;
}

/**
 * getTimestampDivisor
 *   Get the divisor that must be applied to get a real time offset.
 *
 * @return uint32_t
 */
uint32_t
CRingScalerItem::getTimeDivisor() const
{
    return m_intervalDivisor;
}

/*!
 * \brief CRingScalerItem::setIncremental
 * \param enable    enable or disable the incremental mode
 */
void CRingScalerItem::setIncremental(bool enable)
{
    m_isIncremental = enable;
}

/**
 * isIncremental
 *
 * @return bool true if s_isIncremental is true etc.
 */
bool
CRingScalerItem::isIncremental() const
{
    return m_isIncremental;
}

/*!
   set the absolute timestamp of the event.

   \param stamp - the time stampe.
*/
void
CRingScalerItem::setTimestamp(time_t  stamp)
{
    m_timestamp = stamp;
}
/*!
   \return time_t
   \retval the current timestamp.
*/
time_t
CRingScalerItem::getTimestamp() const
{
    return m_timestamp;
}


/*!
  Set a scaler value.
  \param channel - Number of the channel to modify.
  \param value   - The new value for that channel.
  \throw std::out_of_range if channel is out of range
*/
void
CRingScalerItem::setScaler(uint32_t channel, uint32_t value)
{
    m_scalers.at(channel) = value;
}

/*!
  Set a scaler values.
  \param values  - The new values for that channel.
*/
void
CRingScalerItem::setScalers(const std::vector<uint32_t>& values)
{
    m_scalers = values;
}

/*!

  \param channel - Number of the channel we want.
  \return uint32_t
  \retval Value of the selected scaler channel
  \throw std::out_of_range - if the channel number is out of range.
*/
uint32_t
CRingScalerItem::getScaler(uint32_t channel) const
{
    return m_scalers.at(channel);
}
/*!
    \return std::vector<uint32_t>
    \retval - the scalers.
*/
vector<uint32_t>
CRingScalerItem::getScalers() const
{
    return m_scalers;
}

/*!
   \return uint32_t
   \retval number of scalers
*/
uint32_t
CRingScalerItem::getScalerCount() const
{
    return m_scalers.size();
}

uint32_t
CRingScalerItem::getScalerWidth() const
{
    return m_scalerWidth;
}

void CRingScalerItem::setScalerWidth(uint32_t width)
{
    m_scalerWidth = width;
}

/**
 * typeName
 *
 *  Returns the string associated with this type of
 *  ring item ("Scaler").
 *
 * @return std::string - the item type name string.
 */
std::string
CRingScalerItem::typeName() const
{
  return std::string("Scalers");
}
/**
 * toString
 *
 *  Return a textual human readable version of the data
 *  in this item.
 *
 * @return std::string - the text string.
 */

std::string
CRingScalerItem::toString() const
{
  std::ostringstream out;
  out.precision(1);
  out.setf(std::ios::fixed);

  float end   = computeEndTime();
  float start = computeStartTime();
  time_t rawTime(getTimestamp());
  string   time  = std::ctime(&rawTime);
  vector<uint32_t> scalers = getScalers();

  float   duration = end - start;

  out << headerToString(*this);
  out << "Unix Tstamp  : " << time;
  out << "Start Offset : " << start << " seconds" << std::endl;
  out << "End Offset   : " << end << " seconds" << std::endl;
  out << "Incremental? : " << (isIncremental() ? "Yes" : "No") << std::endl;
  out << std::endl;
  out << "Index         Counts                 Rate (counts/sec)\n";
  for (int i=0; i < scalers.size(); i++) {
    char line[128];
    double rate = (static_cast<double>(scalers[i])/duration);

    sprintf(line, "%5d      %9d                 %.2f\n",
        i, scalers[i], rate);
    out << line;
  }


  return out.str();
  
}

/*-------------------------------------------------------
** Private utilities:
*---------------------------------------------------------
*/
/*
 *  Given the number of scalers determine the size of the body.
 */
size_t
CRingScalerItem::bodySize() const
{
  return (7+m_scalers.size())*sizeof(uint32_t);
}
/*
** Throws a range error if the channel number is invalid.
*/
void
CRingScalerItem::throwIfInvalidChannel(uint32_t channel,
                       const char* message) const
{
    if (channel >= m_scalers.size()) {
      throw CRangeError(0, m_scalers.size(), channel,
                        message);
    }
}


} // end of V12 namespace
} // end DAQ
