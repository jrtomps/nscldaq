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
//////////////////////////CRunVariableBuffer.cpp file////////////////////////////////////
#include <config.h>
#include "CRunVariableBuffer.h"    
#include "buftypes.h"              

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
	//Default constructor alternative to compiler provided default constructor
	//Association object data member pointers initialized to null association object 
/*!
  Constructs a run variable buffer. Once this is done, all you
  have to do is call PutEntityString for each variable string you
  want to insert in the buffer, and then Route the buffer.
*/
CRunVariableBuffer::CRunVariableBuffer (unsigned nWords) :
  CNSCLStringListBuffer(nWords)
 
{
  SetType(RUNVARBF); 
} 
