/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include "V12/CRingItemFactory.h"
#include "V12/CRingItem.h"
#include "V12/CPhysicsEventItem.h"
#include "V12/CRingPhysicsEventCountItem.h"
#include "V12/CRingScalerItem.h"
#include "V12/CRingStateChangeItem.h"
#include "V12/CRingTextItem.h"
#include "V12/CPhysicsEventItem.h"
#include "V12/CGlomParameters.h"
#include "V12/CAbnormalEndItem.h"
#include "V12/CCompoundRingItem.h"
#include "V12/CDataFormatItem.h"
#include "V12/DataFormat.h"

#include <vector>
#include <string>
#include <string.h>
#include <set>

namespace DAQ {
  namespace V12 {

static std::set<uint32_t> knownItemTypes;

/**
 * Create a ring item of the correct underlying type as indicated by the
 * ring item type.  Note that the result is dynamically allocated and must be
 * freed by the caller (via delete).
 *
 * @param item - Reference to the item.
 * 
 * @return CRingItem*
 * @retval Pointer to an object that is some subclass of CRingItem.  Note that if the
 *         ring item type is not recognized, a true CRing Item is produced.
 *
 */
std::unique_ptr<CRingItem>
CRingItemFactory::createRingItem(const CRawRingItem& item)
{

    switch (item.type()) {
    // State change:

    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
    {
        return item.as<CRingStateChangeItem>();
    }

        // String list.

    case PACKET_TYPES:
    case MONITORED_VARIABLES:
        return item.as<CRingTextItem>();
        // Scalers:

    case PERIODIC_SCALERS:
        return item.as<CRingScalerItem>();

        // Physics trigger:

    case PHYSICS_EVENT:
        return item.as<CPhysicsEventItem>();
        // trigger count.

    case PHYSICS_EVENT_COUNT:
        return item.as<CRingPhysicsEventCountItem>();

    case RING_FORMAT:
        return item.as<CDataFormatItem>();

    case EVB_GLOM_INFO:
        return item.as<CGlomParameters>();

    case ABNORMAL_ENDRUN:
        return item.as<CAbnormalEndItem>();

    case COMP_BEGIN_RUN:
    case COMP_END_RUN:
    case COMP_PAUSE_RUN:
    case COMP_RESUME_RUN:
    case COMP_PACKET_TYPES:
    case COMP_MONITORED_VARIABLES:
    case COMP_PERIODIC_SCALERS:
    case COMP_PHYSICS_EVENT:
    case COMP_PHYSICS_EVENT_COUNT:
    case COMP_ABNORMAL_ENDRUN:
    case COMP_RING_FORMAT:
    case COMP_EVB_GLOM_INFO:
        return item.as<CCompositeRingItem>();

    default:
        return item.as<CRawRingItem>();
    }
}

/**
 * Determines if a  pointer points to something that might be a valid ring item.
 * - Item must have at least the size of a header.
 * - Item must be a recognized ring item.
 *
 * @param pItem - pointer to the data.
 *
 * @return bool - true if we htink this is a valid ring item.
 */
bool
CRingItemFactory::isKnownItemType(const uint32_t type)
{

  // if necessary stock the set of known item types:

  if(knownItemTypes.empty()) {
    knownItemTypes.insert(BEGIN_RUN);
    knownItemTypes.insert(END_RUN);
    knownItemTypes.insert(PAUSE_RUN);
    knownItemTypes.insert(RESUME_RUN);
    knownItemTypes.insert(RING_FORMAT);

    knownItemTypes.insert(PACKET_TYPES);
    knownItemTypes.insert(MONITORED_VARIABLES);

    knownItemTypes.insert(PERIODIC_SCALERS);
    knownItemTypes.insert(PHYSICS_EVENT);
    knownItemTypes.insert(PHYSICS_EVENT_COUNT);

    knownItemTypes.insert(EVB_GLOM_INFO);
    
    knownItemTypes.insert(ABNORMAL_ENDRUN);
  }

  return knownItemTypes.count(type) > 0;
}

  } // end of V12 namespace
} // end DAQ
