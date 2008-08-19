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
#ifndef __CVMUSBCONTROLMODULE_H
#define __CVMUSBCONTROLMODULE_H

#ifndef __CCONTROLMODULE_H
#include <CControlModule.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CVMUSB;

/*!
   This class is an abstract base class that adapts control module classes to the
   VM-USB specific application.  I'm trying to solve a single problem.  How to adapt
   a controller independent object (CControlHardware) to a controller dependent hardware
   object (e.g. CGDG).  This is done by:
   - Providing access to the CVMUSB object that manages our VME crate and
   - Presnting an adaptation facade that converts the Update, Set, and Get calls
     into calls that have a controller in their signature.
*/
class CVMUSBControlModule : public CControlModule
{
private:
  // Static data

  static CVMUSB*   m_pController;

  // Static functions.
public:
  static void setController(CVMUSB* pController);

  // Canonicals:
public:
  CVMUSBControlModule(std::string name);
  CVMUSBControlModule(const CVMUSBControlModule& rhs);
  CVMUSBControlModule& operator=(const CVMUSBControlModule& rhs);
  int operator==(const CVMUSBControlModule& rhs) const;
  int operator!=(const CVMUSBControlModule& rhs) const;



  // The members below relay to the pure virtual members with the same name
  // but different signatures.
  // i.e. this is the facade:

  virtual void        Initialize();
  virtual std::string Update();     // Update hardware
  virtual std::string Set(const char* what, 
			  const char* value); // Set a device parameter.
  virtual std::string Get(const char* what); // Return a device parameter.

  // Now here's the thing our facade is in front of:

  virtual void Initialize(CVMUSB& vme) = 0;
  virtual std::string Update(CVMUSB& vme) = 0;               //!< Update module.
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value) = 0;            //!< Set parameter value
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter) = 0;        //!< Get parameter value.

};


#endif
