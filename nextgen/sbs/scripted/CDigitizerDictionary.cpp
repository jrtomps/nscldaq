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


////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CDigitizerDictionary.h"    				
#include "CReadableObject.h"

using namespace std;


/*!
 Constructor: since maps know how to do their thing, this
 is just a no-op.
 */
CDigitizerDictionary::CDigitizerDictionary () 
{
} 

/*!
   Since We assume that all of the pointers
   handed to us are dynamic so we delete them
   and let the map take care of itself:
   */
 CDigitizerDictionary::~CDigitizerDictionary ( )  //Destructor - Delete dynamic objects
{
   DestroyMap();
}
// Functions for class CDigitizerDictionary

/*! 

Returns the begin iterator to the modules
in the dictionary.


*/
CDigitizerDictionary::ModuleIterator 
CDigitizerDictionary::DigitizerBegin()  
{ 
   return m_Modules.begin();
}  

/*!  

Returns the end() iterator to the modules
in the dictionary.


*/
CDigitizerDictionary::ModuleIterator 
CDigitizerDictionary::DigitizerEnd()  
{ 
   return m_Modules.end();
}  

/*!

Returns the number of modules in the dictionary.

*/
int 
CDigitizerDictionary::DigitizerSize()  
{ 
   return m_Modules.size();
}  

/*!  

Adds a new digitizer to the list.  If this digitizer has 
the same name as an existing one the existing one is replaced.


*/
void 
CDigitizerDictionary::DigitizerAdd(CReadableObject* pDigitizer)  
{
  if(pDigitizer) {
    m_Modules[pDigitizer->getName()] = pDigitizer;
  }
}  

/*!

Locates a module by name.  If not found,
returns the end iterator.

*/
CDigitizerDictionary::ModuleIterator 
CDigitizerDictionary::DigitizerFind(const string& rName)  
{ 
   return m_Modules.find(rName);
}
/*!
   Destroy the modules that are now in the map:
   NOTE: It's the caller's resonsibility to ensure that the map
         itself is emptied.  All we do is iterate the map doing deletes
	 on each of the modules.
*/
void
CDigitizerDictionary::DestroyMap()
{
  CDigitizerDictionary::ModuleIterator p = DigitizerBegin();
  while(p != DigitizerEnd()) {
    (p->second)->OnDelete();
    delete p->second;
    p++;
  }
}
