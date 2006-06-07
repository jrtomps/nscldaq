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

#ifndef __CCAMACINTERFACEFACTORY_H
#define __CCAMACINTERFACEFACTORY_H


#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CCAMACInterface;
class CCAMACInterfaceCreator;


/*!
    This class creates camac interfaces. Camac interfaces
    are created by providing two wtrings, an interface type,
    and a configuration string.  The interface type is used
    to locate the appropriate creational object (derived
    from CCAMACInterfaceCreator).  To that object is delegated
    the task of creating and configuring the new object.
    - This is an instance of an extensible factory pattern.
    - In realizing this pattern I am trying a new trick of making the creator
      objects opaque in the header so that only the implementation needs to know
      the actual type of data structure needed to hold these items.
    - The factory is itself a realization of the singleton pattern as well.
*/
class CCAMACInterfaceFactory 
{
private:
  static void*                   m_creators; // Opaque container of creators.
  static CCAMACInterfaceFactory* m_pFactory; // singleton instance pointer.
private:
  CCAMACInterfaceFactory();
#ifdef UNIT_TEST_INCLUDE
public:				// Allow factories to be destroyed in tests.
#endif
  ~CCAMACInterfaceFactory();
public:
  CCAMACInterface* createInterface(std::string interfaceType, 
				   std::string configuration);
  void addCreator(std::string type, CCAMACInterfaceCreator* creator);


  static CCAMACInterfaceFactory*   getInstance();
};

#endif
