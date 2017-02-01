/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CAbnormalEndItem.cpp
# @brief  Implements the abnormal end run ring item class.
# @author <fox@nscl.msu.edu>
*/

#include "V12/CAbnormalEndItem.h"
#include "V12/CRawRingItem.h"
#include "V12/DataFormat.h"
#include <typeinfo>
#include <string>

namespace DAQ {
  namespace V12 {

CAbnormalEndItem::CAbnormalEndItem(const CRawRingItem& rhs)
{
    if(rhs.type() != ABNORMAL_ENDRUN) {
        throw std::bad_cast();
    }

    m_evtTimestamp = rhs.getEventTimestamp();
    m_sourceId     = rhs.getSourceId();
}

/**
 * operator==
 *   all abnormal end items are the same.
 * @return true
 */
bool
CAbnormalEndItem::operator==(const CRingItem& rhs) const
{
    if (ABNORMAL_ENDRUN != rhs.type()) return false;
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    return true;
}

/**
 * operator!=
 *  @return !=-
 */
bool
CAbnormalEndItem::operator!=(const CRingItem& rhs) const
{
    return !operator==(rhs);
}


uint32_t CAbnormalEndItem::size() const
{
    return 20;
}

uint32_t CAbnormalEndItem::type() const
{
    return ABNORMAL_ENDRUN;
}

void CAbnormalEndItem::setType(uint32_t type)
{
    if (type != ABNORMAL_ENDRUN) {
        std::string msg("CAbnormalEndRun::setType() only accepts the V12::ABNORMAL_ENDRUN type.");
        throw std::invalid_argument(msg);
    }
}

uint64_t CAbnormalEndItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CAbnormalEndItem::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}

void CAbnormalEndItem::setSourceId(uint32_t id) {
    m_sourceId = id;
}

uint32_t CAbnormalEndItem::getSourceId() const {
    return m_sourceId;
}

bool CAbnormalEndItem::isComposite() const
{
    return false;
}

bool CAbnormalEndItem::mustSwap() const
{
    return false;
}

void CAbnormalEndItem::toRawRingItem(CRawRingItem& item) const
{
    // clear out prior contents of raw ring item body
    item.getBody().clear();

    item.setType(type());
    item.setEventTimestamp(getEventTimestamp());
    item.setSourceId(getSourceId());
}


/* Formatting interface */

/**
 * typeName
 *    Returns a textual name of the item type
 * @return std::string
 */
std::string
CAbnormalEndItem::typeName() const
{
    return "Abnormal End";
}
/**
 * toString
 *
 *   Return a nicely formatted rendition of the ring item.
 * @return std::string
 */
std::string
CAbnormalEndItem::toString() const
{
    std::string result = typeName();
    result += "\n";
    return result;
}

  } // end of V12 namespace
} // end DAQ
