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

#ifndef __CVMESUBSYSTEM_H
#define __CVMESUBSYSTEM_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CPP_ISTREAM
#include <istream>
#ifndef __CPP_ISTREAM
#define __CPP_ISTREAM
#endif
#endif


// Forward references:

class CVMEInterface;




/*!
   This class provides a way to:
   - Establish a mapping between crate numbers and VME hardware interfaces.
   - Process a configuration that describes the VME hardware interfaces into
     such a mapping.

   While more than one mapping can exist in an application, you'd really better
   have a good reason for doing this.. there are good reasons.. for example,
   in code that implements a segmented experiment, to bridge between 
   testing the segments in isolation and the experiment together, it may
   make sense to have a mapping for each segment of the experiment so that
   VME crate numbers don't have to be reassigned when the components of the
   experiment are merged together.

   Configuration files can be read in to establish the mapping the format of
   a configuration file is:

   type configuration

   The crate numbers are assigned from the first available one up in order.
   type is considered to be an interface type, and configuration is whatever else
   is needed to determine which interface of that type to use, or how to 
   initialize it.

   See CVMInterfaceFactory to understand how the type in the configuration file
   relates to interface types.

*/

class CVMESubsystem
{
  // Data types:
public:
  typedef struct _Interface {
    bool            s_mustDelete;
    CVMEInterface*  s_pInterface;
  } Interface, *pInterface;

  typedef std::vector<Interface> Interfaces;
  typedef Interfaces::iterator   InterfaceIterator;
  // Data members:
private:
  static bool m_lockCreated;
  static int  m_lockId;
  static bool m_lockHeld;
  
  static Interfaces  m_Interfaces;

  static CVMESubsystem* m_Singleton;
public:

  // Interface management:
  
  static int installInterface(CVMEInterface& interface, bool deleteWhenDone=false);
  static CVMEInterface* replaceInterface(unsigned int   index, 
				  CVMEInterface& replacement,
				  bool           deleteWhenDone = false);
  static size_t size();
  static InterfaceIterator begin();
  static InterfaceIterator end();
  CVMEInterface& operator[](unsigned int index);


  // File based configuration:

  static int processDescription(STD(string) description);
  static void processDescriptionFile(STD(istream)& file);

  // Subsystem locking:

  static void lock();
  static void unlock();

  static CVMESubsystem& getInstance();

  // Utilities...

protected:
  static CVMEInterface* createFromDescription(STD(string) description);

  static void EnsureSemaphoreExists();

  // Available for unit tests:

#ifdef UNIT_TEST_INCLUDE
public:
#endif
  static bool isLockHeld() {
    return m_lockHeld;
  }
  static void destroyAll();

};

#endif
