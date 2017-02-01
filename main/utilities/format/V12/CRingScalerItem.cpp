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
  except for the absolute timestamp which is set to now.
  \param numScalers - Number of scalers that will be put in the item.
*/
CRingScalerItem::CRingScalerItem(size_t numScalers)
  : CRingScalerItem(0, 0, time(NULL), std::vector<uint32_t>(numScalers,0), true, 1, 32) {}
/*!
  Construct a scaler item with all the info specified.
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
  m_intervalStartOffset(startTime),
  m_intervalEndOffset(stopTime),
  m_timestamp(timestamp),
  m_isIncremental(isIncremental),
  m_intervalDivisor(timeOffsetDivisor),
  m_scalerWidth(scalerWidth),
  m_scalers(scalers)
{}
/**
 * constructor - with body header.
 *
 * Construcs a scaler ring item that has a body header.
 *
 * @param eventTimestamp - The event timestamp for the body header.
 * @param source         - id of the source that created this item.
 * @param barrier        - barrier type or 0 if this was not a barrier.
  \param startTime - incremental scaler interval start time.
  \param endTime   - incremental scaler interval end time
  \param timestamp - Absolute system time at which the item was created.
  \param scalers   - Vector of scaler values.
  @param timeDivisor - The divisor for the start/end times that yields seconds.
                       defaults to 1.
  @param incremental - True (default) if the scaler item is incremental.
 */
CRingScalerItem::CRingScalerItem(
    uint64_t eventTimestamp, uint32_t source, uint32_t startTime,
    uint32_t stopTime, time_t   timestamp, std::vector<uint32_t> scalers,
    uint32_t timeDivisor, bool incremental, uint32_t scalerWidth)
    : m_sourceId(source), \
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
  Casting construction.
  \param rhs - const reference to a RingItem.  We are doing an upcast to
  a scaler buffer.
*/
CRingScalerItem::CRingScalerItem(const CRawRingItem& rhs)
{
  if (rhs.type() != PERIODIC_SCALERS) {
    throw std::bad_cast();
  }
  m_sourceId     = rhs.getSourceId();
  m_evtTimestamp = rhs.getEventTimestamp();

  Buffer::Deserializer<Buffer::ByteBuffer> rhsBody(rhs.getBody(), rhs.mustSwap());

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
  Ordinary copy construction
  \param rhs - The item we are copying into *this
*/
CRingScalerItem::CRingScalerItem(const CRingScalerItem& rhs) :
  m_sourceId(rhs.m_sourceId),
  m_evtTimestamp(rhs.m_evtTimestamp),
  m_intervalStartOffset(rhs.m_intervalStartOffset),
  m_intervalEndOffset(rhs.m_intervalEndOffset),
  m_timestamp(rhs.m_timestamp),
  m_intervalDivisor(rhs.m_intervalDivisor),
  m_isIncremental(rhs.m_isIncremental),
  m_scalerWidth(rhs.m_scalerWidth),
  m_scalers(rhs.m_scalers)
{}

/*!
  Destruction just delegates.
*/
CRingScalerItem::~CRingScalerItem()
{}

/*!
   Assignment.. just the base class assignment with init():

  \param rhs - The item being assigned to *this.
  \return CRingScalerItem&
  \retval *this
*/
CRingScalerItem&
CRingScalerItem::operator=(const CRingScalerItem& rhs)
{
  if (this != &rhs) {
    m_sourceId = rhs.m_sourceId;
    m_evtTimestamp = rhs.m_evtTimestamp;
    m_intervalStartOffset = rhs.m_intervalStartOffset;
    m_intervalEndOffset = rhs.m_intervalEndOffset;
    m_timestamp = rhs.m_timestamp;
    m_intervalDivisor = rhs.m_intervalDivisor;
    m_isIncremental = rhs.m_isIncremental;
    m_scalerWidth = rhs.m_scalerWidth;
    m_scalers = rhs.m_scalers;
  }

  return *this;
}
/*!
    Equality comparison.. there's no point in looking a the pointers.
    so the base class comparison is just fine.
    \param rhs - Item being compared to *this.
    \return int
    \retval 0 - Not equal
    \retval 1 - Equal
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
   Inequality is the logical inverse of equality.
   \rhs - Item being compared to *thisl
   \return int
   \retval 0 - not unequal
   \retval 1 - unequal
*/
bool
CRingScalerItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}
    
/////////////////////////////////////////////////////////////////////////////////////
//
//  Accessors - selectors and mutators.
//


// Accessor member functions.
void      CRingScalerItem::setType(uint32_t type) {
    if (type != PERIODIC_SCALERS) {
        throw std::runtime_error("DAQ::V12::CRingScalerItem::setType() Type must remain PERIODIC_SCALERS.");
    }
}

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

bool      CRingScalerItem::isComposite() const
{
    return false;
}

bool      CRingScalerItem::mustSwap() const
{
    // this is never swapped. If the raw buffer is swapped at construction
    // it is handled then. The contents of objects belonging to this class
    // are always in the native byte ordering.
    return false;
}

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
 * Determine the floating point start timein seconds using the btime and
 * divisor values.
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
 * Compute the interval end time using both the etime and the divisor.
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
  \throw CRangError if channel is too big.
*/
void
CRingScalerItem::setScaler(uint32_t channel, uint32_t value)
{
    throwIfInvalidChannel(channel, "Attempting to set a scaler value");
    m_scalers[channel] = value;
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
  \throw CRangeError - if the channel number is out of range.
*/
uint32_t
CRingScalerItem::getScaler(uint32_t channel) const
{
    throwIfInvalidChannel(channel, "Attempting to get a scaler value");
    return m_scalers[channel];
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
  
/////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities


/*-------------------------------------------------------
** Virtual method overrides
*-------------------------------------------------------*/

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
  return std::string("Scaler");
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

  float end   = computeEndTime();
  float start = computeStartTime();
  time_t rawTime(getTimestamp());
  string   time  = std::ctime(&rawTime);
  vector<uint32_t> scalers = getScalers();
//  for (int i =0; i < scalers.size(); i++) {
//    scalers[i] = scalers[i] & m_ScalerFormatMask; // Mask off unused bits.
//  }

  float   duration = end - start;

  out << time << " : Scalers:\n";
  out << "Interval start time: " << start << " end: " << end << " seconds in to the run\n\n";
  out << headerToString(*this);
  out << (isIncremental() ? "Scalers are incremental" : "Scalers are not incremental") << std::endl;

  out << "Index         Counts                 Rate\n";
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
  size_t  size    = (7+m_scalers.size())*sizeof(uint32_t);
  return  size;
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
