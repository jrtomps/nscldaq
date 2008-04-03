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

      
//////////////////////////CNSCLOutputBuffer.cpp file////////////////////////////////////
#include <config.h>
#include "CNSCLOutputBuffer.h"                  
#include <string>
#include <unistd.h>
#include <RangeError.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

extern bool daq_isJumboBuffer();

// Manifest constants:

static const int hdrWSIZE = 0;	// Offset to size in header (Word).
static const int hdrWTYPE = 1;	// Offset to Type field of header (Word).
static const int hdrWCKS  = 2;	// Offset to checksum field of header (Word).
static const int hdrWRUN  = 3;	// Offset to run number of header (Word).
static const int hdrLSEQ  = 4;	// Offset to sequence number in header.
static const int hdrWENTITY = 6; // Offset to entity count.
static const int hdrWUNUSED1 = 7; // Decomissioned lam mask field.
static const int hdrWCPU  = 8;  // CPU identifier.
static const int hdrWUNUSED2 = 9; // Decomisioned bitregister count.
static const int hdrWBUFFMT = 10; // Buffer format indicator.
static const int hdrWSIG   = 11; // Short signature.
static const int hdrLSIG   = 12; // Long signature.
static const int hdrWJUMBOHIGH = 14;
static const int hdrWUNUSED3 =15; // 14,15, are both unused.
static const int BODYOFFSET = 16;

static const short SSIGNATURE = 0x0102;
static const long  LSIGNATURE = 0x01020304;

static const short REVLEVEL = 5; // lvl 5 - removes unused header items.
static const short JUMBOLEVEL=6;

// Convert int <-> shorts.

union intConvert {
  unsigned int   l;
  unsigned short w[2];
};


// Static class members.


unsigned long CNSCLOutputBuffer::m_nSequence = 0; //Static data member initialization

int CNSCLOutputBuffer::m_ControlTag = 3;
int CNSCLOutputBuffer::m_EventTag   = 2;

//Default constructor alternative to compiler provided default constructor
//Association object data member pointers initialized to null association object 
/*!
  Default constructor.  This is called when declarations of the form e.g.:
  -  CNSCLOutputBuffer  object;
  are performed.
  */
CNSCLOutputBuffer::CNSCLOutputBuffer (unsigned nWords)
   : m_BufferBase(nWords),   
     m_Buffer(&m_BufferBase),
     m_BufferPtr(&m_Buffer),   
     m_nWords(nWords) 
{
  InitializeHeader();
} 

// Functions for class CNSCLOutputBuffer

/*!
    Uses the current buffer pointer to 
    determine the number of words in the buffer.
    The resulting value is set in the buffer size
    word of the buffer header.


*/
void 
CNSCLOutputBuffer::ComputeSize()  
{
  if (daq_isJumboBuffer()) {
    union intConvert lw;
    lw.l = m_BufferPtr.GetIndex();
    m_Buffer[hdrWSIZE]       = lw.w[0];
    m_Buffer[hdrWJUMBOHIGH] = lw.w[1];
  }
  else {
    m_Buffer[hdrWSIZE] = m_BufferPtr.GetIndex();
  }

}  

/*!
    Sets the type field of the buffer
    \param nType - Type of the buffer.

	\param nType - Buffer type field.

*/
void 
CNSCLOutputBuffer::SetType(int nType)  
{
  m_Buffer[hdrWTYPE] = nType;
}  

/*!
    Computes the buffer checksum and places it in the checksum
    field of the buffer.  The checksum is computed such that
    the sum of the used words in the buffer is zero.

    \note The checksum will change as items are added or changed in the
    buffer.  Route automatically calls ComputeChecksum().  While there
    is no programmatic harm in recomputing the checksum over and over again,
    there is a performance loss in doing so.. You are best off letting
    Route() do that for you.

    \note It is assumed that m_Buffer[hdrWSIZE] is correct.

*/
void 
CNSCLOutputBuffer::ComputeChecksum()  
{
  m_Buffer[hdrWCKS] = 0;	// Don't let old cks contribute if set.
 
  int nWords;
  if (daq_isJumboBuffer()) {
    intConvert lw;
    lw.w[0] = m_Buffer[hdrWSIZE];
    lw.w[1] = m_Buffer[hdrWJUMBOHIGH];
    nWords = lw.l;
  }
  else {
    nWords = m_Buffer[hdrWSIZE];
  }

  short cks(0);

  DAQWordBufferPtr p(&m_Buffer); // Pointer iteration much better than idx.
  for(int i = 0; i < nWords; i++) {
    cks += *p;
    ++p;			// Pre increment faster than post.
  }
  m_Buffer[hdrWCKS] = - cks;	// Buffer will now checksum to zero.
}  

/*!
    Begins an entity in the buffer.
    At present this is a no-op.

	\param 

*/
DAQWordBufferPtr 
CNSCLOutputBuffer::StartEntity()  
{
  return m_BufferPtr;
}

/*!
    Ends an entity insertion into the buffer.
    The Entity count field of the buffer is
    incremented.
    \param rpBuf - Pointer to word after event.


*/
void 
CNSCLOutputBuffer::EndEntity(const DAQWordBufferPtr& rpBuf)  
{
  m_BufferPtr = rpBuf;		// Update internal cursor.
  m_Buffer[hdrWENTITY]++;	// And the entity count.
}  

/*!
    Sets the Cpu number field of the buffer.
    The CPU number field is used to identify wich
    system the data in this buffer came from.  By 
    convention, this field is the least two significant
    octets of the system's IP address.. in network 
    byte order.  If the function's parameter value is
    zero, this is determined and used.  Otherwise, the
    parameter is the value. 
    
    \note CPU number can not be zero.

	\param nValue If non zero, this overrides the default
	which comes from gethostid()

*/
void 
CNSCLOutputBuffer::SetCpuNum(unsigned short nValue)  
{
  if(nValue) {
    m_Buffer[hdrWCPU] = nValue;
  }
  else {
    m_Buffer[hdrWCPU] = (short)(gethostid() & 0xffff);
  }
}  

/*!
    Sets the nbit field in the buffer header.

	\param nBitReg [1] - Number of bit registers to set in the
	buffer header.
	
	\note As of buffer revision 5, with the use of tagged packets,
	this field is being decomissioned.  This member is provided
	for compatibility and currently will fill in offset 9
	(hdrWUNUSED2), later versions of this software may make this
	function a no-op.

*/
void 
CNSCLOutputBuffer::SetNbitRegisters(unsigned  nBitReg)  
{
  m_Buffer[hdrWUNUSED2] = nBitReg;
}  

/*!
    Sets the nlam field of the buffer header.

	\param nLams [0] - Number of lam masks.

	\note As of buffer revision 5, with the use of tagged packets,
	this field is being decomissioned.  This member is provided
	for compatibility and currently will fill in offset 7
	(hdrWUNUSED1), later versions of this software may make this
	function a no-op.


*/
void 
CNSCLOutputBuffer::SetLamRegisters(unsigned short nLams)  
{
  m_Buffer[hdrWUNUSED1] = nLams;
}  

/*!
    Puts a stream of words into the buffer and 
    increments the entity count field of the buffer
    header.

	\param void* pEntity, unsigned int nWords

*/
void 
CNSCLOutputBuffer::PutEntity(const void* pEntity, unsigned int nWords)  
{
  DAQWordBufferPtr p = StartEntity();
  const unsigned short*  s = reinterpret_cast<const unsigned short*>(pEntity);

  while(nWords) {
    *p = *s++;
    ++p;
    nWords--;
  }
  EndEntity(p);
 
}  

/*!
    Put a long word in the buffer in native
    system order.  The buffer pointer is affected.
    This is only useful for non-entity words.

	\param rLong - refers to the long word to put.

*/
void 
CNSCLOutputBuffer::PutLong(const unsigned long& rLong)  
{
  union {
    unsigned long l;
    unsigned short w[2];
  } d;
  d.l = rLong;

  *m_BufferPtr = d.w[0];
  ++m_BufferPtr;
  *m_BufferPtr = d.w[1];
  ++m_BufferPtr;
}  

/*!
    Puts a word of data into the buffer. This is only
    useful for non-entity words.

	\param nData - the word to put.

*/
void 
CNSCLOutputBuffer::PutWord(unsigned short nData)  
{
  *m_BufferPtr = nData;
  ++m_BufferPtr;
}  

/*!
    Puts an array of words into the buffer.

	\param pWords - The array to put.
	\param nWords - The number of words to put.

     \note The entity count is not affected, but the buffer pointer is
           incremented.  This is only useful to put words which are not
	   part of a counted entity.

*/
void 
CNSCLOutputBuffer::PutWords(const unsigned short* pWords, unsigned int nWords)  
{
#ifndef HIGH_PERFORMANCE
  while(nWords) {
    *m_BufferPtr = *pWords++;
    ++m_BufferPtr;
    nWords--;
  }
#else /* HIGH_PERFORMANCE */
  m_BufferPtr.CopyIn((unsigned short*)pWords, 0, nWords);
  m_BufferPtr += nWords;

#endif /* HIGH_PERFORMANCE */
}  

/*!
    Put a null terminated character array into the buffer.
    Note that strings in NSCL buffers must be an even number
    of bytes (including their null terminator).  If necessary,
    an additional blank is added to the string.   If
    the string is being put into a fixed size field and is 
    shorter than the buffer field, it is null filled, otherwise
    it is truncated in a manner which allows the
    buffer to contain a trailing null.

	\param pData - Pointer to the string to insert in the buffer
	              (null terminated).
	\param nMaxSize [-1] - Size of the field into which the data
	must fit.  Note that if -1, the entire string is inserted (the
	destination is treated as variable length.

	\note - If nMaxSize is odd, it is incremented to make the
	maximum field size even since this is a word buffer.
*/
void 
CNSCLOutputBuffer::PutString(const char*  pData, int nMaxSize)  
{
  string data(pData);

  // The following code is required if nMaxSize is supplied:

  if(nMaxSize > 0) {
    if(nMaxSize & 1) nMaxSize++; // nMaxSize must be even.

    if(data.size() >= nMaxSize) { // Need to truncate the data
      data = data.substr(0, nMaxSize-1); // Allow a space for the null.
    }
    while(data.size() < nMaxSize-1) {
      data += '\0';	// If too short, fill with nulls.
    }

  }
  // If the string is an even number of bytes it must be padded by one
  // ' '.  Note that if nMaxSize was supplied, the string is already
  // correctly sized as an odd.
  //
  if(!(data.size() & 1)) {
    data += ' ';
  }

  // Now the string 'data' contains the exact number of 'functional'
  // characters to put in the buffer... and that number is odd.
  // We need to put all but the last character in the buffer a word
  // (two characters) at a time.
  // 
  union {
    char c[2];
    unsigned short w;
  } d;				// Ensure endianness is not an issue.
  int nChars =  data.size();
  int i;			// Want loop index after loop.
  for(i = 0; i < nChars - 1; i+=2) {
    d.c[0] = data[i];
    d.c[1] = data[i+1];
    *m_BufferPtr = d.w;
    ++m_BufferPtr;
  }
  // Now take care of the last character which is put along with
  // a terminating null.  This is why we wanted the loop index above
  // to stay in scope.
  d.c[0] = data[i];
  d.c[1] = '\0';
  *m_BufferPtr = d.w;
  ++m_BufferPtr;
}  

/*!
    Set the run number field of the buffer header.

	\param nRun - Run number to insert in the buffer.

*/
void 
CNSCLOutputBuffer::SetRun(unsigned short nRun)  
{
  m_Buffer[hdrWRUN] = nRun;
}  

/*!
    Determines if an entity will fit in the buffer free
    space.  The free space is defined to be the
    words between the current offset of m_BufferPtr 
    and the end of the buffer.

	\param  nWords - Size of the entity.

*/
bool 
CNSCLOutputBuffer::EntityFits(unsigned short  nWords)  
{
  int nOffset = m_BufferPtr.GetIndex();
  nOffset += nWords;		// Here's where the next entity would start.
  return (nOffset <= m_nWords);
}  

/*!
    Submits the buffer to the spectrodaq server for
    distribution.  The buffer state is set to 
    empty again, once this is done... although to
    work around bugs in the spectrodaq server
    it is recommended that this buffer be destroyed
    after a route.


    Full set of actions:
    - The buffer size is computed
    - The buffer checksum is computed.
    - The buffer is routed.
    - If desired, the sequence number is incremented.
    - The buffer is recreated at m_nWords size.
    - The new buffer's header is filled in.

*/
void 
CNSCLOutputBuffer::Route()  
{
  ComputeSize();
  ComputeChecksum();
  m_BufferBase.Route();


}  

/*!
    Seek the buffer pointer to a particular location.
    
    \param nOffset - Offset to seek to (see next).
    \param whence  - Meaning of the offset.  This
    value comes from any of the meanings for lseek, and is
    symbolically defined in <unistd.h>:
    - SEEK_SET - nOffset is an absolute offset (better be >= 0).
    - SEEK_CUR - nOffset is relative to the current value of 
    m_BufferPtr.
    -SEEK_END - nOffset is relative to the end of the buffer and
    better be negative.

    \exception CRangeError - If whence is invalid or nOffset results in 
    an out of bounds condition.
*/
void 
CNSCLOutputBuffer::Seek(int nOffset, int  whence)  
{
  int nAbsOff;
  switch(whence) {
  case SEEK_SET: {		// Absolute seek:
    nAbsOff = nOffset;
  }
  break;
  case SEEK_CUR: {
    nAbsOff = m_BufferPtr.GetIndex() + nOffset;
  }
  break;
  case SEEK_END: {
    nAbsOff = (m_nWords - 1) + nOffset;
  }
  break;
  default:
    throw CRangeError(SEEK_SET, SEEK_END, whence,
		      " CNSCLOUTPUTBuffer::seek- Invalid location for whence");
    return;
  }
  // If control passes here, nAbsOff has the absolute offset desired.
  
  DAQWordBufferPtr p(&m_Buffer);
  p += nAbsOff;
  
  m_BufferPtr = p;
}  

  /*!
    Increments the sequence number.
    

*/
void 
CNSCLOutputBuffer::IncrementSequence()
{
  m_nSequence++;
}  

/*!
    Clears the sequence number.


*/
void 
CNSCLOutputBuffer::ClearSequence()  
{
   m_nSequence = 0;
}
/*!
   Utility function to initialize the buffer header.
   */
void
CNSCLOutputBuffer::InitializeHeader()
{
  union {
    long l;
    short w[2];
  } longbuffer;

  m_BufferPtr         += BODYOFFSET;

  // Initialize the header to known and where possible sensible
  // initial values:

  // Take unused and unknown and set them to zero

  for(int i =0; i < BODYOFFSET; i++) {
    m_Buffer[i] = 0;
  }

  // initialize the words we know.

  longbuffer.l         = m_nSequence;
  m_Buffer[hdrLSEQ]    = longbuffer.w[0];
  m_Buffer[hdrLSEQ+1]  = longbuffer.w[1];
  
  if (daq_isJumboBuffer()) {
    m_Buffer[hdrWBUFFMT]= JUMBOLEVEL;
  }
  else {
    m_Buffer[hdrWBUFFMT] = REVLEVEL;
  }

  m_Buffer[hdrWSIG]    = SSIGNATURE;
  longbuffer.l         = LSIGNATURE;
  m_Buffer[hdrLSIG]    = longbuffer.w[0];
  m_Buffer[hdrLSIG+1]  = longbuffer.w[1];



}
/*!
   Return the number of entities currently in the buffer.
   */

unsigned short
CNSCLOutputBuffer::getEntityCount()
{
  return m_Buffer[hdrWENTITY];
}
#ifdef HIGH_PERFORMANCE

/*!
   Set the entity count to the user defined value:
*/
void
CNSCLOutputBuffer::setEntityCount(unsigned short entities)
{
  m_Buffer[hdrWENTITY] = entities;
}
#endif /* HIGH_PERFORMANCE */
/*!  
    Resize the buffer as requested.  
   \param - newsize int [in] Number of words for the new buffer size.Note that data may be lost
	    if newsize is smaller than the current buffersize.
*/
void
CNSCLOutputBuffer::Resize(int newsize)
{
   m_BufferBase.Resize(newsize); // Wait for resize to finish.
   m_nWords = newsize;		                // update the size word.
}
/*!
   Return the buffer type word (m_Buffer[1]):
*/
unsigned short
CNSCLOutputBuffer::getBufferType()
{
  return m_Buffer[1];
}
