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


#ifndef __CVMUSBCONFIG_H
#define __CVMUSBCONFIG_H


class CConfiguration;

/*!
   This file contains contains the CVMUSBConfig class.  This class is responsible
   for managing the configuration object that is associated with a VM USB readout
   program.  Specifically, it must make and initialize a CConfiguration object
   and destroy that object on demand.  The class at this time contains only static
   member functions, as a pointer to the configuration object will be saved in 
   Globals::pConfig so that it can be located by other software.
 
*/

class CVMUSBConfig
{
public:
  static CConfiguration* create();
  static void            configure(CConfiguration* pConfig);
  static void            destroy();
  static bool            exists();
};
#endif
