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

#include "glom.h"
#include "fragment.h"
#include "fragio.h"
#include <iostream>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <DataFormat.h>
#include <CRingItemFactory.h>
#include <exception>
#include <CAbnormalEndItem.h>

// File scoped  variables:

static uint64_t firstTimestamp;
static uint64_t lastTimestamp;
static uint64_t timestampSum;
static uint64_t fragmentCount;
static uint32_t sourceId;

static bool     firstEvent(true);
static uint8_t*        pAccumulatedEvent(0);
static size_t          totalEventSize(0);
static bool            nobuild(false);
static enum enum_timestamp_policy timestampPolicy;
static unsigned        stateChangeNesting(0);

/**
 * outputGlomParameters
 *
 * Output a GlomParameters ring item that describes how we are operating.
 *
 * @param dt - Build time interval.
 * @param building - True if building.
 */
static void
outputGlomParameters(uint64_t dt, bool building)
{
    pGlomParameters p = formatGlomParameters(dt, building ? 1 : 0,
                                             timestampPolicy);
    io::writeData(STDOUT_FILENO, p, p->s_header.s_size);
}

/**
 * flushEvent
 * 
 * Flush the physics event that has been accumulated
 * so far.
 *
 * If nothing has been accumulated, this is a noop.
 *
 */
static void
flushEvent()
{
  if (totalEventSize) {
    
    // Figure out which timestamp to use in the generated event:
    
    uint64_t eventTimestamp;
    switch (timestampPolicy) {
        case timestamp_policy_arg_earliest:
            eventTimestamp = firstTimestamp;
            break;
        case timestamp_policy_arg_latest:
            eventTimestamp = lastTimestamp;
            break;
        case timestamp_policy_arg_average:
            eventTimestamp = (timestampSum/fragmentCount);
            break;
        default:
            // Default to earliest...but should not occur:
            eventTimestamp = firstTimestamp;
            break;
    }
    
    RingItemHeader header;
    BodyHeader     bHeader;
    bHeader.s_size      = sizeof(BodyHeader);
    bHeader.s_timestamp = eventTimestamp;
    bHeader.s_sourceId  = sourceId;
    bHeader.s_barrier   = 0;
    
    
    header.s_size = totalEventSize + sizeof(header) + sizeof(uint32_t) + sizeof(BodyHeader);
    header.s_type = PHYSICS_EVENT;
    uint32_t eventSize = totalEventSize + sizeof(uint32_t);

    io::writeData(STDOUT_FILENO, &header, sizeof(header));
    io::writeData(STDOUT_FILENO, &bHeader, sizeof(BodyHeader));
    io::writeData(STDOUT_FILENO, &eventSize,  sizeof(uint32_t));
    io::writeData(STDOUT_FILENO, pAccumulatedEvent, 
		  totalEventSize);
    free(pAccumulatedEvent);
    pAccumulatedEvent = 0;
    totalEventSize    = 0;
    firstEvent        = true;
  }
}
/**
 * outputBarrier
 *
 *  Outputs a barrier event. The ring item type of a barrier
 *  depends:
 *  - If the payload can be determined to be a ring item,
 *    it is output as is.
 *  - If the payload can't be determined to be a ring item,
 *    the entire fragment, header and all is bundled
 *    into a ring item of type EVB_UNKNOWN_PAYLOAD
 *    this is an extension that hopefully helps us deal with
 *    non NSCL DAQ things.
 *
 * @param p - Pointer to the ring item.
 *
 */
static void
outputBarrier(EVB::pFragment p)
{
  if(CRingItemFactory::isKnownItemType(p->s_pBody)) {
    
    // This is correct if there is or isn't a body header in the payload
    // ring item.
    
    pRingItemHeader pH = 
      reinterpret_cast<pRingItemHeader>(p->s_pBody);
    io::writeData(STDOUT_FILENO, pH, pH->s_size);
    
    if (pH->s_type == BEGIN_RUN) stateChangeNesting++;
    if (pH->s_type == END_RUN)   stateChangeNesting--;
    if (pH->s_type == ABNORMAL_ENDRUN) stateChangeNesting = 0;

  } else {
    RingItemHeader unknownHdr;
    unknownHdr.s_type = EVB_UNKNOWN_PAYLOAD;
    //
    // Size is the fragment header + ring header + payload.
    // 
    uint32_t size = sizeof(RingItemHeader) +
      sizeof(EVB::FragmentHeader) + p->s_header.s_size;
    unknownHdr.s_size = size;

    io::writeData(STDOUT_FILENO, &unknownHdr, sizeof(RingItemHeader));
    io::writeData(STDOUT_FILENO, p, sizeof(EVB::FragmentHeader));
    io::writeData(STDOUT_FILENO, p->s_pBody, p->s_header.s_size);
  }
}
/**
 * emitAbnormalEnd
 *    Emits an abnormal end run item.
 */
void emitAbnormalEnd()
{
    CAbnormalEndItem end;
    pRingItem pItem= end.getItemPointer();
    EVB::Fragment frag = {{NULL_TIMESTAMP, 0xffffffff, pItem->s_header.s_size, 0}, pItem};
    outputBarrier(&frag);
}

/**
 * acumulateEvent
 * 
 *  This function is the meat of the program.  It
 *  glues fragments together (header and payload)
 *  into a dynamically resized chunk of memory pointed
 *  to by pAccumulatedEvent where  totalEventSize 
 *  is the number of bytes that have been accumulated 
 *  so far.
 *
 *  firstTimestamp is the timestamp of the first fragment
 *  in the acccumulated data.though it is only valid if 
 *  firstEvent is false.
 *
 *  Once the event timestamp exceeds the coincidence
 *  interval from firstTimestamp, the event is flushed
 *  and the process starts all over again.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void
accumulateEvent(uint64_t dt, EVB::pFragment pFrag)
{
  // See if we need to flush:

  uint64_t timestamp = pFrag->s_header.s_timestamp;
  if (nobuild || (!firstEvent && ((timestamp - firstTimestamp) > dt))) {
    flushEvent();
  }
  // If firstEvent...our timestamp starts the interval:

  if (firstEvent) {
    firstTimestamp = timestamp;
    firstEvent     = false;
    fragmentCount  = 0;
    timestampSum   = 0;
  }
  lastTimestamp    = timestamp;
  fragmentCount++;
  timestampSum    += timestamp;
  
  // Figure out how much we're going to add to the
  // event:

  uint32_t fragmentSize = sizeof(EVB::FragmentHeader) +
    pFrag->s_header.s_size;

  // expand the event (or allocate it) and append
  // this data to it.

  uint8_t* pEvent  = 
    reinterpret_cast<uint8_t*>(realloc(pAccumulatedEvent, 
					totalEventSize + fragmentSize));
  uint8_t* pAppendPointer = pEvent + totalEventSize;
  memcpy(pAppendPointer, &(pFrag->s_header), 
	 sizeof(EVB::FragmentHeader));
  pAppendPointer += sizeof(EVB::FragmentHeader);
  memcpy(pAppendPointer, pFrag->s_pBody, 
	 pFrag->s_header.s_size);

  // finish off the book keeping;

  totalEventSize += fragmentSize;
  pAccumulatedEvent = pEvent;

}

static void outputEventFormat()
{
    DataFormat format;
    format.s_header.s_size = sizeof(DataFormat);
    format.s_header.s_type = RING_FORMAT;
    format.s_mbz         = 0;
    format.s_majorVersion = FORMAT_MAJOR;
    format.s_minorVersion = FORMAT_MINOR;
    
    io::writeData(STDOUT_FILENO, & format, sizeof(format));
}

/**
 * Main for the glommer
 * - Parse the arguments and extract the dt.
 * - Until EOF on input, or error, get fragments from stdin.
 * - If fragments are not barriers, accumulate events
 * - If fragments are barriers, flush any accumulated 
 *   events and output the barrier body as a ring item.
 *
 * @param argc - Number of command line parameters.
 * @param argv - array of pointers to the parameters.
 */
int
main(int argc, char**  argv)
{
  // Parse the parameters;

  gengetopt_args_info args;
  cmdline_parser(argc, argv, &args);
  int dtInt = static_cast<uint64_t>(args.dt_arg);
  nobuild      = args.nobuild_given;
  timestampPolicy = args.timestamp_policy_arg;
  sourceId       = args.sourceid_arg;

  outputEventFormat();
  

  std::cerr << (nobuild ? " glom: not building " : "glom: building") << std::endl;

  if (!nobuild && (dtInt < 0)) {
    std::cerr << "Coincidence window must be >= 0 was "
	      << dtInt << std::endl;
    exit(-1);
  }
  uint64_t dt = static_cast<uint64_t>(dtInt);
  nobuild      = args.nobuild_flag;

  /*
     main loop.. .get fragments and handle them.
     two targets for a fragment:
     accumulateEvent - for non-barriers.
     outputBarrier   - for barriers.
  */

  bool firstBarrier(true);
  try {
    while (1) {
      EVB::pFragment p = CFragIO::readFragment(STDIN_FILENO);
      
      // If error or EOF flush the event and break from
      // the loop:
      
      if (!p) {
	flushEvent();
	std::cerr << "glom: EOF on input\n";
        if(stateChangeNesting) {
            emitAbnormalEnd();
        }
	break;
      }
      // We have a fragment:
      
      if (p->s_header.s_barrier) {
	flushEvent();
	outputBarrier(p);
        // First barrier is most likely the begin run...put the glom parameters
        // right after that.
        
        if(firstBarrier) {
            outputGlomParameters(dtInt, !nobuild);
            firstBarrier = false;
        }
      } else {

	// If we can determine this is a valid ring item other than
	// an event fragment it goes out out of band but without flushing
	// the event.

	if (CRingItemFactory::isKnownItemType(p->s_pBody)) {
	  pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(p->s_pBody);
	  if (pH->s_type == PHYSICS_EVENT) {
	    accumulateEvent(dt, p); // Ring item physics event.
	  } else {
	    outputBarrier(p);	// Ring item non-physics event.
	  }
	} else {		// non ring item..treat like event.
	  accumulateEvent(dt, p);
	}
      }
      freeFragment(p);
    }
  }
  catch (std::string msg) {
    std::cerr << "glom: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "glom: " << msg << std::endl;
  }
  catch (int e) {
    std::string msg = "glom: Integer error: ";
    msg += strerror(e);
    std::cerr << msg << std::endl;
  }
  catch (std::exception& except) {
    std::string msg = "glom: ";
    msg += except.what();
    std::cerr << msg << std::endl;
  }
  catch(...) {
    std::cerr << "Unanticipated exception caught\n";

  }
    // Out of main loop because we need to exit.

  return 0;
}
