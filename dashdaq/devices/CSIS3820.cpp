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
#include "CSIS3820.h"

#include <CVMEInterface.h>
#include <CVMEList.h>
#include <CVMEAddressRange.h>

#include "CVMEReadableObject.h"

#include <CInvalidInterfaceType.h>
#include <CBadValue.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////
//////////////////// Constant definitions ///////////////////////
/////////////////////////////////////////////////////////////////

#ifdef CONST
#undef CONST
#endif

#define CONST static const

#ifdef CONST32
#undef CONST32
#endif

#define CONST32 static const uint32_t

// Register offsets:

CONST32   controlStatus(0);
CONST32   moduleIdFirmware(4);
CONST32   acquisitionPreset(0x10);
CONST32   acquisitionCount(0x14);
CONST32   LNEPrescale(0x18);
CONST32   operationMode(0x100);
CONST32   copyDisable(0x104);
CONST32   lneChannelSelect(0x108);
CONST32   countDisables(0x200);
CONST32   clearCounters(0x204);

CONST32   keyReset(0x400);
CONST32   keySDRAMReset(0x404);
CONST32   keyTest(0x408);
CONST32   keyClearCounters(0x414);
CONST32   keyLNE(0x410);
CONST32   keyArm(0x414);
CONST32   keyEnable(0x418);
CONST32   keyDisable(0x41c);

CONST32   shadowRegisters(0x800);
CONST32   counterRegisters(0xa00);

CONST32   SDRAM(0x800000);

CONST32   RegisterSize(0xb00);
CONST32   SDRAMSize(0x800000);


// Control status register bits.. note that these are paired
// xxxOn and xxxOff bits.
//
CONST32 csr_ledOn(1);
CONST32 csr_ledOff(0x10000);

CONST32 csr_25MhzOn(0x10);
CONST32 csr_25MhzOff(0x100000);

CONST32 csr_testOn(0x20);
CONST32 csr_testOff(0x200000);

CONST32 csr_refPulserOn(0x400);
CONST32 csr_refPulserOff(0x400000);

// Bit fields in module/id rev level:

CONST32 fwid_idMask(0xffff0000);
CONST32 fwid_idShift(16);

CONST32 fwid_majorRevMask(0xff00);
CONST32 fwid_minorRevMask(0x00ff);
CONST32 fwid_majorScaler32(0x0100);
CONST32 fwid_majorClock(0xe000);
CONST32 fwid_majorLatch(0xf000);

CONST32 fwid_minorRevmask(0xff);
CONST32 fwid_correctId(0x3820);

// Bits and fields in the Acquisition operation mode register 
// (operationMode):
//

CONST32 mode_Scaler(0);
CONST32 mode_MCS(0x20000000);
CONST32 mode_HSCAL(0x30000000);

CONST32 mode_invertOutputs(0x08000000);
CONST32 mode_outputSDRAM(0x000000000);
CONST32 mode_outputClock(0x010000000);
CONST32 mode_outputUnused(0x03000000);

CONST32 mode_invertInputs(0x00800000);
CONST32 mode_inputNONE(0);
CONST32 mode_inputLneInhibitLne(0x00100000);
CONST32 mode_inputLneInhibitLneAndAll(0x00200000);
CONST32 mode_inputLneInhibitAll(0x00300000);
CONST32 mode_inputInhibitBanks8(0x00400000);

CONST32 mode_addMode(0x00020000);
CONST32 mode_SDRAMisRAM(0x00010000);

CONST32 mode_armFromFPLNE(0x00000000);

CONST32 mode_lneSourceVME(0x00000000);
CONST32 mode_lneSourceFrontPanel(0x00000010);
CONST32 mode_lneSource10Mhz(0x00000020);
CONST32 mode_lneSourceChannelN(0x00000030);
CONST32 mode_lneSourcePresetN(0x00000040);

CONST32 mode_format32(0x0);
CONST32 mode_format24(0x4);
CONST32 mode_format16(0x8);
CONST32 mode_format08(0xc);

CONST32 mode_nonClearing(1); 


CONST short registerAM(0xe);
CONST short blockAM(0x0f);

///////////////////////////////////////////////////////////////
//////////////// Constructors and canonicals //////////////////
///////////////////////////////////////////////////////////////


/*!
   Construction involves constructing the base class and setting
   the configuration items to default values.
   \param interface : CVMEInterface&
      VME interface through which we will see the module.
   \param base      : uint32_t
      VME base address set in the module rotary switches.
*/
CSIS3820::CSIS3820(CVMEInterface& interface, uint32_t base) : 
  CVMEReadableObject(interface, base),
  m_pVme(0)
{
  setConfigurationDefaults();
}
/*!
  Copy construction copy constructs the base class and copies in the
  attributes from the rhs object.
*/
CSIS3820::CSIS3820(const CSIS3820& rhs) :
  CVMEReadableObject(rhs)
{
  copyIn(rhs);
}
/*!
   Destruction is pretty much a no-op.
*/
CSIS3820::~CSIS3820()
{
  delete m_pVme;
}

/*!
   Assignment--- pretty much like copy construction:
*/
CSIS3820&
CSIS3820::operator=(const CSIS3820& rhs) 
{
  if (this != &rhs) {
    CVMEReadableObject::operator=(rhs);
    delete m_pVme;		// Prevent memory leaks.
    copyIn(rhs);
  }
  return *this;
}
/*!
   Equality means referring to the same module.. config is not important.
*/
int
CSIS3820::operator==(const CSIS3820& rhs) const
{
  return CVMEReadableObject::operator==(rhs);
}
int
CSIS3820::operator!=(const CSIS3820& rhs) const
{
  return !(*this == rhs);
}

////////////////////////////////////////////////////////////////////////
/////////////// Configuration setter/getter functions //////////////////
////////////////////////////////////////////////////////////////////////

void
CSIS3820::configMode(CSIS3820::mode operatingMode) 
{
  m_mode = operatingMode;
}
CSIS3820::mode
CSIS3820::cgetMode() const
{
  return m_mode;
}

void
CSIS3820::configEnableRefPulser()
{
  m_enableRefPulser = true;
}
void
CSIS3820::configDisableRefPulser()
{
  m_enableRefPulser = false;
}
bool
CSIS3820::cgetRefPulserEnabled() const
{
  return m_enableRefPulser;
}

void 
CSIS3820::configMCSPreset(uint32_t cycles)
{
  m_mcsPreset = cycles;
}
uint32_t 
CSIS3820::cgetMCSPreset() const
{
  return m_mcsPreset;
}

void
CSIS3820::configLNEPrescale(uint32_t scaledown)
{
  m_lnePrescale = scaledown;
}
uint32_t
CSIS3820::cgetLNEPrescale() const
{
  return m_lnePrescale;
}

void 
CSIS3820::configDisableClearOnLNE()
{
  m_disableClear = true;
}
void
CSIS3820::configEnableClearOnLNE()
{
  m_disableClear = false;
}
bool
CSIS3820::cgetClearOnLNEEnabled() const
{
  return !m_disableClear;
}

void
CSIS3820::configDataFormat(CSIS3820::DataFormat format)
{
  m_dataFormat = format;
}

CSIS3820::DataFormat
CSIS3820::cgetDataFormat() const
{
  return  m_dataFormat;
}


void
CSIS3820::configLNESource(CSIS3820::LNESource source)
{
  m_lneSource = source;
}
CSIS3820::LNESource
CSIS3820::cgetLNESource() const
{
  return m_lneSource;
}

void
CSIS3820::configArmEnableSource(CSIS3820::ArmEnableSource source)
{
  m_armEnableSource = source;
}
CSIS3820::ArmEnableSource
CSIS3820::cgetArmEnableSource() const
{
  return m_armEnableSource;
}

void 
CSIS3820::configSDRAMasFIFO()
{
  m_SDRAMisFIFO = true;
}
void
CSIS3820::configSDRAMasRAM()
{
  m_SDRAMisFIFO = false;
}
bool
CSIS3820::cgetSDRAMisFIFO() const
{
  return m_SDRAMisFIFO;
}

void
CSIS3820::configInputPolarity(CSIS3820::Polarity mode)
{
  m_inputsInverted = (mode == inverted) ? true : false;
}
CSIS3820::Polarity
CSIS3820::cgetInputPolarity() const 
{
  return (m_inputsInverted) ? inverted : normal;
}
void 
CSIS3820::configInputMode(CSIS3820::InputMode mode)
{
  m_inputMode = mode;
}
CSIS3820::InputMode
CSIS3820::cgetInputMode() const
{
  return m_inputMode;
}

void 
CSIS3820::configOutputPolarity(CSIS3820::Polarity mode)
{
  m_outputsInverted = (mode == inverted) ? true : false;
}
CSIS3820::Polarity
CSIS3820::cgetOutputPolarity() const
{
  return (m_outputsInverted) ? inverted : normal;
}
void
CSIS3820::configOutputMode(CSIS3820::OutputMode mode)
{
  m_outputMode= mode;
}
CSIS3820::OutputMode
CSIS3820::cgetOutputMode() const
{
  return m_outputMode;
}

void 
CSIS3820::configCopyDisableMask(uint32_t disableMask)
{
  m_channelCopyDisables = disableMask;
}
uint32_t
CSIS3820::cgetCopyDisableMask() const
{
  return m_channelCopyDisables;
}

void
CSIS3820::configChannelDisableMask(uint32_t disableMask)
{
  m_channelDisables = disableMask;
}
uint32_t
CSIS3820::cgetChannelDisableMask() const
{
  return m_channelDisables;
}

//////////////////////////////////////////////////////////////////////////
/////////////////// Key register immediate functions   ///////////////////
//////////////////////////////////////////////////////////////////////////


/*!
  Load the next event using a VME trigger to do it.
*/
void
CSIS3820::LNE()
{
  regWrite(keyLNE, 0);
}
/*!
   Arm the scalers to start counting on inputs.
*/
void
CSIS3820::Arm()
{
  regWrite(keyArm, 0);
}
/*!
    Reset the module.
*/
void
CSIS3820::Reset()
{
  regWrite(keyReset, 0);
}

////////////////////////////////////////////////////////////////////////////
//////////////////// virtual function overrides. ///////////////////////////
////////////////////////////////////////////////////////////////////////////



/*!
  Initialize the module from the configuration.
  Once initialized, the module is then armed.
  Note that we will throw an invalid interface type if the
  base address supplied does not point at a CSIS3820 module.

*/
void
CSIS3820::initialize()
{
  // Create an address space for user extended that covers the register space
  // of the module:
  

  uint32_t fwid = regRead(moduleIdFirmware);
  if (( (fwid & fwid_idMask) >> fwid_idShift) != fwid_correctId) {
    char msg[100];
    sprintf(msg, "CSIS3820::initialize - module with base address %0x08x is not a 3820",
	    getBase());
    throw CInvalidInterfaceType(msg, "Initializing an SIS 3820");
  }

  // Now figure out what goes in each register we care about and do it.
  //
  Reset();			// To power up defaults.
  regWrite(countDisables, m_channelDisables);
  regWrite(copyDisable,   m_channelCopyDisables);

  uint32_t opMode = 0;
  if (m_disableClear) opMode |= mode_nonClearing | mode_addMode;
  switch (m_dataFormat) {
  case format32:
    opMode |= mode_format32;
    break;
  case format24:
    opMode |= mode_format24;
    break;
  case format16:
    opMode |= mode_format16;
    break;
  case format08:
    opMode |= mode_format08;
    break;
  default:
    throw CBadValue("format32, format24, format16, format08",
		    "unknown", "CSIS3820::initialize");

  }
  switch (m_lneSource) {
  case LNEVME:
    opMode |= mode_lneSourceVME;
    break;
  case LNEFrontPanel:
    opMode |= mode_lneSourceFrontPanel;
    break;
  case LNE10MHz:
    opMode |= mode_lneSource10Mhz;
    break;
  default:
    throw CBadValue("LNEVME, LNEFrontpanel, LNE10MHz",
		    "unknown", "CSIS3820::initialize");
  }

  switch (m_armEnableSource) {
  case ArmFPLNE:
      opMode |= mode_armFromFPLNE;
    break;
  default:
    throw CBadValue("ArmFPLNE", "Unknown", "CSIS3820::initialize");
  }
  if (!m_SDRAMisFIFO) opMode |= mode_SDRAMisRAM;
  
  switch (m_inputMode) {
  case NoInputs:
    opMode |= mode_inputNONE;
    break;
  case LneInhibitLne:
    opMode |= mode_inputLneInhibitLne;
    break;
  case LneInhibitLneAndAll:
    opMode |= mode_inputLneInhibitLneAndAll;
    break;
  case LneInhibitBanks8:
    opMode |= mode_inputInhibitBanks8;
    break;
  default:
    throw CBadValue("NoInputs, LneInhibitLne, LneInhibitLneAndAll,  LneInhibitBanks8",
		    "unknown", "CSIS3820::initialize");
    
  }
  if (m_inputsInverted) opMode |= mode_invertInputs;
  
  switch (m_outputMode) {
  case SDRAM:
    opMode |= mode_outputSDRAM;
    break;
  case Clock:
    opMode |= mode_outputClock;
    break;
  case Unused:
    opMode |= mode_outputUnused;
    break;
  default:
    throw CBadValue("SDRAM, Clock, Unused", "Unknown output mode",
		    "CSIS3820::initialize");
  }
  if (m_outputsInverted) opMode |= mode_invertOutputs;

  switch (m_mode) {
  case Latching:
    opMode |= mode_Scaler;
    break;
  case MCS:
    opMode |= mode_MCS;
    break;
  case Histogramming:
    opMode |= mode_HSCAL;
    break;
  default:
    throw CBadValue(" Latching, MCS, Histogramming", 
		    "Unknown operating mode", "CSIS3820::initialize");
  }
  regWrite(operationMode, opMode);

  // 
  regWrite(LNEPrescale, m_lnePrescale);
  regWrite(acquisitionPreset, m_mcsPreset);
  regWrite(acquisitionCount, 0);
  

  // figure out the bits to set/clear in the controlStatus register.
  // we must set or clear each bit as this register contains setters/clearers.
  // (well we could rely on known defaults but we won't so the code is clearer).

  uint32_t csr = csr_ledOff | csr_25MhzOff | csr_testOff;
  if (m_enableRefPulser) {
    csr |= csr_refPulserOn;
  }
  else {
    csr |= csr_refPulserOff;
  }
  regWrite(controlStatus, csr);
  

  // Arm the module for data taking.


  Arm();
}
/*!
   Read the module into a user buffer.  Which we read depends on the
   mode the module is in.  If in scaler mode, we just read the shadow registers.
   Otherwise, we read one event from the SDRAM (further decisions have to be 
   made there).  Since all decisions about how to read the module aree based on the
   configuration values, rather than values read from the device, it is important
   not to configure the scaler while data taking is in progres..

   \param buffer : void*
      Pointer to the location in which to store the data,  This must have
      at least largestEvent() bytes of storage.
   \return size_t
   \retval Number of bytes actually read.
*/
size_t
CSIS3820::read(void* buffer)
{
  if (m_mode == Latching) {
    return readShadow(buffer);
  } 
  else {
    return readSDRAM(buffer);
  }
}
 
/*!
   Return the number of bytes that could be read in the largest event.
   Since each read only reads a single event, this is 32*sizeof(uint32_t).
*/
size_t
CSIS3820::largestEvent()
{
  return 32*sizeof(uint32_t);
}  

/*!
    Append scaler reads to a VME transaction list.  Since
    decisions about how to readout the module are done based on the module
    configuration rather than register values, it is important that the
    configuration be stabilized by the time this is called.
    \param list : CVMEList&
         List to which to add the readout instructions.
    \return CVMEList&
    \retval reference to the list after modification.

*/
CVMEList&
CSIS3820::addReadoutToList(CVMEList& list)
{
  if (m_mode == Latching) {
    return addShadowRead(list);
  }
  else {
    return addSDRAMRead(list);
  }
}
//////////////////////////////////////////////////////////////////////////////
/////////////////////////// Helper functions (private) ///////////////////////
//////////////////////////////////////////////////////////////////////////////

/*
   Get an address range object associated with the module registers.
   This caches the address range so there's not much overhead in callling
   it as needed.
   Returns: CVMEAddressRange& : reference to an address range that
                                corresponds to the device register page.
*/
CVMEAddressRange& 
CSIS3820::registers()
{
  if (!m_pVme) {
    uint32_t base  = getBase();
    CVMEInterface& interface(getInterface());
    
    m_pVme = interface.createAddressRange(registerAM,
					  base, RegisterSize);
  }
  return *m_pVme;
}
/*
   Write a value to a register.  This is just a pokel on the
   register address range.. we encapsulate the offset scaling.
   Parameters:
    uint32_t offset   - Byte offset to the register to write.
    uint32_t value    - Value to write.

*/
void
CSIS3820::regWrite(uint32_t offset, uint32_t value)
{
  CVMEAddressRange& r(registers());
  r.pokel(offset/sizeof(uint32_t), value);
}
/*
   Read a value from a register.  This is just returning the peekl
   on the register address range.. we encapsulate the offset
   scaling.
   Parameters:
     uint32_t offset  - Byte offset to the register to read.
   Returns:
     uint32_t contents of the register.
*/
uint32_t
CSIS3820::regRead(uint32_t offset)
{
  CVMEAddressRange& r(registers());
  return r.peekl(offset/sizeof(uint32_t));
}

/*
  Set the configuration defaults.
  This is used at construction time rather than initializers.
  Scalers are set up as just a simple latching scaler module.
*/
void
CSIS3820::setConfigurationDefaults()
{
  m_mode                = Latching;
  m_enableRefPulser     = false;
  m_mcsPreset           = 0;
  m_lnePrescale         = 0;
  m_disableClear        = false;
  m_dataFormat          = format32;
  m_lneSource           = LNEVME;
  m_armEnableSource     = ArmFPLNE;
  m_SDRAMisFIFO         = true;
  m_inputsInverted      = false;
  m_inputMode           = LneInhibitLneAndAll;
  m_outputsInverted     = false;
  m_outputMode          = Unused;
  m_channelCopyDisables = 0;
  m_channelDisables     = 0;

}

/*
   copyIn - copies the state of another CSIS3820 to *this.
            Note that since m_pVme is dynamically produced and cached as needed,
	    we get around reference counting issues by setting it to zero.
*/
void
CSIS3820::copyIn(const CSIS3820& rhs)
{ 
  m_pVme                = 0;
  m_mode                = rhs.m_mode;
  m_enableRefPulser     = rhs.m_enableRefPulser;
  m_mcsPreset           = rhs.m_mcsPreset;
  m_lnePrescale         = rhs.m_lnePrescale;
  m_disableClear        = rhs.m_disableClear;
  m_dataFormat          = rhs.m_dataFormat;
  m_lneSource           = rhs.m_lneSource;
  m_armEnableSource     = rhs.m_armEnableSource;
  m_SDRAMisFIFO         = rhs.m_SDRAMisFIFO;
  m_inputsInverted      = rhs.m_inputsInverted;
  m_inputMode           = rhs.m_inputMode;
  m_outputsInverted     = rhs.m_outputsInverted;
  m_outputMode          = rhs.m_outputMode;
  m_channelCopyDisables = rhs.m_channelCopyDisables;
  m_channelDisables     = rhs.m_channelDisables;

}

/*
   Read 32 scalers from the shadow counters.  The shadow counters are always
   the stable value of the last LNE signal in latching scaler mode.
   32 longs is few enough that we can just read these channels with PIO.
*/
size_t
CSIS3820::readShadow(void* buffer)
{
  uint32_t*   p      = static_cast<uint32_t*>(buffer);
  uint32_t    offset = shadowRegisters;

  LNE();			// Latch the values.
  
  for (int i =0; i < 32; i++) {
    *p++ = regRead(offset++);
  }
  return 32*sizeof(uint32_t);
}
/*
   Read data from the SDRAM.   The number of longs read will depend on the
   copyDisableMask.  For each bit in that mask, there's one less item
   to read.
*/
size_t
CSIS3820::readSDRAM(void* buffer)
{
  uint32_t* p   = static_cast<uint32_t*>(buffer);
  uint32_t  n   = liveChannels();


  if (m_SDRAMisFIFO) {
    CVMEAddressRange* mem(getInterface().createAddressRange(registerAM,
							    SDRAM, n*sizeof(uint32_t)));
    for (int i=0; i < n; i++) {
      *p++ = mem->peekl(0);
    }
    delete mem;
  }
  else {
    CVMEAddressRange* mem(getInterface().createAddressRange(registerAM,
							    SDRAM, n*sizeof(uint32_t)));
    for (int i =0; i < n; i++) {
      *p++ = mem->peekl(i/sizeof(uint32_t));
    }
    regWrite(keySDRAMReset, 0);
    delete mem;
  }
}
/*
   Adds the stuff needed to read the shadow registers to a VME
   list.  32 shadow counters are unconditionally read.
*/
CVMEList&
CSIS3820::addShadowRead(CVMEList& list)
{
  list.addWrite32(registerAM, getBase() + keyLNE, 0);
  list.addBlockRead32(blockAM, getBase() + SDRAM, 32);

  
  return list;
}
/*
  Adds stuff to read an event from the SDRAM.  
*/
CVMEList&
CSIS3820::addSDRAMRead(CVMEList& list)
{
  list.addBlockRead32(blockAM, getBase() + SDRAM, liveChannels());
  list.addWrite32(registerAM, getBase() + keySDRAMReset, 0);

  return list;
}
/*
  Analyze the copy disable mask to return the number of channels to read
  from sdram/fifo.
*/
uint32_t
CSIS3820::liveChannels()
{
  uint32_t n = 32;
  // To improve speed, we'll assume that typically later channels are disabled
  // and that those are disabled in contiguous lumps.  So we'll count the
  // disabled channels from high bit to low until there are no more bits.
  //
  uint32_t disables = m_channelCopyDisables;
  uint32_t mask     = 0x80000000;

  while (disables) {
    if (mask & disables) {
      n--;
      disables &= ~mask;
    }

    mask = mask >> 1;
  }
  // n is now the number of channels to read.
  // If we are in fifo mode, then just read n items from the first location 
  // of sdram.

  // If we are not in fifo mode, we must write to the Key sdramfifo reset register
  // after read, and the dwell time must have been set so that there is no pile up.
  //
  // regardless,we assume that indiv. reads are faster than settting up a 
  // dma read.
  return n;
}
