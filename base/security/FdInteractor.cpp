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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CFdInteractor
// Represents an interactor where data is exchanged
// across an entity which can be represented by a 
// single file descriptor... e.g. a pipe.
//

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
#include "FdInteractor.h"    				

using namespace std;


static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved FdInteractor.cpp \n";

// Functions for class CFdInteractor

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Read(int nBytes, void* pBuffer)
//  Operation Type: 
//     
int CFdInteractor::Read(UInt_t nBytes, void* pBuffer)  
{
  // Reads data from the fd (delegates to read(2)
  return read(m_nFd, pBuffer, nBytes);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Write(int nBytes, void* pData)
//  Operation Type: 
//     
int CFdInteractor::Write(UInt_t nBytes, void* pData)  
{
  // Writes data to the peer (delegates to write(2))
  return write(m_nFd, pData, nBytes);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Flush()
//  Operation Type: 
//     
void CFdInteractor::Flush()  
{
  // Flushes data currently in the fd stream.

}
