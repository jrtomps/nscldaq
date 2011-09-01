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

using namespace std;
#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
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

/*!
    This class bridges the gap between the buffer format of the
    VM-USB and the NSCL Buffer format.  We will not bother to
    reformat the data within the buffer, just change the
    header word into an NSCL style buffer.  The data
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
    |   Buffer contents as described |
    | in section 4.6 of the VM USB   |
    | manual                         |
    +- - - - - - -- - - - - - - - - -+

\endverbatim
    
    -#  Buffer size is gotten from the read from the VM_USB
        and indicates the number of bytes of data
        in the body and its header word (does not include either itself nor
	the buffer type.
    -#  The buffer type is one of the following:
        - 1   Run starting... in this case there will be no VM-USB body.
              and the buffersize will be size of Spectrodaq buffer desired.
        - 2   VM-USB data  note that run end is determined by seeing
              the LB bit in the VM-USB header.
    -# We will generate only the following sorts of NSCL data buffers:
        - 11  Begin run.
        - 12  End Run
        - 1   Physics data
        - 2   Scaler Data.
    -# Time stamp is result of time(2) when the buffer was received from
       the VMUSB.

 \note  This class is a separate thread of execution.
 \note  A global variable: gFilledBuffers is a CBufferQueue that contains
        the data shown above and is used to receive raw data buffers from
	the readout thread.
  \note There is no need to start/stop thread each run.   Once a run is over,
        this thread will simply block on the buffer queue until the next run
        emits the begin run buffer.
  \note The global variable bufferMultiplier determines the size of the output buffer size.
        The output buffersize is bufferMultiplier*26*1024+32  
        the 26*1024 represent the size of a single VM-USB buffer (note that we are assuming
        mixed mode and event spanning on.
*/

class COutputThread  : public DAQThread
{
  // Thread local data:
private:
  // These are fetched from the CRun state at start of run.

  uint32_t    m_runNumber;	// Run number;
  std::string m_title;          // Run title

  // other data:
private:
  uint32_t    m_sequence;	   // Buffer sequence number.
  size_t      m_outputBufferSize;  // Bytes in output buffers.
  timespec    m_startTimestamp;    //!< Run start time.
  timespec    m_lastStampedBuffer; //!< Seconds into run of last stamped buffer.
  size_t      m_nOutputBufferSize;       //!< size of output buffer in bytes.
                                   //!< determined at the start of a run.
  uint8_t*    m_pBuffer;	   //!< Pointer to the current buffer.
  uint8_t*    m_pCursor;           //!< Where next event goes in buffer.
  uint8_t*    m_pEventStart;       //!< Pointer to start of current event in buffer.
  size_t      m_nWordsInBuffer;    //!< Number of words already in the buffer.
  size_t      m_nEventsInBuffer;   //!< Number of complete events in the buffer.



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

  // Thread operations are all non-public in fact.. don't want to call them4
  // from outside this class.. only from within the thread.. This includes the
  // thread entry point.

protected:

  virtual int operator()(int argc, char** argv); //!< Entry point.
private:
  void bufferToSpectrodaq(void* pBuffer, 
			  unsigned int sdaqTag,
			  size_t sdaqWords, 
			  size_t copyInSize); // 
  DataBuffer& getBuffer();		      // 
  void freeBuffer(DataBuffer&  buffer);	      // 
  void processBuffer(DataBuffer& buffer);     // 
  void processEvents(DataBuffer& buffer);     //
  void formatControlBuffer(uint16_t type, void* buffer); // 
  void  overflowOutputBuffer();           //
  uint8_t* newOutputBuffer();                //
  void startRun(DataBuffer& buffer); // 
  void endRun(DataBuffer& buffer);   // 
  void flush();			     // 
  void event(void* pData);      //
  void scaler(void* pData);	//
  void fillEventHeader();	// 
  void sendToTclServer(uint16_t* pEvent);
};


#endif
