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

#ifndef __GLOBALS_H
#define __GLOBALS_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CConfiguration;


/*!
  This namespace defines global variables.  We've tried to keep this to minimum.
  Here's what we define/need
  - pConfig : CConfigurtation*           Will hold the daq configuration 
                                         (adcs and scalers).
  - configurationFilename : std::string  Holds the daq configuration filename
  - controlConfigFilename : std::string  Holds the controllable object configuration 
                                         filename.
*/

namespace Globals {
  extern CConfiguration* pConfig;
  extern std::string     configurationFilename;
  extern std::string     controlConfigFilename;

};

#endif
