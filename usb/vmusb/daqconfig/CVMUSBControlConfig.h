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

#ifndef __CVMUSBCONTROLCONFIG_H
#define __CVMUSBCONTROLCONFIG_H

class TclServer;

/*!
  This file contains some static members that understand how to 
  setup a TclServer object to understand how to interpret a control
  configuration file.
*/
class CVMUSBControlConfig
{
public:
  static void configure(TclServer* pServer);
};

#endif
