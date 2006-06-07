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
#include <CCAMACInterfaceFactory.h>
#include <CCAMACInterface.h>
#include <CCAMACInterfaceCreator.h>
#include <map>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Local data types:

typedef map<string, CCAMACInterfaceCreator*> CreatorContainer;
typedef CreatorContainer::iterator                 CreatorIterator;

// Static class members:

void*                   CCAMACInterfaceFactory::m_creators(0);
CCAMACInterfaceFactory* CCAMACInterfaceFactory::m_pFactory(0);

/*!
   Constructor is private as befits a singleton instance.
   getInstance will call it as needed to get the factory created on first ref.
*/
CCAMACInterfaceFactory::CCAMACInterfaceFactory()
{
  m_pFactory = this;
  m_creators = static_cast<void*>(new CreatorContainer);
}
/*!
   Destructor should never get called, but if it does it ensures
   the next get instance works. The assumption is that the creators are all
   dynamically allocated.
*/
CCAMACInterfaceFactory::~CCAMACInterfaceFactory()
{
  m_pFactory = 0;
  CreatorContainer* pMap = static_cast<CreatorContainer*>(m_creators);
  CreatorIterator   i    = pMap->begin();
  while (i != pMap->end()) {
    delete i->second;
    i++;
  }
  delete pMap;
  m_creators = NULL;
}

/*!
     Create an interface.  The creator is located by name and 
     its function call operator is invoked to create/configure the interface.
     \param interfaceType   : string
         Type of the interface being created.  This selects the creator.
     \param configuration   : string
          Type dependent configuration string.
     \return CCAMACInterface*
     \retval non-null - New interface object.
     \retval null     - Failed to create.
*/
CCAMACInterface*
CCAMACInterfaceFactory::createInterface(string interfaceType,
					string configuration)
{
  CreatorContainer* pMap = static_cast<CreatorContainer*>(m_creators);
  CreatorIterator   item = pMap->find(interfaceType);

  if(item != pMap->end()) {
    CCAMACInterfaceCreator& creator(*(item->second));
    return creator(configuration);
  } 
  else {
    return NULL;
  }
}
/*!
    Add a creator for an interface type.  The creator must be an object
    that is derived from CCAMACInterfaceCreator.   If a creator for the type
    exists already, the old one is destroyed silently and transparently.
    \param type    : string
       Type name that the creator will create for.
    \param creator : CCAMACInterfaceCreator*
       Pointer to a dynamically created interface creator.
*/
void
CCAMACInterfaceFactory::addCreator(string type, CCAMACInterfaceCreator* creator)
{
  CreatorContainer* pMap = static_cast<CreatorContainer*>(m_creators);
  CreatorIterator   item = pMap->find(type);

  if (item != pMap->end()) {
    delete item->second;
    pMap->erase(item);
  }
  (*pMap)[type] = creator;
}
/*!
   Return an instance to the factory, creating it if necessary.
*/
CCAMACInterfaceFactory*
CCAMACInterfaceFactory::getInstance()
{
  if (!m_pFactory) {
    new CCAMACInterfaceFactory;	// sets m_pFactory.
  }
  return m_pFactory;
}
