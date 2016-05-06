/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
static const char* Copyright = "(C) Copyright Michigan State University 2006, All rights reserved";
//  CBufferDecoder.cpp
//     Abstract base class which provides decode capability for buffers
//     The idea is to break apart the buffer cracking into software which is
//     aware of buffer bodies and buffer header structures.  This is useful
//     because typically buffer headers don't depend on the experiment, but
//     rather the daq system, while buffer bodies depend on both experiment
//     and DAQ.  CBufferDecoders provide a single repository for buffer header
//     structure knowledge.
//     
//
//   Author:
//      Ron Fox
//      NSCL
//      Michigan State University
//      East Lansing, MI 48824-1321
//      mailto:fox@nscl.msu.edu
//
//////////////////////////.cpp file/////////////////////////////////////////////////////

// Header Files:
#include <config.h>
#include "CBufferDecoder.h"
#include <CEventFormatError.h>
#include <CBuffer.h>
#include <CBufferTranslator.h>
#include <CTranslatorPointer.h>
#include <buftypes.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
// Functions for class CBufferDecoder


CBuffer CBufferDecoder::onBuffer(const CBuffer& buffer) 
{
 return CBuffer();
}


bool
CBufferDecoder::blockMode()
{
    return true;
}


/**
 * OnSourceAttach
 *   Called just after a new event source was attached.
 */
void
CBufferDecoder::OnSourceAttach()
{}
/**
 * OnSourceDetach
 *     Called just before an event source is detached.
 */
void
CBufferDecoder::OnSourceDetach()
{}

/**
 * OnEndFile
 *    Called just after a source sensed an end file.
 */
void
CBufferDecoder::OnEndFile()
{}

////////////////////////////////////////////////////////////////////////////
//
// Function:
//   void ThrowIfNoBuffer(const char* pszWhatImDoing)
// Operation type:
//   Utility function.
//
void CBufferDecoder::ThrowIfNoBuffer(const char* pszWhatImDoing) {
  //  Throws an exception if there is no current buffer.
  if(!m_pBuffer) {
    CEventFormatError e((int)CEventFormatError::knNoCurrentBuffer,
			pszWhatImDoing,
			(UInt_t*)kpNULL, 0, 0);
    throw e;
  }
} 
