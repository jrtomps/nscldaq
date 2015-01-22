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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";
 
// Class: CStringInteractor
// Provides mechanisms to interact with a string.
// E.g. in a network connection where a password
// may have been sent as an initial record.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "StringInteractor.h"    				


using namespace std;


static const  char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved StringInteractor.cpp \n";

// Functions for class CStringInteractor

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Read(unsigned int nBytes, void* pBuffer)
//  Operation Type: 
//     
int CStringInteractor::Read(unsigned int nBytes, void* pBuffer)  
{
  // Reads from the string.

  unsigned int nStringLength = m_sString.length();
  if(m_nReadCursor >= nStringLength) return 0; // EOS is end file.

  // Figure out range of chars to return.

  unsigned int nStart = m_nReadCursor;
  unsigned int nEnd   = nStart + nBytes - 1;
  if(nEnd >= nStringLength) {
    nEnd = nStringLength-1;
  }
  nBytes = nEnd - nStart + 1;
  char* pUserBuffer = (char*)pBuffer;
  while(m_nReadCursor <= nEnd) {
    *pUserBuffer++ = m_sString[m_nReadCursor++];
  }
  return nBytes;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Write(unsigned int nBytes, void* pBuffer)
//  Operation Type: 
//     
int CStringInteractor::Write(unsigned int nBytes, void* pBuffer)  
{
  // "Writes" to the interactor - this is a no-op
  return 0;
}
