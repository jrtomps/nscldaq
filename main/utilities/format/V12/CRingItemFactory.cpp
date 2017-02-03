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
 * value returned by CRawRingItem::type().
 *
 * @param item - Reference to a raw ring item.
 *
 * If the ring item type is recognized, its type will map to particular
 * derived type. The mapping is as follows:
 *
 * +--------------------------+----------------------------+
 * | Input type() value       | Output Class Type          |
 * +--------------------------+----------------------------+
 * | BEGIN_RUN                | CRingStateChangeItem       |
 * +--------------------------+----------------------------+
 * | END_RUN                  | CRingStateChangeItem       |
 * +--------------------------+----------------------------+
 * | PAUSE_RUN                | CRingStateChangeItem       |
 * +--------------------------+----------------------------+
 * | RESUME_RUN               | CRingStateChangeItem       |
 * +--------------------------+----------------------------+
 * | PACKET_TYPES             | CRingTextItem              |
 * +--------------------------+----------------------------+
 * | MONITORED_VARIABLES      | CRingTextItem              |
 * +--------------------------+----------------------------+
 * | PERIODIC_SCALERS         | CRingScalerItem            |
 * +--------------------------+----------------------------+
 * | PHYSICS_EVENT            | CPhysicsEventItem          |
 * +--------------------------+----------------------------+
 * | PHYSICS_EVENT_COUNT      | CRingPhysicsEventCountItem |
 * +--------------------------+----------------------------+
 * | RING_FORMAT              | CDataFormatItem            |
 * +--------------------------+----------------------------+
 * | EVB_BLOM_INFO            | CGlomParameters            |
 * +--------------------------+----------------------------+
 * | ABNORMAL_ENDRUN          | CAbnormalEndItem           |
 * +--------------------------+----------------------------+
 * | COMP_BEGIN_RUN           | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_END_RUN             | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_PAUSE_RUN           | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_RESUME_RUN          | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_PACKET_TYPES        | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_MONITORED_VARIABLES | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_PERIODIC_SCALERS    | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_PHYSICS_EVENT       | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_PHYSICS_EVENT_COUNT | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_ABNORMAL_ENDRUN     | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_RING_FORMAT         | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | COMP_EVB_GLOM_INFO       | CCompositeRingItem         |
 * +--------------------------+----------------------------+
 * | any other types          | CRawRingItem               |
 * +--------------------------+----------------------------+
 *
 * @return CRingItemUPtr (i.e. std::unique_ptr<CRingItem>)
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
 * Determines if a type is known
 *
 * \param type type to check
 *
 * @return bool - true if valid, false otherwise
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
