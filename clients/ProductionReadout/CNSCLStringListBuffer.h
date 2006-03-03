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


//////////////////////////CNSCLStringListBuffer.h file//////////////////////////////////

#ifndef __CNSCLSTRINGLISTBUFFER_H  
#define __CNSCLSTRINGLISTBUFFER_H
                               
#ifndef __CNSCLOUTPUTBUFFER_H
#include "CNSCLOutputBuffer.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif
                               
/*!
   Encapsulates the formatting of any of a number of 
   buffers which consist of lists of string.
   String list buffer bodies contain sequences of
   null terminated strings.  Each string begins on
   a word boundary and, if necessary, a pad blank
   is added to ensure this, all strings are null terminated.
   
 */		
class CNSCLStringListBuffer  : public CNSCLOutputBuffer        
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

    CNSCLStringListBuffer (unsigned nWords = 4096); //!< Default constructor.
     ~ CNSCLStringListBuffer ( ) { } //!< Destructor.

private:
    CNSCLStringListBuffer& operator= (const CNSCLStringListBuffer& rhs); //!< Assignment
    int         operator==(const CNSCLStringListBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CNSCLStringListBuffer& rhs) const {
       return !(operator==(rhs));
    }
    CNSCLStringListBuffer(const CNSCLStringListBuffer& rhs); //!< Copy constructor.
public:

	// Class operations:
public:
     bool PutEntityString (const STD(string)& rString)  ;
 
};

#endif
