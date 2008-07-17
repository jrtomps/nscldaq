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

#ifndef __CCONTROLMODULE_H
#define __CCONTROLMODULE_H

#ifndef __CCONFIGURABLEOBJECT_H
#include <CConfigurableObject.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



/*!
  This is the base class for programmable electronics that are not read out in events.
  Since the USB controllers can only be talked to by a single process, and since they
  must be taken out of data taking mode before single shot operations can be performed,
  The readout frameworks support a Tcl Server taht can be sent commands to control
  specific electronic hardware.

  Each specific framework (e.g. VM-USB or CC-USB) will define a set of supported
  control modules.  The Tcl server takes care of:
  - Initializing the set of objects that get instantiated, and their configuration
    (e.g. base addresses or slot numbers) via a configuration script that is analagous
    to the daqconfig.tcl (controlconfig.tcl). script.
  - If necessary, halting and restarting data taking in order to slip a control function
    in in the middle of an active run.
  - Dispatching requests received by clients to the appropriate objects.

  This is possible because requests to the server are of one of the following forms:

  - Set objectname parametername value
  - Get objectname parametername
  - Update objectname

  where the Set command modifes a settable named parameter in the object named
  objectname to value, Get returns the value of the named parameter from the named object,
  and Update loads the device with any state held internally by the object (e.g.
  recovery after power up).


  CControlModule is an abstract base class.  Concrete implementation will have to implement;
  - Set - Process the set command on the object.
  - Get - Process the get command on the object.
  - Update - Process the update command on the object
  - onAttach to manage the configuration parameters accepted by the module.
  - clone() Create a copy of this object.
  - Optionally Initialize.

*/
class CControlModule : public CConfigurableObject
{

public:

  // Canonicals

  CControlModule();
  virtual ~CControlModule();
  CControlModule(const CControlModule& rhs);
  CControlModule& operator=(const CControlModule& rhs);

private:
  int operator==(const CControlModule& rhs) const;
  int operator!=(const CControlModule& rhs) const;
public:
  // Functions:

  virtual void        Initialize();
  virtual void        onAttach() = 0;   // Attach/init configuration 
  virtual std::string Update() = 0;     // Update hardware
  virtual std::string Set(const char* what, 
			  const char* value) = 0; // Set a device parameter.
  virtual std::string Get(const char* what) = 0; // Return a device parameter.
  virtual CControlModule* clone() = 0; // Virtual copy constructor.
};


#endif
