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

#ifndef __CCAMACINTERFACECREATOR_H
#define __CCAMACINTERFACECREATOR_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CCAMACInterface;

/*!
   This class is an abstract base class for the extensible factory class
   that can generate camac interfaces. (CCAMACInterface objects).
   In addtion to implementing a concrete interface and its crate (controller)
   classes when adding support for a new camac interface type you should
   write a creator and add it to the factory so that configuration file
   driven setups can produce it.
*/
class CCAMACInterfaceCreator 
{
public:
  virtual CCAMACInterface* operator()(std::string configuration) = 0;
};

#endif
