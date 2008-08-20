#ifndef __CAPPLICATION_H
#define __CAPPLICATION_H

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

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


#ifndef __TCL_H
#include <tcl.h>
#ifndef __TCL_H
#define __TCL_H
#endif
#endif

class CConfiguration;
class TclServer;

/*!
  This file defines the application base class.
  The application base class controls the startup flow of the xx-USB readout frameworkds.
  It delegates control to some pure virtual methods which capture the device dependencies of
  each interface (e.g. VM-USB, CC-USB).
  In practice, the main programs of each readout skeleton, will instantiate a subclass
  of this base class, and transfer control to it.

  The usage of the software by default is:

  Readout ?options?

  where options are command line options that take parameters:
  - --daqconfig=file - specifies the DAQ configuration file, defaults to $HOME/config/daqconfig.tcl
  - --ctlconfig=file - specifies the slow control configuration file, defaults to
    $HOME/config/ctlconfig.tcl
  - --device=specification - specifies which of potentially several xx-USB interfaces the
    program should take data on.  'specification' is a device specific specification string.
    for VM-USB/CC-USB this is going to be the serial number string of the desired device.
    If not specified, again action is up to the specific readout skeleton, but typically,
    the first device located on the USB bus is then used.
  - --port=n  - Specifies the port on which the TCL server will listen for slow control connections.
                This defaults to managed which results in using the port manager to allocate
                a port.
  - --application=appname - Used only if --port=managed, in which case appname is the
     name of the application this program will register with the TCL port manager when
     requesting a server port.   This defaults to "SlowControls".

  The following are pure virtual methods and what they are supposed to do in concrete
  subclasses:
  - selectInterface  Must select the actual interface, create device driver object and
    connect it to the acquisition thread object.
  - setupConfiguration - Add configuration commands to the daq configuration so that
    supported device objects can be created and configured.
  - setupTclServer - add device types to the Tcl server so that slow control devices
    can be specified and configured.
  - startOutputThread - Create/start the output thread (derivation of COutputThread).
  - createBuffers() - Must set up the buffer pool with sufficient buffers
                      of the correct size for the interface.


  \note  All member functions are virtual so that if this callback scheme is
         not sufficient for some special applications, specific members can be 
	 overridden.

 */

class CApplication
{

public:
  virtual ~CApplication();	// Ensure that destructor chaining will work.

  // Implemented virtual member functions

public:
  virtual int main(int argc, char** argv);
  virtual std::string selectDAQConfigFile(std::string filespec);
  virtual std::string selectControlConfigFile(std::string filespec);
  virtual void createConfiguration();
  virtual void createMainInterpreter(int argc, char** argv);
  virtual void createTclServer(std::string port, std::string application);

  // Pure virtual methods:

public:
  virtual void selectInterface(std::string specification)        = 0;
  virtual void setupConfiguration(CConfiguration& configuration) = 0;
  virtual void setupTclServer(TclServer& server)                 = 0;
  virtual void startOutputThread()                               = 0;
  virtual void createBuffers()                                   = 0;


private:

  static std::string configFile(std::string userSupplied,
				std::string defaultTail);
  static int tclAppInit(Tcl_Interp* interp);      //  Registers interp commands.
  int getManagedPort(std::string application);
};

#endif
