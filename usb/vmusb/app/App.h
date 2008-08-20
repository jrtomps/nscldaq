#ifndef __APP_H
#define __APP_H
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


#ifndef __CAPPLICATION_H
#include <CApplication.h>
#endif

/*!
   This is a concrete subclass of CApplication.  This class provides
   the pure virtual methods needed to setup a VM-USB readout program.
   Specifically, we must provide:
   - selectInterface  which selects the correct interface, makes
     a device driver object and binds it into the acquisition thread object.
   - setupConfiguration which takes a configuration object and adds the appropriate
     stuff to it so that it can process VM-USB DAQ configuration files.
   - setupTclServer which takes a Tcl Server thread object and adds sufficient stuff
     to it so that it can read in VM-USB control configuration files.

\note The implementation file also provides the main, entry point to the program.

*/
class App : public CApplication
{
public:
  virtual void selectInterface(std::string specification)        ;
  virtual void setupConfiguration(CConfiguration& configuration) ;
  virtual void setupTclServer(TclServer& server)                 ;
  virtual void createBuffers()                                   ;
};

#endif
