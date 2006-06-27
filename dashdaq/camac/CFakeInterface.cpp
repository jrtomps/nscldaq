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
#include "CFakeInterface.h"


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*
  Construct a fake interface.. At this time we'll say that fake interfaces
  support 3 crates... got a better idea?
 */
CFakeInterface::CFakeInterface(string configuration) :
  m_configuration(configuration)
{
  setCrateCount(3);
}
/*
   Destroy the fake interface:
*/
CFakeInterface::~CFakeInterface()
{
  
}

bool 
CFakeInterface::haveCrate(size_t crate)
{
  return false;
}

CCAMACCrate* 
CFakeInterface::removeCrate(size_t crate)
{
  return static_cast<CCAMACCrate*>(NULL);
}

CCAMACCrate&
CFakeInterface::operator[](size_t crate)
{
  return *(static_cast<CCAMACCrate*>(NULL));

}
bool
CFakeInterface::online(size_t crate)
{
  return bool;
}

const char*
CFakeInterface::getConfiguration() const
{
  return m_configuration.c_str();
}
