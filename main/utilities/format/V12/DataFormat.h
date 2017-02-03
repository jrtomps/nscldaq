#ifndef DAQ_V12_DATAFORMAT_H
#define DAQ_V12_DATAFORMAT_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
    Jeromy Tompkins
    Ron Fox
         NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*!
  \file DataFormat.h
  This file contains typedefs for the structures that represent leaf ring items.
  An item has the structure:

\verbatim
 +----------------------------------+
 |  Size of item in bytes (32 bits) |
 +----------------------------------+
 |  32 bit type code of item        |
 +----------------------------------+
 |  64 bit event timestamp          |
 +----------------------------------+
 |  32 bit source id                |
 +----------------------------------+
 |  body (size - 8 bytes of data    |
 +----------------------------------+
\endverbatim

 Where the 32 bit type code is really a 15 bit type code stored in the lower 15 bits of the
32 bit word in the native byte ordering of the originating system.  This allows it to serve as 
a byte order indicator, as data type 0 is not legal, and the top bits of the type code must
be zero.

New in version 12.0 is the ability for ring items to nest. As a result, a ring item can
contain a series of ring items in its body. There is therefore a distinction between
ring items that are leafs and composites. You can think of a ring item as representing
a tree structure. Often we like to diagram these as shown below:

\verbatim

                * (composite)
               / \
  (composite) *   * (leaf)
             / \
     (leaf) *   * (leaf)

\endverbatim

Each node in the graph has an associated ring item header that specifies a size,
type, source id, and event timestamp. The body of a composite is just a series of
ring items, whereas the body of a leaf has some predefined structure.

Composite and leaf types are differentiated by bit 16 of the type code. If set,
the item is composite. Otherwise, it is a leaf. The concept of type consistency is also
part of version 12.0. The rule of type consistency is that all children of a parent
ring item must have the same fundamental type (only least significant 15 bits included).

In version 12.0, the support for the C-style structs is limited. The user is encouraged
to use the CRingItem class and its derived types. Those classes are suitable for IO
and general usage moreso than the structs provided in this header.

*/

/*
   Define the type codes for the items.
   Applications can add user specific types if they use values that are at least
   FIRST_USER_TYPE
*/


#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
namespace DAQ {
  namespace V12 {
#endif

/*
    11.0 and later define a format item that starts the run.
    so that decoders know what format the ring is in.
*/

static const uint16_t FORMAT_MAJOR  = 12;  /* nscldaq-12. */
static const uint16_t FORMAT_MINOR  =  0;  /* nscldaq-x.0 */

static const uint32_t VOID = 0;

/* state change item type codes: */

static const uint32_t BEGIN_RUN       = 0x0001;
static const uint32_t COMP_BEGIN_RUN  = 0x8001;
static const uint32_t END_RUN         = 0x0002;
static const uint32_t COMP_END_RUN    = 0x8002;
static const uint32_t PAUSE_RUN       = 0x0003;
static const uint32_t COMP_PAUSE_RUN  = 0x8003;
static const uint32_t RESUME_RUN      = 0x0004;
static const uint32_t COMP_RESUME_RUN = 0x8004;

// Not quite a state change since we don't know anything about what happened.

static const uint32_t ABNORMAL_ENDRUN      = 0x0005;
static const uint32_t COMP_ABNORMAL_ENDRUN = 0x8005;

/*  Documentation item type codes: */

static const uint32_t PACKET_TYPES              = 0x000a;
static const uint32_t COMP_PACKET_TYPES         = 0x800a;
static const uint32_t MONITORED_VARIABLES       = 0x000b;
static const uint32_t COMP_MONITORED_VARIABLES  = 0x800b;
static const uint32_t RING_FORMAT               = 0x000c; /* Has format major/minor in it. */
static const uint32_t COMP_RING_FORMAT          = 0x800c; 

/* Scaler data: */

static const uint32_t PERIODIC_SCALERS      = 0x0014;
static const uint32_t COMP_PERIODIC_SCALERS = 0x8014;

/* Physics events: */

static const uint32_t PHYSICS_EVENT             = 0x001e;
static const uint32_t COMP_PHYSICS_EVENT        = 0x801e;
static const uint32_t PHYSICS_EVENT_COUNT       = 0x001f;
static const uint32_t COMP_PHYSICS_EVENT_COUNT  = 0x801f;


/* Event builder related items: */

static const uint32_t EVB_GLOM_INFO       = 0x002a; /* GLOM Parameters.                            */
static const uint32_t COMP_EVB_GLOM_INFO  = 0x802a; 

/* User defined item codes */

static const uint32_t FIRST_USER_ITEM_CODE      = 0x4000; /* 0x8000 */
static const uint32_t COMP_FIRST_USER_ITEM_CODE = 0xc000; /* 0x8000 */
                                                      
/* Glom can assign the timestamp policy as follows: */

static const uint16_t GLOM_TIMESTAMP_FIRST   =  0;
static const uint16_t GLOM_TIMESTAMP_LAST    =  1;
static const uint16_t GLOM_TIMESTAMP_AVERAGE =  2;


/* Default timestamp for all types that gets special treatment by the event builder */
static const uint64_t NULL_TIMESTAMP = UINT64_MAX;



// Macro to make packed structs:


#define PSTRUCT struct __attribute__((__packed__))


/*!  All ring items have common header structures: */

typedef PSTRUCT _RingItemHeader {
  uint32_t     s_size;
  uint32_t     s_type;
  uint64_t     s_timestamp;
  uint32_t     s_sourceId;
} RingItemHeader, *pRingItemHeader;


/*!
  Run state changes are documented by inserting state change items that have the
  structure shown below.
*/
typedef PSTRUCT _StateChangeItemBody {
  uint32_t        s_runNumber;
  uint32_t        s_timeOffset;
  uint32_t        s_Timestamp;
  uint32_t        s_offsetDivisor;
  uint32_t        s_titleSize;
  char*           s_title;
} StateChangeItemBody, *pStateChangeItemBody;


/*!
   Scaler items contain run time counters.
*/

typedef PSTRUCT _ScalerItemBody {
  uint32_t        s_intervalStartOffset;
  uint32_t        s_intervalEndOffset;
  uint32_t        s_timestamp;
  uint32_t        s_intervalDivisor;  /* 11.0 sub second time intervals */
  uint32_t        s_scalerCount;
  uint32_t        s_isIncremental;    /* 11.0 non-incremental scaler flag */
  uint32_t        s_scalerWidth;
  uint32_t*       s_scalers;
} ScalerItemBody, *pScalerItemBody;


/*!
  The various documentation Events are just a bunch of null terminated strings that
  are back to back in the body of the ring buffer. item.
*/

typedef PSTRUCT _TextItemBody {
  uint32_t       s_timeOffset;
  uint32_t       s_timestamp;
  uint32_t       s_stringCount;
  uint32_t       s_offsetDivisor;
  char**         s_strings;
} TextItemBody, *pTextItemBody;

/*!
  For now a physics event is just a header and a body of raw bytes.
*/

typedef PSTRUCT _PhysicsEventBody {
  uint8_t*      s_body;
} PhysicsEventBody, *pPhysicsEventBody;

/*!
   Clients that sample physics events will often
   need to know how many physics events have been produced
   so that they can figure out the sampling fraction.
*/
typedef PSTRUCT __PhysicsEventCountItemBody {
  uint32_t       s_timeOffset;
  uint32_t       s_offsetDivisor;
  uint32_t       s_timestamp;
  uint64_t       s_eventCount;	/* Maybe 4Gevents is too small ;-) */
} PhysicsEventCountItemBody, *pPhysicsEventCountItemBody;

/**
 * The ring item format never has an event header.  Just major and minor
 * version numbers:
 */

typedef PSTRUCT _DataFormatBody {
    uint16_t       s_majorVersion;     /* default:FORMAT_MAJOR */
    uint16_t       s_minorVersion;     /* default:FORMAT_MINOR */
} DataFormatBody, *pDataFormatBody;

/**
 *  Information about glom parameters:
 */
typedef PSTRUCT _GlomParametersBody  {
    uint64_t       s_coincidenceTicks;
    uint16_t       s_isBuilding;
    uint16_t       s_timestampPolicy;
} GlomParametersBody, *pGlomParametersBody;


/*!
  This is a generic ring item.
*/

typedef PSTRUCT _RingItem {
    RingItemHeader s_header;
    union {
        union {
            StateChangeItemBody       s_stateChange;
            ScalerItemBody            s_scaler;
            TextItemBody              s_text;
            PhysicsEventBody          s_event;
            PhysicsEventCountItemBody s_physCount;
            DataFormatBody            s_format;
            GlomParametersBody        s_glom;
        } s_leafBody;
        _RingItem** s_compositeBody;
    } s_body;
} RingItem, *pRingItem;

  } // end of V12 namespace
} // end DAQ

#endif

