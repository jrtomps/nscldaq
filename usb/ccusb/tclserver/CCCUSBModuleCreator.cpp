/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCL Daq Development Group
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CCCUSBModuleCreator.cpp
 * @brief Implementation of the module creator for CCCUSBModule objects.
 * @author Jeromy Tompkins <tompkins@nscl.msu.edu>
 */

#include "CCCUSBModuleCreator.h"
#include "CCCUSBModule.h"

/**
 * operator()
 *   The creational
 *
 * @param name - Name of the module.
 * @return CControlHardware* Pointer to the newly created module.
 */
CControlHardware*
CCCUSBModuleCreator::operator()(std::string name)
{
  return new CCCUSBModule(name);
}
