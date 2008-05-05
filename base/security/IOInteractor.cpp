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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CIOInteractor
// Performs the duties of an interactor when there's
// a need for separate input and output connections.
// Typically, m_InputInteractor and m_OutputInteractor
// will be CFdInteractors, but there' s no pressing need
// to make that requirement so we don't.
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
#include "IOInteractor.h"    				


using namespace std;


static char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved IOInteractor.cpp \n";

// Functions for class CIOInteractor

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Read(UInt_t nBytes, void* pBuffer)
//  Operation Type: 
//     
int CIOInteractor::Read(UInt_t nBytes, void* pBuffer)  
{
  // Reads from the m_InputInteractor.
  return m_InputInteractor->Read(nBytes, pBuffer);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Write(UInt_t nBytes, void* pBuffer)
//  Operation Type: 
//     
int CIOInteractor::Write(UInt_t nBytes, void* pBuffer)  
{
  // Writes to the m_OutputInteractor
  return m_OutputInteractor->Write(nBytes, pBuffer);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Flush()
//  Operation Type: 
//     
void CIOInteractor::Flush()  
{
  // Flushes the m_OutputInteractor.
  m_OutputInteractor->Flush();
}
