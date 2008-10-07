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

// Class: CVMEMapCommand                     //ANSI C++

#ifndef __CVMEMAPCOMMAND_H  //Required for current class
#define __CVMEMAPCOMMAND_H

                               //Required for base classes
#ifndef __TCLPROCESSOR_H
#include <TCLProcessor.h>
#endif

#ifndef __VMEMODULE_H
#include <VmeModule.h>
#endif

                         
class CVMEMapCommand  : public CTCLProcessor        
{ 
  unsigned long       m_nPhysBase;  //!< VME  base address of map.
  UInt_t              m_nSize;      //!< Size of segment in bytes.
  UInt_t              m_nCrate;      //!< Crate segment lives in.
  CVmeModule*         m_pSpace;     //!< Accessor for the space.
  std::string         m_Name;       //!< Name of the space.

 
protected:
  
  int Get (CTCLInterpreter& rInterp, CTCLResult& rResult, int nSize,
	   int nArgs, char** pArgs)   ;
  int Set (CTCLInterpreter& rInterp, CTCLResult& rResult, int nSize,
	   int  nArgs, char** pArgs)    ;
  int Usage(CTCLResult& rResult);

public:
  // Constructor.

  CVMEMapCommand ( std::string      Name, 
		   CTCLInterpreter* pInterp,
		   ULong_t          Base, 
		   UInt_t           nSize,
		   UInt_t           nCrate,
		   CVmeModule::Space space);
   ~ CVMEMapCommand ( ); 

   /// Selectors:

   ULong_t getBase() const {
     return m_nPhysBase;
   }
  
public:       
  virtual   int operator() (CTCLInterpreter& rInterp, 
			    CTCLResult& rResult, int nArgs, char** pArgs);
  virtual void OnDelete();

};

#endif
