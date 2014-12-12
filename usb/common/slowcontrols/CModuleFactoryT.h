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

#ifndef __CMODULEFACTORYT_H
#define __CMODULEFACTORYT_H

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

#include <memory>

template<class Ctlr> class CControlHardwareT;
template<class Ctlr> class CModuleCreatorT;

/**
 * @class CModuleFactory
 *    Singleton extensible factory.   The factory creates CControlModules.
 *  
 */
template<class Ctlr>
class CModuleFactoryT {
private:
  static CModuleFactoryT* m_pInstance;

  std::map<std::string, 
           std::unique_ptr<CModuleCreatorT<Ctlr>> > m_Creators;

  // Constructor and destructor of singletons arre private:

  CModuleFactoryT();
  ~CModuleFactoryT();

  // Mechanism to get the singleton pointer:

public:
  static CModuleFactoryT* instance();

  // Manipulation of the factory:

public:
  void addCreator(std::string type, 
                  std::unique_ptr<CModuleCreatorT<Ctlr>> pCreator);
  std::unique_ptr<CControlHardwareT<Ctlr>> create(std::string type);


};

#include <CModuleFactoryT.hpp>

#endif
