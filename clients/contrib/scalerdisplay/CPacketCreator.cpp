#include <config.h>
#include "CPacketCreator.h"    	
#include "CPacket.h"	
#include <assert.h>
#include <string>	

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif	
/*!
   Construct an instance of a Packet module creator.
 
   */
CPacketCreator::CPacketCreator ()
   : CModuleCreator("packet")
{   
} 

/*
   Destroys an instance of a Packet creator.
   */
 CPacketCreator::~CPacketCreator ( ) 
{
}

/*!
	Creates temporary copy objects.
	   \param rhs const CPacketCreator& [in]
		 Reference to copy from.
*/
CPacketCreator::CPacketCreator (const CPacketCreator& rhs ) 
  : CModuleCreator (rhs) 
 
{
} 

/*!
   Assigns a rhs object to *this.
      \param rhs const CPacketCreator& [in]
	    Reference to rhs of assignment operator.
      \return reference to *this after the copy in.
*/
CPacketCreator& 
CPacketCreator::operator= (const CPacketCreator& rhs)
{ 
   if (this != &rhs) {
       CModuleCreator::operator=(rhs);
   }
   return *this;
}

      //Operator== Equality Operator 
int CPacketCreator::operator== (const CPacketCreator& aCPacketCreator) const
{ return 
    (CModuleCreator::operator== (aCPacketCreator));
}

// Functions for class CPacketCreator

/*! 

Returns a new instance of the digitizer module 
creaetd by this creator.  The mdule is new'd into
being and therefore must be deleted by the ultimate
user.  The parameters passed in are also passed
to the module's configuration function.

\param rInterp CTCLInterpreter& [in] Reference to the interpreter running the command.
\param rResult CTCLResult& [in] Referenc to the result string 
	to return.
\param nArgs   int [in]   Number of parameters left on the line.
\param pArgs   char** [in] The parameter strings.

\return CDigitizerModule*  The resulting module.

The command line invoking is is of the form:
\verbatim
	module mname type config params
\endverbatim
  The caller has only eaten up the module command keyword:
  - mname is used to set the modulen name.
  - type is asserted against our type.
  - The remaining parameters are passed to the instantiated module's 
     configuration member.
*/
CDigitizerModule* 
CPacketCreator::Create(CTCLInterpreter& rInterp, CTCLResult& rResult,
					int nArgs, char** pArgs)  
{ 
	assert(nArgs >= 2);
	string mName(*pArgs);       // Module name.
	nArgs--;
	pArgs++;
	assert(*pArgs == getModuleType());
	nArgs--;
	pArgs++;
	
	CPacket* pModule= new CPacket(mName, rInterp);
	pModule->Configure(rInterp, rResult, nArgs, pArgs);
	return pModule;
	
}  

/*!  Function: 	
   string  Help() 
 Operation Type:
    
Purpose: 	

Returns a string describing the module type and
whatever else the module driver author wants to display
about that module type in response to the module -help
command.

*/
string  
CPacketCreator::Help()  
{ 
	return string("Creates a packet 'module'.");
}
