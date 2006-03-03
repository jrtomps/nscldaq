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

 
//////////////////////////CNSCLDocumentationBuffer.h file//////////////////////////////////

#ifndef __CNSCLDOCUMENTATIONBUFFER_H  
#define __CNSCLDOCUMENTATIONBUFFER_H
                               
#ifndef __CNSCLSTRINGLISTBUFFER_H
#include "CNSCLStringListBuffer.h"
#endif
                               
/*!
   Formats a buffer containing documentation about the
   packet types which will appear in the current buffer.
   
 */		
class CNSCLDocumentationBuffer  : public CNSCLStringListBuffer        
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

    CNSCLDocumentationBuffer (unsigned nWords = 4096); //!< Default constructor.
     ~ CNSCLDocumentationBuffer ( ) { } //!< Destructor.
private:
    CNSCLDocumentationBuffer& operator= (const CNSCLDocumentationBuffer& rhs); //!< Assignment
    int         operator==(const CNSCLDocumentationBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CNSCLDocumentationBuffer& rhs) const;
    CNSCLDocumentationBuffer(const CNSCLDocumentationBuffer& rhs); //!< Copy constructor.
public:

};

#endif

