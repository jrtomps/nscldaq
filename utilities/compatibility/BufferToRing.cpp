/**
// Purpose:
//   Convert NSCL buffered data to ring buffer data.  This is a filter specifically,
//   data buffera are processed on stdin and sent to stdout in a format that is 
//   usable by stdinto ring.
//    
*/
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



#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <time.h>
#include <set>
#include <map>
#include <io.h>

#include <buffer.h>
#include <buftypes.h>
#include <DataFormat.h>



static size_t       BUFFERSIZE(8192); // (in bytes).
static uint64_t     eventsInRun(0);   // For PhysicsEventcountItems.

static std::set<int>                 okErrors;	// Acceptable errors in I/O operations.
static std::map<uint16_t, uint32_t>  textTypeMap; // map of text buffer types -> ring buffer item types.
static std::map<uint16_t, uint32_t>  stateTypeMap; // Same as above but for state change buffers.

// Counter used for run statistics:

static uint64_t triggers;




/**
 * Get a buffer of data from stdin (STDIN_FILENO).
 * If necessary multiple read() operation are performed to deal
 * with potential buffering between the source an us (e.g. we are typically
 * on the ass end of a pipe where the pipe buffer may be smaller than an
 * event buffer.
 * @param pBuffer - Pointer to a buffer big enough to hold the event buffer.
 * @param size    - Number of bytes in the buffer.
 *
 * @return bool
 * @retval true - The buffer was read successfully.
 * @retval false - the buffer was not read successfully.
 */
bool getBuffer (uint16_t* pBuffer,  size_t nBytes)
{
  size_t nread;
  try {
    nread = io::readData(STDIN_FILENO, pBuffer, nBytes);
    if (nread < nBytes) return false;
  }
  catch(int e) {
    if (e) {
      fprintf(stderr, "%s : %s\n",
	      "Error on read: ", strerror(e));
    } else {
      fprintf(stderr, "%s\n", "End of file on stdin in mid read");
    }
    return false;
  }
  return true;
}

/**
 * Write a block of data to stdout.
 * As with getBuffer, multiple writes are done..until either the entire data
 * are written or
 * *  A write ends in an eof condition.
 * *  A write ends in an error condition that is not EAGAIN, EWOUDLDBLOCK or EINTR.
 *
 * @param pData - Pointer to the data to write.
 * @param size  - Number of words of data to write.
 * 
 * @return bool 
 * @retval - true on success, false otherwise.
 */

bool writeData (void* pData , size_t size)
{

  try {
    io::writeData(STDOUT_FILENO, pData, size);
  }
  catch(int e) {
    if (e) {
      fprintf(stderr, "%s : %s\n",
	      "Error on write", strerror(e));
    } else {
      fprintf(stderr, "%s\n", "STDOUT closed on us");
    }
    return false;
  }
  return true;
}

/**
 * Map a text buffer type to the closest corresponding
 * Ring item.
 * 
 * @param bufferType  type of the old style buffer.
 * 
 * @return uint32_t ring item type.
 *
 * @note uses the static map textTypeMap, setting it up if it's empty.
 *
 */

uint32_t mapTextBufferType (uint16_t bufferType)
{

  if (textTypeMap.empty())
  {
    textTypeMap[STATEVARBF] = MONITORED_VARIABLES;
    textTypeMap[RUNVARBF]   = MONITORED_VARIABLES;
    textTypeMap[PKTDOCBF]   = PACKET_TYPES;
    textTypeMap[PARAMDESCRIP] = MONITORED_VARIABLES;
  }


  return textTypeMap[bufferType];
}

/**
 * Does the same thing as mapTextBufferType but maps the state transition buffer types.
 * 
 * @param bufferTye - type to map.
 * 
 * @return uint32_t mapped type.
 *
 * @note this uses stateTypeMap to do the mapping, setting it up if needed.
 */

uint32_t mapStateChangeType (uint16_t bufferType)
{

  if (stateTypeMap.empty())
  {
    stateTypeMap[BEGRUNBF] = BEGIN_RUN;
    stateTypeMap[ENDRUNBF] = END_RUN;
    stateTypeMap[PAUSEBF]  = PAUSE_RUN;
    stateTypeMap[RESUMEBF] = RESUME_RUN;
  }

  return stateTypeMap[bufferType];
}



/**
 * Generate phyics data ring items.  Each event results in a write of a correctly
 * formatted ring item to stdout... or an error.  This code assumes the byte ordering
 * is the same as the native system.
 *
 * @param pBuffer - Pointer to the raw data buffer.;
 * 
 * @return bool 
 * @return true - All events in the buffer were successfully written to stdout.
 * @return false - A write to stdout failed or some other error was detected.
 */

bool formatEvents (void* pBuffer)
{
  static    size_t buffers = 0;
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  uint16_t* pBody(reinterpret_cast<uint16_t*>(pHeader+1));
  
  int      wordsLeft(pHeader->nwds - sizeof(bheader)/sizeof(uint16_t));
  int      nEvents(pHeader->nevt);

  RingItemHeader eventHeader;


  // Note: Event size is 32 bits in the ring buffer and 16 bits in the old buffer style.
  // hence the extra sizeof(uint16_t) and the hole after putting in the word count
  // later on:

  for (int i = 0; i < nEvents; i++) {
    uint16_t eventSize = *pBody++;
    pPhysicsEventItem pItem = formatEventItem(eventSize - 1, pBody);
    bool status = writeData(pItem, pItem->s_header.s_size);
    free(pItem);

    if (!status) return false;

    pBody     += eventSize-1;	// Remember the pointer points to the event body.
    wordsLeft -= eventSize;

    if (wordsLeft < 0)
    {
      fprintf(stderr, "**** Bad buffer structure, wordsLeft < 0 (%d) at event %d out of %d\n",
	      wordsLeft, i, nEvents);

      return false;
    }
  }
  // If there are words left that's an error worth reporting too:


  if(wordsLeft) {
    fprintf(stderr, "*** Bad buffer structure, %d words left but no more events according to count (buffer %d)\n",
	    wordsLeft, buffers);
    return false;
  }

  triggers += nEvents;		// Update trigger count seen so far.
  return true;
}

/**
 * Format a trigger count item and write it to stdout
 *
 * @param runTime - Offset into the run.
 * @param stamp   - Absolute timestamp.
 *
 * @return bool - false if failed to write it correctly.
 */
bool formatTriggerCount(uint32_t runTime, time_t stamp)
{
  pPhysicsEventCountItem pItem = formatTriggerCountItem(runTime, stamp, triggers);
  bool status = writeData(pItem, pItem->s_header.s_size);
  free(pItem);
  return status;
}

/**
 * scalerTimestamp 
 *
 *  Extract the 64 bit timestamp from the first 4 unused of the scaler buffer body
 *
 * @param pBody - Pointer to the scaler body.
 *
 * @return uint64_t timestamp value.
 */
uint64_t
scalerTimestamp(sclbody* pBody)
{
  uint64_t timestamp;


  timestamp = (uint16_t)pBody->unused2[0]; // Highest order.

  for (int i =0; i < 3; i++) {
    timestamp = timestamp << 16;
    timestamp |= (uint16_t)pBody->unused1[2-i];
  }
  // S800 kludge here:

  timestamp = timestamp << 3;

  // 

  return timestamp;
}
/**
 * scalerTimeDivisor
 *
 *  Returnthe time divisor for the scaler timebase.
 *
 * @param pBody - pointer to the scaler body.
 *
 * @return uint32_t divisor.
 */
uint32_t
scalerTimeDivisor(sclbody* pBody)
{
  uint32_t result;
  result = pBody->unused2[2];	// high order part.
  result = result << 16;	// shift into position.
  result |= pBody->unused2[1];	// or in low order part.

  return result;
}

/**
 * Function to write scaler buffers to file. Note that NSCLBuffers have start of run
 * offsets but not timestamps, rings have timestamps.  At this time we choose to
 * fill in the current time as a timestamp.  In the future we could calculate
 * the timestamp from the run-time offset and start of run timestamp.
 *
 * @param pBuffer - pointer to the buffer.
 *
 * @return bool
 * @retval true on success, false otherwise.
 */
bool formatScaler (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  sclbody*  pBody(reinterpret_cast<sclbody*>(pHeader+1));
  uint32_t nScalers = pHeader->nevt;

  uint32_t    scalers[nScalers];
 
  time_t      timestamp;
  time(&timestamp);

  // format a trigger time buffer with our timestamp and the scaler end time:

  if (!formatTriggerCount(pBody->etime, timestamp)) {
    return false;
  }


  // For 11.x only make the timestamped item.

  pScalerItem pTSItem = formatNonIncrTSScalerItem(nScalers, timestamp,
								pBody->btime, pBody->etime,
								scalerTimestamp(pBody),
								pBody->scalers,
								scalerTimeDivisor(pBody)							     
								);
  int status = writeData(pTSItem, pTSItem->s_header.s_size);
  free(pTSItem);

  return status;

}

/**
 * Format a strings data buffer.  The actual type of the buffer depends on the 
 * type of the input buffer:
 * \verbatim 
 *   STATEVARBF -> MONITORED_VARIABLES
 *    RUNVARBF  -> MONITORED_VARIABLES
 *    PKTDOCBF  -> PACKET_TYPE
 *    PARAMDESCRIP -> MONITORED_VARIABLES
 *
 * \endverbatim
 *
 *  Note since no timing information is given in the source data, a current timestamp will be
 *  generated bu the run offset time will be set to 0.
 *
 * @param pBuffer - Pointer to the buffer.
 *
 * @return bool
 * @return true on success, false otherwise.
 */

bool formatStrings (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  char*     pBody(reinterpret_cast<char*>(pHeader + 1));
  int       nStrings = pHeader->nevt;

  TextItem header;
  size_t   nStringSize(0);

  time_t stamp;
  time(&stamp);

  const char** pStrings = new const char *[nStrings]; // Will have ptrs to the strings.
  for (int i =0; i < nStrings; i++) {
    pStrings[i]  = pBody;
    
    // Length + null must always be even:

    int length = strlen(pBody) + 1; // length + null.
    if (length %2) length++;	    // Make it even if it's odd.
    pBody += length;		    // next string.
    
  }
  pTextItem pItem = formatTextItem(nStrings, stamp, 0, pStrings, mapTextBufferType(pHeader->type));
  bool status = writeData(pItem, pItem->s_header.s_size);
  free(pItem);
  delete [] pStrings;
  return status;

}
/**
 * Write a state change data buffer. Buffer Type mappings:
 *
 * \verbatim
 *     BEGRUNBF  -> BEGIN_RUN
 *     ENDRNBF   -> END_RUN
 *     PAUSEBF   -> PAUSE_RUN
 *     RESUMEBF  -> RESUME_RUN
 *
 * \endverbatim
 * 
 * @return bool
 * @retval true on success, false on error.
 */

bool formatStateChange (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  ctlbody* pBody(reinterpret_cast<ctlbody*>(pHeader+1));

  StateChangeItem item;

  struct tm bTime;
  char      textualTime[1000];
  sprintf(textualTime, "%d-%d-%d %d:%d:%d",
	  bTime.tm_year = pBody->tod.year,
	  bTime.tm_mon  = pBody->tod.month,
	  bTime.tm_mday = pBody->tod.day,
	  bTime.tm_hour = pBody->tod.hours,
	  bTime.tm_min  = pBody->tod.min,
	  bTime.tm_sec  = pBody->tod.sec);

  strptime(textualTime, 
	   "%Y-%m-%d %T", &bTime);
  time_t stamp = mktime(&bTime);

  pStateChangeItem pItem = formatStateChange(stamp, pBody->sortim, pHeader->run,
					     pBody->title, mapStateChangeType(pHeader->type));
  bool status = writeData(pItem, pItem->s_header.s_size);
  free(pItem);
  return status;

}


/**
 * Based on the buffer type dispatch to the appropriate ring item
 * generating function.
 *
 * @param pBuffer -  Pointer to the raw data buffer.
 *
 * @return bool
 */
bool bufferToRing (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  INT16    type = pHeader->type;

  switch (type) {
    case DATABF:
      return formatEvents(pBuffer);
      break;
    case SCALERBF:
    case SNAPSCBF:
      return formatScaler(pBuffer);
      break;
    case STATEVARBF:
    case RUNVARBF:
    case PKTDOCBF:
    case PARAMDESCRIP:
      return formatStrings(pBuffer);
      break;
    case BEGRUNBF:
      triggers = 0;		// Begin run zeroes the trigger count.
    case ENDRUNBF:
    case PAUSEBF:
    case RESUMEBF:
      return formatStateChange(pBuffer);
      break;
    default:
      fprintf(stderr, "Got a buffer whose type we did not expect: %d\n", type);
      fprintf(stderr, "Ingoring the data in this buffer!!\n");
      return false;
      break;
  }
}


int main (int argc, char *argv[])
{

  // If there's an argument it must be a buffersize:

  argc--; argv++;
  if (argc) {
    int newSize = atoi(*argv);
    if (newSize < BUFFERSIZE) {
      fprintf(stderr, "Buffer size specification %s must be an integer >= %d\n",
	      *argv, BUFFERSIZE);
      exit(EXIT_FAILURE);
    }
    BUFFERSIZE = newSize;
  }

  uint16_t    dataBuffer[BUFFERSIZE/sizeof(uint16_t)];



  // Process the data:

  while (getBuffer(dataBuffer, BUFFERSIZE)) {
      if(!bufferToRing(dataBuffer)) {
	break;
      }

  }
  return 0;
} 

