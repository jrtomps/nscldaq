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

// Class: CInteractor                     //ANSI C++
//
// ABC for security class interactors.
// interactors are objects which allow the
// security classes to get data (e.g. passwords)
// from file-like entities.  An interactor need not,
// however be bound to a file.
// 
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved Interactor.h
//

#ifndef __INTERACTOR_H  //Required for current class
#define __INTERACTOR_H

#include <daqdatatypes.h>        //Required for include files
                               
class CInteractor      
{                       
			
protected:

public:

   // Constructors and other cannonical operations:

  CInteractor ()  
  { 
  } 
  virtual  ~ CInteractor ( )  // Destructor 
  { }  
    
   //Copy constructor  - note that some derived may not allow.

  CInteractor (const CInteractor& aCInteractor ) 
  {
  }                                     

   // Operator= Assignment Operator  - note that some derived may not allow.

  CInteractor& operator= (const CInteractor& aCInteractor) 
  {
    return *this;
  }
 
   //Operator== Equality Operator -comparison makes no sense.
private:
  int operator== (const CInteractor& aCInteractor) const;
public:
	
  // Class interface:

public:

 virtual   int Read (UInt_t nBytes, void* pData)   = 0  ;
 virtual   int Write (UInt_t nbytes, void* pData)   = 0  ;
 virtual   int ReadWithPrompt (UInt_t nPromptSize, void* pPrompt, 
			       UInt_t nReadSize, void* pReadData)    ;

 virtual   void Flush ();
 
};

#endif
