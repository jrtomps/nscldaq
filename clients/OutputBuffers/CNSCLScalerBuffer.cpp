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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CNSCLScalerBuffer.cpp file////////////////////////////////////

#include <config.h>
#include "CNSCLScalerBuffer.h"           
#include "buftypes.h"



#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
// Manifest constants:

static const int offLENDTIME = 16; // End of interval rel to run start.
static const int offUNUSED1  = 18; // 3 words unused
static const int offLBEGTIME = 21; // Begin of interval rel to run start.
static const int offUNUSED2  = 23; // 3 more unused words.
static const int offSCALERS  = 26; // Where the scalers start.

/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CNSCLScalerBuffer  object;
   are performed.
*/
CNSCLScalerBuffer::CNSCLScalerBuffer (unsigned nWords) :
  CNSCLOutputBuffer(nWords)
{
  SetType(SCALERBF);		// By default, ordinary scaler.
  Seek(offSCALERS);		// Pointer -> first scaler.
  getBuffer().SetTag(CNSCLOutputBuffer::m_ControlTag);

} 



// Functions for class CNSCLScalerBuffer

/*!
    Puts a vector of scalers into the buffer.
    \param vScalers - Scalers to insert in the buffer.

	\param vector<unsigned long vScalers

*/
void 
CNSCLScalerBuffer::PutScalerVector(const vector<unsigned long>& vScalers)  
{
  for(int i =0; i < vScalers.size(); i++) {
    PutEntity(&(vScalers[i]), sizeof(unsigned long)/sizeof(short));
  }
}  

void
CNSCLScalerBuffer::PutScalerVector(const vector<uint32_t>& vScalers)
{
  for(int i =0; i < vScalers.size(); i++) {
    PutEntity(&(vScalers[i]), sizeof(unsigned long)/sizeof(short));
  }
}

/*!
    Sets the interval start time in the buffer.  
    The interval start time is a longword value
    representing the 10'ths of a second into the
    run.
    \param nStart - Number of 10'ths of a second
          since the start of the run at which the interval
          started.

	\param unsigned long nStart

*/
void 
CNSCLScalerBuffer::SetStartTime(unsigned long nStart)  
{
  union {
    unsigned long l;
    unsigned short s[2];
  } d;
  d.l = nStart;
  m_Buffer[offLBEGTIME]   = d.s[0];
  m_Buffer[offLBEGTIME+1] = d.s[1];
 
}  

/*!
    Set the interval end time in 10'ths of a 
    second from the beginning of the run.
    \param nEndTime - 10'ths of a second from
          run start of the end of the interval.

	\param unsigned long nEndTime

*/
void 
CNSCLScalerBuffer::SetEndTime(unsigned long nEndTime)  
{
  union {
    unsigned long l;
    unsigned short s[2];
  } d;
  d.l = nEndTime;

  m_Buffer[offLENDTIME]   = d.s[0];
  m_Buffer[offLENDTIME+1] = d.s[1];
 
}
