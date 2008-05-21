
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

#include "CRingBuffer.h"
#include "ringbufint.h"

#include <ErrnoException.h>
#include <RangeError.h>
#include <StateException.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

using namespace std;


/*
  This file implements the CRingBuffer class.  

*/


// Class Data.

size_t CRingBuffer::m_defaultDataSize(DEFAULT_DATASIZE);
size_t CRingBuffer::m_defaultMaxConsumers(DEFAULT_MAX_CONSUMERS);


//////////////////////////////////////////////////////////////////////////////
// 
// Predicate requiring at least x amount of free buffer space.
//

class CRingFreeSpacePredicate : public CRingBuffer::CRingBufferPredicate
{
private:
  size_t m_sizeRequired;
public:
  CRingFreeSpacePredicate(size_t size) : m_sizeRequired(size) {}
  virtual bool operator()(CRingBuffer& ring) {
    return m_sizeRequired > ring.availablePutSpace();
  }

};

/////////////////////////////////////////////////////////////////////////////
//
// Predicate requiring at least x amount of data to be there for this
// consumer.

class CRingDataAvailablePredicate : public CRingBuffer::CRingBufferPredicate
{
private:
  size_t m_sizeRequired;
public:
  CRingDataAvailablePredicate(size_t size) : m_sizeRequired(size) {}
  virtual bool operator()(CRingBuffer& ring) {
    return m_sizeRequired > ring.availableData();
  }
};

//////////////////////////////////////////////////////////////////////////////
// Class level functions.

/*!
  Create a new ring buffer memory segment.  Note that all data that is in the
  segment will be lost and conumers will get lost too.

  \param name       - Name of the ring buffer.  Will have '/' prepended to it.
  \param dataBytes  - Number of data bytes in the file.  This will be extended to the
                      next page boundary if it is not already a multiple of the
                      page size.
  \param maxConsumers - Maximum number of consumers allowed (number of 
                      consumer ClientInformation entries in the ring buffer.

  \throw CErrnoException

*/
void
CRingBuffer:: create(std::string name, 
		     size_t dataBytes,
		     size_t maxConsumer)
{
  string fullName = shmName(name);

  mode_t old      = umask(0); // Don't mask bits off mods below.

  int fd = shm_open(fullName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);


  int olderr = errno;
  umask(old);			// restore umask and errno from shm_open.=
  errno = olderr;

  if (fd < 0) {
    throw CErrnoException("CRingBuffer::create shm_open failed");
  }

  // Figure out the entire size of the shared memory region and truncate the file to that
  // size:

  size_t rawSize   = dataBytes + sizeof(RingHeader) + 
                                 sizeof(ClientInformation)*(maxConsumer+1);
  
  long   pageSize  = sysconf(_SC_PAGESIZE);
  size_t pages     = (rawSize + (pageSize-1))/pageSize;
  size_t shmSize   = pages*pageSize;

  if (ftruncate(fd, shmSize) < 0) {
    throw CErrnoException("CRingBufer::create ftruncate failed");
  }

  // Close the shm special file and then call format to format the ring buffer:

  close(fd);
  format(name, maxConsumer);
}
/*!
   Format an existing ring buffer.
   - Open the shared memory segment
   - Map the shared memory segment to the full size.
   - Format the front of the ring so that there's no producer or consumers.
   - So that the header matches the layout of the buffer.

   \param name         - Name of the ring buffer. (a / will be prepended).
   \param maxConsumers - Maximum number of supported consumers.

   \throw CErrnoException

*/
void 
CRingBuffer::format(std::string name,
		    size_t maxConsumer)
{

  string fullName = shmName(name);

  int fd = shm_open(fullName.c_str(), 
		    O_RDONLY, 0 );	// Don't assum where shm files are
  if (fd < 0) {
    throw CErrnoException("CRingBuffer::Format - open failed");
  }

  // Get the size of the shared memory regino.

  struct stat info;
  if (fstat(fd, &info) < 0) {
    close(fd);
    throw CErrnoException("CRingBuffer::format - stat failed");
  }
  off_t memSize = info.st_size;
  close(fd);


  // Map the ring:

  pRingBuffer        pRing     = mapRingBuffer(fullName);
  pRingHeader        pHeader   = reinterpret_cast<pRingHeader>(pRing);
  pClientInformation pProducer = reinterpret_cast<pClientInformation>(pHeader + 1);
  pClientInformation pClients  = pProducer + 1;

  // Fill in the header:

  pHeader->s_maxConsumer       = maxConsumer;
  pHeader->s_producerInfo      = (reinterpret_cast<char*>(pProducer) - 
				  reinterpret_cast<char*>(pHeader));
  pHeader->s_firstConsumer     = (reinterpret_cast<char*>(pClients) -
				  reinterpret_cast<char*>(pHeader));
  pHeader->s_topOffset         = memSize-1;
  pHeader->s_dataOffset        = sizeof(RingHeader) + 
                                 sizeof(ClientInformation)*(maxConsumer+1);
  pHeader->s_dataBytes         = memSize - pHeader->s_dataOffset;

  // Fill in the client information data structures:

  pProducer->s_offset          = pHeader->s_dataOffset;
  pProducer->s_pid             = -1;

  for (int i=0; i < maxConsumer; i++) {
    pClients->s_offset         = pHeader->s_dataOffset;
    pClients->s_pid            = -1;
    pClients++;
  }

}
/*!
  Set the defafult ring size.  This will be the amount of data the ring can hold
  if that's not supplied at creation time.
  \param byteCount - number of bytes.
*/
void
CRingBuffer::setDefaultRingSize(size_t byteCount)
{
  m_defaultDataSize = byteCount;
}
/*!

  \return size_t
  \retval Return the default amount of data a ring will hold:
*/
size_t
CRingBuffer::getDefaultRingSize()
{
  return m_defaultDataSize;
}
/*!
  Set the default maximum consumer count.
  \param numConsumers  - number of consumers.
*/
void
CRingBuffer::setDefaultMaxConsumers(size_t numConsumer)
{
  m_defaultMaxConsumers = numConsumer;
}
/*!
     \return size_t
     \retval Maximum number of consumers in a ring that has been created
             without specifying that value.
*/
size_t
CRingBuffer::getDefaultMaxConsumers()
{
  return m_defaultMaxConsumers;
}

///////////////////////////////////////////////////////////////////////////////////////
// Constructors and canonicals.


/*!
   Construct a ring buffer object. This will represent a consumer or a
   producer on the specified ring.

   \param name - name of the ring buffer, "/" will be prepended.
   \param mode - Type of access requested.  This should be one of:
               - CRingBuffer::producer for producer access.
               - CRingBuffer::consumer for consumer access.

    \throw CErrnoException Some special errnos though:
    - ENOMEM - No free consumer data structs for consumer access.
    - EACCES - Producer access requested but there's already a producer. connected.
    - EINVAL - The connection mode is invalid.

*/
CRingBuffer::CRingBuffer(string name, CRingBuffer::ClientMode mode) :
  m_pRing(0),
  m_pClientInfo(0),
  m_mode(mode),
  m_pollInterval(DEFAULT_POLLMS)
{
  m_pRing = mapRingBuffer(shmName(name));

  // Now that we're mapped the remainder of the constructor must execute in a try
  // block so that failure will allow us to unmap the ring.
  //
  try {
    if (m_mode == producer) {
      if (m_pRing->s_producer.s_pid == -1) {
	m_pClientInfo         = &(m_pRing->s_producer);
	m_pClientInfo->s_pid  = getpid(); // leave the offset where it was.

      }
      else {
	errno = EACCES;
	throw CErrnoException("CRingBuffer::CRingBuffer - already a producer");
      }
    } 
    else if (m_mode == consumer) {
      allocateConsumer();
    }
    else if (m_mode == manager) {
      
    }
    else {			// Invalid connection mode.
      errno = EINVAL;
      throw CErrnoException("CRingBuffer::CRingBuffer - invalid connection mode");
    }
  }
  catch (...) {
    unMapRing();
    throw;
  }
}
/*!
   Destructor ... release our our client info and unmap.
*/
CRingBuffer::~CRingBuffer()
{
  if (m_mode != manager) {
    m_pClientInfo->s_pid = -1;
  }
  unMapRing();

  // Zeroing pointers ensures that attempts to use this object will segflt.

  m_pRing       = 0;	      
  m_pClientInfo = 0;
}
/////////////////////////////////////////////////////////////////////////
// Member functions that manipulate the ring buffer.
//

/*!
   Put data into the ring buffer.
   - Block until there's sufficient space to do the put.
   - Put the data, wrapping as needed.
   - Update the put pointer.

   \note  The ring buffer must have been attached to in producer mode or
          a CStateException will be thrown.

    \param pBuffer - Pointer to the buffer containing the data to put in the buffer.
    \param nBytes  - Number of bytes to transfer.
    \param timeout - The timeout for the write.  The amount of time to block for the 
                     data to become available is determined by this.  The default value
		     is ULONG_MAX which works out to approximately 136 years.. which is
		     close enough to infinite that it can be treated as infinite.
		     A value of 0 will not block.

    \return size_t
    \retval nBytes  - The buffer was written to the ring buffer, all of it.
    \retval 0       - Wait for available space timed out.

    \throw CRangeException - The amount of data to write is larger than the
                            size of the ring buffer data segment.
    \throw CStateException - This CRingBufferObject is not open for producer use.
*/

size_t
CRingBuffer::put(void* pBuffer, size_t nBytes, unsigned long timeout)
{
  // Require that we are the producer:

  if (m_mode != producer) {
    throw CStateException(modeString().c_str(), "producer", 
			  "CRingBuffer::put");
  }

  // Ensure the ring is big enough for the data:

  if (nBytes > m_pRing->s_header.s_dataBytes) {
    throw CRangeError(0, m_pRing->s_header.s_dataBytes, nBytes,
		      "CRingBuffer::put");

  }
  // Block until we have space. 

  CRingFreeSpacePredicate condition(nBytes);
  int status = blockWhile(condition, timeout);
  if (status) {
    return 0;			// timed out.
  }
  // Now we need to figure out how to put the data in the ring.
  // We may need to wrap the data across the top of the buffer.
  //

  off_t ringBase = m_pRing->s_header.s_dataOffset;
  off_t ringTop  = m_pRing->s_header.s_topOffset;
  char* pDataBase= reinterpret_cast<char*>(m_pRing) + ringBase;
  char* pPut     = reinterpret_cast<char*>(m_pRing) + m_pClientInfo->s_offset;

  if ((m_pClientInfo->s_offset + nBytes) <=  (ringTop+1)) {

    // Can move all at once...

    memcpy(pPut, pBuffer, nBytes);


  }
  else {
    // Need to move in two chunks:

    size_t firstSize = ringTop+1 - m_pClientInfo->s_offset;
    size_t secondSize= nBytes - firstSize;

    

    memcpy(pPut, pBuffer, firstSize);                     // Move the first chunk.

    char* pSecond = reinterpret_cast<char*>(pBuffer) + firstSize;
    memcpy(pDataBase, pSecond, secondSize);              // Move the second chunk. 

  }
  Skip(nBytes);

  // If we got this far success:

  return nBytes;
}

/*!
   Get data from the ring buffer.  This object must have been opened 
   in producer mode else a CStateException is thrown.

   \param pBuffer  - Pointer to the buffer in which the data from the ring will be put.
   \param maxBytes - Number of bytes the buffer can hold... or rather the maximum number 
                     of bytes the caller wants transferred.
   \param minBytes - The minimum number of bytes the caller wants transferred.
                     The caller will block until this number of bytes is available for it,
		     or until the timeout specified.
   \param timeout  - The maximum number of seconds to block for data.  The default is
                     ULONG_MAX which is about 136 years or essentially forever.  A value
		     of zero will never block.

   \return size_t
   \retval > 0  - The number of data bytes transferred to the buffer. this should be
                  between minBytes and maxBytes.  It will be the number of available bytes
                  at the time at least minBytes became available (but no more than
                  maxBytes).
    \retval 0   - The wait timed out.

    \throw  CStateException - this object is not a consumer object.
    \throw  CRangeException - minBytes is larger than the size of the ring buffer.
                              If we did not treat this as an exception, the caller would
                              block forever.
*/
size_t
CRingBuffer:: get(void*        pBuffer, 
		  size_t       maxBytes, 
		  size_t       minBytes, 
		  unsigned long timeout)
{
  // Ensure we are a consumer:

  if (m_mode != consumer) {
    throw CStateException(modeString().c_str(), "consuemer",
			  "CRingBuffer::get");
  }
  // Ensure that we won't block forever:

  if (minBytes > m_pRing->s_header.s_dataBytes) {
    throw CRangeError(0, m_pRing->s_header.s_dataBytes, minBytes,
		      "CRingBuffer::get");
  }

  // Wait until we have at least the desired numbe of bytes:

  CRingDataAvailablePredicate condition(minBytes);
  int status = blockWhile(condition, timeout);

  if (status) {
    return 0;			// Timed out.
  }
  // Figure out how much data we'll transfer:

  size_t transferSize =availableData();
  if (transferSize > maxBytes) {
    transferSize = maxBytes;
  }

  peek(pBuffer, transferSize);
  Skip(transferSize);

  return transferSize;
  
}
/*!
   Peek at the contents of the ring buffer.   
   This code returns data from the ring buffer for a consumer without
   advancing the get pointer.  This function never blocks

   \param pBuffer - Pointer to the buffer that will contain the data transferred.
   \param maxBytes- Maximum number of bytes to transfer to pBuffer

   \return size_t
   \retval Number of bytes read (could be zero, can't be larger than maxBytes).

   \throw CStateException - This object is not a consumer.

   \note if maxBytes is larger than the ring buffer size this is not an error, just
         a gaurantee that the returned value will be less than maxBytes.
*/
size_t
CRingBuffer::peek(void*   pBuffer,
		  size_t  maxBytes)
{
  size_t transferSize = availableData();
  if (transferSize == 0) {
    return 0;			// no data.
  }
  if (transferSize > maxBytes) {
    transferSize = maxBytes;
  }


  // Get some key values

  off_t ringBase = m_pRing->s_header.s_dataOffset;
  off_t ringTop  = m_pRing->s_header.s_topOffset;
  char* pDataBase= reinterpret_cast<char*>(m_pRing) + ringBase;
  char* pGet     = reinterpret_cast<char*>(m_pRing) + 
                   m_pClientInfo->s_offset;	// Put data starting here.

  // Decide if this can be transferred in one or two chunks:

  if (m_pClientInfo->s_offset + transferSize <= (ringTop+1)) {

    // only need a single transfer:

    memcpy(pBuffer, pGet, transferSize);

  }
  else {
    // Need two chunks worth of transfer.

    size_t firstSize = ringTop+1 - m_pClientInfo->s_offset;
    size_t secondSize= transferSize - firstSize;

    memcpy(pBuffer, pGet, firstSize);
    memcpy(reinterpret_cast<char*>(pBuffer) + firstSize,
	   pDataBase, secondSize);
  }
  return transferSize;
  
}
/*!
   Skip the user's get/put pointer ahead.  Only consumers can call this.
   Use this to avoid transferring data you know you won't use.

   \param nBytes - Number of bytes to skip.

   \throw CRangeError     - nBytes > than the ring buffer.
   \throw CStateException - This is not a consumer object

   \note it's actually the private Skip utility that is going to throw
   CRangeError exceptions.
*/
void
CRingBuffer:: skip(size_t nBytes)
{
  if (m_mode != consumer) {
    throw CStateException(modeString().c_str(), "consumer",
			  "CRingBuffer::skip");

  }
  Skip(nBytes);
}
/////////////////////////////////////////////////////////////////////////////////
// Manage the blocking latencies.

/*!
   Set the blocking poll interval (the length of the usleep) in milliseconds.

   \param newValue - Number of milliseconds between calls of the
                     blocking predicate.
   \return unsigned long
   \retval the prior value of this parameter.
*/
unsigned long
CRingBuffer::setPollInterval(unsigned long newValue)
{
  unsigned long ret = m_pollInterval;
  m_pollInterval = newValue;
  return ret;
}
/*!
    \return unsigned long
    \retval The time between calls to the blocking predicate.
*/
unsigned long
CRingBuffer::getPollInterval()
{
  return m_pollInterval;
}
///////////////////////////////////////////////////////////////////////////////
//  Inquiry member functions.


/*!
   \return size_t
   \retval the number of bytes of space available in which to put new data in
           the ring buffer.
*/
size_t
CRingBuffer::availablePutSpace()
{
  pRingHeader pHeader   = reinterpret_cast<pRingHeader>(m_pRing);
  size_t      consumers = pHeader->s_maxConsumer;

  pClientInformation pClients = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
								      pHeader->s_firstConsumer);

  // figure out the minimum free space:

  size_t minFree =  pHeader->s_dataBytes-1;
  for (int i = 0; i < consumers;  i++) {
    if(pClients->s_pid != -1) {
      size_t avail     = availableData(pClients);
      size_t freeBytes = pHeader->s_dataBytes - avail - 1; // -1 ensures put never catches get.
      if (freeBytes < minFree) minFree = freeBytes;
    }

    pClients++;
  }
  return minFree;
}


/*!
   Return the number of bytes of available data for the object represented
   by us.  This can be called for producers, as well as consumers, producers
   will always get a 0.

   \return size_t
   \retval Number of bytes a get can do without blocking.

*/
size_t
CRingBuffer::availableData() 
{

  return availableData(m_pClientInfo);

    
}


/*! 
  Get information about the usage of the ring buffer.
  This can be used in a management/diagnostic tool that determines and reports
  ring buffer utilization.
  \return CRingBuffer::Usage
  \retval Describes a snapshot of the ring buffer usage.

*/
CRingBuffer::Usage
CRingBuffer::getUsage()
{
  pRingHeader         pHead      = &(m_pRing->s_header);
  pClientInformation  pProducer  = &(m_pRing->s_producer);
  pClientInformation  pConsumers = m_pRing->s_consumers;

  Usage  result;
  result.s_bufferSpace = pHead->s_dataBytes;
  result.s_putSpace    = availablePutSpace();
  result.s_maxConsumers= pHead->s_maxConsumer;
  result.s_producer    = pProducer->s_pid;

  // Get information about all the consumers:

  for (int i =0; i < result.s_maxConsumers; i++) {
    if (pConsumers->s_pid != -1) {
      pair<pid_t, size_t> info;
      info.first  = pConsumers->s_pid;
      info.second = difference(*pProducer, *pConsumers);
      result.s_consumers.push_back(info);
    }
    pConsumers++;
  }
  // Figure out the max/min data available.
  // Special case of no consumers means that the 0 space is available for both.
  
  if (result.s_consumers.size() == 0) {
    result.s_maxGetSpace = result.s_minGetSpace = 0;
  }
  else {
    result.s_maxGetSpace = 0;
    result.s_minGetSpace = result.s_bufferSpace;
    for (int i =0; i < result.s_consumers.size(); i++) {
      if (result.s_consumers[i].second > result.s_maxGetSpace) 
	result.s_maxGetSpace = result.s_consumers[i].second;
      if (result.s_consumers[i].second < result.s_minGetSpace) 
	result.s_minGetSpace = result.s_consumers[i].second;
    }
  }
  return result;
}


///////////////////////////////////////////////////////////////////////////////
//  Blocking functions:

/*!
    Implements a generalized blocking function.  The idea is that we will
    block the caller until some condition is no longer met.  The condition
    is captured in the form of a functional object that is derived from 
    CRingBuffer::CRingBufferPredicate.

    \param pred    - The redicate object that controls how long we block.
    \param timeout - The maximum number of seconds we'll block.  The
                     default value is ULONG_MAX which is about 136 years
		     or essentially indefinitely.  A value of 0 will
		     never block.
    \return int
    \retval 0    - Blocking ended normally.
    \retval -1   - Blocking timed out.
*/
int 
CRingBuffer::blockWhile(CRingBuffer::CRingBufferPredicate& pred, unsigned long timeout)
{
  // Lower the latencey be special casing the timeout == 0:

  if (timeout) {
    time_t start = time(NULL);
    while (pred(*this)) {
      time_t now = time(NULL);
      if ((now - start) >= timeout) 
	return -1; // timeout
      usleep(m_pollInterval * 1000); // wait a bit before checking condition.
    }
    
    return 0;			// condition no longer true.
  }
  else {
    return pred(*this) ? -1 : 0;
  }
}

//////////////////////////////////////////////////////////////////////////////
//  Management functions:

/*!
   Force the release of the producer.  This requires that we are connected as a manager.
   
*/
void
CRingBuffer::forceProducerRelease()
{
  if (m_mode != manager) {
    throw CStateException(modeString().c_str(), "manager",
			  "CRingBuffer::forceProducerRelease");
  }
  m_pRing->s_producer.s_pid = -1;
}

/*!
  Force the release of a consumer.  This requires that we are connected as a manager.
  \param slot - the number of the slot of the consumer to release. 

 \throw CStateException - the mode is not manager.
 \trhow CRangeException - the slot is out of range.

*/
void
CRingBuffer::forceConsumerRelease(unsigned slot)
{
  if (m_mode != manager) {
    throw CStateException(modeString().c_str(), "manager",
			  "CRingBuffer::forceConsumerRelease");
  }
  if (slot >= m_pRing->s_header.s_maxConsumer) {
    throw CRangeError(0, m_pRing->s_header.s_maxConsumer -1, 
		      slot,
		      "CRingBuffer::forceConsumerRelease");
  }
  m_pRing->s_consumers[slot].s_pid = -1;
}

//////////////////////////////////////////////////////////////////////////////
// Private utility functions

/*******************************************************************/
/*  Return the amount of data available for a single client.       */
/******************************************************************/

size_t
CRingBuffer::availableData(ClientInformation* pInfo)
{
  // Locate the producer client information.

  pRingHeader pHeader          = reinterpret_cast<pRingHeader>(m_pRing);
  pClientInformation pProducer = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
								       pHeader->s_producerInfo);

  return difference(*pProducer, *pInfo);
  
}

/*******************************************************************/
/*  Construct the full device special file for a shared memory     */
/*  segment given the 'simple' name. This just prepends a '/' to   */
/*  the simple name                                                */
/*******************************************************************/
string
CRingBuffer::shmName(string rawName)
{
  return string("/") + rawName;
}

/*******************************************************************/
/*  Maps to the specified full name ring buffer.                   */
/*******************************************************************/

RingBuffer* 
CRingBuffer::mapRingBuffer(std::string fullName)
{
  // Open the shared mem special file.

  int fd = shm_open(fullName.c_str(), O_RDWR, 0);
  if (fd < 0) {
    throw CErrnoException("CRingBuffer::mapRingBuffer failed shm_open");
  }
  
  // Figure out how big it is... assume that it's a multiple of
  // pagesize:

  struct stat info;
  if(fstat(fd, &info) < 0) {
    throw CErrnoException("CRingBuffer::mapRingBuffer failed fstat");
  }

  //  Map it:

  void* pMem = mmap(0, info.st_size, PROT_READ | PROT_WRITE,  MAP_SHARED,
		    fd, 0);
  if (pMem == MAP_FAILED) {
    throw CErrnoException("CRingBuffer::mapRingBuffer failed mmap");
  }

  // Close the file.

  close(fd);


  return reinterpret_cast<RingBuffer*>(pMem);

}
/******************************************************************/
/* Unmap the ring buffer.                                         */
/******************************************************************/

void
CRingBuffer::unMapRing()
{
  // Compute the size of the region:

  pRingHeader pHeader = &(m_pRing->s_header);

  size_t ringSize = pHeader->s_dataBytes + sizeof(RingHeader) +
                   (pHeader->s_maxConsumer+1)*sizeof(ClientInformation);
  munmap(m_pRing, ringSize);
  
}
/******************************************************************/
/* Allocate an unused consumer description record.  On success    */
/* the consumer is filled in with our pid, and an offset that's   */
/* equal to the put pointer.  On failure a CErrnoException is     */
/* thrown that with ENOMEM as the reason, since the only error    */
/* is for there to be no free consumer blocks.                    */
/******************************************************************/ 
void
CRingBuffer::allocateConsumer()
{
  pRingHeader pHeader    = &(m_pRing->s_header);
  size_t      nConsumers = pHeader->s_maxConsumer;
  pClientInformation p   = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) + 
							   pHeader->s_firstConsumer);
  pClientInformation put = reinterpret_cast<pClientInformation>(reinterpret_cast<char*>(m_pRing) +
							   pHeader->s_producerInfo);

  for (int i =0; i < nConsumers; i++) {
    if (p->s_pid == -1) {
      p->s_pid = getpid();
      p->s_offset = put->s_offset;
      m_pClientInfo = p;
      return;
    }

    p++;
  }
  errno = ENOMEM;
  throw CErrnoException("CRingBuffer::allocateConsumer");

}
/******************************************************************/
/*  Compute the 'space' between two pointers, dealing with wraps. */
/*  The first pointer is assumed to be ahead of the second.       */
/******************************************************************/

size_t
CRingBuffer::difference(ClientInformation& producer, ClientInformation& consumer)
{
  // If the producer is bigger than the consumer it's a simple difference:

  if (producer.s_offset >= consumer.s_offset) {
    return producer.s_offset - consumer.s_offset;
  }
  // Otherwise, the answer is the sum of how far the consumer offset is from the
  // top of the ring and how far the producer is from the bottom.
  // +1.
  pRingHeader pHeader = &(m_pRing->s_header);

  size_t topSize = pHeader->s_topOffset - consumer.s_offset;
  size_t botSize = producer.s_offset   - pHeader->s_dataOffset;
  return topSize + botSize+1;
}

/******************************************************************/
/* Move the object's pointer ahead the designated number of bytes */
/* wrapping if needed.                                            */
/******************************************************************/
void
CRingBuffer::Skip(size_t nBytes)
{
  pRingHeader pHeader = &(m_pRing->s_header);

  m_pClientInfo->s_offset += nBytes;
  if (m_pClientInfo->s_offset > pHeader->s_topOffset) {
    m_pClientInfo->s_offset = (m_pClientInfo->s_offset - pHeader->s_topOffset) +
                              pHeader->s_dataOffset - 1;
  }
}
/***************************************************************/
/* Return the stringified mode                                 */
/**************************************************************/
string
CRingBuffer::modeString() const
{
  switch (m_mode) {
  case consumer:
    return string("consumer");
  case producer:
    return string("producer");
  case manager:
    return string("manager");
  default:
    return string("*INVALID MODE*");
  }

}
