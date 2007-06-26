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
//////////////////////////CNSCLControlBuffer.cpp file////////////////////////////////////

#include <config.h>
#include "CNSCLControlBuffer.h"                  
#include <time.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
// Manifest constants:

static unsigned int offBTITLE = 16;
static unsigned int offLTIME  = 56;
static unsigned int offSBFTIME= 58;
static unsigned int BufferSize(offSBFTIME + 8);
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CNSCLControlBuffer  object;
   are performed.
*/
CNSCLControlBuffer::CNSCLControlBuffer (unsigned int nWords) :
  CNSCLOutputBuffer(nWords)
 
{
  getBuffer().SetTag(CNSCLOutputBuffer::m_ControlTag);
  SetTime();
} 

// Functions for class CNSCLControlBuffer

/*!
    Puts the title string in the buffer:
    - If the title string is less than 79 characters
      it is blank padded.
    - If the  title string is more than 79 characters
      it is truncated.
    - The 80'th character of the title string will always
       be a null ensuring the title can be used as a
       C Null terminated string.

	\param const string& rTitle.

*/
void 
CNSCLControlBuffer::PutTitle(const string& rTitle)  
{
  Seek(offBTITLE);
  PutString(rTitle.c_str(), 80); // Insert the title string.
  Seek(BufferSize);

}  

/*!
    Sets the time offset field in 10'ths of a second
    from the run start.
    \param nTime - longword time offset.

	\param unsigned long nTime

*/
void 
CNSCLControlBuffer::PutTimeOffset(unsigned long nTime)  
{
  Seek(offLTIME);
  PutLong(nTime);
  Seek(BufferSize);
}
/*!
  Put the current time in the buffer in NSCL DAQ format.
  */
void
CNSCLControlBuffer::SetTime()
{
  time_t t;
  time(&t);
  struct tm st;
  st = *(localtime(&t));
  
  Seek(offSBFTIME);
  PutWord(st.tm_mon+1);
  PutWord(st.tm_mday);
  PutWord(st.tm_year+1900);	// Unix years are relative to 1900.
  PutWord(st.tm_hour);
  PutWord(st.tm_min);
  PutWord(st.tm_sec);
  PutWord(0);			// Unix doesnt' give up 1/10'ths.
  Seek(BufferSize);
}

/*!
   Put a specific timestamp inthe buffer:
*/
void
CNSCLControlBuffer::PutTimestamp(struct tm stamp)
{
  Seek(offSBFTIME);
  PutWord(stamp.tm_mon+1);
  PutWord(stamp.tm_mday);
  PutWord(stamp.tm_year+1900);	// Unix years are relative to 1900.
  PutWord(stamp.tm_hour);
  PutWord(stamp.tm_min);
  PutWord(stamp.tm_sec);
  PutWord(0);			// Unix doesnt' give up 1/10'ths.
  Seek(BufferSize);

}
