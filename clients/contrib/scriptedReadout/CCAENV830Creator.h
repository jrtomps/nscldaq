/*!
  Creates a CCAENV830 readout module.   This module is suitable for use
  in the main readout section for systems that require e.g. precise absolute
  timing to be read with an event.  Typically, this class will be instantiated
  and registered with a module command creational object. e.g.
  \verbatim
  ...
  // Make a new interpreter:

  pInterp = new CTCLInterpreter();
  Tcl_Init(pInterp->getInterpreter());

  // Create the module creation infrastructure:

  pDictionary = new CDigitizerDictionary;
  pReader     = new CReadOrder(pInterp, pDictionary);
  pCreator    = new CModuleCommand(pInterp,
				   pDictionary,
				   pReader);
  pCreator->AddCreator(new CCAENV830);
  ...
  \endverbatim

  The creator is then used by the Module command to match
  the module type recognized by the CCAENV830Creator ("caenv830")
  to instantiate modules that can then be configured by the user
  through TCL scripting.

*/
#ifndef __CCAENV830CREATOR_H  //Required for current class
#define __CCAENV830CREATOR_H

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

class CTCLInterpreter;
class CTCLResult;
class CReadableObject;


class CCAENV830Creator  : public CModuleCreator        
{
private:
   
	/* Constructors and other canonical operations: */
	
public:
  CCAENV830Creator ();
  virtual ~CCAENV830Creator ( );
  CCAENV830Creator (const CCAENV830Creator& rhs );
  CCAENV830Creator& operator= (const CCAENV830Creator& rhs);
  int operator== (const CCAENV830Creator& rhs) const;
  int operator!= (const CCAENV830Creator& rhs) const {
      return !(operator==(rhs));
   }

  // Class operations:

public:

   virtual   CReadableObject* Create (CTCLInterpreter& rInterp, 
				       CTCLResult& rResult, 
				       int nArgs, char** pArgs)   ; // 
   virtual   string  Help (); // 

};

#endif
