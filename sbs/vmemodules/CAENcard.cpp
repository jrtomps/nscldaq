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


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";

#include <config.h>


#include "CAENcard.h"
#include <string>
#include <unistd.h>
#include <stdint.h>

#include <assert.h>

using namespace std;



//   Convenience function for longword swapping.
#define swaplong(n)  (((n >> 16) & 0xffff) | ((n & 0xffff) << 16))
//

#define VME_CRATE_SIZE      24	// Slots in a VME crate.

#define CAEN_REGISTERSIZE   0x2000 // Size of CAEN Register set.
#define CAEN_ROMSIZE        0x1000 // Size of CAEN ROM page.
#define CAEN_ROMOFFSET      0x8000 // Offset from base of ROM page.


// Bits we need in registers:

// Threshold memory.

#define KILLBIT  0x100		// Kill bit in threshold registers.

// Bit Set/Clear 1 register:

#define RESET       0x080
#define SELADDR     0x010

// Status register 1:


#define DREADY      0x001
#define GDREADY     0x002
#define BUSY        0x004
#define GBUSY       0x008


// Bit Set/Clear 2 register.

#define OFFLINE     0x002
#define CLEARDATA   0x004
#define KEEPOVER    0x008
#define KEEPTHRESH  0x010
#define KEEPINVALID 0x020
#define COMMONSTOP  0x400
#define STEPTHR     0x100
#define AUTOIN      0x800
#define EMPTY       0x1000
#define SLIDESUB    0x2000
#define ALLTRG      0x4000


// Control register1:

#define CTL1BLKEND   0x04
#define CTL1PROGRST  0x10
#define CTL1BERRENA  0x20
#define CTL1ALIGN64  0x40


// Multicast control register:

#define MCSTFIRST    2
#define MCSTLAST     1


// Status register 2:

#define SR2EMPTY     2		// MEB empty
#define SR2FULL      4		// MEB full.


// The structure below represents the CAEN register set:
//

struct Registers {
  unsigned long   Buffer[0x1000/sizeof(long)]; // 0x0000
  unsigned short  FirmwareRev;                 // 0x1000
  unsigned short  GeoAddress;	               // 0x1002
  unsigned short  MCSTAddress;                 // 0x1004
  unsigned short  BitSet1;	               // 0x1006
  unsigned short  BitClear1;	               // 0x1008
  unsigned short  InterruptLevel;              // 0x100a
  unsigned short  InterruptVector;             // 0x100c
  unsigned short  Status1;	               // 0x100e
  unsigned short  Control1;                    // 0x1010
  unsigned short  HighAddress;	               // 0x1012
  unsigned short  LowAddress;                  // 0x1014
  unsigned short  Reset;                       // 0x1016
  unsigned short  pad1;                        // 0x1018
  unsigned short  MCSTControl;                 // 0x101a
  unsigned short  pad2;                        // 0x101c
  unsigned short  pad3;                        // 0x101e
  unsigned short  EventTrigger;	               // 0x1020
  unsigned short  Status2;                     // 0x1022
  unsigned short  EventCounterLow;             // 0x1024
  unsigned short  EventCounterHigh;            // 0x1026
  unsigned short  IncrementEvent;              // 0x1028
  unsigned short  IncrementOffset;             // 0x102a
  unsigned short  LoadTestRegister;            // 0x102c
  unsigned short  FCLRWindow;                  // 0x102e
  unsigned short  pad4;                        // 0x1030
  unsigned short  BitSet2;                     // 0x1032
  unsigned short  BitClear2;                   // 0x1034
  unsigned short  WMemoryTestAddress;          // 0x1036
  unsigned short  MemoryTestHigh;              // 0x1038
  unsigned short  MemoryTestLow;               // 0x103a
  unsigned short  CrateSelect;                 // 0x103c
  unsigned short  TestEventWrite;              // 0x103e
  unsigned short  EventCounterReset;           // 0x1040
  unsigned short  pad5[15];                    // 0x1042-105e
  unsigned short  TDCRange;                    // 0x1060
  unsigned short  pad7;                        // 0x1062
  unsigned short  RMemoryTestAddress;          // 0x1064
  unsigned short  pad8;                        // 0x1066
  unsigned short  SWComm;                      // 0x1068
  unsigned short  SlideConstant;               // 0x106a
  unsigned short  pad10;	               // 0x106c
  unsigned short  pad11;                       // 0x106e
  unsigned short  AAD;                         // 0x1070
  unsigned short  BAD;                         // 0x1072
  unsigned short  pad9[6];                     // 0x1074-107e
  unsigned short  Thresholds[32];              // 0x1080
  
};
#define QDCIPedestal TDCRange    // same register offsets.

// The structure below represents the part of the board ROM
// we care about... it holds a few chunks of useful info about
// the board:

struct ROM {
  unsigned short  pad1[0x26/sizeof(short)];  // 0x8000 - 0x8024
  unsigned short  OUIMSB;                    // 0x8026
  unsigned short  pad2;                      // 0x8028
  unsigned short  OUI;                       // 0x802a
  unsigned short  pad3;                      // 0x802c
  unsigned short  OUILSB;                    // 0x802e
  unsigned short  pad4;                      // 0x8030
  unsigned short  Version;                   // 0x8032
  unsigned short  pad5;                      // 0x8034
  unsigned short  BoardIdMSB;                // 0x8036
  unsigned short  pad6;                      // 0x8038
  unsigned short  BoardId;                   // 0x803a
  unsigned short  pad7;                      // 0x803c
  unsigned short  BoardIdLSB;                // 0x803e
  unsigned short  pad8[7];                   // 0x8040-0x804c
  unsigned short  Revision;                  // 0x804e
  unsigned short  pad9[0xb0/sizeof(short)];  // 0x8050 - 0x8100
  unsigned short  pad10[0xe00/sizeof(short)]; // 0x8100 - 0x8ffe
  unsigned short  pad11;                     // 0x8f00
  unsigned short  SerialMSB;                 // 0x8f02
  unsigned short  pad12;                     // 0x8f04
  unsigned short  SerialLSB;                 // 0x8f06
};


// Byte offset associated with a structure/field.


#define Offset(structname, field)  \
((unsigned int)&(((structname*)0x0)->field))

/// Short offset associated with a struct/field.

#define ShortOffset(structname, field) Offset(structname, field)/sizeof(short)
#define LongOffset(structname, field)  Offset(structname, field)/sizeof(long)

/// Read the buffer:
///
#define ReadBuffer   (((volatile Registers*)m_pModule)->Buffer[0])



/*!  Utility to read a block of data from the data buffer (FIFO).
     This is mainly here to optimize non mapped VME interfaces.
     \param pDest   - Pointer to the destination.
     \param nLongs  - Number of longs to read.
*/
void
CAENcard::ReadBufferBlock(int* pDest, Int_t nLongs)
{

  for(int i = 0; i < nLongs; i++) {
    int Datum = ReadBuffer;
    *pDest++ = swaplong(Datum);
  }

}


/*!
  Constructs a CAEN card.  The card can be addressed either
  geographically  or via base addres switches.  If geographical
  addressing is used, the slot value comes from the card position
  in the crate.  Otherwise it is programmed by the slotNum parameter.
  
  \param slotNum  This specifies the slot in the VME crate in 
  which the module resides.  If Geo is true, the module in that
  slot of the crate will be initialized.  If Geo is false, the
  module at physical address nBase will be initialized and its
  virtual slot number set to slotNum.
  
  
  \param crateNum The crate number in which the module lives.
  This selects which of the VME crates the module is in.  VME
  crate numbers can be determined using the cratelocator utility.
  The crate number is also programmed into the crate register so
  that the module crate field in the data stream is correct.
  
  \param Geo  
  - true if the slot supports geographical addressing
  - false if the slot requires 'thumbwheel addressing'
  
  \param nBase long [in] 
  The base address of the module.  Only has meaning if the 
  Geo parameter is false.
  
 */
CAENcard::CAENcard(int slotNum, int crateNum , 
		   bool Geo, long nBase) :
  m_nSlot(slotNum),
  m_nCrate(crateNum),
  m_fGeo(Geo),
  m_nBase(nBase),
  m_pModule(0),
  m_nFirmware(0)
{
  assert(sizeof(Registers) == 0x10c0);    // else struct misdefined.
  assert(sizeof(ROM) == 0xf08);          // else struct misdefined.
  slotInit();
}
/*!
  Copy constructor: Constructs a new card from an existing card
  object.  A new map is established to the card for this object.
  The card is not re-initialized, but remains in the state defined
  by the existing card.

  \param card The card to copy.


  */
CAENcard::CAENcard(const CAENcard& card) :
  m_nSlot(card.m_nSlot),
  m_nCrate(card.m_nCrate),
  m_fGeo(card.m_fGeo),
  m_nBase(card.m_nBase),
  m_pModule(0),
  m_fSmallThresholds(false)
{
  MapCard();			// Map the card.
}

/*!
  Assignment operator.  This card is destroyed, and replaced with
  a copy of the card described by <em>card</em>.  this is not
  initialized, but remains in the state defined by the rhs of the
  assignment.

  \param card The CAENcard value to assign to the target 
              of the assignment (this).
  \return Dereferenced version of this.

  */
CAENcard& CAENcard::operator=(const CAENcard& card)
{
  if(this != &card) {
    if(this->m_pModule) DestroyCard(); // Destroy old memory map.
    m_nSlot   = card.m_nSlot;
    m_nCrate = card.m_nCrate;
    m_fGeo   = m_fGeo;
    m_nBase  = card.m_nBase;
    MapCard();
  }
  return *this;
}



/*!

  This function:
  - Maps to the card and
 - Initializes it to a standard, well defined state.
  
  The card location is assumed to already have been set up in 
  the member variables (except for m_pModule which is filled in by
  the call to MapCard).

  \throw string - If there were problems a string exception is
     thrown with the string containing the reason for the problem.

*/
void 
CAENcard::slotInit()
{

  MapCard();

  reset();			// clears the event count and buffer
  channelOn(-1);		// Enable all channels.
  setCrate(m_nCrate);		// Set the module's crate number.
  
  if(cardType() == 775) {	// Set defaults for a TDC:
    // set use common start mode
    // with thresholds at 14ns.

    commonStart();
    setRange(0xFF);
    setThreshold(-1, 0x19);

  }
  else if((cardType() == 785) || 
	  (cardType() == 1785)) {	// Set the defaults for a Peak ADC:
      // thresholds to ~15mV 

    setThreshold(-1, 0x01);

  }
  else if((cardType() == 792) || (cardType() == 862)) {	// Set defaults for a QDC:

      setThreshold(-1, 0);	// Maybe later default raise this??
  }
  

}

/*! 

  Destructor.  The card memory is unmapped.

*/
CAENcard::~CAENcard()
{
  DestroyCard();
}

/*!
  Get the card type.  At initialization time, a short-lived map is
  made to the device configuration ROM and the card type is read
  and stored in m_nCardType.  This function just returns that value:
*/
int 
CAENcard::cardType()
{
  return m_nCardType;
}

/*!
  Each module has a crate number register that can be programmed
  with a virtual VME crate number.  The constructor defaults this
  to the physical VME crate number passed in.  If it's more 
  convenient to reprogram this to be a virtual crate number this
  function does the trick.

  \param crateNum (int [in]):
     New virtual crate numger.

*/
void 
CAENcard::setCrate(int crateNum)
{

  volatile Registers* pModule = (volatile Registers*)m_pModule;
  pModule->CrateSelect = crateNum & 0x0ff; // Bottom 8 bits only.

}

/*!
  See setCrate for more information.  This function reads the 
  virtual crate register.  To get the physical crate address 
  use getPhysicalCrate()

  \return Current value of the physical crate number.
*/
int 
CAENcard::getCrate()
{

  volatile Registers* pModule = (volatile Registers*)m_pModule;
  return (pModule->CrateSelect & 0xff);

}

/*!
  This function programs the modules thresholds.  When the ADC is
  triggered, the peaks sensed are compared against their per channel
  thresholds.  If below, the peak is not digitized, and supressed 
  from the readout, unless keepUnderThresholdData has been enabled.

  This function programs the threshold for a single channel or 
  sets all channels to a common threshold.  The kill bit (channel
  disable) is preserved.

  Performing a hardware reset will clear the
  thresholds, but  no hardware  default values defined. 
  The constructor does set all
  thresholds to reasonable defaults, however.

  \param ch (int [in]): 
    The channel on which to set the threshold.  
    Use a value of -1 to set the
    threshold of all channels on the card.
  \param threshold (int [in]):
    The low 8-bits of this value are used to set the 
    threshold on the
    desired channels.

*/
void 
CAENcard::setThreshold(int ch, int threshold)
{
  volatile Registers* pModule = (volatile Registers*)m_pModule;
  threshold = threshold & 0x00FF;
  
  if( ch < 0 ) {
    for(int i = 0; i < 32; i++) {
      setThreshold(i,threshold); // Set individual channel threshold
    }
  }
  else {

    Registers* pModule      = (Registers*)m_pModule;
    pModule->Thresholds[ch] = threshold |
                             (pModule->Thresholds[ch] & KILLBIT);

  }
}

/*!
  Set the threshold given a voltage.

  \param ch  (int [in]):
  The channel on which to set the threshold.  
  Use a value of -1 to set the
    threshold of all channels on the card.
  \param voltage (double [in]):
  The approximate voltage to which the threshold is to be set.  The
    conversion has a resolution of approximately 0.016 volts.

*/
void 
CAENcard::setThresholdVoltage(int ch, double voltage)
{
  if(m_fSmallThresholds) {
    setThreshold(ch,(int)(voltage*64.0));  
  }
  else {
    setThreshold(ch, (int)(voltage*8.0));
  }
}

/*!
  Enables the retension of data that is under the threshold.
  Any under threshold data that is kept in the event buffer 
  has the under-threshold bit
  set in the channel data.  
  This setting is cleared when reset() is called.
*/
void CAENcard::keepUnderThresholdData()
{
  Bitset2(KEEPTHRESH);
}

/*!
  This setting is cleared if reset() is called. The default value is to dicard the
  under threshold conversions.
*/
void CAENcard::discardUnderThresholdData()
{
  Bitclear2(KEEPTHRESH);

}
/*!
  Enables the retension of data that overflows the digitizer's 
  dynamic range.  This setting is cleared by a reset() call, or
  a call to discardOverflowData.

*/
void CAENcard::keepOverflowData()
{
  Bitset2(KEEPOVER);
}

/*!
  This setting is cleared if reset() is called. 
  The default value is to discard the
  conversions that result in an overflow.
*/
void CAENcard::discardOverflowData()
{
  Bitclear2(KEEPOVER);

}

/*!
  Instructs a tdc to keep invalid data.  Invalid data are stops
  received in the middle of reset operations, and are only
  applicable to V775 modules.

  \throw a string exception if the module is not a V775
*/
void
CAENcard::keepInvalidData() 
{ 

  if(cardType() == 775) {
    Bitset2(KEEPINVALID);
  }
  else {
    throw string("keepInvalidData - Module is not a V775 TDC");
  }
}
/*!
  Instructs a tdc to discard any invalid data (see keepInvalidData).

  \throw a string exception if the module is not a V775

*/
void 
CAENcard::discardInvalidData()
{
  if(cardType() == 775)
  {
    Bitclear2(KEEPINVALID);
  }
  else
  {
    throw string("discardInvalidData - Module is not a V775 TDC");
  }
}
/*!
 * Enable the production of a header/trailer if the module was 
 * gated but had no channels with valid conversions.
 * (e.g. set the EMPTY bit in the BIS2 register.
 */
void
CAENcard::emptyEnable()
{
	Bitset2(EMPTY);
}
/*!
 * Disable the production of header/trailer if the module
 * was  gated but had no channels with valid conversions
 * This is the power up default.
 * (clear the EMPTY bit in the BIC2 register.
 */
void
CAENcard::emptyDisable()
{
	Bitclear2(EMPTY);
}

/*!

  Sets a TDC to common start mode.  In this mode, time is measured
  beginning with the common gate input (start) and 
  ended for each channel
  by the individual channel input pulses (stops).

  \throw string indicating the module is not a 775 tdc if it is some
  other module.

*/
void CAENcard::commonStart()
{
  //make sure that the card is a TDC and initialized
  if(cardType() == 775)
  {
    Bitclear2(COMMONSTOP);
  }
  else
  {
    throw string("commonStart()  - Module is not a V775 TDC");
  }
}

/*!
  \return Zero indicates a successful write while -1 indicates that either the card
    is not a TDC or it is not yet initialized.

  This setting is cleared by a call to reset(). The TDC has slightly better minimum
  detectable signal in this mode.
*/
void
CAENcard::commonStop()
{
  //make sure that the card is a TDC and initialized
  if(cardType() == 775)
  {
    volatile Registers* pRegisters = (volatile Registers*)m_pModule;
    Bitset2(COMMONSTOP);
    
  }
  else
  {
    throw string("commonStop() - Module is not a V775 TDC");
  }
}

/*!
  Sets the conversion range of a V775 tdc.   The range
  is between 140 and 1200ns represented as follows:
  - 0x1e    - 1200ns
  - 0xff    -  140ns

  Values outside this range are illegal.  The constructor defaults
  the range to 140ns.

  \param range (int [in]):
     A raw value used to set the range on the TDC with.


  \throw string exception containing explanatory text if:
  - The range value is illegal.
  - The module is not a V775.
*/
void
CAENcard::setRange(int range)
{
  //make sure that the card is a TDC

  if(cardType() == 775)
  {
    if(range > 0x001D && range < 0x0100)
    {

      volatile Registers* pRegisters = (volatile Registers*)m_pModule;
      pRegisters->TDCRange  = (short int)range;


    }
    else
    {
      throw 
	string("setRange - TDC Range value must be in [0x1e,0xff");
    }
  }
  else
  {
    throw string("setRange - Module is not a V775 TDC");
  }
}

/*!
  Set the current injection pedestals for a V792 qdc.  Qdc's
  operate by storing input current on a capacitor.  The 
  capacitor has some leakage current that is normally negligible,
  however, this leakage current can become significant for 
  integration widths that are long.  The V792 qdc has a high
  high precision constant current source that can inject a 
  compensating current into the cap. during the gate period.  This
  function sets the amount of current injected.

  \param ped (int [in]):
  A raw value to set the pedestal current on the QDC with. 

  \throw string exception if this is not a V792 module.
*/
void 
CAENcard::setPedestalCurrent(int ped)
{
  //make sure that the card is a QDC
  int type = cardType();
  if((type == 792) || (type == 862))
  {
    ped = ped & 0xff;

    volatile Registers* pRegisters    = (volatile Registers*) m_pModule;
    pRegisters->QDCIPedestal = (ped); 

  }
  else
  {
    throw string("setPedestalCurrent - module is not a V792 or V862 QDC");
  }
}
/*!
  This function forces the module into offline mode so that 
  no new data is produced.
  Data may still be present in the event buffer, however. 
  This setting is cleared
  when reset() is called.
*/
void 
CAENcard::cardOff()
{
  Bitset2(OFFLINE);
}

/*!
  Sets a card to online (undoes the effects of a cardOff).
*/
void 
CAENcard::cardOn()
{
  Bitclear2(OFFLINE);
}

/*!

  The kill bit in the threshold memory for the channel is set 
  without
  destroying the threshold values. 
  Call clearData() to ensure that all data in
  the event buffer reflects this change.

  The kill bit is not cleared by clearData() or reset(). 
  Performing a hardware reset
  will clear the values, but there is no default value defined. 
  The constructor does
  ensure that all channels are on, however.

  \param ch (int [in]):
  The channel to suppress.  A value of -1 indicates that all channels will
    be supressed.

*/
void CAENcard::channelOff(int ch)
{

  if( ch < 0 ) {
    for( int i = 0; i < 32; ++i )
      channelOff(i);
  }
  else {
    if(ch < 32) {

       volatile Registers* pRegisters = (volatile Registers*)m_pModule;
       pRegisters->Thresholds[ch] |= KILLBIT;   

    } else {
      throw string("channelOff - channel number must be in [0,31]");
    }
  }
}

/*!

  The kill bit in the threshold memory for the channel is 
  cleared without
  destroying the threshold values. Call clearData() to ensure 
  that all data in
  the event buffer reflects this change. 
  An overflow, under threshold, or invalid
  data condition can still cause the channel to be supressed.

    \param ch (int [in]):
    The channel to unsuppress.  A value of -1 indicates that all channels will
    be unsupressed.

*/
void 
CAENcard::channelOn(int ch)
{

  if( ch < 0 ) {
    for( int i = 0; i < 32; ++i )
      channelOn(i);
  }
  else {
    if(ch < 32) {

       volatile Registers* pRegisters = (volatile Registers*)m_pModule;
       pRegisters->Thresholds[ch] &= ~(KILLBIT);

    } 
    else {
      throw string("channelOn - channel number must be in [0,31]");
    }
  }

}

/*!
  Clear the module event counter.

*/
void 
CAENcard::resetEventCounter()
{
  // Any write triggers the reset!


  ((volatile Registers*)m_pModule)->EventCounterReset = 1;



  ((volatile Registers*)m_pModule)->EventCounterReset = 1;

}

/*!
  Clear the module's multievent data buffers.
  This requires setting and clearing the clear data bit:
*/
void CAENcard::clearData()
{
   Bitset2(CLEARDATA);
   Bitclear2(CLEARDATA);
};

/*!
  Peforms a soft reset on the module using the reset bit of the
  BitSet/Clear1 registers.

  - This call does not change the threshold values or any of 
    the settings that might
    disrupt communication with the module. 
  - This call does set the module to discard
    under threshold, overflow, and invalid data. 
  - The crate value is set to zero.
  - The range, pedestal current, and start mode are all set to 
    default values
    which are, unfortunately, not specified by the documentation.
*/
void 
CAENcard::reset()
{
  Bitset1(RESET);
  Bitclear1(RESET);
};

/*!
  \return \li true indicates that there is data in the event buffer
          \li false indicates that the event buffer is empty

  This function is called by all of the readEvent functions before they 
read any data.
*/
bool 
CAENcard::dataPresent()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status1 & DREADY) != 0);

};

/*!
  Read a single event from a module into a user buffer.

  \param buf (int* [out]):
  A pointer to local memory that has already been allocated.
  Should be at least 34 * 4 = 136 bytes to hold the header, 
  footer, and 32 channels of data.

  \return \li \> 0 indicates the number of BYTES of data placed in buf
          \li 0 indicates that no data was placed in the buffer

*/
int 
CAENcard::readEvent(void* buf)
{

  int* pBuf((int*)buf);
  if(dataPresent())
  {
    int n = 1;			// Header at least is read.
    int  Header     = ReadBuffer;
    int* pHeader    = pBuf;	// To fix channel count.
    int  nRawChancnt= (Header >> 8) & 0x3f;
    *pBuf++         = swaplong(Header);
    if(0 && (getFirmware() >= 0x808) ) {	// Raw chancount reliable... NOT
      
      ReadBufferBlock(pBuf, nRawChancnt+1);
      return (nRawChancnt+2)*sizeof(long);

    } else {
      int datum;
      do {
	datum   = ReadBuffer;
	*pBuf++ = swaplong(datum);
	n++;
	datum = (datum >> 24) & 7;
      } while ( (datum == 0) && (n <= 34));
      if(datum != 4) {		// The trailer should be of type 4.
	cerr << " Data type in terminating long wrong: " << hex 
	     <<datum << dec << endl;
	m_nInvalidTrailers++;
      } 
      if ((n-2) < nRawChancnt) m_nChancountHigh++;
      if ((n-2) > nRawChancnt) m_nChancountLow++;
      Header &= 0xffffC0ff;	// Channel count is sometimes wrong.
      Header |= ((n-2) << 8);	// this fixes it.
      *pHeader = swaplong(Header);
      m_nEvents++;		// Count an event taken.
      return n * sizeof(long);			// Rely on the trailer!!
    }
  }else {
    // cerr << "Readout called but no data present\n";
    return 0;
  }
}


/*!
   Set an iped value.
   \param channel - Channel to set.
   \param value   - New value for pedestal only bottom 8 bits are used.

*/
void CAENcard::setIped(int value)
{
  setPedestalCurrent(value);
}
/*!
   Return the value of an Iped register.
   \param channel - Channel to get.
*/
int CAENcard::getIped()
{
  if((cardType() == 792) || (cardType() == 862)) {
    return ((volatile Registers*)m_pModule)->QDCIPedestal;
  } else {
    throw string("getIped - Module is not a V792 or V862 QDC");
  }

}
/*!
   Given the values in 
   - m_nSlot  - Geographical slot address.
   - m_nCrate - Physical crate number.
   - m_fGeo   - true if module should be initially accessed via geo.
   - m_nBase  - Base address (used if !m_fGeo).

   - Creates a map to the data buffer and registers of the module.
     and stores it in m_pModule.
   - Determines the module type by mapping the PROM memory and
     storing the module type (if legal) in m_nCardType.
   - If necessary programs the module's Geo register (!m_fGeo).
   - If necessary, programs the modules base address and re-maps
     the module for base address mapping (m_fGeo).

     \throw string describing why the function failed. 
*/
void CAENcard::MapCard()
{
  //ensure that the slot and crate specified stay within bounds

   m_nCrate = m_nCrate & 0xff;  //not important enough to give an error for, just discard the extra

   if(m_nSlot > VME_CRATE_SIZE)
   {
      throw string("Invalid slot number specified to MapCard(). ");
   }
   

     //the card is not mapped yet, so do it

   void* fd;

   volatile ROM *pRom;

   if( m_fGeo) {		// Geographical addressing...

     fd   = CVMEInterface::Open(CVMEInterface::GEO, m_nCrate);
     m_pModule= (volatile unsigned short*) 
                   CVMEInterface::Map(fd, 
				      (unsigned long)m_nSlot << 19, 
				      CAEN_REGISTERSIZE);
     pRom     = (ROM*)
                      CVMEInterface::Map(fd,
					 ((long)m_nSlot <<19) + 
					 CAEN_ROMOFFSET,
					 CAEN_ROMSIZE);

   }
   else {                                    // A32 addressing.
     fd = CVMEInterface::Open(CVMEInterface::A32, m_nCrate);
     m_pModule = (volatile unsigned short*) CVMEInterface::Map(fd,
							  m_nBase,
						 CAEN_REGISTERSIZE);
     pRom = (ROM*)CVMEInterface::Map(fd,
				     m_nBase + CAEN_ROMOFFSET,
				     CAEN_ROMSIZE);
      // The geographical address register must be programmed
     // in case the module or crate don't support geo.

     ((volatile Registers*)m_pModule)->GeoAddress = m_nSlot;

   }

   // Get the module type from the PROM and unmap the prom,
   // as that's all we use it for:


   unsigned int  msb = pRom->BoardIdMSB & 0xff;
   unsigned int  mid = pRom->BoardId & 0xff;
   unsigned int  lsb = pRom->BoardIdLSB & 0xff;

   m_nCardType = (msb << 16) | (mid << 8) | lsb;


   m_nSerialno  = pRom->SerialMSB << 8 |
                 pRom->SerialLSB;
   m_nHardwareRev = pRom->Revision;

   // We can now unmap the prom:
   
   CVMEInterface::Unmap(fd, (void*)pRom, CAEN_ROMSIZE);

   /* 
      To determine that the experimenter is not lying to us
      about how the VME is stuffed, we require that the module
      identifier be of a supported module type:
      V775, V785, V792, V862, or V1785 and that, for good measure,
      it's geographical address register match the slot
      the module is in.  For random data this should
      make a pretty miniscule chance that we'll be fooled
      by an empty slot. 1/32 * (1/2&24)
   */

   int nSlot = ((volatile Registers*)m_pModule)->GeoAddress & 0x1f;

   
   if(!(
	( m_nSlot  ==  nSlot )   &&
	( (m_nCardType == 775) || 
	  (m_nCardType == 785) || 
	  (m_nCardType == 792) ||
	  (m_nCardType == 862) ||
	  (m_nCardType == 1785))
	))   {   //either an invalid board or no board is present in this slot
     CVMEInterface::Unmap(fd, 
			  (void*)m_pModule, CAEN_REGISTERSIZE);
     
     CVMEInterface::Close(fd);

     char buffer[128];
     sprintf(buffer, "Card in crate %d, slot %d is incompatible or missing %d\n",
	     m_nCrate, m_nSlot, m_nCardType);
     throw string(buffer);
       
       string("Card is incompatable type or not inserted");
   }
   
   
   //  Now that we've accessed the card successfully,
   //  If the initial mapping was geographical, we need to
   //  re-map using our 'standard' address scheme.  Otherwise
   //  we won't be able to read the event buffer which makes
   //  the module hard to readout. Note that if !m_fGeo,
   //  we already have a non-geo map, so this section gets skipped.

   
   if(m_fGeo) {                              // Transition to A32...
     // Set the address registers of the module so that it will
     // recognize at m_nSlot << 24:

     ((volatile Registers*)m_pModule)->HighAddress = m_nSlot;
     ((volatile Registers*)m_pModule)->LowAddress  = 0;

     // Enable address recognition based on the address registers
     // (bypass the rotary switches).

     Bitset1(SELADDR);
     
     //destroy GEO24 mmap and file descriptor
     // Now remap using A32 addressing:


     CVMEInterface::Unmap(fd, (void*)m_pModule, CAEN_REGISTERSIZE);
     CVMEInterface::Close(fd);

     fd = CVMEInterface::Open(CVMEInterface::A32, m_nCrate);
     
     //create A32D16 mmap
     
     m_pModule = (volatile unsigned short *) 
     CVMEInterface::Map(fd, 
	  	        (long)m_nSlot << 24, 
			CAEN_REGISTERSIZE);

   }
   m_nFd = fd;			// Save for destruction.
  
}


/*!
   Unmap the card.  This is a required part of several operations
   including:
   - destruction
   - assignment
*/
void CAENcard::DestroyCard()
{

  CVMEInterface::Unmap(m_nFd, 
		       (void*)m_pModule,
		       CAEN_REGISTERSIZE);
  CVMEInterface::Close(m_nFd);

}

/*!
   Return the firmware revision info:
*/
int CAENcard::getFirmware() 
{
  if (!m_nFirmware) {

    m_nFirmware = ((Registers*)m_pModule)->FirmwareRev;

  }
  return m_nFirmware;
}
/*! 
   Set the fast clear window width.
   \param n (int [in]):
     the value to program in the FCLRWindow register.
*/
void
CAENcard::setFastClearWindow(int n)
{

  Registers* pRegs = (Registers*)m_pModule;
  pRegs->FCLRWindow = n;

}
/*!
   Enable the small threshold mode.
   In this mode, the threshold is shifted by only 1 bit
   to the left before being applied to the data word.
   See disableSmallThresholds to turn this off.
   The module default state on creation is small thresholds
   disabled.

  Note that the threshold registers will  not be modified as a result of
  this call.
*/
void
CAENcard::enableSmallThresholds()
{
  Bitset2(STEPTHR);
  m_fSmallThresholds = true;

}
/*!
  Disable the small threshold mode.
  In this mode, the threshold is shifted by 4 bits to the
  left before being applied to the data word.
  See enableSmallThresholds to turn this on.
  The default state on creation is small thresohlds enabled.

  Note that the threshold registers will  not be modified as a result of
  this call.
*/
void
CAENcard::disableSmallThresholds()
{
  Bitclear2(STEPTHR);
  m_fSmallThresholds = false;
}


/*!
   Set up a card's chain membership. 

   \param cbltaddr (int in) :
      The top 8 bits of the A24 address at which the chain
      will be recognized.
   \param where (ChainMember in):
      Where in the chain the module lives:
      - NotInChain - Module is not in a chain.
      - FirstInChain - Module is physically first in a chain.
      - LastInChain - Module is last in the chain.
      - IntermediateInChain - Module is in the middle of a chain.
*/
void
CAENcard::SetCBLTChainMembership(int cbltaddr,
				 CAENcard::ChainMember where)
{
  // Compute membership mask:

  int mask;
  switch (where) {
  case NotInChain:
    mask = 0;
    break;
  case FirstInChain:
    mask = MCSTFIRST;
    break;
  case LastInChain:
    mask = MCSTLAST;
    break;
  case IntermediateInChain:
    mask = MCSTFIRST | MCSTLAST;
    break;
  default:
    throw string("CAENcard::SetCBLTChainMembership: Invalid chain membership selector");
  }

  ((Registers*)m_pModule)->MCSTControl = mask;
  ((Registers*)m_pModule)->MCSTAddress = cbltaddr;
  ((Registers*)m_pModule)->Control1    = CTL1BERRENA;

}
/*!
   Return true if global data ready is set in SR1
*/
bool
CAENcard::gdataPresent()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status1 & GDREADY) != 0);


}
/*!
   Return true if this module is busy.
*/
bool
CAENcard::Busy()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status1 & BUSY) != 0);

}
/*!
  Return true if the global busy is set (at least one module
  in the set of modules with bussed busys is busy).
*/
bool
CAENcard::gBusy()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status1 & GBUSY) != 0);

}
/*!
   Return true if the module's MEB is completely full.
*/
bool
CAENcard::MEBFull()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status2 & SR2FULL) != 0);

}
/*!
  Return true if the module's MEB is empty.
*/
bool
CAENcard::MEBEmpty()
{

  volatile Registers* pReg = (volatile Registers*)m_pModule;
  return ((pReg->Status2 & SR2EMPTY) != 0);

}
/*!
   Cleare the internal statistics maintained by the object.
*/
void
CAENcard::ClearStatistics()
{
  m_nInvalidTrailers = 0;
  m_nChancountHigh   = 0;
  m_nChancountLow    = 0;
  m_nEvents          = 0;
}
/*!
   From time to time these modules are observed to not supply a 
   trailer in the event stream. This function returns the number
   of times this has happened since the last call to 
   ClearStatistics
*/
int
CAENcard::InvalidTrailerCount() 
{
  return m_nInvalidTrailers;
}
/*!
  From time to time these modules are observed to give a header
  with  a channel count larger than the actual number of channels
  read before either a trailer or an invalid word is read.  This
  function returns the number of times this has happened since the
  last call to ClearStatistics
*/
int
CAENcard::ChannelsTooBigCount()
{
  return m_nChancountHigh;
}
/*!
  From time to time these modules are observed to return more data
  than indicated by the channel count in their header.  This 
  function returns the number of times this happened since the
  last call to ClearStatistics
*/
int
CAENcard::ChannelsTooSmallCount()
{
  return m_nChancountLow;
}
/*!
  Returns the number of events read with data ready set since
  the last time ClearStatistics was called.
*/
int
CAENcard::EventCount()
{
  return m_nEvents;
}
/*!
    Set a mask in the bit set 1 register. 
   \param mask (short [in]):
      the mask to set.
*/
void
CAENcard::Bitset1(short mask) 
{

  volatile Registers* pRegisters = (volatile Registers*)m_pModule;
  pRegisters->BitSet1   = mask;

}
/*!
   Set a mask in the bit clear 1 register.
    \param mask (short [in]): 
      the mask to set
*/
void
CAENcard::Bitclear1(short mask)
{

  volatile Registers* pRegisters = (volatile Registers*)m_pModule;
  pRegisters->BitClear1   = mask;

}
/*!
   Set a mask in the bit 2 set register.
*/
void
CAENcard::Bitset2(short mask)
{

  volatile Registers* pRegisters = (volatile Registers*)m_pModule;
  pRegisters->BitSet2   = mask;

}
/*!
   Set a mask in the bit clear 2 register.
   \param mask (short [in]):
     The mask to set.
*/
void
CAENcard::Bitclear2(short mask)
{

  volatile Registers* pRegisters = (volatile Registers*)m_pModule;
  pRegisters->BitClear2   = mask;

}
