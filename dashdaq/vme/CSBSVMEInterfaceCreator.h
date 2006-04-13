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


#ifndef __CSBSVMEINTERFACECREATOR_H
#define __CSBSVMEINTERFACECREATOR_H

#include "CVMEInterfaceCreator.h"

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



// Forward classes

class CVMEInterface;


/*!
   This is a VME Interface creator that can be registered with
   the  CVMEInterfaceFactory to create CSBSVMEInterface objects.
   See CVMEInterfaceCreator for more information about the role
   of interface creators.
   The creation method accepts as a configuration, a number
   that represents the interface number being created.
*/
class CSBSVMEInterfaceCreator : public CVMEInterfaceCreator
{
public:
  virtual ~CSBSVMEInterfaceCreator() {}

  virtual CVMEInterface* operator()(STD(string) type,
				    STD(string) configuration);
};


#endif
