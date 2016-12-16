#ifndef DAQ12_DATAFORMAT_H
#define DAQ12_DATAFORMAT_H

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

/*!
  \file DataFormat.h
  This file contains typedefs for the structures that will be put into
  ring buffers for event data.  Event data Ring buffers are filled with items.
  An item has the structure:

\verbatim
 +----------------------------------+
 |  Size of item in bytes (32 bits) |
 +----------------------------------+
 |  32 bit type code of item        |
 +----------------------------------+
 |  body (size - 8 bytes of data    |
 +----------------------------------+
\endverbatim

  Where the 32 bit type code is really a 16 bit type code stored in the lower 16 bits of the 
32 bit word in the native byte ordering of the originating system.  This allows it to serve as 
a byte order indicator, as data type 0 is not legal, and the top bits of the type code must
be zero.

Further as of nscldaq-12.0, each body has a minimal form of:

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

static const uint16_t FORMAT_MAJOR  = 11;  /* nscldaq-11. */
static const uint16_t FORMAT_MINOR  =  0;  /* nscldaq-x.0 */

/* state change item type codes: */
static const uint32_t VOID = 0;

static const uint32_t BEGIN_RUN       = 0x0001;
static const uint32_t COMP_BEGIN_RUN  = 0x8001;
static const uint32_t END_RUN         = 0x0002;
static const uint32_t COM_END_RUN     = 0x8002;
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


/* Note timestamped nonincremental scalers absorbed into incremental scalers. */

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
static const uint16_t GLOM_TIMESTAMP_AVERAGE = 2;

static const uint64_t NULL_TIMESTAMP = UINT64_MAX;


/* Longest allowed title: */


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
  structure shown below.  After 11.0, they may or mey  not have abody header
  as reflected by the fact that they contain a union as shown below:

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
   Scaler items contain run time counters.  As of 11.0 they may or may  not have
   a body header too:
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
  char**           s_strings;
} TextItemBody, *pTextItemBody;

/*!
  For now a physics event is just a header and a body of uint16_t's.
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
    uint16_t       s_majorVersion;     /* FORMAT_MAJOR */
    uint16_t       s_minorVersion;     /* FORMAT_MINOR */
} DataFormat, *pDataFormat;

/**
 *  Information about glom parameters:
 */
typedef PSTRUCT _GlomParameters  {
    RingItemHeader s_header;
    uint32_t       s_mbz;
    uint64_t       s_coincidenceTicks;
    uint16_t       s_isBuilding;
    uint16_t       s_timestampPolicy;   /* See GLOM_TIMESTAMP_* above */
    
} GlomParameters, *pGlomParameters;


/*!
  This  is the most basic item.. a generic item.  It consists only of the
  header and a generic body
*/

typedef PSTRUCT _RingItem {
  RingItemHeader s_header;
  union {
    StateChangeItemBody       s_stateChange;
    ScalerItemBody            s_scaler;
    TextItemBody              s_text;
    PhysicsEventBody          s_event;
    PhysicsEventCountItemBody s_physCount;
    DataFormat                s_format;
    GlomParameters            s_glom;

  } s_body;
} RingItem, *pRingItem;

/**
  Below are functions that are available to format ring types.
  Note that all of these return a pointer that must be free(3)'d.
*/
#ifdef __cplusplus
extern "C" {
#endif

//  pPhysicsEventItem  formatEventItem(size_t nWords, void* pPayload);
//  pPhysicsEventCountItem formatTriggerCountItem(uint32_t runTime, time_t stamp,
//                                                uint64_t triggerCount);
//  pScalerItem         formatScalerItem(unsigned scalerCount, time_t timestamp,
//				      uint32_t btime, uint32_t etime,
//                                      void* pCounters);
//  pScalerItem         formatNonIncrTSScalerItem(unsigned scalerCount, time_t timestamp,
//						       uint32_t btime, uint32_t etime,
//						       uint64_t eventTimestamp, void* pCounters,
//						       uint32_t timebaseDivisor);
//  pTextItem          formatTextItem(unsigned nStrings, time_t stamp, uint32_t runTime,
//				    const char** pStrings, int type);
//  pStateChangeItem   formatStateChange(time_t stamp, uint32_t offset, uint32_t runNumber,
//				       const char* pTitle, int type);
  
//  /* Since 11.0 these functions were added: */
  
//  pDataFormat           formatDataFormat();
//  pGlomParameters       formatGlomParameters(uint64_t coincidenceWindow, int isBuilding,
//                                             int timestampPolicy);
  
//  pEventBuilderFragment formatEVBFragment(
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    uint32_t payloadSize, const void* pPayload
//  );
//  pEventBuilderFragment formatEVBFragmentUnknown (
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    uint32_t payloadSize, const void* pPayload
//  );
//  pPhysicsEventItem formatTimestampedEventItem(
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    uint32_t payloadSize, const void* pPayload
//  );
//  pPhysicsEventCountItem formatTimestampedTriggerCountItem (
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    uint32_t runTime, uint32_t offsetDivisor, time_t stamp, uint64_t triggerCount
//  );
//  pScalerItem formatTimestampedScalerItem(
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    int isIncremental, uint32_t timeIntervalDivisor, uint32_t timeofday,
//    uint32_t btime, uint32_t etime, uint32_t nScalers, void* pCounters
//  );
//  pTextItem formatTimestampedTextItem(
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    unsigned nStrings, time_t stamp, uint32_t runTime,
//    const char**pStrings, int type,
//    uint32_t timeIntervalDivisor
//  );
//  pStateChangeItem formatTimestampedStateChange(
//    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
//    time_t stamp, uint32_t offset, uint32_t runNumber, uint32_t offsetDivisor,
//    const char* pTitle, int type
//  );
//  pAbnormalEndItem formatAbnormalEndItem();
  
//  void* bodyPointer(pRingItem pItem);
//  uint32_t itemSize(const RingItem* pItem);
//  uint16_t itemType(const RingItem* pItem);
//  int      mustSwap(const RingItem* pItem);

#ifdef __cplusplus
} // end of extern

  } // end of V11 namespace
} // end DAQ
#endif

#endif
