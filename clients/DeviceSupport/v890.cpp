#include <config.h>
#include <v890.h>
#include <string>
#include <CVMEInterface.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

static const unsigned int CAENPROMOffset(0x4000);
static const unsigned int assinine_delay(10);
const int CAENV890::m_nTdcClock(25);	// TDC clock in ns.
const int CAENV890::m_nTdcMaxVal(4095); // Maximum TDC value.


typedef u_char byte;


//   PROM layout

struct CAENProm {
  byte    m_pad0[0x26];		// 0x4000
  u_short m_OUIMsb;		// 0x4026
  u_short m_pad1;		// 0x4028
  u_short m_OUI;		// 0x402a
  u_short m_pad2;		// 0x402c
  u_short m_OUILsb;		// 0x402e
  u_short m_pad3;		// 0x4030
  u_short m_Version;		// 0x4032
  u_short m_pad4;		// 0x4034
  u_short m_BoardTypeMSB;	// 0x4036
  u_short m_pad5;		// 0x4038
  u_short m_BoardType;		// 0x403a
  u_short m_pad6;		// 0x403c
  u_short m_BoardTypeLSB;	// 0x403e
  byte    m_pad7[0x0e];		// 0x4040
  u_short m_Revision;		// 0x404e
  byte    m_pad8[0xeb2];	// 0x4050
  u_short m_SerialMsb;		// 0x4f02
  u_short m_pad9;		// 0x4f04
  u_short m_SerialLsb;		// 0x4f06
};
static const unsigned int CAENPROMSize(sizeof(CAENProm));


// Module register map.

struct CAENRegisters {
  u_long  m_OutputBuffer[0x1000/sizeof(u_long)]; // 0x0000
  u_short m_ControlRegister;	// 0x1000
  u_short m_StatusRegister;	// 0x1002
  u_short m_Ader32;		// 0x1004
  u_short m_Ader24;		// 0x1006
  u_short m_EnableAder;		// 0x1008
  u_short m_IPL;		// 0x100a
  u_short m_Vector;		// 0x100c
  u_short m_Geo;		// 0x100e
  u_short m_MCSTBase;		// 0x1010
  u_short m_MCSTCtl;		// 0x1012
  u_short m_Reset;		// 0x1014
  u_short m_Pad;		// 0x1016
  u_short m_Clear;		// 0x1018
  u_short m_SWEventReset;	// 0x101a
  u_short m_SWTrigger;		// 0x101c
  u_short m_EventCounter;	// 0x101e
  u_short m_EventStored;	// 0x1020
  u_short m_AlmostFullLvl;	// 0x1022
  u_short m_BLTEventNumber;	// 0x1024
  u_short m_FWRevision;		// 0x1026
  u_short m_TestLow;		// 0x1028
  u_short m_TestHigh;		// 0x102a
  u_short m_OutProgControl;	// 0x102c
  volatile u_short m_Micro;	// 0x102e
  volatile u_short m_MicroHandshake; // 0x1030

};
static const unsigned int CAENRegisterSize(sizeof(CAENRegisters));

// Status Register bits:

const u_short STAT_DATAREADY(1);


// Control register bits:

const u_short CTL_BERREN(1);
const u_short CTL_TERM(2);
const u_short CTL_TERMSW(4);
const u_short CTL_ENABLE_EMPTY(8);
const u_short CTL_ALIGN_64(16);
const u_short CTL_TEST(32);

// Micro handshake register:


const u_short UH_WRITE_OK(1);
const u_short UH_READ_OK(2);

// Data format bits:

const u_long DATA_GLOBALTYPE_MASK(0xc0000000);
const u_long DATA_GLOBALTYPE_GBLHEADER(0x40000000);
const u_long DATA_GLOBALTYPE_DATA(0x00000000);
const u_long DATA_GLOBALTYPE_GBLEOB(0x80000000);
const u_long DATA_GLOBALTYPE_FILLER(0xc0000000);

const u_long DATA_TDCTYPE_MASK(0xf8000000);
const u_long DATA_TDCTYPE_HEADER(0x08000000);
const u_long DATA_TDCTYPE_CHANNEL(0x00000000);
const u_long DATA_TDCTYPE_EOB(0x18000000);
const u_long DATA_TDCTYPE_ERROR(0x20000000);


/*!
   Constructor;  Create a V890 module.  The parameters are stored
   for use by the member functions that actually create the module.
   The parameters interact depending on the state of the
   geo parameter:

   \param nSlot (int [in]):
      The slot number for the device.  If the geo parameter is true,
      this is used to 
      - establish initial memory access to the device.
      - verify that the device is a V890
      - set the base address register to a predictable value and
        remap the device at that location.
     If geo is false, this value is programmed into the geo 
     register.  Note that this programming will only work if the
     module does not take it's geo address from the extended CERN
     p3 connector.
   \param nCrate (int [in] = 0):
      The VME crate number for the device.
   \param fGeo (bool [in] = true):
      If true the module is initially accessed in geographical mode
      and then reprogrammed into A32 space according to its slot.
      If false, the module is accessed soley through the address
      in the base parameter.
   \param lBase (unsigned long [in]):
      The base address to use to access the module if geo is false.
      This parameter is ignored if geo is true.
*/
CAENV890::CAENV890(int           nSlot,
		   int           nCrate,
		   bool          fGeo,
		   unsigned long lBase) :
  m_nSlot(nSlot),
  m_nCrate(nCrate),
  m_fGeographical(fGeo),
  m_lBase(lBase),
  m_pModule(0),
  m_pVmeDevice(0),
  m_nModuleType(0)
{
  MapModule();

}
   
/*!
   Destructor: destroys the memory map that was held on the device.
*/
CAENV890::~CAENV890() 
{
  if(m_pModule && m_pVmeDevice) { // Have mapping and file: unmap.
    CVMEInterface::Unmap(m_pVmeDevice,
			 m_pModule,
			 CAENRegisterSize);
  }
}

/*!
   Create mappings to both the module registers and prom
   given a device an the base address.  The register mappings
   will be returned in m_pModule and the prom map will
   be returned in the function return value.
   This function is a protected utility.
   \param pfd (void* [in]): 
      pointer to handle from CVMEInterface::Open
      (could be a geo or an A32 mapping).
   \param base (unsigned long[in]):
      Base address of the module (registers).
*/
void*
CAENV890::MapRegions(void* pfd, unsigned long base)
{
  m_pModule = CVMEInterface::Map(pfd, base, CAENRegisterSize);
  return      CVMEInterface::Map(pfd, base + CAENPROMOffset,
				 CAENPROMSize);
}


/*!
  Create a mapping to the module.  See the constructor for 
  information about how the member variables will be set up.
  The base address of the module will be put in m_pModule.

  \throw  string
    - "Can't open VME bus" if the vme bus could not be opened.
    - "Can't make Geo map" if the geographical map failed.
    - "Can't make A32 map" if the a32 map failed.
    - "Module is not V890" if the module address register is wrong.
*/
void
CAENV890::MapModule()
{
  // Open the vme crate.

  void* pDevice;
  CAENProm* pProm;
  unsigned long mapbase;
  try {
    if(m_fGeographical) {
      pDevice = CVMEInterface::Open(CVMEInterface::GEO, 
                                    m_nCrate);
      mapbase = m_nSlot << 19;
    }
    else {
      pDevice = CVMEInterface::Open(CVMEInterface::A32, 
                                    m_nCrate);
      mapbase = m_lBase;
    }
  }

  catch (string& reason) {
    throw (string("Can't open VME bus: ") + reason);
  }

  // Map to the module registers and prom initially.


  string MapFail1("Can't make ");

  MapFail1 += m_fGeographical ? "Geo map: " : "A32 map: ";

  try {
    pProm = (CAENProm*)MapRegions(pDevice, mapbase);

  }
  catch (string& reason) {
    throw (MapFail1 + reason);
  }
  
  m_pVmeDevice = pDevice;
  m_nModuleType = ReadModuleType(pProm);
 

  // Unmap the prom 
 
  CVMEInterface::Unmap(pDevice, pProm, CAENPROMSize);

  // The module must be a v890:

  if(m_nModuleType != 890) {
    CVMEInterface::Unmap(pDevice, m_pModule, sizeof(CAENRegisterSize));
    char message[100];
    sprintf(message, "Module in slot %d is not a V890",
	    m_nSlot);
    throw string(message);
  }
  Reset();			// Reset the module.

  // If the mapping was geographical, program base address and
  // remap as A32 with slot derived addressing.  This is needed in order to
  // allow us to access the buffer memory.  We choose a base address of
  //   slot << 24 
  // which allows us to just programthe geographical address in adrhigh and
  // leave adr low empty.
  //
  volatile CAENRegisters* pRegisters = (volatile CAENRegisters*)m_pModule;
  try {
    if(m_fGeographical) {
      pRegisters->m_Ader32 = m_nSlot; // Set the address base registers
      pRegisters->m_Ader24 = 0;
      pRegisters->m_EnableAder = 1;       // Enable address reloc.
      
      CVMEInterface::Unmap(m_pVmeDevice, m_pModule , CAENRegisterSize);
      CVMEInterface::Close(m_pVmeDevice);
      m_pVmeDevice = CVMEInterface::Open(CVMEInterface::A32, m_nCrate);
      pProm     = (CAENProm*)MapRegions(m_pVmeDevice, (m_nSlot << 24));

      int mtype = ReadModuleType(pProm);
      assert(mtype = m_nModuleType);

      CVMEInterface::Unmap(m_pVmeDevice, pProm,CAENPROMSize);

    }
    else {			// program geo address.
      pRegisters->m_Geo = m_nSlot;
    }
  }
  catch(string& reason) {
    string Exception("Couldn't remap geo->a32 ");
    Exception += reason;
    throw Exception;
  }
  
}


/*!
   Return the module slot as programmed in the hardware.  For a geographially
   addressed module, this will be the physical slot.  For a base addressed
   module (no JAUX), this will be the value programmed in the register.
*/
unsigned 
CAENV890::getSlotRegister() const
{
  return (((volatile CAENRegisters*)m_pModule)->m_Geo) & 0x1f;
}

/*!
   Set the empty event enable on or off.
   \param state (bool [in]):
      - true - Empty events will send header/trailer sets.
      - false- empty events are completely supressed (default on reset).
*/
void
CAENV890::EmptyEvent(bool state)
{
  u_short control = ((volatile CAENRegisters*)m_pModule)->m_ControlRegister;
  control        &= ~CTL_ENABLE_EMPTY;	// Turn off the bit...
  if(state)  control |= CTL_ENABLE_EMPTY;	// Turn it back on.
  ((volatile CAENRegisters*)m_pModule)->m_ControlRegister = control;
}
/*!
   Determine if empty event headers have been enabled.
   \return true if so.
*/
bool
CAENV890::EmptyEventOn()
{
  u_short control = ((volatile CAENRegisters*)m_pModule)->m_ControlRegister;
  return (control & CTL_ENABLE_EMPTY) != 0;
}
/*!
    Do a soft reset of the module.  Note that this operation will require
    about 1 second.
*/
void
CAENV890::Reset()
{
  ((volatile CAENRegisters*)m_pModule)->m_Reset = 0;
  usleep(1*1024*1024);		// That's a megasecond :-).
}
/*!
  Do a module clear:
*/
void
CAENV890::Clear()
{
  ((volatile CAENRegisters*)m_pModule)->m_Clear = 0;
}

/*!
    Read the module into an intenal buffer.
    The read from the memory buffer proceds until
    either there's no more data to read (signified by a filler word).
    or the buffer size is reached.

    \param nBufferSize (u_int [in]):
       Size of the buffer in longs.
    \param pBuffer (void* [in]):
       Pointer to the buffer

    \return unsigned int
       Number of words read.  0 if none were read.
*/
u_int
CAENV890::Read(u_int nBufferSize, void* pBuffer)
{
  u_long* p = (u_long*)pBuffer;
  u_long datum;
  u_int  nRead(0);

  volatile CAENRegisters* pModule = (CAENRegisters*)m_pModule;

  if(nBufferSize > sizeof(pModule->m_OutputBuffer)) {
    throw string("CAENV890::Read buffersize is too big");
  }

  while(((datum = pModule->m_OutputBuffer[nRead]) & DATA_GLOBALTYPE_MASK) 
	!= DATA_GLOBALTYPE_FILLER) {
    *p++ = datum;
    nRead++;
  }
  return nRead;
}

/*!
   Set the module into or out of test mode with a pattern.
   \param enable (bool [in]):
      true to enable test mode.
   \param nPattern (u_long [in] default = 0):
      if enable = true, nPattern is written in to the test register.
      otherwise it's ignored.
*/
void
CAENV890::TestMode(bool enable, u_long nPattern)
{
  volatile CAENRegisters* pModule=(volatile CAENRegisters*)m_pModule;
  
  if(enable) {
    pModule->m_ControlRegister |= CTL_TEST;
    pModule->m_TestLow = nPattern & 0xffff;
    pModule->m_TestHigh= nPattern >> 16;
  } 
  else {
    pModule->m_ControlRegister &= ~CTL_TEST;
  }
}
/////////////////////////////////////////////////////////////////////////
// 
// Functions that operateon acquisition modes:
//

/*!
   Load the default configuration into the TDC through the micro controller.
   This implies that:
   - Trigger matching is disabled.
   - Window width is 500ns
   - Window Offset i -1usec.
   - Reject margin is 100ns
   - Extra search margin is 200ns
   - All channels are enabled.
*/
void
CAENV890::LoadDefaultConfig()
{

  // Write the command code:
  
  WriteMicro(0x0500);	// Load the code.
}
/*!
    Return true if trigger matching is enabled.
    This involves a command write of 0x0200 and a read
    of data from the micro register.
*/
bool
CAENV890::isTriggerMatching()
{
  WriteMicro(0x0200);


  u_short data = ReadMicro();

  return ((data & 1) != 0);

}
/*!
    Read the trigger configuration from the tdc.
    \param pTriggerInfo (short* [out]):
      An array of 5 words to hold the trigger information:
      - 0 The match window.
      - 1 The trigger offset.
      - 2 The extra search window 
      - 3 The reject margin
      - 4 LSB = 1 if trigger time subtraction enabled

*/
void 
CAENV890::ReadTriggerConfig(TriggerInfo* pTriggerInfo)
{
  ReadMicroArray(0x1600, 5, (short*)pTriggerInfo);

}
/*!
   Enables the module to run in trigger match mode.  See section 2.2
   of the manual for more information.  This mode is mutually exclusive
   from Continuous storage mode (see SetContinousStorageMode()).
*/
void
CAENV890::SetTriggerMatchingMode()
{
  WriteMicro(0x0000);
}
/*!
  Enables the module to run in continuous storage mode (see ection 2.2 of the
  manual for more infomration).  This mode is mutually exclusive from
  trigger match mode (see SetTriggerMatchingMode()).
*/
void
CAENV890::SetContinuousStorageMode()
{
  WriteMicro(0x0100);
}
/*!
   Determines if the module is in trigger matching mode.
   \return bool
   - true - the module is in trigger match mode.
   - false- the module is in continuous storage mode.
*/
bool
CAENV890::isTriggerMatchingMode()
{
  u_short mode;
  ReadMicroArray(0x0200, 1, &mode);
  return ((mode & 1) == 1);
}

//////////////////////////////////////////////////////////////////////////
//   Functions that operate on the channel enables:

/*!
   Enable an individual channel
   \param nChan (int [in]): 
     The channel to enable.
*/
void
CAENV890::EnableChannel(int nChan)
{
  if(nChan > 127) {
    char message[80];
    sprintf(message, "CAENV890::EnableChannel channel value %d out of range",
	    nChan);
    throw string(message);
  }
  u_short op = 0x4000 | nChan;
  WriteMicro(op);
}
/*!
  Disable an individual channel:
  \param nChan (int [in]):
     Selects the channel to disable.
*/
void
CAENV890::DisableChannel(int nChan)
{
  if(nChan > 127) {
    char message[80];
    sprintf(message, "CAENV890::DisableChannel channel value %d out of range",
	    nChan);
  }
  u_short op = 0x4100 | nChan;
  WriteMicro(op);
}
/*!
   Enable all channels.
*/
void
CAENV890::EnableAllChannels()
{
  WriteMicro(0x4200);
}
/*!
  Disable all channels.
*/
void
CAENV890::DisableAllChannels()
{
  WriteMicro(0x4300);
}
/*!
  Set the enables on all channels in a single operation.
  \param pEnables (u_short* [in]):
      An array of 8 channel enable masks:
      - Word 0 has channels 0-15 (low to high bit order).
      - Word 1 has channels 16-31
      - ...
*/
void
CAENV890::SetChannelMask(u_short* pMask)
{
  WriteMicroArray(0x4400, 8, pMask);

}
/*!
   Read the channel enables pattern from the TDC Micro controller.
   \param pEnables (u_short* [out]):
      array into which the masks will be stored. Should be 8 elements.
      word 0 has 0-15, 1 0-16 etc. low bits -> high bit order.
*/
void
CAENV890::ReadChannelEnables(u_short* pTriggerInfo)
{
  ReadMicroArray(0x4500, 8, pTriggerInfo);
}
/*!
   Set the trigger window width.
   \param ns (int [in]):
      ns to set the trigger window to.  Note that the resolution of the
      trigger window is 25ns, so this is <em>rounded</em> to the nearest
      25ns.
   \throw string
      if the ns parameter is not in the range 0-102400 ns.
*/
void
CAENV890::SetWindowWidth(int ns)
{
  int MaxNs = TicksToNs(m_nTdcMaxVal);
  if(ns > MaxNs) {
    char message[100];
    sprintf(message, "CAENV890::SetWindowWidth %d out of range of 0-%d",
	    ns, MaxNs);
    throw string(message);
  }
  int steps = NsToTicks(ns);
  if(steps > 4095) {
    char message[100];
    sprintf(message, 
	    "CAENV890::SetWindowWidth computed with value %d out of [0,%d]",
	    steps, m_nTdcMaxVal);
  }
  WriteMicroArray(0x1000, 1, &steps);
  
}
/*!
  Get the trigger window width currently set in the module.
  \return int
    ns of trigger width.
*/
int
CAENV890::GetWindowWidth()
{
  TriggerInfo TriggerData;;
  ReadTriggerConfig(&TriggerData);
  int steps = TriggerData.Window;
  return TicksToNs(steps);
}
/*!
   Set the window offset of the tdc:
   \param ns - (int [in]):
      Window offset in ns.  This is converted to nearest
      25ns clock tick prior to programming.
   \throw string
      Exceptions if the resulting clock tick count is out of the
      valid range of -2048 to 40.
*/
void
CAENV890::SetWindowOffset(int ns)
{
  int ticks = NsToTicks(ns);
  if ((ticks < -2048) || (ticks > 40) ) {
    char msg[100];
    sprintf(msg, "CAENV890::SetWindowOffset offset %d is out of range",
	    ns);
    throw string(msg);
  }

  WriteMicroArray(0x1100, 1, &ticks);
}
/*!
   Get the current value of the tdc window offset.
   \return int 
     Window in ns.
*/
int
CAENV890::GetWindowOffset()
{
  TriggerInfo info;
  ReadTriggerConfig(&info);
  return TicksToNs(info.Offset);
}
/*!
   Enable trigger subtraction mode.
*/
void
CAENV890::EnableTriggerSubtraction()
{
  WriteMicro(0x1400);
}
/*!
  Disable trigger time subtraction.
*/
void
CAENV890::DisableTriggerSubtraction()
{
  WriteMicro(0x1500);
}
/*!
   Return the state of the trigger subtraction mode.
   \return bool
    - true trigger subtraction is enabled.
    - false trigger subtraction is disabled.
*/
bool
CAENV890::TriggerSubtractionEnabled()
{
  TriggerInfo info;
  ReadTriggerConfig(&info);
  return (info.SubtractionEnabled & 1) != 0;
}
/*!
   Set edge detection mode.  
   \param edge (CAENV80::EdgeDetectSelection [in]):
   Desired edge detection mode.
   \throw string
     If the detection mode is invalid (ints look too much like enums).
*/
void
CAENV890::SetEdgeDetection(CAENV890::EdgeDetectSelection edge)
{
  if((edge < EdgePairs) || (edge > EitherEdge)) {
    throw string("CAENV890::SetEdgeDetection invalid edge selector");
  }
  WriteMicroArray(0x2200, 1, &edge);
}
/*!
   Retrieve the edge detection mode.
   \return CAENV890::EdgeDetectSelection
   the current mode.
*/
CAENV890::EdgeDetectSelection
CAENV890::GetEdgeDetection()
{
  short value;
  ReadMicroArray(0x2300, 1, &value);
  return (EdgeDetectSelection)(value & 0x3);
}
/*!

    Set the module resolution.
    \param res (CAENV890::ResolutionSelection [in]):
       Resolution to select. 
    \throw string
       Throws an error message if the resolution is illegal.
*/
void
CAENV890::SetResolution(CAENV890::ResolutionSelection res)
{
  if((res < ps800) || (res > ps100)) {
    throw string("CAENV890::SetResolution invalid resolution value");
  }
  WriteMicroArray(0x2400, 1, &res);

}
/*!
  Get the module resolution.
  \return CAENV890::ResolutionSelection
*/
CAENV890::ResolutionSelection
CAENV890::GetResolution()
{
  short res;
  ReadMicroArray(0x2600, 1, &res);
  return (ResolutionSelection)(res & 0x3);
}
/*!
   Enable the ADC to produce a header and EOB (collectively called
   delimieters).
*/
void
CAENV890::EnableDelimeters()
{
  WriteMicro(0x300);
}
/*!
   Disable the ADC production of header and EOB (collectively called
   delimeters).
*/
void
CAENV890::DisableDelimeters()
{
  WriteMicro(0x3100);
}
/*!
   Determine if delimeters are on or off (see EnableDelimeters and
   DisableDelimeters).
   \return bool
    - true if delimeters are enabled.
    - false if delimeters are disabled.
*/
bool
CAENV890::DelimetersEnabled()
{
  short flag;
  ReadMicroArray(0x3200, 1, &flag);
  return (flag & 1) != 0;
}
/*!
  Set the maximum number of hits that will be returned for an event.
  This value is a per TDC value if I read the manual correctly, that is
  If you set a value of 1, you could get 4 hits.  See 5.5.5 and read
  carefully!!!
  \param hits (CAENV890::MaxHitSelection [in]):
     Selects the maximum number of hits to accept.
  \thros string
     error messgae if hits is invalid.
 */
void
CAENV890::SetMaxHits(CAENV890::MaxHitSelection hits)
{
  if(hits > HitMaxNoLimit) {
    throw string("CAENV890::SetMaxHits - Invalid value for hits parameter");
  }
  WriteMicroArray(0x3300, 1, &hits);
}

/*!
   Returns the maximum number of hits that will be returned for an event.
   See SetMaxHits for more information about the trickiness of this function.
*/
CAENV890::MaxHitSelection
CAENV890::GetMaxHits()
{
  short hits;
  ReadMicroArray(0x3400, 1, &hits);
  return (MaxHitSelection)(hits & 0xf);
}
/*!
   Write a word to the micro controller.
   - Loop until the UH_WRITE_OK bit goes true in the handshake register.
   - Wait an assinine and undocumented 12ms
   - write the data.

   \param opcode (u_short [in]):
      The opcode to write.
*/
void
CAENV890::WriteMicro(u_short opcode)
{
  volatile CAENRegisters* pModule = (volatile CAENRegisters*)m_pModule;

  while((pModule->m_MicroHandshake & UH_WRITE_OK) == 0)
    ;
  usleep(assinine_delay);
  pModule->m_Micro = opcode;
}

/*!
   Read a word from the micro controller.
   - Loop until UH_READ_OK is set in the handshake register.
   - Wait an assinine and undocumented 12ms
   - read the data.

   \return  u_short
      The data from the micro.
*/
u_short
CAENV890::ReadMicro()
{
  volatile CAENRegisters* pModule = (volatile CAENRegisters*)m_pModule;
  while((pModule->m_MicroHandshake & UH_READ_OK) == 0)
    ;
  usleep(assinine_delay);
  return pModule->m_Micro;
}
/*!
   Utility function to do a read operation on the micro sequencer that
   returns several data items.
   \param opcode (u_short [in]): 
      The read opcode to perform.
   \param nWords (u_short [in]):
      The number of words to read after the opcode.
   \param pBuffer (void* [out]):
      pointer to the buffer to hold the results of the operation.
*/
void
CAENV890::ReadMicroArray(u_short opcode, u_short nWords, void* pBuffer)
{
  u_short* p((u_short*)pBuffer);
  WriteMicro(opcode);
  for(int i=0; i < nWords; i++) {
    *p++ = ReadMicro();
  }
}
/*!
   Utility function to write a set of words to the micro.
   \param opcode (u_short [in]): 
      The read opcode to perform.
   \param nWords (u_short [in]):
      The number of words to write after the opcode.
   \param pBuffer (void* [out]):
      pointer to the buffer to of data words to write.

*/
void
CAENV890::WriteMicroArray(u_short opcode, u_short nWords, void* pBuffer)
{
  u_short* p((u_short*)pBuffer);
  WriteMicro(opcode);
  for(int i =0; i < nWords; i++) {
    WriteMicro(*p++);
  }
}
/*!
    Utility function to read the module type given a pointer to the PROM.
*/
int
CAENV890::ReadModuleType(void* prom)
{
  CAENProm* p((CAENProm*)prom);
  return ((p->m_BoardTypeMSB & 0xff) << 16) |
         ((p->m_BoardType & 0xff)    <<  8)   |
         ((p->m_BoardTypeLSB & 0xff) );
}
/*!
   Utility to convert ticks to ns. 
   \param ticks (int [in]):
      Number of m_nTdcClock ns clocks to convert.
   \return int
      ns represented by ticks.

This conversion will be exact!!
*/
int
CAENV890::TicksToNs(int ticks)
{
  return ticks*m_nTdcClock;
}
/*!
   Utility to convert ns to ticks.  
   \param ns (int [in]):
      Number of ns to convert.
   \return int
      nearest number of clock ticks represented by ns.
*/
int
CAENV890::NsToTicks(int ns)
{
  // ns can be signed!!!!

  bool negative = ns < 0;
  int  rounder  = (negative ? -1 : 1)*m_nTdcClock/2;
  int ticks;
  return (ns + rounder)/m_nTdcClock;

}
/*!
   Test for data in the output buffer.
*/
bool
CAENV890::DataReady()
{
  CAENRegisters* pModule = (CAENRegisters*)m_pModule;
  return (pModule->m_StatusRegister & STAT_DATAREADY) != 0;
}
