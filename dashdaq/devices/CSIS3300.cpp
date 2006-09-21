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
#include "CSIS3300.h"
#include <CVMEList.h>
#include <CVMEInterface.h>
#include <CVMEAddressRange.h>
#include <CVmeDMATransfer.h>

#include <RangeError.h>
#include <CInvalidInterfaceType.h>

#include <stdlib.h>
#include <stdio.h>

using namespace std;

/// Some nice definitions:

#define Const(varname)  static  uint32_t varname =

static const uint8_t RegisterAmod(0x0d); // Supervisory extended data.
static const uint8_t BlockAmod(0x0f); // Supervisory extended block xfer.

static const size_t pageToSize[9] = {
  (1 << 17),			// 128K
  (1 << 14),			// 16K
  (1 << 12),			// 4K
  (1 << 11),			// 2K
  (1 << 10),			// 1K
  (1 << 9),			// 512
  (1 << 8),			// 256
  (1 << 7)			// 128
};				// These are in the same order as the PageSize enum.

/////////////////////////////////////////////////////////////////////
//////////////// SIS 3300 Register map and defs. ///////////////////
////////////////////////////////////////////////////////////////////

// The register page for the SIS 3300 s in 6 disjoint segments.
// The first two are a general control page and an 'all adcs' page.
// The last 4 are specific to an ADC group and have a common layout.
//



// First page are control status and keys:

Const(ControlStatus)     0x0000;
Const(IDFirmware)        0x0004;
Const(InterruptConfig)   0x0008;
Const(InterruptControl)  0x000c;
Const(AcqControl)        0x0010;
Const(StartDelay)        0x0014;
Const(StopDelay)         0x0018;
Const(TimeStampDivider)  0x001c;
Const(KeyReset)          0x0020;
Const(StartSampling)     0x0030;
Const(StopSampling)      0x0034;
Const(StartAutoSwitch)   0x0040;
Const(StopAutoSwitch)    0x0044;
Const(ClearBank1Full)    0x0048;
Const(ClearBank2Full)    0x004c;
Const(EventTimeStamp1)   0x1000;
Const(EventTimeStamp2)   0x2000;

Const(CSRPageSize)       0x3000;

// Second page are registers that have effect on all groups of the module
// (the channels are into 4 parts divided, 2 channels per group.


Const(AllOffset)                  0x100000; // Offset to this register page from base.
Const(EventConfigAll)             0x0000;
Const(TriggerThresholdAll)        0x0004;
Const(TriggerFlagClearCounterAll) 0x001c;
Const(ClockPredividerAll)         0x0020;
Const(NumSamplesAll)              0x0024;
Const(TriggerSetupAll)            0x0028;
Const(MaxEventCountAll)           0x002c;
Const(EventDir1All)               0x1000;
Const(EventDir2All)               0x2000;

Const (AllPageSize)               0x3000;

// Event information pages for specific ADC's all have the same format:

Const(ADCGroup1Offset)           0x200000;
Const(ADCGroup2Offset)           0x280000;
Const(ADCGroup3Offset)           0x300000;
Const(ADCGroup4Offset)           0x380000;

Const(EventConfigGroup)          0x0000;
Const(TriggerThresholdGroup)     0x0004;
Const(Bank1AddressGroup)         0x0008;
Const(Bank2AddresssGroup)        0x000c;
Const(Bank1EventCountGroup)      0x0010;
Const(Bank2EventCountGroup)      0x0014;
Const(ADCSampleGroup)            0x0018; // Single sammple value for the group.
Const(TriggerFlagClearCounterGroup) 0x001c;
Const(ClockPredividerGroup)      0x0020;
Const(NumSamplesGroup)           0x0024;
Const(TriggerSetupGroup)         0x0028;
Const(MaxEventCountGroup)        0x002c;
Const(EventDirBank1Group)        0x1000;
Const(EventDirBank2Group)        0x2000;


Const(ADCPageSize)               0x3000;

// Sample memory:

Const(Bank1Group1Offset)         0x400000;
Const(Bank1Group2Offset)         0x480000;
Const(Bank1Group3Offset)         0x500000;
Const(Bank1Group4Offset)         0x580000;

Const(Bank2Group1Offset)         0x600000;
Const(Bank2Group2Offset)         0x680000;
Const(Bank2Group3Offset)         0x700000;
Const(Bank2Group4Offset)         0x780000;

Const(GroupSampleSize)           0x080000;

static const uint32_t bank1Offsets[4] = {
  Bank1Group1Offset,
  Bank1Group2Offset,
  Bank1Group3Offset,
  Bank1Group4Offset
};

static const uint32_t bank2Offsets[4] = {
  Bank2Group1Offset,
  Bank2Group2Offset,
  Bank2Group3Offset,
  Bank2Group4Offset
};

static const uint32_t* bankOffsets[2] = {
  bank1Offsets,
  bank2Offsets
};

// Now bits and bit fields  in the various registers...

// Control status register is CSR_ symbols below;  
// To paraphrase T.S. Elliot:
//   ... you may think that I'm mad as a hatter;
//    when I tell you each bit as 3 separate names...
// This register is a 
// combined bit set/bit clear register on write
// (CSR_BSxxx and CSR_BCxxx symbosl), and has status information that
// may or may not line up with corresponding bitset or clears and
// will therefore have (CSR_RDxxxx) names.  So....to paraphrase
//
Const(CSR_BSLedOn)             0x00000001;
Const(CSR_BCLedOn)             0x00010000;
Const(CSR_RDLedStatus)         0x00000001;

Const(CSR_BSUserOutput)        0x00000002;
Const(CSR_BCUserOutput)        0x00020000;
Const(CSR_RDUserOutput)        0x00000002;

Const(CSR_BSEnableTriggerOut)  0x00000004;
Const(CSR_BCEnableTriggerOut)  0x00040000;
Const(CSR_RDEnableTriggerOut)  0x00000004;

Const(CSR_BSInvertTriggerOut)  0x00000010;
Const(CSR_BCInvertTriggerOut)  0x00100000;
Const(CSR_RDInvertTriggerOut)  0x00000010;

Const(CSR_BSTrigOnArmedAndStarted) 0x00000020;
Const(CSR_BCTrigOnArmedAndStarted) 0x00200000;
Const(CSR_RDTrigOnArmedAndStarted) 0x00000020;

Const(CSR_BSInternalTriggerRouting) 0x00000040;
Const(CSR_BCInternalTriggerRouting) 0x00400000;
Const(CSR_RDInternalTriggerRouting) 0x00000040;

Const(CSR_BSBankFullOutput1)   0x00000100;
Const(CSR_BCBankFullOutput1)   0x01000000;
Const(CSR_RDBankFullOutput1)   0x00000100;

Const(CSR_BSBankFullOutput2)   0x00000200;
Const(CSR_BCBankFullOutput2)   0x02000000;
Const(CSR_RDBankFullOutput2)   0x00000200;

Const(CSR_BSBankFullOutput3)   0x00000400;
Const(CSR_BCBankFullOutput3)   0x04000000;
Const(CSR_RDBankFullOutput3)   0x00000400;

Const(CSR_RDUserInput)         0x00010000;
Const(CSR_RDP2_TEST_IN)        0x00020000;
Const(CSR_RDP2_RSET_IN)        0x00040000;
Const(CSR_RDP2_SAMPLE_IN)      0x00080000;

//  The IDFirmware register becomes ID_
//  This is a straight read only register with 3 bit fields.
//  The fields are defined by ID_xxxMASK and ID_xxxSHIFT
//  values.  To extract a field:
//     (value & ID_xxxMASK) >> ID_xxxSHIFT
//  To put a field back:
//     register |= (value << ID_xxxSHIFT) & ID_xxxMASK.
//

Const(ID_minorMASK)          0x000000ff;
Const(ID_minorSHIFT)         0;

Const(ID_majorMASK)          0x0000ff00;
Const(ID_majorSHIFT)         8;

Const(ID_moduleIdMASK)       0xffff0000;
Const(ID_moduleIdSHIFT)      16;

Const(ID_3300)                0x3300; // ID field value for SIS 3300.
Const(ID_3301)                0x3301; // ID field value for SIS 3301.

// The Acquisition and control register (ACTL) writes as bit set/bit clears
// and reads as various status values.  The naming conventions in this
// register are the same as for the CSR:   ACTL_BSxxxx ACTL_BCxxxx and
// ACTL_RDxxxx.
//
Const(ACTL_BSSample1)       0x00000001;
Const(ACTL_BCSample1)       0x00010000;
Const(ACTL_RDSample1)       0x00000001;

Const(ACTL_BSSample2)       0x00000002;
Const(ACTL_BCSample2)       0x00020000;
Const(ACTL_RDSample2)       0x00000002;

Const(ACTL_BSAutoBankSwitch) 0x00000004;
Const(ACTL_BCAutoBankSwitch) 0x00040000;
Const(ACTL_RDAutoBankSwitch) 0x00000004;

Const(ACTL_BSAutoStart)      0x00000010;
Const(ACTL_BCAutoStart)      0x00100000;
Const(ACTL_RDAutoStart)      0x00000010;

Const(ACTL_BSMultiEvent)     0x00000020;
Const(ACTL_BCMultiEvent)     0x00200000;
Const(ACTL_RDMultiEvent)     0x00000020;

Const(ACTL_BSStartDelay)     0x00000040;
Const(ACTL_BCStartDelay)     0x00400000;
Const(ACTL_RDStartDelay)     0x00000040;

Const(ACTL_BSStopDelay)      0x00000080;
Const(ACTL_BCStopDelay)      0x00800000;
Const(ACTL_RDStopDelay)      0x00000080;

Const(ACTL_BSFPStartStop)    0x00000100;
Const(ACTL_BCFPStartStop)    0x01000000;
Const(ACTL_RDFPStartStop)    0x00000100;

Const(ACTL_BSP2StartStop)    0x00000200;
Const(ACTL_BCP2StartStop)    0x02000000;
Const(ACTL_RDP2StartStop)    0x00000200;

Const(ACTL_BSFPGateMode)     0x00000400;
Const(ACTL_BCFPGateMode)     0x04000000;
Const(ACTL_RDFPGateMode)     0x00000400;

Const(ACTL_BSFPRandomClock)  0x00000800;
Const(ACTL_BCFPRandomClock)  0x08000000;
Const(ACTL_RDFPRandomClock)  0x00000800;

   // Clock is a bit field:

Const(ACTL_BSClockSrcMASK)   0x00007000;
Const(ACTL_BCClockSrcMASK)   0x70000000;
Const(ACTL_BSClockSrcSHIFT)  12;
Const(ACTL_BCClockSrcSHIFT)  28;
Const(ACTL_RDClockSrcMASK)   0x00007000;
Const(ACTL_RDClockSrcSHIFT)  12;

Const(ACTL_BSMultiplex)      0x00008000;
Const(ACTL_BCMultiplex)      0x80000000;
Const(ACTL_RDMultiplex)      0x00008000;

Const(ACTL_RDBusy)           0x00010000;
Const(ACTL_RDBankSwBusy)     0x00040000;
Const(ACTL_RDBank1Busy)      0x00100000;
Const(ACTL_RDBank1Full)      0x00200000;
Const(ACTL_RDBank2Busy)      0x00400000;
Const(ACTL_RDBank2Full)      0x00800000;


// The event configuration register (ECR) has a common format for the
// All and group versions of the register.  The claim is that
// really the ALL register is the only one that matters.
// The register is a bunch of bit fields and we already know what those
// naming conventions are.
//
Const(ECR_PageSizeMASK)      0x00000007;
Const(ECR_PageSizeSHIFT)     0;

Const(ECR_AutoWrapMASK)      0x00000008;
Const(ECR_GateChainMASK)     0x00000010;

Const(ECR_ExternalRCMMASK)   0x00000800;
Const(ECR_MuxModeMASK)       0x00008000;

Const(ECR_AverageMASK)       0x00070000;
Const(ECR_AverageSHIFT)      16;

//  Threshold registers (THR_) contains the thresholds and comparision
//  directions for the two ADC's in a group.  The user will need to know
//  what kind of ADC they are playing with as the
//  3300 has 11 bit sof threshold while the 3301, 14bits.
//  The position of the LE:~GT bit is the same in both models.
//  The two adcs will be called Odd and Event... and SIS numbers from
//  1 (odd).
//

Const(THR_ADCEvenThresholdMASK)  0x00001fff;
Const(THR_ADCEvenThresholdSHIFT) 0;
Const(THR_ADCEvenLT)            0x00008000;

Const(THR_ADCOddThresholdMASK)   0x1fff0000;
Const(THR_ADCOddThresholdSHIFT)  16;
Const(THR_ADCOddLT)              0x80000000;

// The trigger setup register (TSR) control the fraction and width of the
// CFD.  The fraction is specified as a numerator and denominator.
//

Const(TSR_DenominatorMASK)       0x0000000f;
Const(TSR_DenominatorSHIFT)      0;
Const(TSR_NumeratorMASK)         0x00000f00;
Const(TSR_NumeratorSHIFT)        8;
Const(TSR_WidthMASK)             0x000f0000;
Const(TSR_WidthShift)            16;
Const(TSR_CFDEnable)             0x01000000;

// The data format depends on the module type.
// We will define e.g. DATA3300_xxx
// and DATA3301_xxx symbols.  The user will have to query
// the module type to understand which to use.
//

Const(DATA3300_EvenCMask)       0x00000fff;
Const(DATA3300_EvenCSHIFT)      0;
Const(DATA3300_EvenOverRange)   0x00001000;
Const(DATA3300_GateChainFirst)  0x00008000;
Const(DATA3300_OddCMask)        0x0fff0000;
Const(DATA3300_OddCShift)       16;
Const(DATA3300_OddOverRange)    0x10000000;
Const(DATA3300_UserBit)         0x80000000;

Const(DATA3301_EvenCMask)       0x00003fff;
Const(DATA3301_EvenOverRange)   0x00004000;
Const(DATA3301_GateChainFirst)  0x00008000;
Const(DATA3301_OddCMask)        0x3fff0000;
Const(DATA3301_OddOverRange)    0x40000000;
Const(DATA3301_OddUserBit)      0x80000000;

static const  uint32_t GroupRegisterOffsets[4] = {
  ADCGroup1Offset, ADCGroup2Offset, 
  ADCGroup3Offset, ADCGroup4Offset};


//////////////////////////////////////////////////////////////////////
/////////////////////// Canonical operations /////////////////////////
//////////////////////////////////////////////////////////////////////

/*!
   Construct an SIS3300 object that will control an FADC module.
   The configuration is set to default values.
   You must use the config*** functions to configure the module,
   the initialize() to set it up according to the configuration.
   \param interface : CVMEInterface&
      References an object representing the interface connected to the
      vme crate in which the module has been installed.
   \param base : uint32_t
      Module base address as set by its rotary switches.
   
*/
CSIS3300::CSIS3300(CVMEInterface& interface, uint32_t base) :
  CVMEReadableObject(interface, base),
  m_baseAddress(base),
  m_pRegisters(0),
  m_pCommonRegisters(0)
{
  for(int i =0; i< 4; i++) {
    m_ppGroupRegisters[i] = 0;
  }
  setDefaults();
}
/*!
   Copy construct an SIS 3300.  This constructs another instance of an
   SIS3300 that initially is a duplicate of the rhs one.  Note that it
   is possible for the configuration of these modules to diverge which 
   could be a bad thing depending on how this is used.
   \param rhs : CSIS3300&
      Reference to the CSIS3300 obeject that will be cloned to produce this.
*/
CSIS3300::CSIS3300(const CSIS3300& rhs) :
  CVMEReadableObject(rhs)
{
  copyIn(rhs);
}
/*!
  Destruction is just a matter of killing off any cached register
  address range object.
*/
CSIS3300::~CSIS3300()
{
  delete m_pRegisters;
}
/*!
  Assignment is just a copy in:
*/
CSIS3300&
CSIS3300::operator=(const CSIS3300& rhs)
{
  if (this != &rhs) {
    CVMEReadableObject::operator=(rhs);
    copyIn(rhs);
  }
  return *this;
}
/*!
  Modules are equal if their base addresses are the same and their
  vme interfaces are equal.  
*/
int 
CSIS3300::operator==(const CSIS3300& rhs) const
{
  return (CVMEReadableObject::operator==(rhs)      &&
	  (m_baseAddress == rhs.m_baseAddress));

}
/*!
   Modules are not equal if equality test fails:
*/
int
CSIS3300::operator!=(const CSIS3300& rhs) const
{
  return !(*this == rhs);
}


////////////////////////////////////////////////////////////////////
///////////////////// Configuration management /////////////////////
////////////////////////////////////////////////////////////////////
/*!
   Configure the trigger output to be enabled or not.
 The triggers is set when an input signal makes its threshold condition.
 one typical use of this is to start or stop digitization based on this signal.
  \param enabled : bool
     - true to enable the output 
     - false to disable the output.
*/
void
CSIS3300::configTriggerOutput(bool enabled)
{
  m_triggerOutputEnabled = enabled;
}

/*!
    Return a bool indicating if the trigger output is enabled.
    \return bool
    \retval true Trigger output is enabled.
    \retbal false Trigger output is disabled.
*/
bool
CSIS3300::cgetTriggerOutputIsEnabled() const
{
  return m_triggerOutputEnabled;
}
/*!
   Set the trigger outupt to be inverted. that is active only if the
   trigger condition is not made.
  \param inverted : bool
  - true  - invert output.
  - false - don't invert output.
*/
void
CSIS3300::configTriggerOutputInverted(bool invert) 
{
  m_triggerOutputInverted = invert;
}
/*!
   Return the trigger output inversion config param
   \return bool
   \retval true - trigger output will be inverted.
   \retval false - Trigger output will not be inverted. 

   \note This is qualified by the trigger output enable and
        trigger active when armed states.
   \note initialize() must be called to actually program the hardware
         to the configuration.
*/
bool
CSIS3300::cgetTriggerOutputIsInverted() const
{
  return m_triggerOutputInverted;
}

/*!
  Configure the trigger output to only work when the module is armed.
  This is used in conjunction with configTriggerOutput to determine when
  the trigger output can fire.
  \param activeWhenArmed : bool
   - true to only activate the trigger output when the module is armed.
   - false to allow trigger outputs even if the module is idling.
*/
void
CSIS3300::configTriggerActiveWhenArmed(bool activeWhenArmed)
{
  m_triggerActiveWhenArmed = activeWhenArmed;
}
/*!
   Return the configuration state last set by configTriggerActiveWhenArmed.
   Note that the module will only be programmed to the configuration by a call 
   to initialize()
   \return bool
   \retval true - Trigger is only active when the module is  armed.
   \retval false - Trigger output can be active any time the trigger condition is
                   met.
*/
bool
CSIS3300::cgetTriggerIsActiveWhenArmed() const
{
  return m_triggerActiveWhenArmed;
}
/*!
    Configure which outputs will show the bank full condition.
    \param mask : uint32_t
       Mask of outputs.  can be any bit pattern in 0-7, each bit
       represents and output 1,2, or 3  Set bits indicate that output
       will reflect the bank full state transition.
*/
void
CSIS3300::configBankFullOutputMask(uint32_t mask)
{
  m_bankFullOutputMask =  mask;
}
/*!
   Retrive the mask of outputs that will reflect the bank full state.
*/
uint32_t
CSIS3300::cgetBankFullOutputMask() const
{
  return m_bankFullOutputMask;
}
/*!
   Configure the bank 1 clock.
   \param enable : bool
      - true adc clock will be routed to the bank 1 memory.
      - false adc clock will not be routed to the bank 1 memory.
*/
void
CSIS3300::configEnableBank1Clock(bool enabled)
{
  m_bank1ClockEnable = enabled;
}
/*!
    \return bool
    \retval true - Clock is enabled to bank 1.
    \retval false - Clock is not enabled to bank 1
*/
bool
CSIS3300::cgetBank1ClockIsEnabled() const
{
  return m_bank1ClockEnable;
}
/*!
   Configure the bank 2 clock.
   \param enable : bool
   - true clock is routed to bank 2
   - false clock is not routed to bank 2
*/
void
CSIS3300::configEnableBank2Clock( bool enabled)
{
  m_bank2ClockEnable = enabled;
}
/*!
   \return bool
   \retval true clock is enabled to bank 2
   \retaval false clock is not enabled to bank 2.
*/
bool
CSIS3300::cgetBank2ClockIsEnabled() const
{
  return m_bank2ClockEnable;
}
/*!
   Enable an auto switch of which bank of memory has the clock.
   The idea is that to double buffer the module you would e.g.
   Enable bank1 clock, enable autoswitch, then as bank 1 fills you read it out
   as bank2 fills etc.
   \param enabled : bool
   - true  Auto switch is enabled.
   - false Auto switch is disabled.
*/
void
CSIS3300::configAutoBankSwitchEnabled(bool enabled)
{
  m_autoBankSwitch = enabled;
}
/*!
   \return bool
   \retval true - The auto bank switch feature is configured on.
   \retval false - The auto bank switch feature is configured off.
*/
bool
CSIS3300::cgetAutoBankSwitchIsEnabled() const
{
  return m_autoBankSwitch;
}
/*!
  Enable the random clock capability.  Some applications use the
  FADC not to take a time series of a signal, but to clock a multiplexed
  signal into the module (See HiRA e.g.).  In this case the external clock
  represents a request to sample which is honored on the next internal Clock 
  cycle of the module.  But the external clock need be neither symmetric,
  nor even regular.
  \param enabled : bool
   - true  Random clocking is enabled.
   - false Random clocking is disabled.
*/
void
CSIS3300::configRandomClockEnable(bool enabled)
{
  m_randomClockMode = enabled;
}
/*!
   \return bool
   \retval true - Random clock mode is enabled.
   \retval false - Random clock mode is disabled.
*/
bool
CSIS3300::cgetRandomClockIsEnabled() const
{
  return m_randomClockMode;
}
/*!
   Configure the auto start. 
   \param enabled : bool
   - true Auto start enabled.
   - false Auto start disbled.
*/
void
CSIS3300::configAutoStartEnable(bool enabled)
{
  m_autoStart = enabled;
}
/*!
   \return bool
   \retval true - Auto start is enabled.
   \retval false - Auto start is disabled.
*/
bool
CSIS3300::cgetAutoStartIsEnabled() const
{
  return m_autoStart;
}

/*!
  Configure in multi-event mode...where the memory is divided into event
  slabs, several events can be taken and then read out at once.
  \param enable : bool
     -true Multievent mode enabled.
     -false Multievent mode disabled.
*/
void
CSIS3300::configMultiEventMode(bool enable)
{
  m_multiEventMode = enable;
}
/*!
   \return bool
   \retval true Multi event mode  is enabled.
   \retval fals Multi event mode is disabled.
*/
bool
CSIS3300::cgetIsMultiEventMode() const
{
  return m_multiEventMode;
}
/*!
  Enable/disable the start delay.  See also
  configStartDelay.  It is  possible to delay the start of conversions
  for a specified time in terms of clock ticks relative to the actual
  start trigger.  This enables that delay.
  \param enable : bool
   - true - Enable the delay
   - false - disable the delay.
*/
void
CSIS3300::configEnableStartDelay(bool enable)
{
  m_enableStartDelay = enable;
}
/*!
   \return bool 
   \retval true Start delay is enabled.
   \retval false Start delay is disabled.
*/
bool
CSIS3300::cgetStartDelayIsEnabled() const
{
  return m_enableStartDelay;
}

/*!
    Enable/disable the stop delay.  stop delay is especially useful
    for looking at wave forms prior to the desired start time.
    The idea is that you free run the ADC, then use the trigger as a stop
    that is delayed to ensure you get the time bite you want.
    \param enabled : bool
    - true stop delay is enabled.
    - false stop delay is disabled.
*/
void
CSIS3300::configEnableStopDelay(bool enable)
{
  m_enableStopDelay = enable;
}
/*!
   \return bool
   \retval true - Stop delay is enabled.
   \retval false - Stop delay is disabled.
*/
bool
CSIS3300::cgetStopDelayIsEnabled() const
{
  return m_enableStopDelay;
}
/*!
  Enable the front panel start/stop input.
  \param enable : bool
  - true front panel is enabled to provide start or stop.
  - false front panel does not provide start or stop.
*/
void
CSIS3300::configEnableFrontPanelStartStop(bool enable)
{
  m_enableFPStartStop = enable;
}
/*!
    \return bool
    \retval true - Front panel start/stop is enabled.
    \retval false - Front panel start/stop is disabled.
*/
bool
CSIS3300::cgetFrontPanelStartStopIsEnabled() const
{
  return m_enableFPStartStop;
}
/*!
   Enable front panel gate mode.  In this case the 'start' input
   actually describes the time bite over which the conversion takes place.
   Conversions are remembered over the interval between the falling and rising
   edge of the gate.
   \param enable :bool
    - true   - Gate mode enabled.
    - false  - Gate mode disabled.

*/
void
CSIS3300::configEnableFPGateMode(bool enable)
{
  m_enableFPGateMode = enable;
}
/*!
    \return bool
    \retval true - front panel gate mode is enabled.
    \retval false - front panel gate mode is disabled.
*/
bool
CSIS3300::cgetFPGateModeIsEnabled() const
{
  return m_enableFPGateMode;
}

/*!
    Configure the external clock for random mode.  Seems redundant with configuring
    a random clock but it is a separate setting in the module so?...
    \param on : bool
        - true  Enable external random clock
        - false Disable external random clock.
*/
void 
CSIS3300::configExternRandomClockMode(bool on)
{
  m_externRandomClockMode = on;
}
/*!
    \return bool
    \retval true - external random clock mode is enabled.
    \retval false - external random clock mode is disabled.
*/
bool
CSIS3300::cgetIsExternRandomClockMode() const
{
  return  m_externRandomClockMode;
}
/*!
   Configure the clock source.
   \param source : CSIS3300::ClockSource
      - external Clock comes from an external input.
      - internal3pt125KHz, CLock is internal at 3.125khz.
      - internal6pt250KHz  Clock is internal at 6.25 khz.
      - internal12500KHz,  Clock is internal 12.5Mhz (12500Khz).
      - internal20_25MHz,  20Mhz or 25 Mhz depending on module clock.
      - internal40_50MHz,  40Mhz or 50Mhz depending on module clock.
      - internal80_100Mhz  80Mhz or 100Mhz depending on module clock.
*/
void
CSIS3300::configClockSource(CSIS3300::ClockSource source)
{
  m_clockSource = source;
}
/*
   \return CSIS3300::ClockSource
   \retval the clock source see configClockSource for possible values.
*/
CSIS3300::ClockSource
CSIS3300::cgetClockSource() const
{
  return m_clockSource;
}
/*!
   Set the module in or out of multiplex mode.
    \param enabled : bool
       - true multiplexed mode.
       - false not multiplexed mode.
*/
void
CSIS3300::configMultiplexMode( bool enabled)
{
  m_enableMuxMode = enabled;
}
/*!
   \return bool
   \retval true  -Multiplex mode enabled.
   \retval false - Multiplex mode disabled.
*/
bool
CSIS3300::cgetMultiplexMode() const
{
  return m_enableMuxMode;
}
/*!
   Configure the start delay... the number of ticks between a start pulse
   and the actual start.
   \param delay : uint16_t
      Number of ticks between start pulse and module start.
*/
void
CSIS3300::configStartDelay(uint16_t delay)
{
  m_startDelay = delay;
}
/*!
   Get the start delay.
   \return uint16_t 
   \retval The start delay in module clock ticks.

*/
uint16_t 
CSIS3300::cgetStartDelay() const
{
  return m_startDelay;
}
/*!
   Set the module stop delay.
   \param delay : uint16_t
      Number of module clock ticks between the stop input and the actual
      stop.
*/
void
CSIS3300::configStopDelay(uint16_t delay)
{
  m_stopDelay = delay;
}
/*
    \return uint16_t
    \retval delay between module stop pulse and actual stop.
*/
uint16_t
CSIS3300::cgetStopDelay() const
{
  return m_stopDelay;
}

  
/*!
    Configure the time stamp pre-deivider. Each event has a timestamp
    associated with it.  The timestamp is essentially a module clock count, 
    however this count can be divided by the timestamp pre-divider set by this
    function.
    \param divisor : uint16_t
      The divisor for the timestamp
*/
void
CSIS3300::configTimestampPredivider(uint16_t divisor)
{
  m_timestampPredivider = divisor;
}
/*!
   \return uint16_t
   \retval Predivisor of the timestamp.
*/
uint16_t
CSIS3300::cgetTimestampPredivider() const
{
  return m_timestampPredivider;
}
/*!
   Configure the pagesize of the SIS3300 memory  The page size
   is effectively the size of a single event.
   \param size  : CSIS3300::PageSize
   - page128 page is 128 samples
   - page256 page is 256 samples.
   - page512 page is 512 samples.
   - page1K Page is 1024 samples.
   - page2K page is 2*1024 samples.
   - page4K page is 4*1024 samples.
   - page16K page is 16*1024 samples.
   - page128K page is 128*1024 samples.
*/
void
CSIS3300::configPageSize(CSIS3300::PageSize size)
{
  m_pageSize =  size;
}
/*!
   \return CSIS3300::PageSize
   \retval pagesize selected See configPageSize for information about the
           possible return values.
*/
CSIS3300::PageSize
CSIS3300::cgetPageSize() const
{
  return m_pageSize;
}
/*!
    Turn on or off wrap at end of page.  Typically this is used in free running
    stop modes, when you want acquisition to be confined to a specific page of
    the event memory... and then with autostart, gate chaining to force the
    next event to the next page.
    \param enable : bool
      - true wrap is on.
      - false wrap is off.
*/
void
CSIS3300::configWrapEnable(bool enable)
{
  m_wrap = enable;
}
/*!
    \return bool 
    \retval true - wrap is enabled.
    \retval false - wrap is disabled.
*/
bool
CSIS3300::cgetWrapIsEnabled() const
{
  return m_wrap;
}
/*!
   Turn on or off gate chaining.
   \param enable : bool
    - true gate chaining is enabled.
    - valse gate chaining is disabled.
*/
void
CSIS3300::configGateChaining(bool enabled)
{
  m_gateChaining = enabled;
}
/*!
   \return bool
   \retval true - gate chaning is enabled.
   \retval false - gate chaining is disabled.
*/
bool
CSIS3300::cgetIsGateChaining() const
{
  return m_gateChaining;
}
/*!
   Set the threshold and direction of the threshold for a channel.
   \param channel : unsigned
     Number of the channel to set.  Must be in the range 0-7.
   \param value : CSIS3300::Threshold
      A threshold structure which has the following elements:
      - s_value : uint16_t - The threshold value.
      - s_le    : bool - true if threshold is made when the signal is
                          .LE. the threshold value false if the threshold
			  is made when the signal is .GT. the value.

    \throw CRangeError - if the channel number is invalid.
*/
void
CSIS3300::configChannelThreshold(unsigned channel, CSIS3300::Threshold value)
{
  if (channel < 8) {
    m_thresholdInfo[channel] = value;
  }
  else {
    throw CRangeError(0, 7, channel,
		      "CSIS3300::configChannelThreshold, channel value");
  }
}
/*!
   Get the threshold info for one channel.
   
*/
CSIS3300::Threshold
CSIS3300::cgetChannelThreshold(unsigned channel)  const
{
  if (channel < 8) {
    return m_thresholdInfo[channel];
  } 
  else {
    throw CRangeError(0, 7, channel,
		      "CSIS3300::cgetChannelThreshold, channel value");
  }
}
/*!
    Configure the clock predivider. Together with the clock source the
    this determines the actual conversion rate of the ADC.
    \param divisor : uint8_t
      The scaledown for the clock.
*/
void
CSIS3300::configClockPredivider(uint8_t divisor)
{
  m_clockPredivider = divisor;
}
uint8_t
CSIS3300::cgetClockPredivider() const
{
  return m_clockPredivider;
}

/*!
   Configure the number of samples in a multiplexor daq.
   \param samples : uint8_t 
      Number of samples.

*/
void
CSIS3300::configMuxModeSamples(uint8_t samples)
{
  m_muxModeSamples = samples;

}
uint8_t
CSIS3300::cgetMuxModeSamples() const
{
  return m_muxModeSamples;
}

/*!
    Configure the CFD fraction numerator.
    \param numerator : uint8_t
     New numerator value.
*/
void
CSIS3300::configCFDNumerator(uint8_t numerator)
{
  m_cfdNumerator = numerator;
}
uint8_t
CSIS3300::cgetCFDNumerator() const
{
  return m_cfdNumerator;
}
/*!
  Configure the CFD fraction denominator.
  \param denominator : uint8_t
     New demoniator value.
*/
void
CSIS3300::configCFDDenominator(uint8_t denominator)
{
  m_cfdDenominator = denominator;
}
uint8_t
CSIS3300::cgetCFDDenominator() const
{
  return m_cfdDenominator;
}
/*!
   Configure the CFD output width value.
   \param width : uint8_t
*/
void
CSIS3300::configCFDWidth(uint8_t width)
{
  m_cfdWidth = width;
}
uint8_t
CSIS3300::cgetCFDWidth() const
{
  return m_cfdWidth;
}
/*!
   Enable or disble the cfd threshold.
*/
void
CSIS3300::configCFDEnable(bool enable) 
{
  m_cfdEnabled = enable;
}
bool
CSIS3300::cgetCFDIsEnabled() const
{
  return m_cfdEnabled;
}

/*!
    Configure the maximum number of events in a gate chain.
    \param maxEvents : uint16_t
      Max events in a chain.
*/
void
CSIS3300::configChainMaxEvents(uint16_t maxEvents)
{
  m_chainMaxEvents = maxEvents;
}
uint16_t
CSIS3300::cgetChainMaxEvents() const
{
  return m_chainMaxEvents;
}
/*!
   Configure the gropu enables mask
   \param mask :uint8_t 
      bitmask with one bit per channel group indicating which channels will be read out.
      There is actually no way to disable conversions in a channel and, in fact,
      no reason to do so as each channel as an FADC chip.
*/
void
CSIS3300::configGroupEnables(uint8_t mask)
{
  m_groupReadMask = mask;
}
uint8_t
CSIS3300::cgetGroupEnables() const
{
  return m_groupReadMask;
}

/*!
    Enable a tag to appear at the beginning of each group.
    The tag will consist of a count of the 16 bit words (conversions*2)
    followed by the group number.  The count is 32 bit long and the
    tag a 16 bit word.
    \param tagThem : bool
    - true add the tag.
    - false don't add the tag.
*/
void
CSIS3300::configTagGroups(bool tagThem)
{
  m_tagGroups = tagThem;
}
bool
CSIS3300::cgetTagGroups() const
{
  return m_tagGroups;
}
/*!
    Configure whether or not the entire FADC packet will be tagged.
    If so the tag will consist of a 32 bit lonword word count
    followed by a 16 bit tag id word.
    \param tagThem : bool
    - true Tag them with the id in the tagId parameter.
    - false no tagging done.
    \param tagId : uint16_t
      Id of the tag to apply to the packet.
*/
void
CSIS3300::configDataTag(bool tagThem, uint16_t tagId)
{
  m_tagData = tagThem;
  m_dataTag = tagId;
}
/*!
   \return bool
   \retval true The entire packet will be tagged.
   \retval false the entire packet will not be tagged.
*/
bool
CSIS3300::cgetTaggingData() const
{
  return m_tagData;
}
/*!
   \return uint16_t
   \retval tag of the entire data packet if data tagging is enabled.
          meaningless otherwise.
*/
uint16_t
CSIS3300::cgetDataTag() const
{
  return m_dataTag;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////// Direct device access ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*!
   Return the value of the firmware/id register.
*/
uint32_t
CSIS3300::readFirmware()
{
  CVMEAddressRange& regs(getControlRegisters());
  return regs.peekl(IDFirmware/sizeof(uint32_t));
}

/*!
   Reset the module.
*/
void
CSIS3300::reset()
{
  key(KeyReset);
}
/*!
   Programmatically start sampling
*/
void
CSIS3300::start()
{
  key(StartSampling);
}
/*!
   Programmatically stop sampling.
*/

void 
CSIS3300::stop()
{
  key(StopSampling);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Data taking phase functions ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  This is a monster :-(, first we check that the device is 
  a valid one (via readFirmware and extracting the model number).
  Then we reset it.
  And the monster part -- process the configuration variables turning them into
  module programming finally, we arm the device.
*/
void 
CSIS3300::initialize()
{
  reset();

  uint32_t fwid = readFirmware();
  uint32_t model= (fwid & ID_moduleIdMASK) >> ID_moduleIdSHIFT;

  if ((model != ID_3300) && (model != ID_3301)) {
    // Not a compatible module:
    
    char msg[100];
    sprintf(msg, "Module id for base address 0x%08x is not a 3300/1: Got 0x%x(16)",
	    m_baseAddress, model);
    throw CInvalidInterfaceType(string(msg), 
				"CSIS3300::initialize() - checking module type");
  }
  // Start the monster part:
  // we work our way from back to front of the configuration as
  // I think this is the best way to ensure that we get the programming
  // order right.

  CVMEAddressRange& csrs(getControlRegisters());
  CVMEAddressRange& all(getControlRegisters());
  
  writeReg(all, MaxEventCountAll, m_chainMaxEvents);
  
  // build/write the CFD control register:

  uint32_t tsetupRegister = 0;
  if (m_cfdEnabled) {
    tsetupRegister   |= TSR_CFDEnable;
    tsetupRegister   |= (m_cfdNumerator << TSR_NumeratorSHIFT) & TSR_NumeratorMASK;
    tsetupRegister   |= (m_cfdDenominator << TSR_DenominatorSHIFT) & TSR_DenominatorMASK;
  }
  writeReg(all, TriggerSetupAll , tsetupRegister);

  writeReg(all, NumSamplesAll, m_muxModeSamples); // multiplexor sample count.
  writeReg(all, ClockPredividerAll, m_clockPredivider);

  // Thresholds are done for each group:

  int group = 1;

  for (int i=0; i < 4; i += 2, group++) {
    CVMEAddressRange& groupreg(getGroupRegisters(group));

    uint32_t oddth   = m_thresholdInfo[i].s_value; // Numbered from 1 in sis.
    uint32_t eventh  = m_thresholdInfo[i+1].s_value;
    bool     oddle   = m_thresholdInfo[i].s_le;
    bool     evenle  = m_thresholdInfo[i].s_le;

    uint32_t rvalue  = 0;
    rvalue |= (eventh << THR_ADCEvenThresholdSHIFT) & THR_ADCEvenThresholdMASK;
    rvalue |= (oddth  << THR_ADCOddThresholdSHIFT)  & THR_ADCOddThresholdMASK;
    if (evenle) rvalue |= THR_ADCEvenLT;
    if (oddle)  rvalue |= THR_ADCOddLT;

    writeReg(groupreg, TriggerThresholdGroup, rvalue);
    
  }
  // Set the Event configuration register (common one).

  uint32_t eventConfigReg = 0;
  eventConfigReg  = (m_pageSize << ECR_PageSizeSHIFT) & ECR_PageSizeMASK; 
  if (m_wrap)         eventConfigReg |= ECR_AutoWrapMASK;
  if (m_gateChaining) eventConfigReg |= ECR_GateChainMASK;
  if (m_externRandomClockMode) eventConfigReg |= ECR_ExternalRCMMASK;
  if (m_enableMuxMode)eventConfigReg |= ECR_MuxModeMASK;
  writeReg(all, EventConfigAll, eventConfigReg);
			

  // Now a bunch of numeric registers:

  writeReg(csrs, TimeStampDivider,  m_timestampPredivider);
  writeReg(csrs, StopDelay,         m_stopDelay);
  writeReg(csrs, StartDelay,        m_startDelay);
  
  // Compute and set the acquisition control register (AcqControl).
  // This register is a bunch of bitset/bitclears.
  // We're going to clear all the bits and then set the ones that we want.

  writeReg(csrs, AcqControl,        (uint32_t)0xffff0000);
  uint32_t acq  = 0;
  if (m_bank1ClockEnable)   acq |= ACTL_BSSample1;
  if (m_bank2ClockEnable)   acq |= ACTL_BSSample2;
  if (m_autoBankSwitch)     acq |= ACTL_BSAutoBankSwitch;
  if (m_multiEventMode)     acq |= ACTL_BSMultiEvent;
  if (m_enableStartDelay)   acq |= ACTL_BSStartDelay;
  if (m_enableStopDelay)    acq |= ACTL_BSStopDelay;
  if (m_enableFPStartStop)  acq |= ACTL_BSFPStartStop;
  if (m_enableFPGateMode)   acq |= ACTL_BSFPGateMode;
  if (m_randomClockMode)    acq |= ACTL_BSFPRandomClock;
  acq |= (m_clockSource << ACTL_BSClockSrcSHIFT) & ACTL_BSClockSrcMASK;
  
  writeReg(csrs, AcqControl, acq);

  // Interrupts off/disabled:

  writeReg(csrs, InterruptControl, 0);
  writeReg(csrs, InterruptConfig, 0);

  // Compute the csr and set it up... This is also a bit set/clear register
  // and therefore is done by clearing everything and then setting what we need.
  //
  
  writeReg(csrs, ControlStatus, 0xffff0000);
  uint32_t csrreg = CSR_BSTrigOnArmedAndStarted | CSR_BSInternalTriggerRouting;
  if (m_triggerOutputEnabled)    csrreg  |= CSR_BSEnableTriggerOut;
  if (m_triggerOutputInverted)  csrreg  |= CSR_BSInvertTriggerOut;
  if (m_bankFullOutputMask & 1) csrreg  |= CSR_BSBankFullOutput1;
  if (m_bankFullOutputMask & 2) csrreg  |= CSR_BSBankFullOutput2;
  if (m_bankFullOutputMask & 4) csrreg  |= CSR_BSBankFullOutput3;


  writeReg(csrs, ControlStatus, csrreg);
 
  m_currentBank = 0;

}

/*!
   Read an event from the FADC.  How this is done is dependent on the setup.
   See the comments in the following code for information about the major readout
   modes.  Note that each major readout mode may have different submodes with
   additional special needs.
*/

size_t 
CSIS3300::read(void* buffer)
{
  uint16_t* p(static_cast<uint16_t*>(buffer));
  size_t readSize(0);
  if (m_tagData) {
    p         += 2;		// packet size.
    *p++       = m_dataTag;	// Packet tag.
    readSize  += 3;		// Words in header.
  }
  
  // Simple single event read:

  if (!m_multiEventMode) {
    readSize += readSingle(p);		// Read event 0 from bank 0.
  }
  else if (!m_autoBankSwitch) {	// Multi events, but only from bank 0.
    // Assumption is that the max # events is the # events we have:

    readSize += readMulti(p, 0, m_chainMaxEvents-1);
  }
  else {			// Multi block:
    readSize +=  readMultiBuffered(p, m_currentBank,
			     0, m_chainMaxEvents-1);
    m_currentBank = (m_currentBank + 1) % 2; // Swap banks. (0->1, 1->0).
  }
  if (m_tagData) {
    uint32_t* pCount(static_cast<uint32_t*>(buffer));
    *pCount = readSize;
  }

  return readSize;
}


/*!
  Return the maximum number of bytes an event can return.  This is probably
  the only piece of hardware I support hat will have to figure it out based
  on the configuration.. since the range is so broad...and we don't want
  the caller to think we always will be 128Ksamples*8channels.

  We consider the following factors:
  - Number of groups that will be read.
  - Page size.
  - Enveloping.
  \return size_t
  \retval The maximum number of bytes of data that could be returned by
          a read() call.

*/
size_t 
CSIS3300::largestEvent()
{
  size_t samples = pageSizeToSampleCount(); // Samples per group.
  samples       *= sizeof(uint32_t);        // Each sample is 32 bits.

  // Figure out how many groups are read.

  uint32_t groupsRead = 0;
  for (int i =0; i < 4; i++) {
    if (m_groupReadMask & (1 << i)) {
      groupsRead++;
    }
  }
  samples = samples*groupsRead;	           // Total samples read.
  size_t envelopeSize = 0;
  if (m_dataTag) {		// Entire event encapsulated.
    envelopeSize +=   sizeof(uint32_t) + sizeof(uint16_t);
  }

  if (m_tagGroups) {
    envelopeSize += groupsRead*(sizeof(uint32_t) + sizeof(uint16_t));
  }
  return samples + envelopeSize;
}




/*!
  Not really sure how to implement this as it is highly 
  dependent on the way the SIS3300 is setup and may require
  conditionals that just are not supported in most list processors.
*/
CVMEList& 
CSIS3300::addReadoutToList(CVMEList& list)
{
  return list;
}


///////////////////////////////////////////////////////////////////////////////
/////////////////////// Additional read functions /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

size_t 
CSIS3300::readSingle(void* buffer, uint32_t event)
{
  CVMEAddressRange& group(getCommonRegisters());
  uint32_t eventDir  = eventDirOffset();
  uint32_t dirEntry  = readReg(group, eventDir + event*sizeof(uint32_t));
  uint32_t endSample = ((dirEntry - 1) & 0x1ffff);

  // Now the start sample.  If there was no wrap, it's just 0.  If there was a wrap
  // it's given by dirEntry.

  uint32_t startSample(0);	// Assume no wrap...
  if (dirEntry & 0x80000) {
    startSample = dirEntry & 0x1ffff; // It's here if there was a wrap.
  }
  // The wrap sample is given by the pagesize:

  uint32_t wrapSample = pageSizeToSampleCount();

  // convert all this sample nonsense to offsets from the appropriate bank group.

  uint32_t startOffset = sampleToOffset(startSample, event);
  uint32_t endOffset   = sampleToOffset(endSample,   event);
  uint32_t wrapOffset  = sampleToOffset(wrapSample,  event);

  // Transfer the data using readGroup which knows how/if to encapsulate the
  // group in a group packet...we only read groups that have the
  // corresponding group enables bit set.


  return readSelectedGroups(buffer, startOffset, endOffset, wrapOffset);

}
/*!
   Read several events from the current bank  memory.
   \param buffer  : void*
      target of the read.
   \param firstEvent : uint32_t 
       The first event number.
   \param lastEvent : uint32_t
       Index of the last event number.

   \return size_t
   \retval number of bytes read.

   Requires:
   - All the events specified have been completely read.
   - lastEvent is after firstEvent.
      
*/
size_t 
CSIS3300::readMulti(void* buffer, uint32_t firstEvent, uint32_t lastEvent)
{
 
  CVMEAddressRange& common(getCommonRegisters());
  uint32_t          dir = eventDirOffset();



  // The event directory, and event size tell me where the first event
  // starts, and the last event ends. The wrap offset we set so that
  // no wrapping occurs.
  
  uint32_t startOffset = readReg(common, dir + firstEvent*sizeof(uint32_t)) -
                         pageSizeToSampleCount();
  uint32_t endOffset   = readReg(common, dir + lastEvent*sizeof(uint32_t));
  uint32_t wrapOffset  = endOffset+sizeof(uint32_t);
  
  // presumably the caller will know the event size and will know how
  // to deal with the soup of events we hand him/her.

  return readSelectedGroups(buffer, startOffset, endOffset, wrapOffset);

}
/*!
   Reads several events from the selected bank of memory.
   \param buffer : void*
      Pointer to the target buffer.
   \param block : uint32_t 
     block selector (0, or 1).
   \param firstEvent : uint32_t 
       First event number.
   \param lastEvent  : uint32_t
      Last event number.
   \return size_t
   \retval number of bytes read.

*/
size_t 
CSIS3300::readMultiBuffered(void* buffer, int block,
			    uint32_t firstEvent, uint32_t lastEvent)
{
  // A bit tricky.

  uint32_t currentBank = m_currentBank;
  m_currentBank        = block;
  size_t   size        = readMulti(buffer, firstEvent, lastEvent);
  m_currentBank        = currentBank;

  return size;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////// Utilities  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Set the default values for all the data members in the configuration:
// See the comments below for more information.  

void
CSIS3300::setDefaults() 
{
  m_triggerOutputEnabled   = true;
  m_triggerOutputInverted  = false;
  m_triggerActiveWhenArmed = true;
  m_bankFullOutputMask     = 0;
  m_bank1ClockEnable       = true;

  m_bank2ClockEnable       = false;
  m_autoBankSwitch         = false;
  m_randomClockMode        = false;
  m_autoStart              = false;
  m_multiEventMode         = false;

  m_enableStartDelay       = false;
  m_enableStopDelay        = false;
  m_enableFPStartStop      = true;
  m_enableFPGateMode       = false;
  m_externRandomClockMode  = false;

  m_clockSource            = internal80_100Mhz;
  m_enableMuxMode          = false;
  m_startDelay             = 0;
  m_stopDelay              = 0;
  m_timestampPredivider    = 0;

  m_pageSize               = page1K;
  m_wrap                   = false;
  m_gateChaining           = false;

  // All thresholds are set so that the trigger won't ever fire.

  for (int i = 0; i < 8; i++) {	
    m_thresholdInfo[i].s_value = 0x3fff; 
    m_thresholdInfo[i].s_le    = false;
  }
  
  m_clockPredivider        = 0;
  m_muxModeSamples         = 0;
  m_cfdEnabled             = false;
  m_cfdNumerator           = 0;
  m_cfdDenominator         = 0;

  m_cfdWidth               = 0;
  m_chainMaxEvents         = 0;
  m_groupReadMask          = 0x0f;
  m_tagGroups              = true;
  m_tagData                = false;

  m_dataTag                = 0x1234;

}
// Utility used in copy construction and assignment to copy the configuration
// of a rhs element to *this.
//

void
CSIS3300::copyIn(const CSIS3300& rhs)
{
  // Clear the register cache.

  delete m_pRegisters;
  delete m_pCommonRegisters;
  for (int i =0; i < 4; i++) {
    delete m_ppGroupRegisters[i];
  }
  m_baseAddress = rhs.m_baseAddress;

  // This is a bit of a cheat.  All of the copyable configuration information
  // is located in memory between m_IMUSTBEFIRST and m_IMUSTBELAST.
  // just copy that block of memory to our vars:


  memcpy(&m_IMUSTBEFIRST, 
	 &(rhs.m_IMUSTBEFIRST),
	 (&m_IMUSTBELAST) - (&m_IMUSTBEFIRST));


}

// get various address ranges; The address ranges are cached so that there's
// only any overhead getting them the first time.
// 

// Get the first (control register address page)

CVMEAddressRange&
CSIS3300::getControlRegisters()
{
  if (!m_pRegisters) {
    CVMEInterface& Interface(getInterface());
    m_pRegisters  = Interface.createAddressRange(RegisterAmod,
						 m_baseAddress,
						 CSRPageSize);
  }
  return *m_pRegisters;
}

// Get the common control registers for all banks:

CVMEAddressRange&
CSIS3300::getCommonRegisters()
{
  if (!m_pCommonRegisters) {
    CVMEInterface& interface(getInterface());
    m_pCommonRegisters = interface.createAddressRange(RegisterAmod,
						       m_baseAddress + AllOffset,
						       AllPageSize);
  }
  return *m_pCommonRegisters;
}
//  Get group registers for an adc group... The caller must validate
//  the group argument which is 1,2,3,4
//
CVMEAddressRange&
CSIS3300::getGroupRegisters(uint8_t group)
{
  group--;
  if (!m_ppGroupRegisters[group]) {
    uint32_t offset = GroupRegisterOffsets[group];
    CVMEInterface& interface(getInterface());
    m_ppGroupRegisters[group] = interface.createAddressRange(RegisterAmod,
							     m_baseAddress + offset,
							     ADCPageSize);
  }
  return *(m_ppGroupRegisters[group]);
}
//  Write 0 to a register (e.g. for key functions).

void
CSIS3300::key(uint32_t reg)
{
  CVMEAddressRange& csr(getControlRegisters());
  csr.pokel(reg/sizeof(uint32_t), 0);
}

/*  Write a reg.
 */
void CSIS3300::writeReg(CVMEAddressRange& page, uint32_t reg, uint32_t value)
{
  page.pokel(reg/sizeof(uint32_t), (long)value);
}
/* Read a reg.
 */
uint32_t
CSIS3300::readReg(CVMEAddressRange& page, uint32_t reg)
{
  return page.peekl(reg/sizeof(uint32_t));
}

/*
   Returns the samples that are in the current page size in 
   m_pageSize
*/
size_t
CSIS3300::pageSizeToSampleCount() const
{
  return pageToSize[m_pageSize];
}
/*
   From a sample and event number, return the offset into a 
   block of group sample memory.  This must be added to the base address
   of the group in a specific block.
   Parameters:
     uint32_t sample   - The sample number.
     uint32_t event    - The event number.
  Returns:
    The offset into the block of memory.
*/
size_t 
CSIS3300::sampleToOffset(uint32_t sample, uint32_t event)
{
  size_t eventBase = event*pageSizeToSampleCount()*sizeof(uint32_t);
  return (eventBase + sample*sizeof(uint32_t));
}
/*
  Read the selected groups with readGroup.
  Parameters:
    void*    p           - Destination of the read.
    uint32_t startOffset - Start offset of the event.
    uint32_t endOffset   - End offset of the event.
    uinte32_t wrapOffset - Where wrapping should occur if anywhere.

*/
size_t
CSIS3300::readSelectedGroups(void* buffer, uint32_t startOffset, uint32_t endOffset,
		     uint32_t wrapOffset)
{
  uint8_t* p(static_cast<uint8_t*>(buffer));
  size_t   totalCount(0);

  for (int i =0; i < 3; i++) {
    if (m_groupReadMask & (1 << i)) {

      size_t thisRead = readGroup(p, m_currentBank, 
				  i+1, startOffset, endOffset, wrapOffset);
      totalCount += thisRead;
      p          += thisRead;
    }
  }
  return totalCount;
  
}
/*
   Read a sample group.
   Parameters:
     buffer      - pointer to the memory into which the sample will be read.
     block       - Block number to read. (0/1).
     group       - Which group to read (1-4) 
     startOffset - Offset to the start of the event in the event memory buffer.
     endOffset   - Offset to the end of the event in the event memory buffer.
     wrapOffset  - Offset that defines where the memory wraps.

   Returns:
      Number of bytes read.

There are two cases: startOffset <= endOffset - just read in one fell swoop.
                      startOffset>  endOffset - Read from start -> wrap and then
                                                from 0 to wrap.



*/
size_t
CSIS3300::readGroup(void* buffer, int block, int group, 
		    uint32_t startOffset, uint32_t endOffset, uint32_t wrapOffset)
{
  size_t transfers = 0;
  
  if (startOffset <= endOffset) { // single read...
    uint32_t groupOffset = bankOffsets[block][group-1];
    uint32_t startAddress = startOffset + groupOffset + m_baseAddress;
    uint32_t endAddress   = endOffset   + groupOffset + m_baseAddress;
    CVmeDMATransfer* pDma = getInterface().createDMATransfer(BlockAmod,
							      CVMEInterface::TW_32,
							      startAddress,
							      endAddress - startAddress);
    transfers += pDma->Read(buffer);
    delete pDma;
  }
  else {
    size_t this1 = 0;
    char* p(static_cast<char*>(buffer));
    this1       += readGroup(p, block, group, startOffset, wrapOffset, wrapOffset);
    p           += this1;
    transfers   += this1;
    this1       = readGroup(p, block, group, 0, endOffset, wrapOffset);
    transfers  += this1;
  }


  return transfers;
}
/*
  Returns the event directory offset for the block that is current.
*/
uint32_t
CSIS3300::eventDirOffset() const
{
  if(m_currentBank == 0) {
    return EventDir1All;
  }
  else {
    return EventDir2All;
  }
}
