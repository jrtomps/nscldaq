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
#include "CPH7xx.h"

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


/*************************************************************************/
/*                               Local constants:                        */
/*************************************************************************/

// Bits in the Ph76xx control register:

static const uint16_t PED_ENABLE(0x01);
static const uint16_t LT_ENABLE (0x02);
static const uint16_t UT_ENABLE (0x04);


// Configuration value constraints.

static CConfigurableObject::limit One(1); 
static CConfigurableObject::limit Zero(0);
static CConfigurableObject::limit FourK(4095);
static CConfigurableObject::limit FULL16(0xffff);
static CConfigurableObject::limit LastSlot(23);
static CConfigurableObject::limit ChannelCount(16);

static CConfigurableObject::Limits SlotLimits(One, LastSlot); // CAMAC crate.
static CConfigurableObject::Limits ParamLimits(Zero, FourK); // Ped, llt, hlt.
static CConfigurableObject::Limits IdLimits(Zero, FULL16);

// The parameter arrays need to be constrained to have exactly
// ParamLimits elements, all of whom are in the range ParamLimits.

static CConfigurableObject::ListSizeConstraint ParamListSize = {ChannelCount,
								ChannelCount};
static 
CConfigurableObject::TypeCheckInfo ParamsOk(CConfigurableObject::isInteger,
					   &ParamLimits);
static 
CConfigurableObject::isListParameter ParamValidity = {ParamListSize,
						      ParamsOk};
						  


/************************************************************************/
/*               Canonical member function implementations.             */
/************************************************************************/

/*!
   Construction is largely a no-op because the configuration is
   hooked to this modules at attachment time.

*/
CPH7xx::CPH7xx()  :
  m_pConfiguration(0)
{
}
/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CPH7xx::CPH7xx(const CPH7xx& rhs) :
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
  \return CPH7xx&
  \retval *this
*/
CPH7xx&
CPH7xx::operator=(const CPH7xx& rhs)
{
  return *this;
}

/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CPH7xx::~CPH7xx()
{
}

/*********************************************************************/
/*          Implemetations of the CReadoutHardware interface         */
/*********************************************************************/


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
CPH7xx::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // Create the parameters:

  configuration.addParameter("-slot", CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-sparse", CConfigurableObject::isBool,
			     NULL, "true");
  configuration.addParameter("-readhits", CConfigurableObject::isBool,
			     NULL, "true");


  configuration.addParameter("-pedestals", CConfigurableObject::isIntList,
			     &ParamValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  configuration.addParameter("-llt", CConfigurableObject::isIntList,
			     &ParamValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  configuration.addParameter("-hlt", CConfigurableObject::isIntList,
			     &ParamValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  configuration.addParameter("-usellt", CConfigurableObject::isBool,
			     NULL, "false");
  configuration.addParameter("-usehlt", CConfigurableObject::isBool,
			     NULL, "false");
  configuration.addParameter("-usepedestals", CConfigurableObject::isBool,
			     NULL, "false");
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
CPH7xx::Initialize(CCCUSB& controller)
{
  // If the slot has not been set by the user it will be 0, 
  // in that case, throw an exception as we can't really initialize the
  // module.

  int slot = getIntegerParameter("-slot");
  if (!slot) {
    throw "A PH7xx module has not had a slot configured";
  }

  // Inhibit the crate, everything else is in a try/catch block that
  // ensures that we re-enable the crate in case of problems.
  // All CAMAC operations are checked for X/Q to be true indicating
  // the module was actually in the slot and accepted our
  // operations... Failures throw so that's why we need the try/catch block.

  controller.inhibit();		// assume this works for now.

  try {
    checkedControl(controller,
		   slot, 0, 9, 
		   "Failed to clear Ph7xx in slot %d"); // clear

    // figure out the control register bits; and write it:

    bool usepeds  = getBoolParameter("-usepedestals");
    bool usellt   = getBoolParameter("-usellt");
    bool usehlt   = getBoolParameter("-usehlt");

    uint16_t controlRegister = 0;
    if (usepeds) controlRegister |= PED_ENABLE;
    if (usellt)  controlRegister |= LT_ENABLE;
    if (usehlt)  controlRegister |= UT_ENABLE;
    
    checkedWrite16(controller,
		   slot, 0, 19, controlRegister,
   "Ph7xx control register write failed in slot %d (a=%d f=%d) value = 0x%04x");
    // Program the parameters that we are supposed to program.
    // Once more we only program the parameters that are enabled
    // as the CC-USB is painfully slow in single shot mode.

    // Each operation requires a parameter memory select and then
    // a bunch of writes.  The parameter memory selects are >sigh< write
    // operation which don't actually write data (we use a dummy value of 0).

    if (usepeds) {
      checkedWrite16(controller,
		     slot, 0, 17, 0,
		     "Selecting slot %d Ph7xxx pedestal memory");
      vector<uint16_t> pedestals;
      getArray("-pedestals", pedestals);
      for (int i=0; i < 16; i++) {
	checkedWrite16(controller,
		       slot, i, 20, pedestals[i],
            "Pedestal write failed to Ph7xx in slot %d (a=%d f=%d), value=%d");
      }
    }
    if (usellt) {
      checkedWrite16(controller,
		     slot, 1, 17, 0,
		     "Selecting slot %d Ph7xxx low level threshold memory");
      vector<uint16_t> llts;
      getArray("-llt", llts);
      for (int i=0; i < 16; i++) {
	checkedWrite16(controller,
		       slot, i, 20, llts[i],
	       "LLT write failed to Ph7xxx in slot %d (a=%d f=%d) value= %d");
      }
    }
    if (usehlt) {
      checkedWrite16(controller,
		    slot, 2, 17, 0,
		    "Selecting slot %d Ph7xxx high level threshold memory");
      vector<uint16_t> hlts;
      getArray("-hlt", hlts);
      for (int i =0; i < 16; i++) {
	checkedWrite16(controller,
		       slot, i, 20, hlts[i],
	       "HLT Write failed to Ph7xxx in slot %d (a=%d f=%d) value = %d");
      }
    }


    // Enable the module LAMs and remove the inhibit.
    
    checkedControl(controller,
		   slot, 0, 26, 
		   "Enabling LA on Ph7xxx in slot %d");
    controller.uninhibit();
  }
  catch(...) {
    controller.uninhibit();
    throw;
  }
}

/*!
   Add the commands required to read the module to a CCUSB readout list.
   In our case we add the following:
   - If -readhits is requested, read the hit register (lamwaited)
   - If -sparse is requested, do a sparse read, if not, read all 16 channels.
   \param list - the CCCUSBReadoutList into which to put the instructions.
*/
void
CPH7xx::addReadoutList(CCCUSBReadoutList& list)
{
  // We're going to need the following parameters:
  // -slot     - where the module lives.
  // -readhits - If we are supposed to put a hit register read in.
  // -sparse   - How to read the module.

  int   slot   = getIntegerParameter("-slot");
  bool  hits   = getBoolParameter("-readhits");
  bool  sparse = getBoolParameter("-sparse");
  int   id     = getIntegerParameter("-id");

  bool  waitLam= true;   	// First operation will wait for lam.

  list.addMarker(id);

  if (hits) {
    list.addRead16(slot, 1, 6, waitLam); // read hit register.
    waitLam = false;
  }
  // How we read depends on the mode.. if -sparse, we read
  // with a qstop. If not, a Q-scan (as all channels should return a Q).

  if (sparse) {
    list.addQStop(slot, 0, 4, 16+1); // At most 16 channels of data + no q.
  }
  else {
    list.addQScan(slot, 0, 0, 16); // Read all 16 channels of data.
  }
  // Clear the module data/lam:

  list.addControl(slot, 3, 11);	// Reset hit register, lam  & memory.

}
/*!

   Clone ourself.  This is essentially a virtual contstructor.
   We just do a new *this

*/
CReadoutHardware*
CPH7xx::clone() const
{
  return new CPH7xx(*this);
}

/*********************************************************************/
/*                Utilities to access the configuration              */
/*********************************************************************/

// Return the named integer parameter.
// The configuration validators have ensured the parameter actually
// is an integer.

unsigned int
CPH7xx::getIntegerParameter(string name)
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
CPH7xx::getBoolParameter(string name)
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
CPH7xx::getArray(string name, vector<uint16_t>& value)
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
CPH7xx::checkedControl(CCCUSB& controller,
		       int n, int a, int f, string message)
{
  uint16_t qx;
  int      status = controller.simpleControl(n,a,f, qx);
  check(status, qx, 
	n,a,f,0,
	string("Error in checkedControl: "), message);
}
/*
  This function is similar to checkedControl but does a write.
  parameters are the same, but the data parameter is the write data.
*/
void
CPH7xx::checkedWrite16(CCCUSB& controller,
		       int n, int a, int f, uint16_t data,
		       string message)
{
  uint16_t qx;
  int      status = controller.simpleWrite16(n,a,f, data, qx);
  check(status, qx,
	n,a,f,data,
	string("Error in checkedWrite16: "), message);
}

/*
   Checks the status of a camac function and if it has failed,
   throws an appropriate string exception.
   The string exception is constructed as follows:
   prefix : reason : formatted-text.
   where formatted-text is constructed by doing an 

   sprintf(fomrmattedtext, format, n,a,f,d)
   
*/
void
CPH7xx::check(int status, uint16_t qx,
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
