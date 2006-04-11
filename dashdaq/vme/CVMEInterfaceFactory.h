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

#ifndef __CVMEINTERFACEFACTORY_H
#define __CVMEINTERFACEFACTORY_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



// Forward classes:

class CVMEInterfaceCreator;
class CVMEInterface;

/*!
    The VME interface factory is an extensible factory that
    is responsible for generating VME interfaces that match
    some textual description.  The textual description is
    a word of interface type followed by configuration information
    that is not interpreted by this factory, but by the interface
    construction objects.

    The factory follows the pattern of an extensible factory.
    An extensible factory consists of a class that has
    a creation method, and a registration method, as well
    as an arbitrary number of creational objects.
    The registration method is used to add a creational object
    and its matching criteria to the factory's creational object list.
    The creation method receives some type information which is used
    to select the appropriate creational object.  The creational object
    then is delegated the work of actually producing the requested object.
    This factory pattern supports the creation of factories for hierarchies
    of classes that are provided by an application framework but can be
    extended by the users of that framework in a way that allows users
    of that extension to use the normal factory mechanisms for object
    creation.

    In our case:
    - Matching criteria are a simple string, the interface type,
      which is the first textual word of an arbitrary string.
    - Creational objects create CVMEInterface* derived objects, each
      derived class must have a parallel creational class derived
      from CVMEInterfaceCreator.
    - The remainder of the arbitrary string that contained the interface
      type is passed without interpreteration to the creational
      object and can be used to configure the interface (e.g. it may
      contain a hardware unit number of the interface in the system).
    - The factory consists of static methods since we want a single
      shared registry.
    - The registry is held as static data in the implementation file rather
      than as a static member in order to reduce the number of includes in
      this header.
*/

class CVMEInterfaceFactory
{
public:
  static void addCreator(std::string type, CVMEInterfaceCreator& creator);
  static CVMEInterface* create(std::string description);

  // The following are made public to unit test software, but
  // may or may not also be utilities used by the class.

#ifdef UNIT_TEST_INCLUDE
public:
#else
private:
#endif
  static void clearRegistry();	// empty the factory registry.
  static CVMEInterfaceCreator* findCreator(std::string type);
};





#endif
