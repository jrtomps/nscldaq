////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////

#include <config.h>
#include "CCAENV830Creator.h"    				
#include "CCAENV830Module.h"
#include "CReadableObject.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>

#include <assert.h>


using namespace std;


/*!
   Creates a creational object for CAENV830 objects.

   
*/
CCAENV830Creator::CCAENV830Creator ()
   : CModuleCreator(string("caenv830"))
{   
} 
/*!
   Destructor: in this case doesn't need to  do any thing.
*/
CCAENV830Creator::~CCAENV830Creator ( )
{
}
/*!
   Copy constructor just invokes base class copy constructor.
   \param rhs const CCAENV830Creator& [in] 
      The source of the copy construction.
*/
CCAENV830Creator::CCAENV830Creator (const CCAENV830Creator& rhs ) 
  : CModuleCreator (rhs) 
{
} 

 /*!
 
    Assignment operator.. Again, this is done by the base class:
    \param rhs const CCAENV830Creator& [in]  
       right hand side of the assignment operator.
      \return CCAENV830Creator& reference to this after the
	 assignment.  This is done to support operator chaining.
 */
CCAENV830Creator& 
CCAENV830Creator::operator= (const CCAENV830Creator& rhs)
{ 
   if(this != &rhs) {
      CModuleCreator::operator=(rhs);
   }
  return *this;
}
/*!
   Compare for equality.  Since there is no member data for this 
   class, we rely on the base class's ideas of equality:
   \param rhs const CCAENV830Creator& [in]
      reference to the object we are being compared with.
      
   \return int   nonzero if the objects are considered identical and zero if not.
*/
int 
CCAENV830Creator::operator== (const CCAENV830Creator& rhs) const
{
   return (CModuleCreator::operator==(rhs));
}

// Functions for class CCAENV830Creator

/*!  

Returns a new instance of the scalermodule 
creaetd by this creator.  The mdule is new'd into
being and therefore must be deleted by the ultimate
user.  The parameters passed in are also passed
to the module's configuration function.
Note that the digitizer module is really a 
CScalerModule*

   \param rInterp  CTCLInterpreter& [in]
      Refers to the interpreter that is exeucting the module creational
      command.
   \param rResult CTCLResult& [out]
      Refers to the result object for the interpreter.  The result object
      will be empty if all went ok.  Otherwise it will be a string describing the
      reason the creation failed or did not go completly as planned.
   \param nArgs  int [in]
      Number of unprocessed command line options.
   \param pArgs char** [in]
      List of the remaining parameters.  Only the command has been eaten up.
      The module name is used in instantiating the module, and the type is 
      assumed to be correct (else we wouldn't get called).  Remaining parameters
      are considrered to be configuration parameters and are parsed by the object
      once created.
      
      \return CReadableObject*  A pointer to the resulting module.
   
   The configuration command will be of the form:
   \verbatim
   module name "caenv820" ?optional-config-params?
   \endverbatim.
   
   Where:
      - name is used to refer to the module and will be the configuration command
        for the new moduile.
      - "caenv820" is the module type, indicating that a CAENV830/830 scaler
	 is being created.
      - ?optional-config-params? are optional parameters used to 
         configure the module after instantiation.  These parameters are in pairs
	 of the form keyword value  and can be any set of parameters that
	 are acceptable to name config.

*/
CReadableObject* 
CCAENV830Creator::Create(CTCLInterpreter& rInterp, CTCLResult& rResult, 
				    int nArgs, char** pArgs)  
{ 
   assert(nArgs >= 2);            // Need to have enough parameters.
   string sName(*pArgs);
   nArgs--; pArgs++;
   
   assert(*pArgs == getModuleType());
   nArgs--;
   pArgs++;
   
   CReadableObject* pModule = new CCAENV830Module(sName, rInterp);
   if(nArgs) {
     int status = pModule->Configure(rInterp, rResult, nArgs, pArgs);
     if(status != TCL_OK) {	// If configure fails, it will fill in 
       delete pModule;		// rResult, but we must delete and return null.
       pModule = (CReadableObject*)NULL;
     }
   }   
   return pModule;
   
}  

/*!  Function: 	
 Returns a string describing the module type and
whatever else the module driver author wants to display
about that module type in response to the module -help
command.

*/
string  
CCAENV830Creator::Help()  
{
   return string("Creates a CAEN 830 32 channel scaler.");
}
