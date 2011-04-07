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

#include <config.h>
#include "CV6533.h"

#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

// Register offset:'

#define Const(name) static const uint32_t name =

// Board parameters (common to all channels):

Const BoardVmax     0x50;
Const BoardImax     0x54;
Const BoardStatus   0x58;
Const Firmware      0x5c;

// Each Channels has a common format that is described later:

Const(Channels[6])  = {
  0x080, 0x100, 0x180, 0x200, 0x280, 0x300
};

// Board configuration parameters:

Const(ChannelCount)   0x8100;
Const(Description)    0x8102;
Const(Model)          0x8116;
Const(SerialNo)       0x811e;
Const(VmeFirmware)    0x8120;

//  Below are the register offsets within each of the Channels array elements:

Const(Vset)      0x00;
Const(Iset)      0x04;
Const(VMon)      0x08;
Const(Imon)      0x0c;
Const(PW)        0x10;
Const(ChStatus)  0x14;
Const(TripTime)  0x18;
Const(SVMax)     0x1c;
Const(RampDown)  0x20;
Const(RampUp)    0x24;
Const(PwDown)    0x28;
Const(Polarity)  0x2c;
Const(Temp)      0x30;

// Global status register bits:

Const(Chan0Alarm)    0x0001;
Const(Chan1Alarm)    0x0002;
Const(Chan2Alarm)    0x0004;
Const(Chan3Alarm)    0x0008;
Const(Chan4Alarm)    0x0010;
Const(PwrFail)       0x0080;
Const(OverPwr)       0x0100;
Const(MaxVUncal)     0x0200;
Const(MaxIUncal)     0x0400;

// Individual channel status register bits.

Const(On)           0x0001;
Const(RampUp)       0x0002;
Const(RampDown)     0x0004;
Const(OverCurrent)  0x0008;
Const(OverVoltage)  0x0010;
Const(UnderVoltage) 0x0020;
Const(MaxV)         0x0040;
Const(MaxI)         0x0080;
Const(Trip)         0x0100;
Const(OverPower)    0x0200;
Const(Disabled)     0x0400;
Const(Interlocked)  0x0800;
Const(Uncalibrated) 0x1000;


//  Address modifier to use when accessing the board:

static const uint8_t amod = CVMUSBReadoutList::a32PrivData;

/*------------------------------------- Canonical methods */

/**
 * Construct the object.  For now we won't do anything
 * with the monitored registers.  We will just let those
 * populate as the update/monitor list runs.
 */
CV6533::CV6533(string name) :
  CControlHardware(name),
  m_pConfiguration(0)
{
}
/**
 * Copy construction; clone already does what we need
 * just invoke it:
 */
CV6533::CV6533(const CV6533& rhs) :
  CControlHardware(rhs)
{
  *this = clone(rhs);
}
/**
 * Destruction is not really needed since we are not going to be
 * destroyed for the life of the program.
 */
CV6533::~CV6533()
{
}
/**
 *  Assignment is also taken care of by clone for the most part.
 * Just need to be sure we don't do anything on a self assign.
 * @param rhs - The object being assigned to us.
 * @return CV6533
 * @retval *this
 */
CV6533&
CV6533::operator=(const CV6533& rhs)
{
  if (this != &rhs) {
    clone(rhs);
  }
  return *this;
}
/**
 * Equal configuration implies equality.
 * @param rhs  - The object to check with.
 */
int
CV6533::operator==(const CV6533& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/**
 ** Inequality as usual is the inverse of equality:
 ** @param rhs - the object to compare with *this
 */
int
CV6533::operator!=(const CV6533& rhs) const
{
  return !(*this == rhs);
}
/*------------------ Operations for running the module -----*/


/**
 * This function is called when the object is attached
 * to its configuration.  We must save the configuration
 * and register our parameters.  At present, for HV
 * units we are not going to save any configuration.
 * that would be the responsibility of the GUI the user
 * interacts with
 * @param configuration - Configuration object that will
 *                        maintain our object's configuration.
 */
void
CV6533::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", \
			     CConfigurableObject::isInteger,
			     NULL, string("0"));
}
/**
 * Called to initalize the module.  For us initialization is
 * playing it safe: All the voltages are set to zero.
 * Each channel is turned off.
 * @param vme   - Object that proxies for the VM-USB 
 *                controller module.
 */
void
CV6533::Initialize(CVMUSB& vme)
{


  CVMUSBReadoutList list;	// List of configuration ops.
  for (int i =0; i < 6; i++) {
    turnOff(list, i);
    setRequestVoltage(list, i, 0.0);
  }
  // Execute the list.
  size_t buffer;		// No actual reads..
  int status = vme.executeList(list,
			       &buffer, sizeof(buffer),
			       &buffer);

}
/**
 * For devices with write-only registers, this function
 * would set the device to a configuration that matches
 * state held in the driver.  This function serves
 * no purpose for the CAEN V6533 and thus is a no-op.
 * @param vme - VM-USB proxy object.
 * @return string
 * @retval OK
 */
string
CV6533::Update(CVMUSB& vme)
{
  return "OK";
}
/**
 *  Processes set operations for the device.  See
 * the header for a description of the supported 
 * parameters.
 * @param vme         - Proxy object for the VM-USB controller.
 * @param parameter   - Name of the parameter to set.
 *                      in the parmeter set described in the
 *                      header the n trailing each paramter name
 *                      is the channel number (0-5).
 * @param value       - Value of the parameter.
 * @return string
 * @retval "OK"   - The set worked correctly.
 * @retval "ERROR - ..." Some error occured. The remainder
 *                       of the string describes
 *                       the error
 */
string
CV6533::Set(CVMUSB& vme, string parameter, string value)
{
  // The meaning of the value depends on the actual parameter
  // type.  What this function does is determine the
  // root parameter name and the channel number (if
  // applicable) and dispatch to the appropriate function.

  unsigned channelNumber;
  CVMUSBReadoutList list;
  try {

    if (sscanf(parameter.c_str(), "v%u", &channelNumber) == 1) {
      setRequestVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "i%u",
		    &channelNumber) == 1) {
      setRequestCurrent(list, channelNumber, atof(value.c_str()));
    }
    else if(sscanf(parameter.c_str(), "on%u", &channelNumber) == 1) {
      setChannelOnOff(list, channelNumber, strToBool(value));
    }
    else if (sscanf(parameter.c_str(), "ttrip%u", &channelNumber) == 1) {
      setTripTime(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "svmax%u", &channelNumber) == 1) {
      setMaxVoltage(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "rdown%u", &channelNumber) == 1) {
      setRampDownRate(list, channelNumber, atof(value.c_str()));
    } 
    else if (sscanf(parameter.c_str(), "rup%u", &channelNumber) == 1) {
      setRampUpRate(list, channelNumber, atof(value.c_str()));
    }
    else if (sscanf(parameter.c_str(), "pdownomode&u", &channelNumber) ==1 ) {
      setPowerDownMode(list, channelNumber, value);
    }
    else {
      throw string("Unrecognized parameter");
    }

    // If there's a non zero length list, execute it.
    
    if (list.size() > 0) {
      size_t buffer;
      int status= vme.executeList(list,
				  buffer, sizeof(buffer),
				  &buffer);
      if (status != 0) {
	throw string("VME list execution failed");
      }
    }
  }
  catch(string msg) {
    string error = "ERROR - ";
    error += msg;
    return error;
  }
  
  return string("OK");
}
/**
 * Get - this function retrieves the value of a parameter from the device.
 *       note that if a parameter is in the monitor set it will still be read from the device
 *       though the monitored data will be updated by the read that results.
 *       For readings that are per channel, we are going to save some time by 
 *       reading all values from all channels.. When data taking is in progress, this will
 *       reduce the number of times the Get needs to be invoked and hence the number
 *       of times the DAQ needs to be stopped.
 *  @param vme        - Object that stands in for the VM-USB controller.
 *  @param parameter  - Name of the paramter to get.. in the case of the per channel parameters
 *                      leave off the channel number as all channesl will be read and returned
 *                      as a Tcl List.
 * @return string
 * @retval "ERROR -..." An error occured, the remainder of the text string is a human readable
 *                      error message.
 * @retval "OK xxx"   Everything worked.  If xxx represents a single paramter (e.g. globalmaxv
 *                    this will just be a number.  If the requested parameter was a per channel value
 *                    xxx will be a correctly formatted Tcl list that contains the per channel
 *                    values of the parameter.
 */
string
CV6533::Get(CVMUSB& vme, string parameter)
{
  // error handing in the utilities means tossing exceptions with the error message.
  // the individual utilities will return the correct appropriate output (not the ok)

  string ok = "OK ";
  string result;
  try {
    // Big honking if/else to deal with all the parameters... all oparameters are readable
    // and the utilities will take care of updating the member variables:

    if (parameter == "globalmaxv") {
      result = getGlobalMaxV(vme);
    }
    else if (parameter == "globalmaxI") {
      result = getGlobalMaxI(vme);
    }
    else if (parameter == "v") {
      result = getChannelVoltages(vme);
    }
    else if (parameter == "i") {
      result = getChannelCurrents(vme);
    }
    else if (parameter == "on") {
      result = getOnOffRequest(vme);
    }
    else if (parameter == "vact") {
      result = getActualVoltages(vme);
    }
    else if (parameter == "iact") {
      result = getActualCurrents(vme);
    }
    else if (parameter == "status") {
      result = getChannelStatuses(vme);
    }
    else if (parameter == "ttrip") {
      result = getTripTimes(vme);
    }
    else if (parameter == "svmax") {
      result = getSoftwareVmax(vme);
    }
    else if (parameter == "rdown") {
      result = getRampDownRates(vme);
    }
    else if (parameter == "rup") {
      result = getRampUpRates(vme);
    }
    else if (parameter == "pdownmode") {
      result = getPowerDownModes(vme);
    }
    else if (parameter == "polarity") {
      result = getPolarities(vme);
    }
    else if (parameter == "temp") {
      result = getTemperatures(vme);
    }
    else {
      throw "Invalid parameter";
    }
  }
  catch(string msg) {
    string error = "ERROR - ";
    error       += msg;
    return error;
  }
  ok += result;
  return result;
}
/**
 * clone another CV6533 into this...this implements a virtual copy constructor.
 *
 * @param rhs - the object to clone into *this
 */
void
CV6533::clone(const CControlHardware& rhs)
{
  m_pConfiguration = new CControlHardware(rhs.m_pConfiguration); // Already has has config params registered.
  m_globalStatus   = rhs.m_globalStatus;
  memcpy(m_channelStatus, rhs.m_channelStatus, sizeof(m_channelStatus));
  memcpy(m_voltages,      rhs.m_voltages,      sizeof(m_voltages));
  memcpy(m_currents,      rhs.m_currents,      sizeof(m_currents));
  memcpy(m_temperatures,  rhs.m_temperatures,  sizeof(m_temperatures));
								  
}

/*----------------------------------  Monitoring --------------------------*/

/**
 * Add our list to the monitoring functions.  Unfortunately, the whole
 * layout of the register set and 16 bitedness means we're going to be doing
 * single shot transfers rather than blocks.
 * @param vmeList - VME readout list to which we will be adding data.
 */
void
CV6533::addMonitorList(CVMUSBReadoutList& vmeList)
{
  uint32_t base  = getBase();
  // Global status:

  vmeList.addRead16(base + BoardStatus, amod);
  
  // Channel stuff will be grouped by channel so that looping can be done both here and
  // when unpacking:

  for(int i =0; i < 6; i++) {
    vmeList.addRead16(base + Channels[i] + ChStatus, amod); // Channel status.
    vmeList.addRead16(base + Channels[i] + VMon,     amod); // Actual voltage.
    vmeList.addRead16(base + Channels[i] + IMon,     amod); // Monitored voltage.
    vmeList.addRead16(base + Channels[i] + Temp,     amod); // Channel temperature.
  }

}
/**
 * Unpack the data from the monitor list into our member data.
 * @param pData      - Pointer to where in the buffer to start.
 * @param remaining  - Words of data left in the list.
 * @return void*
 * @retval pointer to the next unused part of pData.
 */
void
CV6533::processMonitorList(void* pData, size_t remaining)
{
  // For now don't bother with remaining... TODO: Figure out some sort of
  // error handling strategy if remaining is too small to satisfy our needs.

  uint16_t*   p = reinterpret_cast<uint16_t*>(pData);

  m_globalStatus = *p++;		// The global status register is first.
  for (int i = 0; i < 6; i++) {		// followed by per channel data.
    m_channelStatus[i] = *p++;
    m_voltages[i]      = *p++;
    m_currents[i]      = *p++;
    m_temperatures[i]  = *p++;
  }
  return p;
}  


/*----------------------------- Utility functions ----------------------------- */

/**
 * Get the base address of the module from the configuration.
 * @return uint32_t
 * @retval - Value of the -base configuration value.
 */
uint32_t
CV6533::getBase()
{
  return m_pConfiguration->getUnsignedParameter("-base");
}

/**
 * Add a command to turn a specified channel off to a VME list.
 * @param list    - CVMUSBReadoutList to which to add the command.
 * @param channel - Channel to which to add the command (not range checked).
 */
void
CV6533::turnOff(CVMUSBReadoutList& list, unsigned int channel)
{
  list.addWrite16(getBase() + Channels[channel] + PW , amod, 0);
}
/**
 * Add a request to set the requested voltage for a channel to a VME operation list.
 * @param list     - CVMUSBReadoutList& to which the request is added.
 * @param channel  - Channel number whose request voltage will be modified.
 * @param value    - Floating point voltage to request.
 *
 * @note Channel and value are not range checked.
 */
void
CV6533::setRequestVoltage(CVMUSBReadoutList& list, unsigned int channel, float value)
{
  uint16_t vrequest = (uint16_t)(value * 10.0); // Request resolution is .1 V  see manual 3.2.2.1

  list.addWrite16(getBase() + Channels[channel] + Vset, amod, vrequest);
}
/**
 * Add a request to set the requested current max for a specified channel to a VME operation list.
 * @param list    - The CVMUSBReadoutList& to add to.
 * @param channel - Number of the channel to modify.
 * @param value   - The micro-amp current limit.
 * 
 * @note neither value nor channel are range checked.
 */
void
CV6533::setRequestCurrent(CVMUSBReadoutList& list, unsigned int channel, float value)
{
  uint16_t irequest = (uint16_t)(value/0.05); // Register value.
  list.addWrite16(getBase() + Channels[channel] + Iset, amod, irequest);
} 
/**
 * Add a request to turn a channel on or off to a readout list.
 * @param list     - CVMUSBReadoutList& to which the request will be added.
 * @param channel  - Number of the channel to set.
 * @param on       - true to turn channel on or false to turn it off.
 *
 * @note The channel number is not range checked.
 */
void
CV6533::setChannelOnOff(CVMUSBReadoutList& list, unsigned int channel, bool value)
{
  list.addWrite16(getBase() + Channels[channel] + PW, amod, 
		  value ? 1 : 0); // Not going to assume anything about true/false representation.
}
/**
 * Set the trip time.  Presumably this is the amount of time
 * an out of condition state is allowed to persist
 * before the channel trips off.
 * @param list   - VMUSB list to which the request will
 *                 be added.
 * @param channel - Which channel should be affected.
 * @param time    - Time in seconds.
 *
 */
void 
CV6553::SetTripTime(CVMUSBReadoutList& list, unsigned int channel, float time)
{
  uint16_t timeval = (uint16_t)(time/0.1); // register value.
  list.addWrite16(getBase() + Channels[channel] + TripTime,
		  amod, timeval);
}
/**
 * Set the maximum allowed voltage (software voltage limit)
 * for the channel.  If a voltage setting is made above
 * this limit, the setting is limited to this level.
 * @param list    - VMUSB list to which the request is added
 * @param channel - Channel that should be affected.
 * @param voltage - Voltage limit (in volts).
 */
void
CVM6533::setMaxVoltage(CVMUSBReadoutList& list,
		       unsigned int channel, float voltage)
{
  uint16_t voltval = (uint16_t)(voltage/.1);
  list.addWrite16(getBase() + Channels[channel] + SVMax,
		  amod, voltval);
}
/**
 * Set the rate at which a channel ramps down.
 * @param list    - VMUSB list to which to add the request.
 * @param channel - Channel to modify.
 * @param rate    - Rate in Volts/sec
 */
void
CV6533::setRampDownRate(CVMUSBReadoutList& list,
			unsigned int channel, float rate)
{
  uint16_t rateVal = (uint16_t)rate;
  list.addWrite16(getBase() + Channels[channel] + RampDown,
		  channel, rateVal);
}
/**
 * set the rate at which a channel ramps up in voltage.
 * @param list    - VMUSB list to which to add the request.
 * @param channel - Channel number affected.
 * @param rate    - Ramp rate in Volts/sec.
 */
void
CV6533::setRampDownRate(CVMUSBReadoutList& list,
			unsigned int channel,
			float rate)
{
  uint16_t rateVal = (uint16_t)rate;
  list.addWrite16(getBase() + Channels[channel] + RampUp);
}
/**
 * Set the power down mode.
 * @param list   - VMUSB list to which the request is added.
 * @param channel- Number of the channel to modify.
 * @param mode   - Mode to use  This can be one of two values:
 *                 - kill : power off in a hurry.
 *                 - ramp : Ramp down at the ramp down rate.
 */
void
CV6533::setPowerDownMode(CVMUSBReadoutList& list,
			 unsigned int chanel, string mode)
{
  uint16_t modeVal;
  if (mode == "kill") {
    modeVal = 0;
  }
  else if (mode == "ramp")  {
    modeVal = 1;
  }
  else {
    throw string("Illegal mode value");
  }
  list.addWrite16(getBase() + Channels[channel] + PwDown,
		  amod, modeVal);

}
/**
 * Convert a string to a boolean..throws an exception if the string is not a valid bool.
 * @param value - value to convert.
 * @return bool
 * @retval true - if the string is a valid representation of true (as defined by CConfigurableObject).
 * @retval false - if the string is a valid representatino of false (as defined by CConfigurableObject).
 * @throw string if the string is not a valid bool.
 */
bool
CV6533::strToBool(string value)
{
  if (!CConfigurableObject::("null", value, NULL)) {
    throw string(" Invalid booleanvalue");
  }
  return CConfigurableObject::strToBool(value);
}
