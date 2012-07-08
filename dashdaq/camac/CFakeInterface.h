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

#ifndef __CFAKEINTERFACE_H
#define __CFAKEINTERFACE_H

#ifndef __CCAMACINTERFACE_H
#include <CCAMACInterface.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CFakeInterface : public CCAMACInterface
{
private:
  std::string m_configuration;
public:
  CFakeInterface(std::string configuration = std::string(""));
  virtual ~CFakeInterface();

  virtual bool         haveCrate(size_t crate);
  virtual void         addCrate(CCAMACCrate& crate, size_t number) {}
  virtual CCAMACCrate* removeCrate(size_t crate);
  virtual CCAMACCrate& operator[](size_t crate);
  virtual bool         online(size_t crate);

public:
  const char* getConfiguration() const;
};

#endif