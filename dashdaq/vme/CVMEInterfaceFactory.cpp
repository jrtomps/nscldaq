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

#include <config.h>
#include "CVMEInterfaceFactory.h"
#include <map>
#include "CVMEInterfaceCreator.h"
#include "parseUtilities.h"

using descriptionFile::firstWord;
using descriptionFile::stripLeadingBlanks;

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

typedef map <string, CVMEInterfaceCreator*> Registry;

static Registry factoryRegistry;




/*!
   Register a creator with the factory.  The creator overwrites
   any existing registration for that type.

   \param type : string [in]
      The interface type that should match this creator.
   \param creator : CVMEInterfaceCreator& [in]
      Reference to the creator to add.
*/
void 
CVMEInterfaceFactory::addCreator(string type, CVMEInterfaceCreator& creator)
{
  factoryRegistry[type] = &creator;
}


/*!
   Create an interface object as described by a configuration line.
   The configuration line is stripped of leading blanks, 
   The first word is considered to be the device type, and the
   leading blank stripped remainder the configuration.
   \param description : std::string [in]
      The device description.
   \return CVMEInterface*
   \retval NULL - No matching description type... or description is blank.
   \retval !NULL- Pointer to a new interface that was created.
*/
CVMEInterface*
CVMEInterfaceFactory::create(string description)
{
  description   = stripLeadingBlanks(description);
  string type   = firstWord(description);
  string config = description.substr(type.size());
  config        = stripLeadingBlanks(config);

  CVMEInterfaceCreator* pCreator = findCreator(type);
  if (pCreator) {
    return (*pCreator)(type, config);
  }
  else {
    return static_cast<CVMEInterface*>(NULL);
  }

}

/*!
   Find the creational in the registry that matches the supplied type
   and return a pointer to it.  If there is no match, 
   a NULL is returned instead.
*/
CVMEInterfaceCreator*
CVMEInterfaceFactory::findCreator(string type)
{
  Registry::iterator p = factoryRegistry.find(type);
  if(p  != factoryRegistry.end()) {
    return p->second;
  }
  else {
    return static_cast<CVMEInterfaceCreator*>(NULL);
  }
}

// clear the registry of all creators.
// this is done quick and dirty and is intended for use by unit test systems.
// 
void 
CVMEInterfaceFactory::clearRegistry()
{
  factoryRegistry.clear();
}
