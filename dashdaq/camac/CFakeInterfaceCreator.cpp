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

#include <config.h>
#include "CFakeInterfaceCreator.h"
#include "CFakeInterface.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CFakeInterfaceCreator::CFakeInterfaceCreator()
{
}

CFakeInterfaceCreator::~CFakeInterfaceCreator()
{
}

CCAMACInterface*
CFakeInterfaceCreator::operator()(string configuration)
{
  return new CFakeInterface(configuration);
}
