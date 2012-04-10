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
 * Return true if an I/O errno is not an allowed one.
 * 
 * @param error - the  errno to check.
 *
 * @return bool - True if the error is a bad one.
 *
 * @note okErrors is a set that will contain the 'allowed' errors.
 */
bool badError(int error)
{
  // Stock the okErrors set if empty:

  if (okErrors.empty())
  {
    okErrors.insert(EAGAIN);
    okErrors.insert(EWOULDBLOCK);
    okErrors.insert(EINTR);
  }

  // Not in the set -> true.

  return (okErrors.count(error) == 0);
}



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
  uint8_t* pDest(reinterpret_cast<uint8_t*>(pBuffer));
  size_t    residual(nBytes);
  ssize_t   nRead;

  // Read the buffer until :
  //  error other than EAGAIN, EWOULDBLOCK  or EINTR
  //  zero bytes read (end of file).
  //  Regardless of how all this ends, we are going to emit a message on sterr.
  //

  while (residual) {
    nRead = read(STDIN_FILENO, pDest, residual);
    if (nRead == 0)		// EOF
    {
      fprintf(stderr, "End of file on stdin in the middle of a read.\n");
      return false;
    }
    if ((nRead < 0) && badError(errno) )
    {
      perror("Error reading stdin in the middle of a read.\n");
      return false;
    }
    // If we got here and nread < 0, we need to set it to zero.
    
    if (nRead < 0)
    {
      nRead = 0;
    }

    // Adjust all the pointers and counts for what we read:

    residual -= nRead;
    pDest  += nRead;
  }
  // If we get here the read worked:

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
  uint8_t* pSrc(reinterpret_cast<uint8_t*>(pData));
  size_t   residual(size);
  ssize_t  nWritten;

  while (residual) {
    nWritten = write(STDOUT_FILENO, pSrc, residual);
    if (nWritten == 0) {
      fprintf(stderr, "Pipe closed writing stdout in the middle of a write\n");
      return false;
    }
    if ((nWritten == -1) && badError(errno)) {
      perror("Error writing to stdout in the middle of a write");
      return false;
    }
    // If an error now it must be a 'good' error... set the nWritten to 0 as no data was
    // transferred:

    if (nWritten < 0)
    {
      nWritten = 0;
    }
    // adjust the pointers, and residuals:


    residual -= nWritten;
    pSrc     += nWritten;
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
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  uint16_t* pBody(reinterpret_cast<uint16_t*>(pHeader+1));
  
  int      wordsLeft(pHeader->nwds - sizeof(bheader)/sizeof(uint16_t));
  int      nEvents(pHeader->nevt);

  RingItemHeader eventHeader;


  // Note: Event size is 32 bits in the ring buffer and 16 bits in the old buffer style.
  // hence the extra sizeof(uint16_t) and the hole after putting in the word count
  // later on:

  for (int i = 0; i < nEvents; i++) {
    uint16_t eventSize = *pBody;

    eventHeader.s_size = eventSize*sizeof(uint16_t) + sizeof(RingItemHeader) +sizeof(uint16_t);
    eventHeader.s_type = PHYSICS_EVENT;

    uint32_t eventSizeL = eventSize +1;

    if (!writeData(&eventHeader, sizeof(eventHeader))) {
      return false;
    }
    if (!writeData(&eventSizeL, sizeof(uint32_t))) {
      return false;
    }
    if (!writeData(pBody+1, (eventSize-1) * sizeof(uint16_t))) {
      return false;
    }

    // Adjust wordsLeft, pBody etc... if wordsLeft < 0 that's an error
    // worth reporting


    pBody += eventSize;
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
    fprintf(stderr, "*** Bad buffer structure, %d words left but no more events according to count\n",
	   wordsLeft);
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
  PhysicsEventCountItem item;
  item.s_header.s_size = sizeof(PhysicsEventCountItem);
  item.s_header.s_type = PHYSICS_EVENT_COUNT;
  item.s_timeOffset    = runTime;
  item.s_timestamp     = stamp;
  item.s_eventCount    = triggers;

  return writeData(&item, sizeof(PhysicsEventCountItem));
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

  ScalerItem  header;		// We won't use the s_scalers field.
  uint32_t    scalers[nScalers];
 
  time_t      timestamp;
  time(&timestamp);

  // format a trigger time buffer with our timestamp and the scaler end time:

  if (!formatTriggerCount(pBody->etime, timestamp)) {
    return false;
  }

  header.s_header.s_size = sizeof(ScalerItem) + ((nScalers-1)*sizeof(uint32_t));
  header.s_header.s_type = INCREMENTAL_SCALERS;
  header.s_intervalStartOffset = pBody->btime;
  header.s_intervalEndOffset   = pBody->etime;
  header.s_scalerCount         = nScalers;
  header.s_timestamp           = timestamp;

  memcpy(scalers, pBody->scalers, nScalers * sizeof(uint32_t));
  bool ok = writeData(&header, sizeof(header) - sizeof(uint32_t));
  if(!ok) {
    fprintf(stderr, "Error writing the header of a scaler buffer\n");
    return false;
  }
  return writeData(&scalers, nScalers * sizeof(uint32_t));
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
  char*    pStrings(0);
  
  time_t stamp;
  time(&stamp);

  // Fill in as much of the header as we can at this time:

  header.s_header.s_type = mapTextBufferType(pHeader->type);
  header.s_timeOffset  = 0;	// Not avaialble in the data.
  header.s_timestamp   = stamp;
  header.s_stringCount = nStrings;

  // Fill in the strings...counting the size as we go..we just keep reallocing
  // pStrings to make space.  Since the source strings are blank filled to 
  // make them even length, nothing special is needed there.

  for (int i = 0; i < nStrings; i++) {
    size_t s = strlen(pBody) + 1;	// size of one string + null terminator
    pStrings = reinterpret_cast<char*>(realloc(pStrings, nStringSize + s));
    memcpy(&(pStrings[nStringSize]), pBody, s);		// Copy in with null terminator.
    nStringSize += s;
  }
  // Compute the full item size:
  
  header.s_header.s_size = sizeof(header) - sizeof(char) + nStringSize;

  bool ok = writeData(&header, sizeof(header)  - sizeof(char));
  if (!ok) {
    free(pStrings);
    fprintf(stderr, "Write failed while writing text item header\n");
  }
  ok = writeData(pStrings, nStringSize);
  free(pStrings);
  return ok;
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

  // Fill in the item:
  
  item.s_header.s_type = mapStateChangeType(pHeader->type);
  item.s_header.s_size = sizeof(item);
  item.s_runNumber     = pHeader->run;
  item.s_timeOffset    = pBody->sortim;
  item.s_Timestamp     = stamp;
  strcpy(item.s_title, pBody->title);


  return writeData(&item, sizeof(item));

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
      fprintf(stderr, "Ingoring the data in this buffer!!\n", type);
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

