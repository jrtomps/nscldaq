/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt
*/

/**
 * @file CGlomParameters.cpp
 * @brief  Encapsulates a ring item that contains glom parameters.
 * @author  Jeromy Tompkins
 */
#include "V12/CGlomParameters.h"
#include "V12/DataFormat.h"
#include "V12/CRawRingItem.h"
#include "Deserializer.h"
#include <sstream>

namespace DAQ {
  namespace V12 {

/**
 * @note the definition below requires that the order of array elements matches
 *       the timestamp policy values
 */

static const char* policyNames[4] = {
    "first", "last" , "average", "Error - undefined"
};


/*-------------------------------------------------------------------------
 * Canonical methods
 *-----------------------------------------------------------------------*/

/*!
 * \brief Constructor for the glom parameters
 * \param interval      number of ticks to use for correlation window
 * \param isBuilding    whether glom was correlating events
 * \param policy        how glom assigned a timestamp to the built event
 *
 * The timestamp and source id are implicitly defined to be NULL_TIMESTAMP and
 * 0, respectively.
 */
CGlomParameters::CGlomParameters(uint64_t interval, bool isBuilding,
                                 CGlomParameters::TimestampPolicy policy)
    : CGlomParameters(V12::NULL_TIMESTAMP, 0, interval, isBuilding, policy)
{

}

/**
 * \brief Construct from data explicitly
 *
 * @param interval   - Number of ticks in the event building coincidence window.
 * @param isBuilding - If true the output of glom is built rather than just
 *                     ordered/formatted events.
 * @param policy     - The timestamp policy used by glom.
 */

/*!
 * \brief CGlomParameters::CGlomParameters
 *
 * \param evtTstamp     the event timestamp
 * \param source        the source id
 * \param interval      the number of ticks to use for the correlation window
 * \param isBuilding    whether glom was correlating events
 * \param policy        how glom assigned a timestamp to the built event
 *
 *
 */
CGlomParameters::CGlomParameters(
    uint64_t evtTstamp, uint32_t source, uint64_t interval, bool isBuilding, CGlomParameters::TimestampPolicy policy
) : m_evtTimestamp(evtTstamp),
    m_sourceId(source),
    m_coincTicks(interval),
    m_isBuilding(isBuilding),
    m_policy(policy)
{
}
/**
 * destructor
 */
CGlomParameters::~CGlomParameters()
{
    
}


/**
 * \brief Construct from a raw ring item
 *
 * \param rhs - CRingItem reference from which we will construct.
 *
 * \throws std::bad_cast if the rhs is not representing an EVB_GLOM_INFO item
 * \throws std::invalid_argument if policy is not one of the enumerated values provided
 */
CGlomParameters::CGlomParameters(const CRawRingItem& rhs)
{
    if (rhs.type() != EVB_GLOM_INFO) throw std::bad_cast();

    m_sourceId = rhs.getSourceId();
    m_evtTimestamp = rhs.getEventTimestamp();

    Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(rhs.getBody(), rhs.mustSwap());

    uint16_t temp;
    stream >> m_coincTicks;
    stream >> temp; m_isBuilding = temp;
    stream >> temp;

    switch(temp) {
    case 0:
        m_policy = first;
        break;
    case 1:
        m_policy = last;
        break;
    case 2:
        m_policy = average;
        break;
    default: {
        std::string errmsg("CGlomParameters(const CRawRingItem& ) ");
        errmsg += "Parsed invalid timestamp policy.";
        throw std::invalid_argument(errmsg);
    }
    }
}

/*!
 * \brief Equality comparison operator
 *
 * \param rhs the item being compared to *this
 *
 * \return true  if all data members are not the same
 * \retval false otherwise
 */
bool
CGlomParameters::operator==(const CRingItem& rhs) const
{
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    const auto pItem = dynamic_cast<const CGlomParameters*>(&rhs);
    if (!pItem) return false;
    if (m_coincTicks != pItem->coincidenceTicks()) return false;
    if (m_isBuilding != pItem->isBuilding()) return false;
    if (m_policy != pItem->timestampPolicy()) return false;

    return true;
}
/*!
 * \brief Inequality comparison operator
 *
 * @param rhs the item being compared to *this.
 *
 * @return true if at least one data member is not the same
 * \retval false otherwise
 */
bool
CGlomParameters::operator!=(const CRingItem& rhs) const
{
    return ! (*this == rhs );
}

/*!
 * Always returns V12::EVB_GLOM_INFO
 * \return V12::EVB_GLOM_INFO
 */
uint32_t CGlomParameters::type() const
{
    return EVB_GLOM_INFO;
}

/*!
 * \brief CGlomParameters::setType
 * \param type  must be V12::EVB_GLOM_INFO
 *
 * CGlomParameters objects _must_ always have the type V12::EVB_GLOM_INFO
 *
 * \throws std::invalid_argument if called with type other than V12::EVB_GLOM_INFO
 */
void CGlomParameters::setType(uint32_t type)
{
    if (type != EVB_GLOM_INFO) {
        std::string errmsg("CGlomParameters::setType() only accepts V12;:EVB_GLOM_INFO.");
        throw std::invalid_argument(errmsg);
    }
}

/*! \return 32 (always the same number of bytes) */
uint32_t CGlomParameters::size() const
{
    return 32;
}

uint32_t CGlomParameters::getSourceId() const
{
    return m_sourceId;
}

void CGlomParameters::setSourceId(uint32_t id)
{
    m_sourceId = id;
}


uint64_t CGlomParameters::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CGlomParameters::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}

/*!
 * The data members are always stored in native byte order.
 *
 * \return false
 */
bool CGlomParameters::mustSwap() const
{
    return false;
}

/*!
 * The CGlomParateters class always represents a leaf type
 *
 * \return false
 */
bool CGlomParameters::isComposite() const
{
    return false;
}

/*!
 * \brief Serialize the data into a raw ring item
 *
 * \param raw   the ring item to be filled
 */
void CGlomParameters::toRawRingItem(CRawRingItem& raw) const
{
    raw.setType(EVB_GLOM_INFO);
    raw.setSourceId(m_sourceId);
    raw.setEventTimestamp(m_evtTimestamp);

    auto& body = raw.getBody();
    body.clear();

    body << m_coincTicks;
    body << (m_isBuilding ? uint16_t(1) : uint16_t(0));
    body << uint16_t(m_policy);

}



/**
 * coincidenceTicks
 *
 * @return uint64_t - the number of ticks glom used in its coincidence window
 *                    this is only meaningful if isBuilding() returns
 *                    true.
 */
uint64_t
CGlomParameters::coincidenceTicks() const
{    
    return m_coincTicks;
}
/**
 * isBuilding
 *
 * @return bool - true if glom is glueing event fragments together.
 */
bool
CGlomParameters::isBuilding() const
{
    return m_isBuilding;
}
/**
 * timestampPolicy
 *
 * @return CGlomParameters::TimestampPolicy - the timestamp policy from
 *         the ring item.
 */
CGlomParameters::TimestampPolicy
CGlomParameters::timestampPolicy() const
{
    return static_cast<TimestampPolicy>(m_policy);
}
/*---------------------------------------------------------------------------
 * Object methods
 *-------------------------------------------------------------------------*/

/*!
 * @return std::string - textual version of the ring type.
 */
std::string
CGlomParameters::typeName() const
{
    return std::string("Glom Parameters");
}
/**
 * \brief Convert the data to a string representation
 *
 * @return std::string - a textual dump of the ring item contents.
 */
std::string
CGlomParameters::toString() const
{
    std::stringstream out;
    
    out << headerToString(*this);
    out << "Is Building? : " << (isBuilding() ? "Yes" : "No" ) << "\n";
    if (isBuilding()) {
        out << "Coinc. Width : " << coincidenceTicks() << std::endl;

        unsigned tsPolicy = static_cast<unsigned>(timestampPolicy());
        if (tsPolicy >= sizeof(policyNames)/sizeof(char*)) {
            tsPolicy = sizeof(policyNames)/sizeof(char*) - 1;
        }
        out << "Tstamp Policy: " << policyNames[tsPolicy] << "\n";
    }
    return out.str();
}

  } // end of V12 namespace
} // end DAQ
