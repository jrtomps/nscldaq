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

#ifndef __CVMEINTERFACECREATOR_H
#define __CVMEINTERFACECREATOR_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward class definitions:

class CVMEInterface;

/*!
    VME Interface Creators are registered in the VME interface factory.
    Their job is to make that factory extensible.   When the factory
    receives a request to create a VME interface, it will search for a matching
    creator.  The matching creator is then asked to create the appropriate VME
    interface object on behalf of the factory.
    See CVMEInterfaceFactory for more on this.
    This is part of the extensible factory pattern realized for CVMEInterface
    creation.
*/
class CVMEInterfaceCreator
{
public:
  // Canonical methods.

  virtual ~CVMEInterfaceCreator();

  // Pure virtual methods.

  virtual CVMEInterface* operator()(std::string type,
				    std::string configuration) = 0;
};


#endif
