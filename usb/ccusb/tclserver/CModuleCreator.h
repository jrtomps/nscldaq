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
#ifndef __CMODULECREATOR_H
#define __CMODULECREATOR_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

/**
 * @file CModuleCreator.h
 * @brief Defines the ABC for the module creator.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include <memory>

#include <CControlHardware.h>

/**
 * @class CModuleCreator
 *
 *     Creates modules of a specific type.  This is an ABC
 */
class CModuleCreator
{
public:
  virtual std::unique_ptr<CControlHardware> operator()() = 0; // Concrete classes must implement.
};



#endif
