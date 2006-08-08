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
#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif 
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

#ifndef __CBUFFERQUEUE_h
#include <CBufferQueue.h>
#endif

/*!
    \file DataBuffer
        Defines the structure of data buffers that will be
        passed around from readout thread to output thread.
        These are managed in two buffer queues.:
	gFilledBuffers is the queue of buffers with data and
        gFreeBuffers is a queue of empty buffers available for
	use by the readout thread.
*/
struct DataBuffer {
  uint32_t   s_bufferSize;
  uint32_t   s_bufferType;
  time_t     s_timeStamp;	//!< When the buffer was received.
  uint16_t   s_rawData[1];	//!< Really larger than that

};


extern CBufferQueue<DataBuffer*>  gFilledBuffers;
extern CBufferQueue<DataBuffer*>  gFreeBuffers;

//  A couple of useful unbound functions:

extern DataBuffer* createDataBuffer(uint32_t bodySize); //!< Create a new data buffer.
extern void        destroyDataBuffer(DataBuffer* pBuffer); //!< Free data buffer.
