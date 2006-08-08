#/*
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
#include "CAEN32.h"
#include <CVMEInterface.h>
#include <CVMEList.h>
#include <CVMEAddressRange.h>
#include <CInvalidInterfaceType.h>
#include <CDeviceIncapable.h>
#include <RangeError.h>
#include <stdlib.h>

using namespace std;

// Register map of the device (these are byte offsets).

#ifndef CONST
#define CONST static const 
#endif

CONST uint32_t MEB(0x0);
CONST uint32_t Firmware(0x1000);
CONST uint32_t GeographicalAddress(0x1002);
CONST uint32_t MCSTCBLTAddress(0x1004);
CONST uint32_t BitSet1(0x1006);
CONST uint32_t BitClear1(0x1008);
CONST uint32_t Status1(0x100e);
CONST uint32_t CBLTControl(0x101a);
CONST uint32_t Status2(0x1020);
CONST uint32_t FClear(0x102e);
CONST uint32_t BitSet2(0x1032);
CONST uint32_t BitClear2(0x1034);
CONST uint32_t CrateSelect(0x103c);
CONST uint32_t FullScaleRange(0x1060); // if tdc
CONST uint32_t Iped(0x1060);	       // If qdc
CONST uint32_t Thresholds(0x1080);

CONST uint32_t BoardIDMSB(0x8036);
CONST uint32_t BoardID(0x803a);
CONST uint32_t BoardIDLSB(0x803e);

CONST uint32_t BoardSerialLSB(0x8f06);
CONST uint32_t BoardSerialMSB(0x8f02);

CONST size_t   registerPageSize(0x9000); // This is approximate but good enough.

// Status register 1 bits I need:

CONST uint16_t S1DReady(1);
CONST uint16_t S1GBLDReady(2);
CONST uint16_t S1Busy(4);


// Status register 2 bits I need:

CONST uint16_t S2Empty(2);
CONST uint16_t S2Full(4);

//  Bit set 1 register bits I need:

CONST uint16_t B1Reset(0x80);

// CBLT Control register codes:

CONST uint16_t CBLTDisabled(0);
CONST uint16_t CBLTFirst(2);
CONST uint16_t CBLTLast(1);
CONST uint16_t CBLTMid(3);

// Bit set 2 register bits I need

CONST uint16_t B2Offline(2);
CONST uint16_t B2ClearData(0x4);
CONST uint16_t B2OverRange(8);
CONST uint16_t B2LowThreshold(0x10);
CONST uint16_t B2ValidControl(0x20);
CONST uint16_t B2SmallThresholds(0x100);
CONST uint16_t B2CommonStop(0x400);

// Threshold bits:

CONST uint16_t THKill(0x100);

// Data bits:

CONST uint32_t TypeMask(0x07000000);
CONST uint32_t TypeHeader(0x02000000);
CONST uint32_t TypeData(0x00000000);
CONST uint32_t TypeTrailer(0x04000000);
CONST uint32_t TypeEmpty(0x06000000);

CONST uint32_t CountMask(0x00003f00);
CONST uint32_t CountMaskComplement(0xffffc0ff);
CONST uint32_t CountShift(8);

// Address modifiers:

CONST short registerAM(0xe); // Supervisory extended data.
CONST short blockAM(0x0f);


//////////////////////////////////////////////////////////////////////////
///////////////////// 

/*!
   Construct a CAEN 32 module driver instance.
   We must do the following:
   - Construct our base class object.
   - Set the configuration member data to default values.
     These default values will be described in the configuration
     function comments rather than listed exhaustively here.
   - Determine the device's card type so that we know which 
     device specific calls to allow or, more importantly disallow.

   \param interface : CVMEInterface&
       References the interface that knows how to talk to the VME crate
       the module has been installed in.
   \param base : uint32_t
       Provides a base address for the module.  This is set in the module's
       four rotary switches as described in the hardware manual for the device.
   
*/
CAEN32::CAEN32(CVMEInterface& interface, uint32_t base) :
  CVMEReadableObject(interface, base),
  m_pRegisters(0),
  m_base(base)
{
  m_pRegisters = interface.createAddressRange(registerAM, base, registerPageSize);
  m_cardType = getCardType();

  setConfigurationDefaults();

  if (!validDeviceType()) {
    char msg[100];
    sprintf(msg, "CAEN32 at %08x, failed with invalid card type: %d\n",
	    base, m_cardType);
    throw CInvalidInterfaceType(msg, "Constructing a CAEN32");
  }
}

/*!
   Copy construction is a matter of copying the base class and using
   copyIn to also copy all the instance data.  The nice thing about using
   this configure/initalize scheme is that state like this can be just
   freely copied around. 
*/
CAEN32::CAEN32(const CAEN32& rhs) :
  CVMEReadableObject(rhs)
{
  copyIn(rhs);
}
/*!
   Destruction must destroy the register address window.
*/
CAEN32::~CAEN32()
{
  delete m_pRegisters;
}
/*!
    Assignment is very similar to copy constuction:
*/
CAEN32&
CAEN32::operator=(const CAEN32& rhs)
{
  if (this != &rhs) {
    CVMEReadableObject::operator=(rhs);
    copyIn(rhs);
  }
  return *this;
}
/*!
   Modules are identical if their base and vme crate are
   identical; We don't bother comparing the configurations.
*/
int
CAEN32::operator==(const CAEN32& rhs) const
{
  return CVMEReadableObject::operator==(rhs);
}
/*!
   Modules are not equal if they are !(operator==).
*/
int
CAEN32::operator!=(const CAEN32& rhs) const
{
  return !(*this == rhs);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////// Direct device inquiry ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
    Retrieves the firmware revision level from the device.
    This involves an immediate access to the device.

*/
uint16_t 
CAEN32::getFirmware()
{
  return readreg(Firmware);
}
/*!
    Gets the card type from the device.  The card type is stored in the module's
    configuration prom in three separate bytes.   These must be stitched together
    into a single uint32.
*/
uint32_t 
CAEN32::getCardType()
{
  uint32_t bidhigh = readreg(BoardIDMSB);
  uint32_t bidmid  = readreg(BoardID);
  uint32_t bidlow  = readreg(BoardIDLSB);

  return (bidhigh << 16) | (bidmid << 8) | bidlow;
}

/*!
   Gets the device serial number.  This is stored in two consecutive words
   of the module configuration prom, however only the bottom 8 bits of each
   word is used.
*/
uint16_t
CAEN32::getSerial()
{
  uint32_t serialLow = readreg(BoardSerialLSB) & 0xff;
  uint32_t serialHi  = readreg(BoardSerialMSB) & 0xff;

  return (serialLow) | (serialHi << 8);
}

///////////////////////////////////////////////////////////////////////////
///////////////// Configuration setting and inquiry ///////////////////////
///////////////////////////////////////////////////////////////////////////


// VME Virtual crate number:

void 
CAEN32::configCrate(uint8_t crate)
{
  m_crate = crate;
}
uint8_t 
CAEN32::cgetCrate() const
{
  return m_crate;
}
// Thresholds:

void
CAEN32::configThresholds(std::vector<uint16_t> thresholds)
{
  if (thresholds.size() != 32) {
    throw string("CAEN32::configThresholds - threshold array is not 32 elements");
  }
  m_thresholds = thresholds;
}
vector<uint16_t>
CAEN32::cgetThresholds() const
{
  return m_thresholds;
}


// VME software slot number...or alternatively disable that:

void
CAEN32::configHardwareGeo(bool hasHardwareGeo)
{
  m_hardwareGeo = hasHardwareGeo;
}
void
CAEN32::configGeo(uint8_t geo)
{
  m_softGeo = geo;
}
uint8_t
CAEN32::cgetGeo() const
{
  return m_softGeo;
}

// Set the actions that control what will be supressed:

void
CAEN32::configUnderThresholdAction(CAEN32::action what)
{
  m_keepUnderThreshold = (what == keep) ? true : false;
}
void
CAEN32::configOverflowAction(CAEN32::action what)
{
  m_keepOverflows = (what == keep) ? true : false;
}
void
CAEN32::configInvalidAction(CAEN32::action what)
{
  m_keepInvalid = (what == keep) ? true : false;
}

//  Card and channel enables:

void
CAEN32::configCardEnable(bool enabled) 
{
  m_enable = enabled;
}
bool
CAEN32::cgetCardEnable() const
{
  return m_enable;
}

void
CAEN32::configChannelEnable(uint8_t channel, bool enabled)
{
  validChannel(channel ,"Configuring CAEN32 channel enables");
  m_channelEnables[channel] = enabled;


}
bool
CAEN32::cgetChannelEnable(uint8_t channel) const
{
  validChannel(channel, "Getting CAEN32 channel enable");
  return m_channelEnables[channel];
}


//  Fast clear interval:

void
CAEN32::configFastClearInterval(uint16_t window)
{
  m_fastClearWindow = window;
}
uint16_t
CAEN32::cgetFastClearInterval() const
{
  return m_fastClearWindow;
}

/*!
   There are two modes in which the thresholds are interpreted:
   - small  - The threshold value is directly compared to the conversion.
   - large  - The threshold value is shifted left 4 bits and then compared
              (16x threshold is used).

*/
void
CAEN32::configThresholdMeaning(CAEN32::thresholdRange range)
{
  m_smallThresholds = (range == small) ? true : false;
}

CAEN32::thresholdRange
CAEN32::cgetThresholdMeaning() const
{
  return  (m_smallThresholds) ? small : large;
}

// Set up the chained block/multiblock transfer membership:

/*!
   There are two parameters that need to be set for chained block readout:
   - cbltPosition - The position in the chain of this module which can be any of:
      - CAEN32::First   - Module is left most in the chain.
      - CAEN32::Middle  - Module is not at the end of the chain.,
      - CAEN32::Last    - MOdule is the right most in the chain.
      - CAEN32::Unchained - Module is not in any chain.
  - cbltBase   - the base address of the chain.
*/
void
CAEN32::configCBLTMembership(CAEN32::cbltPosition membership, uint32_t cbltBase)
{
  m_chainPosition = membership;
  m_cbltBase = cbltBase;
}
CAEN32::cbltPosition
CAEN32::cgetCBLTPosition() const
{
  return m_chainPosition;
}
uint32_t
CAEN32::cgetCBLTBase() const
{
  return m_cbltBase;
}

/*!
    Configure the module to clear or not clear after being read.
    This essentially determines if the module will be used in multievent mode.
    \param clear : bool
        True to clear after read.
*/
void
CAEN32::configClearAfterRead(bool clear)
{
  m_clearAfterRead = clear;
}
/*!
   Get the state of the clear after read config.
*/
bool
CAEN32::cgetClearAfterRead() const
{
  return m_clearAfterRead;
}

///////////////////////////////////////////////////////////////////////////////
//////////////////// V775 specific functions
///////////////////////////////////////////////////////////////////////////////

/*!
   Configure the TDC mode: CAEN32::commonstart or CAEN32::commonstop
*/
void 
CAEN32::configTDCMode(CAEN32::tdcmode mode)
{
  if (isTdc()) {
    m_TDCcommonStop = (mode == commonstop) ? true : false;
  }
  else {
    throw CDeviceIncapable("Set the common start/stop mode",
			   "CAEN32::configTDCMode",
			   "Module is not a CAEN V775");
  }
}
CAEN32::tdcmode
CAEN32::cgetTDCMode() const
{
  if(isTdc()) {
    return m_TDCcommonStop ? commonstop : commonstart;
  }
  else {
    throw CDeviceIncapable("Get the common start/stop mode",
			   "CAEN32::cgetTDCMode",
			   "Module is not a CAEN32 TDC");
  }
}

void
CAEN32::configTDCRange(uint16_t range)
{
  if (isTdc()) {
    m_TDCtimeRange = range;
  }
  else {
    throw CDeviceIncapable("Configure TDC range",
			   "CAEN32::configTCRange",
			   "Module is not a CAEN32 TDC");
    
  }
}
uint16_t
CAEN32::cgetTDCRange() const
{
  if (isTdc()) {
    return m_TDCtimeRange;
  } 
  else {
    throw CDeviceIncapable("Get tdc range information",
			   "CAEN32::cgetTDCRang",
			   "Module is not a CAEN32 TDC");
  }
}
////////////////////////////////////////////////////////////////////////////////
//////////////////// QDC (V862, V792 only. /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
   QDC's work by charging a capacitor for the duration of the gate.
   Once the gate is released, the charge on the capacitor is digitized.
   If the gates are long, the leakage current in the capacitor can
   impose a siginifant negative offset on the result.  This is compensated
   for by injecting a fixed charge to the capacitor as the gate opens.
   If this charge is at least the leaked charge for the duration of the gate,
   no range will be lost.

   \param iped :uint16_t
      The injected charge. See the QDC manual's description of the iped
      register for more information about this.
*/
void 
CAEN32::configQDCCompensationCurrent(uint16_t iped)
{
  if (isQdc()) {
    m_QDCIped = iped;
  }
  else {
    throw CDeviceIncapable("Set QDC IPed value",
			   "CAEN32::configQDCompensationCurrent",
			   "Module is not a CAEN32 QDC");
  }
}
uint16_t
CAEN32::cgetQDCCompensationCurrent() const
{
  if(isQdc()) {
    return m_QDCIped;
  }
  else {
    throw CDeviceIncapable("Get QDC IPed value",
			   "CAEN32::cgetQDCCompensationCurrent",
			   "Module is not a CAEN32 QDC");
  }
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device control functions ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
   Clears the contents of the MEB.  Reading the MEB after this will,
   depending on the device setup, return an 'invalid' longword or a
   bus error.
*/
void 
CAEN32::clearData()
{
  bitset2(B2ClearData);
  bitclr2(B2ClearData);

}

/*!
   Resets the module.  This is done by  doing a soft reset in the 
   bitset1 register as suggested by CAEN.  The manual provides a reset key
   register but also recommends against using for some reason.
*/
void
CAEN32::reset()
{
  bitset1(B1Reset);
  bitclr1(B1Reset);
}
/*!
   Determines if this module has data present.  Data is present if at least
   one complete event has been stored in the MEB.
*/
bool
CAEN32::dataPresent()
{
  return (status1()  & S1DReady) != 0;
}
/*!
   The control bus on the front panel of the modules has a wire-tied or of
   the DREADY bits.  This is fed back to the modules and can be read to determine
   if an entire set of modules has at least one module with data.
*/
bool
CAEN32::globalDataPresent()
{
  return (status1()  & S1GBLDReady) != 0;
}

/*!
    Determines if the module is busy. Note that since these modules have
    a multi-event buffer, the busy time may be as short as the conversion time.
*/
bool
CAEN32::isBusy()
{
  return (status1() != 0);
}
/*!
  Returns true if the module has no complete events in the MEB.
*/
bool
CAEN32::isEmpty()
{
  return (status2() & S2Empty) != 0;
}
/*!
  Returns true if the MEB is full.
*/
bool
CAEN32::isFull()
{
  return (status2() & S2Full) != 0;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////// Virtual function overrides ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
     Initialize the module given the data in the configuration members.
     Note that if the module is geographically addressed,  We will not set
     the geograhpical address, and that's the only effect of that.
*/
void
CAEN32::initialize()
{
  reset();

  // Take it from the top:  Crate:

  writereg(CrateSelect, m_crate);

  // If permitted, the geo address:

  if (!m_hardwareGeo) {
    writereg(GeographicalAddress, m_softGeo);
  }
  // Build up the bits for bitset2 and bitclear2:
  
  uint16_t sets(0);
  uint16_t clears(0);
  if (!m_keepUnderThreshold) {
    clears |= B2LowThreshold;
  }
  else {
    sets   |= B2LowThreshold;
  }
  if (!m_keepOverflows) {
    clears |=  B2OverRange;
  }
  else {
    sets |= B2OverRange;
  }
  if (!m_keepInvalid) {
    clears |= B2ValidControl;
  }
  else {
    sets |= B2ValidControl;
  }
  if (isTdc()) {
    if (m_TDCcommonStop) {
      sets |= B2CommonStop;
    }
    else {
      clears |= B2CommonStop;
    }
  }
  usleep(1);			// Dunno why this should be needed but it is.
  bitset2(sets);
  bitclr2(clears);

  // If TDC, time range:

  if (isTdc()) {
    writereg(FullScaleRange, m_TDCtimeRange);
  }
  // If QDC, IPEDL:

  if (isQdc()) {
    writereg(Iped, m_QDCIped);
  }

  // Module enable:

  if (m_enable) {
    bitclr2(B2Offline);
  }
  else {
   bitset2(B2Offline);
  }
  // The enables (their inverses actuall), are merged as kill bits into
  // the thresholds which are programmed into each of the threshold values
  //
  for (int i =0; i < 32; i++) {
    uint16_t value  = m_thresholds[i];
    if (!m_channelEnables[i]) {
      value |= THKill;
    }
    setThreshold(i, value);
  }
  // Fast clear window:

  writereg(FClear, m_fastClearWindow);

  // Small thresholds

  if (m_smallThresholds) {
    bitset2(B2SmallThresholds);
  }
  else {
    bitclr2(B2SmallThresholds);
  }
  // CBLT chain setup.

  if (m_chainPosition != Unchained) {
    uint16_t control;
    switch (m_chainPosition) {
    case First:
      control = CBLTFirst;
      break;
    case Last:
      control = CBLTLast;
      break;
    case Middle:
      control = CBLTMid;
    }
    
    writereg(MCSTCBLTAddress, m_cbltBase);
    writereg(CBLTControl, control);
  }
  else {
    writereg(CBLTControl, CBLTDisabled);
  }
}

/*!
   Read a single event from the MEB.  Note that
   firmware prior to 8.09 or so did not always handle the
   end of event and word count properly. We will therefore
   - Use the type field to know when we have finished reading an event.
   - Fix up the channel count in the first longword (header).

   \param buffer : void*
       Pointer to at least 34 longwords worth of storage in which
       this event is stored.
    \return size_t
    \retval number of bytes read from the device.

    It is the user's responsibility to be sure there is data in the device.
    If the first long from the device is not a header, then 0 is returned,
    and no data are read.

*/
size_t
CAEN32::read(void* buffer)
{
  uint32_t* pHeader = static_cast<uint32_t*>(buffer); // Where header goes.
  uint32_t* pBody   = pHeader; pBody++;	              // Event body goes here.
  
  // Get the header if it isn't return 0:

  uint32_t header  = meb();
  if ((header & TypeMask) != TypeHeader) {
    return 0;
  }

  // Read the body until we have a non data item (which we also put in the buffe.

  uint32_t datum;
  do {
    datum = meb();
    *pBody++       = datum;

  } while ((datum & TypeMask) == TypeData);

  // Compute the channel count:

  uint32_t channelsRead = (pBody - pHeader) - 2; // header and trailer too.
  header               &= CountMaskComplement;
  header               |= channelsRead << CountShift;
  *pHeader              = header;

  if (m_clearAfterRead) {
    clearData();
  }

  return (channelsRead + 2)*sizeof(uint32_t);


}
/*!
   Add the readout of this module to a stack.
   To do this we have to rely on having firmware that 
   will produce reliable count fields... 
   Then we can turn this into a field extracted counted  transfer.
   I think we need to specify a block transfer AMOD to do this in 
   most controllers
*/

CVMEList&
CAEN32::addReadoutToList(CVMEList& theList)
{
  theList.defineCountField(8, 0x3f);
  theList.addCountFieldRead32(blockAM,  m_base + MEB);

  return theList;
}
/*!
   Return the number of bytes in the largest event.
*/
size_t
CAEN32::largestEvent()
{
  return 34*sizeof(uint32_t);
} 

////////////////////////////////////////////////////////////////////////////////
////////////////////// Private utilities ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// setConfigurationDefaults:
//        Set the module default configuration.
//        It is assumed that by the time we hit here, m_cardType is
//        already set correctly.
//
void
CAEN32::setConfigurationDefaults()
{
  m_crate              = 0;		  
  m_hardwareGeo        = false;
  m_softGeo            = 1;	
  
  // Since our threshold values are only a guess; dont' enable any supression.

  m_keepUnderThreshold = true;
  m_keepOverflows      = true;
  m_keepInvalid        = true;

  // TDC specific stuff:

  if (isTdc()) {
    m_TDCcommonStop  = true;
    m_TDCtimeRange   = 0xff;	// full range, not full resolution.
  }
  if (isQdc()) {
    m_QDCIped   = 0x80;		// 1/2 full value for current injection.
  }
  m_enable = true;
  m_channelEnables.clear();
  m_channelEnables.insert(m_channelEnables.begin(), 32, true);
  m_thresholds.clear();
  m_thresholds.insert(m_thresholds.begin(), 32, 0x10); // It's a guess.
  
  m_fastClearWindow = 0;
  m_smallThresholds = true;
  m_chainPosition   = Unchained;
  m_cbltBase        = 0xaa;
  m_clearAfterRead  = true;
}      


// copyIn  - Copy configuration/members etc. from another object.
//         NOTE: If data members are added to this class, they must be dealt with here
//               to preserve copy semantics.  Better here, however than both the
//               copy constructor and assignment operators.
//

void
CAEN32::copyIn(const CAEN32& rhs)
{
  /// The memory map has to be recreated....

  delete m_pRegisters;
 
  m_pRegisters = rhs.getInterface().createAddressRange(registerAM, rhs.m_base,
						       registerPageSize);
  // Other than that it's clear sailing.

  m_base               = rhs.m_base;
  m_cardType           = rhs.m_cardType;
  m_crate              = rhs.m_crate;
  m_hardwareGeo        = rhs.m_hardwareGeo;
  m_softGeo            = rhs.m_softGeo;
  m_keepUnderThreshold = rhs.m_keepUnderThreshold;
  m_keepOverflows      = rhs.m_keepOverflows;
  m_keepInvalid        = rhs.m_keepInvalid;
  m_TDCcommonStop      = rhs.m_TDCcommonStop;
  m_TDCtimeRange       = rhs.m_TDCtimeRange;
  m_QDCIped            = rhs.m_QDCIped;
  m_enable             = rhs.m_enable;
  m_channelEnables     = rhs.m_channelEnables; // Vectors assign, if their elements do.
  m_thresholds         = rhs.m_thresholds;
  m_smallThresholds    = rhs.m_smallThresholds;
  m_chainPosition      = rhs.m_chainPosition;
  m_cbltBase           = rhs.m_cbltBase;
  m_clearAfterRead     = rhs.m_clearAfterRead;
}

// True if the module is one of the supported device types;

bool
CAEN32::validDeviceType() const
{
  return   ((m_cardType    == 775)            ||
	    (m_cardType    == 785)            ||
	    (m_cardType    == 729)            ||
	    (m_cardType    == 862)            ||
	    (m_cardType    == 1785));
}

// Throw if invalid channel.
void
CAEN32::validChannel(uint8_t channel, const char* msg) 
{
  if (channel >= 32) {
    throw CRangeError(0, 31, channel, msg);
  }
}

// true if card is a tdc.

bool
CAEN32::isTdc() const
{
  return m_cardType  == 775;  
}

// True if card is a qdc.
bool
CAEN32::isQdc() const
{
  return (m_cardType == 792) || (m_cardType == 862);

}

// Register I/O:

uint16_t 
CAEN32::readreg(uint32_t offset) 
{
  return m_pRegisters->peekw(offset/sizeof(uint16_t));
}
void
CAEN32::writereg(uint32_t offset, uint16_t value)
{
  m_pRegisters->pokew(offset/sizeof(uint16_t), value);
}

void
CAEN32::bitset1(uint16_t mask)
{
  writereg(BitSet1, mask);
}
void
CAEN32::bitclr1(uint16_t mask)
{
  writereg(BitClear1, mask);
}

void
CAEN32::bitset2(uint16_t mask)
{
  writereg(BitSet2, mask);
}
void
CAEN32::bitclr2(uint16_t mask)
{
  writereg(BitClear2, mask);
}

// Return the contents of the staus 1 register.

uint16_t
CAEN32::status1()
{
  return readreg(Status1);
}

uint16_t
CAEN32::status2()
{
  return readreg(Status2);
}

uint32_t 
CAEN32::meb()
{
  return m_pRegisters->peekl(MEB/sizeof(uint32_t));
}

void
CAEN32::setThreshold(uint32_t index, uint16_t value)
{
  m_pRegisters->pokew(Thresholds/sizeof(uint16_t) + index, value);
}
