/*!
    Creational class for the packet module.  The packet module
   is a container for other modules.
*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CPACKETCREATOR_H  //Required for current class
#define __CPACKETCREATOR_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CMODULECREATOR_H     //CModuleCreator
#include "CModuleCreator.h"
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif


class CPacketCreator  : public CModuleCreator        
{
public:
	//   Constructors and other cannonical operations.
	
  CPacketCreator ();
  ~CPacketCreator ( );
  CPacketCreator (const CPacketCreator& aCPacketCreator );
  CPacketCreator& operator= (const CPacketCreator& aCPacketCreator);
  int operator== (const CPacketCreator& aCPacketCreator) const;


  // Class operations:

public:

   virtual   CDigitizerModule* Create (CTCLInterpreter& rInterp, 
				       CTCLResult& rResult, 
				       int nArgs, char** pArgs)   ; 
   virtual   string  Help (); 

};

#endif
