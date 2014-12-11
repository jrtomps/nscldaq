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


/**
 * @file CModuleFactory.cpp
 * @brief Implementation of the control module factory.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include "CModuleFactory.h"
#include "CModuleCreator.h"

#include <CControlHardware.h>



/**
 *  The instance pointer (static)
 */

CModuleFactory* CModuleFactory::m_pInstance(0);

/**
 * Constructor is null for now but must be defined to
 * make it private.
 */
CModuleFactory::CModuleFactory() {}

/**
 *  similarly for the destructor
 */
CModuleFactory::~CModuleFactory() {}


/**
 * instance (static)
 *    @return CModuleFactory*  The pointer to the singleton instance.
 */
CModuleFactory*
CModuleFactory::instance()
{
  if (!m_pInstance) {
    m_pInstance = new CModuleFactory;
  }
  return m_pInstance;
}

/**
 * addCreator
 *
 *  Adds a new creator to the factory.
 *
 * @param type - The type of the creator.
 * @param pCreator - Pointer to a creator (best if dynamically allocated).
 *
 * @note duplicate type - last one wins for now. - TODO - detect and object
 */
void
CModuleFactory::addCreator(std::string type, CModuleCreator* pCreator)
{
  m_Creators[type] = pCreator;
}
/**
 * create
 *
 *   Asks a module creator to create a control module.
 *
 * @param type - The type of control module to create.
 * @param name - Name of the module
 *
 * @return CControlHardware* - Pointer to the created module.
 * @retval NULL - no such type.
 */
  std::unique_ptr<CControlHardware>
CModuleFactory::create(std::string type)
{
  std::map<std::string, CModuleCreator*>::iterator p = m_Creators.find(type);
  if (p != m_Creators.end()) {
    return std::unique_ptr<CControlHardware>((*(p->second))());
  } else {
    return nullptr;
  }
}
