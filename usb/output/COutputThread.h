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

#ifndef __COUTPUTTHREAD_H
#define __COUTPUTTHREAD_H

#ifndef __THREAD_H
#include <Thread.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward definitions:

struct DataBuffer;
class CRingBuffer;
/*!
    This class bridges the gap between the buffer format of the
    USB devices and ring buffers.  
  
    The data
    we are using will come from a buffer queue.  Elements
    of the buffer queue will have the following format:

\verbatim
    +--------------------------------+
    |  Buffer size                   | 
    +--------------------------------+
    |  Buffer Type                   |
    +--------------------------------+
    | Time stamp                     |
    +--------------------------------+
    |   Buffer contents as gotten    |
    | from the device, see the       |
    | manual                         |
    +- - - - - - -- - - - - - - - - -+

\endverbatim
    
    -#  Buffer size is gotten from the read from the device
        and indicates the number of bytes of data
        in the body and its header word (does not include either itself nor
	the buffer type.
    -#  The buffer type is one of the following:
        - 1   Run starting... in this case there will be no VM-USB body.
        - 2   USB data  note that run end is determined by seeing the
	      last buffer indicator in the buffer.
    -# We will generate only the following sorts of NSCL data buffers:
        - 11  Begin run.
        - 12  End Run
        - 1   Physics data
        - 2   Scaler Data.
    -# Time stamp is result of time(2) when the buffer was received from
       the VMUSB.

 \note  This class is a separate thread of execution.
 \note  This is an abstract base class that captures the common elements
        of the output thread for the CC and VM usb devices.
 \note  A global variable: gFilledBuffers is a CBufferQueue that contains
        the data shown above and is used to receive raw data buffers from
	the readout thread.
  \note There is no need to start/stop thread each run.   Once a run is over,
        this thread will simply block on the buffer queue until the next run
        emits the begin run buffer.
  
  This is an abstract base class as there are some elements of the
  output thread that are device specific.  The device specific elements are
  captured in the pure virtual functions:
  - bodySize   - Returns the number of words in a buffer body.
  - eventCount - Given a DataBuffer*, this should return the number of events in 
    the body part of the buffer.
  - headerSize - Given a DataBuffer*, returns the number of words prior to the
    first event in the buffer.
  - eventSize - Given a pointer to an event within the buffer, returns the number
    of words in the event.
  - processEvent - Called for each event in the buffer, this is expected to call
    back to event or scaler to create/dispose of the appropriate ring item.


*/

class COutputThread  : public Thread
{
  // Thread local data:
private:
  // These are fetched from the CRun state at start of run.

  uint32_t    m_runNumber;	// Run number;
  std::string m_title;          // Run title

  // other data:
private:
  time_t      m_startTimestamp;    //!< Run start time.
  time_t      m_lastStampedBuffer; //!< Seconds into run of last stamped buffer
  uint64_t    m_eventCount;	   //!< Number of events this run.
  CRingBuffer* m_pRing;            // Where the data goes..

  // Constuctors and other canonicals.

public:
  COutputThread();
  virtual ~COutputThread();
private:
  COutputThread(const COutputThread& rhs);
  COutputThread& operator=(const COutputThread& rhs);
  int operator==(const COutputThread& rhs) const;
  int operator!=(const COutputThread& rhs) const;
public:

  // Thread operations are all non-public in fact.. don't want to call them
  // from outside this class.. only from within the thread.. This includes the
  // thread entry point.

protected:

  virtual void run();

  DataBuffer& getBuffer();
  void freeBuffer(DataBuffer&  buffer);

  void processBuffer(DataBuffer& buffer);
  virtual void processUSBData(DataBuffer& buffer);

  void startRun(time_t when);
  void endRun(time_t when);
  void scaler(time_t when, 
	      int    number,
	      uint32_t* pScalers);
  void event(uint32_t size, void* pBody);
  void eventCount(time_t when);

  // The concrete interface specific implementation must
  // provide the following:

  virtual unsigned bodySize(DataBuffer& buffer)    = 0;
  virtual unsigned eventCount(DataBuffer& buffer) = 0;
  virtual unsigned headerSize(DataBuffer& buffer) = 0;
  virtual uint32_t eventSize(uint16_t* pEvent)    = 0;
  virtual void     processEvent(time_t when, 
				uint32_t size, 
				uint16_t* pEvent) = 0;


    // utilities.

private: 
  void openRing();


};

#endif
