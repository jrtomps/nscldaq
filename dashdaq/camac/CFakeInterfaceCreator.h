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

#ifndef __CFAKEINTERFACECREATOR_H
#define __CFAKEINTERFACECREATOR_H

#ifndef __CCAMACINTERFACECREATOR_H
#include <CCAMACInterfaceCreator.h>
#endif

//   Test harness class.  This is a camac interface creator that generates.
//   fake interfaces for low level tests of the factory.
//
class CFakeInterfaceCreator : public CCAMACInterfaceCreator
{
public:
  CFakeInterfaceCreator();
  virtual ~CFakeInterfaceCreator();
  virtual CCAMACInterface* operator()(STD(string) configuration);
};


#endif
