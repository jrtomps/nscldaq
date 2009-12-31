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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>
#include "CAD811.h"

#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;


/************************************************************************/
/*                        Local constants                               */
/************************************************************************/

// Configuration value constants and limits:

static CConfigurableObject::limit One(1); 
static CConfigurableObject::limit Zero(0);
static CConfigurableObject::limit FULL16(0xffff);
static CConfigurableObject::limit LastSlot(23);

static CConfigurableObject::Limits SlotLimits(One, LastSlot); // CAMAC crate.
static CConfigurableObject::Limits IdLimits(Zero, FULL16);


/**********************************************************************/
/*  Canonical Member function implementation                          */
/**********************************************************************/


/*!
   Construction is largely a no-op because the configuration is
   hooked to this modules at attachment time.

*/
CAD811::CAD811()  :
  m_pConfiguration(0)
{
}
/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CAD811::CAD811(const CAD811& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}

/*!
  Assignment should probably have been made illegal.  We'll keep our
  current idea of the configuration.
  \param rhs  - The module that is being assigned to us.
  \return CAD811&
  \retval *this
*/
CAD811&
CAD811::operator=(const CAD811& rhs)
{
  return *this;
}

/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CAD811::~CAD811()
{
}
/*************************************************************************/
/*  Implementing the CReadoutHardware Interface                          */
/*************************************************************************/


/*!
   This function is called when the module is attached to its configuration.
   It is responsible for defining the set of parameters that can be
   set, their types and validity checkers that enforce any value/type
   constraints on the configuration values, as well as parameter initial
   values.

   See the header file for a definition of the set of parameters, types
   and defaults that will be defined for this module.

   \param configuration - reference to the configuration object that
                          will be associated with this module.

*/
void
CAD811::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  configuration.addParameter("-slot", CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-id", CConfigurableObject::isInteger,
			     &IdLimits, "0");
}
/*!
   Just prior to data taking, the module is initialized in accordance
   with its configuration. If a parameter array is disabled, I'm not going
   to bother loading it.  This reduces the configuration time to the set
   of operations that are absolutely necessary.

   Prior to intializing the module, the crate is inhibited and the module
   cleared to ensure it remains not-busy during initialization.
   Following initialization, the crate Inhibit is released.

   \param controller  a reference to a CCCUSB& controller object that
                      is connected to the CCUSB we are using.


*/
void
CAD811::Initialize(CCCUSB& controller)
{
  int slot = getIntegerParameter("-slot");

  // Slot must be non zero or the module was not configured:

  if(!slot) {
    throw "An AD811 module has not been cofigured (-slot missing).";
  }
  // The module requires no hardware initialization..other than a clear.

  
  checkedControl(controller, slot, 12, 11,
		 "Clearing AD811 in slot %d");
  
}

/*!
   Add the commands required to read the module to a CCUSB readout list.
   In our case we add the following:
   - Insert the marker in the event.
   - Read all registrees with f2 so the module auto-clears.

   \param list - the CCCUSBReadoutList into which to put the instructions.
*/
void
CAD811::addReadoutList(CCCUSBReadoutList& list)
{
  int slot = getIntegerParameter("-slot");
  int id   = getIntegerParameter("-id");

  // Insert the markers:

  list.addMarker(id);

  // Read  & clear the channels:

  for (int  i = 0; i < 8; i++) {
    list.addRead16(slot, i, 2, false); // user must set the delay.
  }

}


/*!

   Clone ourself.  This is essentially a virtual contstructor.
   We just do a new *this

*/
CReadoutHardware*
CAD811::clone() const
{
  return new CAD811(*this);
}
/*********************************************************************/
/*                Utilities to access the configuration              */
/*********************************************************************/

// Return the named integer parameter.
// The configuration validators have ensured the parameter actually
// is an integer.

unsigned int
CAD811::getIntegerParameter(string name)
{
  string value = m_pConfiguration->cget(name);
  return strtoul(value.c_str(), NULL, 0);
}

// Get the value of a boolean parameter.
// In this case, we create the set of valid true values.  If the
// string matches at least on of them (is in the set), 
// then we can return true.. otherwise false.
// Once again, the validator, and our initial value, ensure the
// string is a valid bool
//
bool
CAD811::getBoolParameter(string name)
{
  string value = m_pConfiguration->cget(name);
  set<string>  trueValues;
  trueValues.insert("true");
  trueValues.insert("yes");
  trueValues.insert("yes");	// Valid true values.
  trueValues.insert("1");
  trueValues.insert("on");
  trueValues.insert("enabled");

  return (trueValues.count(value) != 0);	// value matches one of the set members.

}
// Retrieve an array of uint16_t values.
// 
void
CAD811::getArray(string name, vector<uint16_t>& value)
{
  int    argc;
  const char **argv;
  string sValue = m_pConfiguration->cget(name);
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);

  assert(argc == 16);		// Validator should have done this.

  for(int i =0; i < 16; i++) {
    value.push_back(static_cast<uint16_t>(strtol(argv[i], NULL, 0)));
  }
  

  Tcl_Free((char*)argv);

   
}

/**********************************************************************/
/*              Checked immediate CAMAC operations                    */
/**********************************************************************/

/*
   This function performs an immediate control operation on the
   CCUSB.  The return value and the q/x are checked.  If any of them
   indicate the command could not be executed, a string exception
   is thrown.  The user supplies a formatting string
   that can contain appropriate sprintf control codes that insert
   the slot, subaddress, and function code (in that order) into their
   message.  The message is prepended with a stock message explaining
   what went wrong (e.g. "Bad status on CCUSB control operation: user msg")
   and then thrown as a string.

   Parameter:
    controller - Reference to the CCUSB controller object on which the
                 operation will be attempted.
    n          - Target Slot
    a          - Target subadress
    f          - Function code to do.
    message    - Message template.

*/
void
CAD811::checkedControl(CCCUSB& controller,
		       int n, int a, int f, string message)
{
  uint16_t qx;
  int      status = controller.simpleControl(n,a,f, qx);
  check(status, qx, 
	n,a,f,0,
	string("Error in checkedControl: "), message);
}/*
   Checks the status of a camac function and if it has failed,
   throws an appropriate string exception.
   The string exception is constructed as follows:
   prefix : reason : formatted-text.
   where formatted-text is constructed by doing an 

   sprintf(fomrmattedtext, format, n,a,f,d)
   
*/
void
CAD811::check(int status, uint16_t qx,
	      int n, int a, int f, int d,
	      string prefix, string format)
{
  string message = prefix;
  bool   trouble = false;

  if (status != 0) {
    message += " CCUSB operation failed : ";
    trouble = true;
  }
  // If q/x are missing, then the most serious would be a missing
  // X and then finally a missing Q
  //
  if (!trouble && ((qx & (CCCUSB::Q | CCCUSB::X)) != (CCCUSB::Q | CCCUSB::X))) {
    trouble = true;
    if ((qx & CCCUSB::X) == 0) {
      message += " No X response from module : ";
    }
    else {
      message += " No Q response from module : ";
    }
  }
  if (trouble) {
    char formattedText[1000];
    snprintf(formattedText, sizeof(formattedText), format.c_str(), n,a,f,d);
    message += formattedText;
    throw message;
  }
}
