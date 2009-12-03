/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef __CCAENV792CREATOR_H  //Required for current class
#define __CCAENV792CREATOR_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CMODULECREATOR_H     //CModuleCreator
#include "CModuleCreator.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif
 
/*!
  Objects of this class can be registered with the Module command to 
  create CAEN V792 QDC modules. This is done by overriding the 
  Create member function.
  The Create member now serves two purpoes:
  - Instantiate a module.
  - Pass the configuration parameters on the tail of the command to the 
    module's Configure function.
*/
class CCAENV792Creator  : public CModuleCreator        
{
public:
	// Consructors and other canonical operations:
	
  CCAENV792Creator ();
  virtual ~CCAENV792Creator ( );  
   CCAENV792Creator (const CCAENV792Creator& aCCAENV792Creator );
   CCAENV792Creator& operator= (const CCAENV792Creator& aCCAENV792Creator);
   int operator== (const CCAENV792Creator& aCCAENV792Creator) const;

  // Class operations:

public:

   virtual   CReadableObject* 
		  Create (CTCLInterpreter& rInterp, CTCLResult& rResult, 
			     int nArgs, char** pArgs)   ; // 
   virtual   std::string  Help ()   ; 

};

#endif
