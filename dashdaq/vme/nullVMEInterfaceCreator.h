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

#ifndef __NULLVMEINTERFACECREATOR_H
#define __NULLVMEINTERFACECREATOR_H

#include "CVMEInterfaceCreator.h"

// Forward definitions:

class CVMEInterface;

/*
   This class creates nullVMEInterface objects on behalf of the
   extensible vme interface factory. Null VME interfaces are only
   used to test the system and should not be used in any production
   software.
*/
class nullVMEInterfaceCreator : public CVMEInterfaceCreator
{
public:
  virtual CVMEInterface* operator()(std::string type,
				    std::string configuration);

};


#endif
