////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CScalerModule.h"    				
#include <TCLInterpreter.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a scaler module.
   All of the work is done by our base class:
   \param rName  const string& [in]:
      A reference to the name that will be
      given to the module.
   \param rInterp CTCLInterpreter& [in]
      A reference to the interpreter that will be used
      to parse the module configuration commands.
*/
CScalerModule::CScalerModule (const string& rName, 
			      CTCLInterpreter& rInterp) :
   CReadableObject(rName, rInterp)
{   
    
} 
CScalerModule::~CScalerModule ( )  //Destructor - Delete dynamic objects
{
}
