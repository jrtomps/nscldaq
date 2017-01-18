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


CGlomParameters::CGlomParameters(uint64_t interval, bool isBuilding,
                                 CGlomParameters::TimestampPolicy policy)
    : CGlomParameters(V12::NULL_TIMESTAMP, 0, interval, isBuilding, policy)
{

}




/**
 * constructor
 *
 * This is the 'normal' constructor.  It builds a fully fledged glom parameters
 * data item.
 *
 * @param interval   - Number of ticks in the event building coincidence window.
 * @param isBuilding - If true the output of glom is built rather than just
 *                     ordered/formatted events.
 * @param policy     - The timestamp policy used by glom.
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
 * copy constructor - generic
 *
 * If the rhs is not an EVB_GLOM_INFO item, we'll throw a bad_cast.
 *
 * @param rhs - CRingItem reference from which we will construct.
 */
CGlomParameters::CGlomParameters(const CRawRingItem& rhs)
{
    if (type() != EVB_GLOM_INFO) throw std::bad_cast();        
}
/**
 * operator==
 *
 * @param rhs the item being compared to *this
 *
 * @return int - nonzero if equality.
 */
int
CGlomParameters::operator==(const CGlomParameters& rhs) const
{
    return 1;
}
/**
 * operator!=
 *
 * @param rhs the item being compared to *this.
 *
 * @return int  non zero if not equal.
 */
int
CGlomParameters::operator!=(const CGlomParameters& rhs) const
{
    return ! (*this == rhs );
}

/*----------------------------------------------------------------------------
 * Selectors
 *--------------------------------------------------------------------------*/


uint32_t CGlomParameters::type() const
{
    return EVB_GLOM_INFO;
}

void CGlomParameters::setType(uint32_t type)
{
    if (type != EVB_GLOM_INFO) {
        std::string errmsg("CGlomParameters::setType() only accepts V12;:EVB_GLOM_INFO.");
        throw std::invalid_argument(errmsg);
    }
}

uint32_t CGlomParameters::size() const
{
    return 36;
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

bool CGlomParameters::mustSwap() const
{
    return false;
}


bool CGlomParameters::isComposite() const
{
    return false;
}

void CGlomParameters::toRawRingItem(CRawRingItem& raw) const
{

}



/**
 * coincidenceTicks
 *
 * @return uint64_t - the number of ticks glom used in its coincidence window
 *                    this is meaningful, however only if isBuilding() returns
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

/**
 * typeName
 *
 * @return std::string - textual version of the ring type.
 */
std::string
CGlomParameters::typeName() const
{
    return std::string("Glom Parameters");
}
/**
 * toSTring
 *
 * @return std::string - a textual dump of the ring item contents.
 */
std::string
CGlomParameters::toString() const
{
    CGlomParameters* This = const_cast<CGlomParameters*>(this);
    std::stringstream    out;
    
    out << headerToString(*this) << std::endl;
    out << "Glom is " << (isBuilding() ? "" : " not ") << "building events\n";
    if (isBuilding()) {
        out << "Event building coincidence window is: "
            << coincidenceTicks() << " timestamp ticks\n";
    }
    unsigned tsPolicy = static_cast<unsigned>(timestampPolicy());
    if (tsPolicy >= sizeof(policyNames)/sizeof(char*)) {
        tsPolicy = sizeof(policyNames)/sizeof(char*) - 1;
    }
    out << "TimestampPolicy : policyNames[tsPolicy]\n";
    
    return out.str();
}

  } // end of V11 namespace
} // end DAQ
