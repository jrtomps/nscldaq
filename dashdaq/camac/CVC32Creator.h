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

#ifndef __CVC32CREATOR_H
#define __CVC32CREATOR_H


#ifndef __CCAMACINTERFACECREATOR_H
#include <CCAMACInterfaceCreator.h>
#endif


/*!
   Create a Wiener Vc32 interface.  This can control only a single crate.
   Configuration information consists of the following keyword value pairs:
\verbatim
   KEYWORD                Value:
    -vme                  VME crate in which the interface is installed (default 0)
    -base                 Base address of the module (no default).
\endverbatim

*/
class CVC32Creator : public CCAMACInterfaceCreator
{
public:
  CVC32Creator();
  virtual ~CVC32Creator();
  virtual CCAMACInterface* operator()(STD(string) configuration);

};

#endif
