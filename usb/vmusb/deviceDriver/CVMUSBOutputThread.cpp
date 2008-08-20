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


#include <config.h>
#include "CVMUSBOutputThread.h"
#include <DataBuffer.h>

// header bit fields.

static const unsigned EventCountMask(0xfff);
static const unsigned LastBuffer(0x800);

// Event header bit fields:

static const unsigned EventLengthMask(0xfff);
static const unsigned Continuation(0x1000);
static const unsigned StackMask(0xe000);
static const unsigned ScalerStack(0x2000);

static const unsigned HeaderSize(1);


/*!
   Returns the number of useful data words in the body of the buffer.
   In our case we have to remove the end of buffer delimeters from the count
   returned by the USB read:
   \param buffer - Data buffer struct.
   \return unsigned
   \retval Numbe of used words in the buffer.

*/
unsigned
CVMUSBOutputThread::bodySize(DataBuffer& buffer)
{
  return buffer.s_bufferSize/sizeof(uint16_t) - 2;
}

/*!

    \param buffer -reference to a data buffer struct.
    \return unsigned
    \retval number of events in the data buffer.
*/

unsigned
CVMUSBOutputThread::eventCount(DataBuffer& buffer)
{
  uint16_t*   pRawBuffer = buffer.s_rawData;
  return     (*pRawBuffer & EventCountMask);
}

/*!
   \param buffer - Refernce to a data buffer struct.
   \return unsigned
   \retval the size of the buffer header in words (how many words
           the caller needs to skip to get to the first event.
*/
unsigned
CVMUSBOutputThread::headerSize(DataBuffer& buffer)
{
  return HeaderSize;
}
/*!
   Returns the size of an event given a pointer to the event itself.
   The size returned is the size in words; including any header information.
   
   \param pEvent - Pointer to the event.
   \return uint32_t
   \retval Size of the event, including any header.

   \note This size will be passed back without modification to processEvent.
*/
uint32_t
CVMUSBOutputThread::eventSize(uint16_t* pEvent)
{
  uint32_t size(0);		// Size is accumulated here.
  uint16_t header;
  do {
    header      = *pEvent++;
    uint32_t s  = header & EventLengthMask;
    pEvent     += s;		// next chunk if there is one.
    size       += s+1;		// Count the headers in the size.
  } while( header & Continuation);

  return size;
}
/*!
  Process an event from the buffer:
  \param when   - Timestamp for when the event occured (really buffer timestamp).
  \param size   - event size gotten from eventSize() above.
  \param pEvent - Pointer to the event (what was passed in to eventSize above).

*/
void
CVMUSBOutputThread::processEvent(time_t when, uint32_t size, uint16_t* pEvent)
{

  // We're going to dispose of this as a scaler or an event:

  if (*pEvent & ScalerStack) {
    // For now we assume scalers all live in one fragment.
    // if that is ever not true, we need to 
    // re-marshall the scalers into another bit of storage:

    pEvent++;
    size--;            // Skip the header.

    int           number   = size*sizeof(uint16_t)/sizeof(uint32_t); // Scalers are 32 bits wide.
    uint32_t*     pScalers = reinterpret_cast<uint32_t*>(pEvent);

    scaler(when, number, pScalers);
  }
  else {
    event(size, pEvent);
  }
}
