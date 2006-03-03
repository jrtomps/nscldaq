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

//////////////////////////CRunVariableBuffer.h file//////////////////////////////////

#ifndef __CRUNVARIABLEBUFFER_H  
#define __CRUNVARIABLEBUFFER_H
                               
#ifndef __CNSCLSTRINGLISTBUFFER_H
#include "CNSCLStringListBuffer.h"
#endif
                               
/*!
  Formats a run variable buffer. Run variable buffers contain
  lists of strings describing the value of variables which are
  constant within the duration of the run.  Examples of variables which
  could be put in this buffer are:
  - Current Run number.
  - Current Run Title
  - Current scaler count
  - List of people on shift.

 */		
class CRunVariableBuffer  : public CNSCLStringListBuffer        
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

    CRunVariableBuffer (unsigned nWords=4096); //!< Default constructor.
     ~ CRunVariableBuffer ( ) { } //!< Destructor.
private:
    CRunVariableBuffer& operator= (const CRunVariableBuffer& rhs); //!< Assignment
    int         operator==(const CRunVariableBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CRunVariableBuffer& rhs) const;
    CRunVariableBuffer(const CRunVariableBuffer& rhs); //!< Copy constructor.
public:


};

#endif
