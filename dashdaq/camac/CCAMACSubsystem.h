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

#ifndef __CCAMACSUBSYSTEM_H
#define __CCAMACSUBSYSTEM_H

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

#ifndef __CPP_ISTREAM
#include <istream>
#ifndef __CPP_ISTREAM
#define __CPP_ISTREAM
#endif
#endif

class CCAMACInterface;

/*!
   This is a singleton pattern class that maintains all of the 
CAMAC interfaces that are registered with the system.
In addition to individual registration, this class provides
support for registering interfaces by description and by a file
of descriptions.  Intent, although not policy, is that typically
an experiment will have a CAMAC configuration file and that
will be used to provide a consistent view of the CAMAC subsystem
for all applications that make up the experiment.

*/
class CCAMACSubsystem {
  // Exported types:
public:
  typedef STD(vector)<CCAMACInterface*> InterfaceContainer;
  typedef InterfaceContainer::iterator  InterfaceIterator;
  // Private data:
private:
  static CCAMACSubsystem*                m_pInstance; // Singleton instance pointer.
  static InterfaceContainer              m_interfaces;

  // Constructors and canonicals.  Note that as befits a singleton class, these
  // are also private to prevent uncontrolled construction/destruction.

private:
  CCAMACSubsystem();
  ~CCAMACSubsystem();
  CCAMACSubsystem(const CCAMACSubsystem& rhs); // no copy constructor either...
  CCAMACSubsystem& operator=(CCAMACSubsystem& rhs); // nor assignment.
  int  operator==(const CCAMACSubsystem& rhs) const; // and may as well prevent
  int  operator!=(const CCAMACSubsystem& rhs) const; // compares as well.

  // Now to the public interfaces:
public:
  static CCAMACSubsystem* getInstance();
  size_t           addInterface(CCAMACInterface& interface);
  CCAMACInterface* removeInterface(size_t index);
  InterfaceIterator begin();
  InterfaceIterator end();
  size_t            size();
  CCAMACInterface*  operator[](size_t index);
  size_t            createInterface(STD(string) description);
  void              createInterfaces(STD(istream)& descriptionFile);
#ifdef UNIT_TEST_INCLUDE
public:
#else
private:
#endif
  void              clearInterfaces(); // For idempotent tests.
};


#endif
