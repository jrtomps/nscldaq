#ifndef __CSCALERMODULE_H  //Required for current class
#define __CSCALERMODULE_H

//
// Include files:
//

#ifndef __HISTOTYPES_H
#include <histotypes.h>
#endif

                               //Required for base classes
#ifndef __CDIGITIZERMODULE_H   // CDigitizerModule
#include "CDigitizerModule.h"
#endif
 
#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

// Forward class definitions:

class CTCLInterpreter;
 
/*!
Abstract base class for a scaler module.  The only
difference between a scaler module and a digitizer
is an overload for the Read member function that
allows the module to read into a buffer that is
pointed to by a long*
*/
class CScalerModule  : public CDigitizerModule        
{

public:
	// Constructors and other cannonical member functions:

  CScalerModule (const string& rName,
		 CTCLInterpreter& rInterp);
 ~ CScalerModule ( );  
  
  // Copy like operations are illegal and therefore comparison makes no
  // sense either:
  
private: 
   CScalerModule (const CScalerModule& aCScalerModule );
   CScalerModule& operator= (const CScalerModule& aCScalerModule);
   int operator== (const CScalerModule& aCScalerModule) const;
   int operator!= (const CScalerModule& aCScalerModule) const;
public:

public:

  // Class operations:

public:

  virtual   ULong_t* Read (ULong_t* pBuffer)   = 0 ; // 

};

#endif
