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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CInteractor
// ABC for security class interactors.
// interactors are objects which allow the
// security classes to get data (e.g. passwords)
// from file-like entities.  An interactor need not,
// however be bound to a file.
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
#include "Interactor.h"    				


using namespace std;


static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved Interactor.cpp \n";

// Functions for class CInteractor

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     ReadWithPrompt(UInt_t nPromptSize, void* pPrompt, UInt_t nReadSize, void* pReadData)
//  Operation Type: 
//     
int 
CInteractor::ReadWithPrompt(UInt_t nPromptSize, void* pPrompt, 
                            UInt_t nReadSize, void* pReadData)  
{
  // Reads data after issuing a prompt.
  // Default implementation is to write,
  // flush and read.
  //
  // Returns number of bytes READ

  Write(nPromptSize, pPrompt);
  Flush();
  return Read(nReadSize, pReadData);
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Flush()
//  Operation Type: 
//     
void CInteractor::Flush()  
{
  // Flushes pending I/O (if it's an I/O based 
  // interactor) on the interactor.  Default implementation
  // is a no-op.
  //
  
}
