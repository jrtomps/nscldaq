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

#ifndef __CCCUSBMODULECREATOR_H
#define __CCCUSBMODULECREATOR_H


/**
 * @file CCCUSBModuleCreator.h
 * @brief Creates CTclControlModule objects...wrappers for Tcl driver instances.
 * @author Ron Fox <fox@nscl.msu.edu>
 */


#ifndef __CMODULECREATOR_H
#include <CModuleCreator.h>
#endif


/**
 * Concrete CModuleCreator that created Tcl driver wrapper objects.
 */
class CCCUSBModuleCreator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()(std::string name);
};


#endif
