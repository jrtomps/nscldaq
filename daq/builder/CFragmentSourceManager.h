#ifndef __CFRAGMENTSOURCEMANAGER_H
#define __CFRAGMENTSOURCEMANAGER_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __CPPRTL_CSTRING
#include <cstring>
#ifndef __CPPRTL_CSTRING
#define __CPPRTL_CSTRING
#endif
#endif

class CFragmentSource;



/*!
   Fragment sources come in families.  Each family of fragment sources has an associated 
   manager that is responsible for global initialization and for providing access to specific
   fragment sources.  

   This class is an abstract base class that provides common services for all fragment
   source manager types.

*/

class CFragmentSourceManager
{
public:
  // nested types:

  typedef std::map<CFragmentSource*, bool>  FragmentSources;
  typedef FragmentSources::iterator         FragmentSourceIterator;

  // Object member data:

protected:

  FragmentSources   m_mySources;


  // Actions on driver instances.

public:
  void start(CFragmentSource* pInstance);
  void stop(CFragmentSource* pInstance);
  bool isRunning(CFragmentSource* pInstance);

  // Configuration methods:

public:
  std::string validateConfig(std::string paramName, std::string param Value,
			     CFragmentSource* pInstance);

  void config(std::string paramName, std::string paramValue, 
	      CFragmentSource* pInstance);

  std::string cget(std::string paramName,
		   CFragmentSource* pInstance);
  std::map<std::string, std::string> cget(CFragmentSource* pInstance);

  // Getting at the internal data:

  FragmentSourceIterator begin();
  FragmentSourceIterator end();
  FragmentSourceIterator find(CFragmenSource* pInstance);
  size_t                 size();

  // Virtual methods

public:
  virtual CFragmentSource* createDriver() = 0;  // Must insert driver pointer and false -> map.
  virtual std::string      getName()  = 0;

  // Utility methods:

protected:
  void throwIfNoInstance(CFragmentSource* pInstance);
  void setState(CFragmentSource* pInstance, bool newState);
};

#endif
