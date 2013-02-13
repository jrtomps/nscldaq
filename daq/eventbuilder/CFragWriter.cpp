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

/**
 * @file CFragwriter.cpp
 * @brief Fragment writer (fragment -> ring item -> file number) class implementation
 */

#include "CFragWriter.h"
#include "fragment.h"
#include <io.h>

#include <DataFormat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>

/**
 * Constructor
 *
 * @param fd - File descriptor to output ring fragments on.
 */
CFragWriter::CFragWriter(int fd) :
  m_fd(fd) {}

/**
 * operator() 
 *
 * Convert a fragment to an EVB_FRAGMENT ring item and output it to
 * stdout.
 *
 * @param pFragment - Really a pointer to a 'flattened' fragment.  A
 *                    flattened fragment is a fragment header that is immediately
 *                    followed by its payload.
 */
void
CFragWriter::operator()(void* pFragment)
{
  // Cast the pointer to a fragment header and also locate the payload.

  EVB::pFragmentHeader pHeader = reinterpret_cast<EVB::pFragmentHeader>(pFragment);
  void*                pPayload = reinterpret_cast<void*>(pHeader+1); // payload after the fragment.

  /*
    Create the ring item except for the data section
    the - sizeof(uint8_t) removes the stub of a 1 byte payload that is part of the
    struct (s_body).
  */
 
  EventBuilderFragment itemHeader;
  itemHeader.s_header.s_type = EVB_FRAGMENT;
  itemHeader.s_header.s_size = sizeof (itemHeader) + pHeader->s_size - sizeof(uint8_t);
  itemHeader.s_timestamp     = pHeader->s_timestamp;
  itemHeader.s_sourceId      = pHeader->s_sourceId;
  itemHeader.s_payloadSize   = pHeader->s_size;
  itemHeader.s_barrierType   = pHeader->s_barrier;

  // Write the header and the payload.

  Write(sizeof(itemHeader) - sizeof(uint8_t), &itemHeader);
  Write(itemHeader.s_payloadSize, pPayload);

}

/**
 * Write
 *
 * Write a block of data to the file descriptor.  If needed multiple underlying
 * write(2) calls are made.
 * 
 * @param size - Number of bytes to write.
 * @param pBuffer - Pointer to the first word of the buffer to write.
 */
void
CFragWriter::Write(size_t nBytes, void* pBuffer)
{
  try {
    io::writeData(m_fd, pBuffer, nBytes);
  }
  catch(int e) {
    if(e) {
      throw std::string(strerror(e));
    } else {
      throw std::string("Premature end of file");
    }
  }
}

