// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//

#ifndef __CDIGITIZERDICTIONARY_H  //Required for current class
#define __CDIGITIZERDICTIONARY_H

//
// Include files:
//


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

 
#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

class CDigitizerModule;
/*!
Encapsulates a set of modules that are managed by a
CModuleCommand and can be inserted into a CReadOrder
objects.  This is really a small wrapper for 
a map<STD(string), CDigitizerModule*>
*/
class CDigitizerDictionary      
{
public:
   typedef STD(map)<STD(string), CDigitizerModule*> DigitizerMap;
   typedef DigitizerMap::iterator ModuleIterator;
private:
	DigitizerMap m_Modules;

public:
   //	
  CDigitizerDictionary ();
  ~CDigitizerDictionary ( );
private:
  CDigitizerDictionary (const CDigitizerDictionary& rhs);
  CDigitizerDictionary& operator=(const CDigitizerDictionary& rhs);
  int operator== (const CDigitizerDictionary& rhs) const;
  int operator!= (const CDigitizerDictionary& rhs) const;
public:

// Selectors:

public:
   DigitizerMap getMap() const {
      return m_Modules;
   }
// Mutators:

protected:
   void setMap(DigitizerMap& rMap) {
      m_Modules = rMap;
   }
public:

   ModuleIterator DigitizerBegin ()   ; // 
   ModuleIterator DigitizerEnd ()   ; // 
   int DigitizerSize ()   ; // 
   void DigitizerAdd (CDigitizerModule* pDigitizer)   ; // 
   ModuleIterator DigitizerFind (const STD(string)& rName)   ; // 
   void Remove(ModuleIterator p) {
      m_Modules.erase(p);
   }

   // Utility functions:
protected:
   void DestroyMap();
};

#endif
