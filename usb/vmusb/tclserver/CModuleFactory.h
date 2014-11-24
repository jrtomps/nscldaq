/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMODULEFACTORY_H
#define __CMODULEFACTORY_H

/**
 * @file CModuleFactory.h
 * @brief Defines a factory for control modules in the Tcl server.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

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

class CControlHardware;
class CModuleCreator;

/**
 * @class CModuleFactory
 *    Singleton extensible factory.   The factory creates CControlModules.
 *  
 */
class CModuleFactory {
private:
  static CModuleFactory* m_pInstance;

  std::map<std::string, CModuleCreator*> m_Creators;

  // Constructor and destructor of singletons arre private:

  CModuleFactory();
  ~CModuleFactory();

  // Mechanism to get the singleton pointer:

public:
  static CModuleFactory* instance();

  // Manipulation of the factory:

public:
  void addCreator(std::string type, CModuleCreator* pCreator);
  CControlHardware* create(std::string type);


};
#endif
