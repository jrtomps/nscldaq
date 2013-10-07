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
#include "CXLM.h"

#include "CReadoutModule.h"

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include<iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
/////////////////////  Class level constants  ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//  These constants define bits in the XLM bus request/ownership register:

const uint32_t CXLM::REQ_A(0x00000001);
const uint32_t CXLM::REQ_B(0x00000002);
const uint32_t CXLM::REQ_X(0x00010000);

/////////////////////////////////////////////////////////////////////////////
/////////////////////  File level constants /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// The XLM memory layout relative to the base address:

// Address modifiers we are going to use:

static const uint8_t   registerAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   sramaAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   blockTransferAmod(CVMUSBReadoutList::a32UserBlock); 

//  Base addresses of 'unstructured' areas.

static const uint32_t  SRAMA(0x000000);	//  Base address of static ram A
static const uint32_t  SRAMB(0x200000);	//  Base address of static RAM B
static const uint32_t  FPGABase (0x400000); //  Base address of FPGA `register' set.
static const uint32_t  DSP  (0x600000); //  Base address of DSP interface.
static const uint32_t  InterfaceBase(0x800000);

// Interface layout:

static const uint32_t  BusRequest (0x800000); // Register for submitting bus requests.
static const uint32_t  Interrupt  (0x800004); // Interrupt/reset register.
static const uint32_t  FPGABootSrc(0x800008); // Select boot source for FPGA.
static const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.
static const uint32_t  BUSAOwner  (0x810000); // Shows who has bus A (SRAM A)
static const uint32_t  BUSBOwner  (0x810004); // Shows who has bus B (SRAM B)
static const uint32_t  BUSXOwner  (0x810008); // Shows who has bus X (FPGA).
static const uint32_t  IRQSerial  (0x820048); // Write for IRQ id reads serial number.
static const uint32_t  POLLISR    (0x820824); // 'mailbox' betweenFPGA and DSP.

// Note that the REQ_A/B/X definitions above define the bits in the bus request register
// therefore we don't define these bits here.

// Bits in Interrupt register; Note these are 'negative logic bits' in that to assert
// the reset, the bit shown below should be set to zero.

static const uint32_t InterruptResetFPGA    (0x00000001);
static const uint32_t InterruptResetDSP     (0x00000002);
static const uint32_t InterruptInterruptFPGA(0x00010000);
static const uint32_t InterruptInterruptDSP (0x00020000);

// Boot source register contents.  This defines where the FPGA loads its
// microcode from.  There are 4 ROM locations as well as the possibility
// to load the microcode in to SRAM A.  This is coded rather than bits.

static const uint32_t BootSrcRom0 (0x00000000);	// Load from sector 0 of PROM.
static const uint32_t BootSrcRom1 (0x00000001);	// Load from sector 1 of PROM.
static const uint32_t BootSrcRom2 (0x00000002); // Load from sector 2 of PROM.
static const uint32_t BootSrcRom3 (0x00000003); // Load from sector 3 of PROM.
static const uint32_t BootSrcSRAMA(0x00010000);	// Load from SRAM A image.

// The ForceOffBus register only has a single bit.... the LSB.  When set, the FPGA
// and DSP are forced off the bus.  When clear they are allowed to arbitrate for the bus.
// XLM Manual 4.3.1.4 states that even when that bit is set the VME bus host must arbitrate
// for the bus via the BusRequest register.

static const uint32_t ForceOffBusForce(0x00000001); // Force all but VME off bus.

// BUS*Owner values.  These describe who is actually the owner of the specific bus.

static const uint32_t BusOwnerNone(0);	// Bus not owned.
static const uint32_t BusOwnerVME (1);	// VME host owns bus.
static const uint32_t BusOwnerFPGA(2);	// Bus owned by FPGA.
static const uint32_t BusOwnerDSP (3);	// Bus owned by DSP.

// The IRQ/serial number register is some mess of bits; both in/out
// we're just going to define a function for extracting the serial number from a read:

static inline uint32_t serial(uint32_t rawRegister)
{
  return (rawRegister && 0x1ff) >> 16; // Bits 16-25 of the 32 bit data word 4.3.1.6 Xlm manual
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonical methods ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*!
  Default construction just ensures that references to the configuration will
  SEGFault until the configuration has been attached:
*/
CXLM::CXLM() :
  m_pConfiguration(0)
{}
/*!
  The copy constructor will clone the configuration if it's define in the
  object being copied:
  @param rhs  The object that we will be making a copy of.
*/
CXLM::CXLM(const CXLM& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
  }
}
/*!
  Destruction:
  Well if we were copy constructed, we will leak our configuration.
  Otherwise the configuration is managed by the guy that owned us.
  If ambitious we could deal with this by having a flag to determine if
  we were copy constructed... however for now just let this defect be.

*/
CXLM::~CXLM()
{
}

/*!
   Assignment.. we're just going return *this .. really assignment should
   not have been implemented. There's evidently an infinite recursion loop if we
   actually try to implement operator=.
   @param rhs What we are `assigning' to *this.
   @return CXLM&
   @retval *this
*/
CXLM&
CXLM::operator=(const CXLM& rhs)
{
  return *this;
}
////////////////////////////////////////////////////////////////////////////////////
///////////////// CReadoutHardware interface methods   /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
   Note that CXLM is an abstract base class.  This means that we won't be 
   implementing all of the CReadoutHardwareFunctions... only a minimal set that
   we need for all XLM applications.
*/

/*!
  Handle the attachment of the configuration object to our object.
  We're going to define two parameters, the base address and the path to the
  firmware file.  The firmware file will be validated with our custom validator.
  @param configuration Reference to the configuration object.

*/
void
CXLM::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration; // Save for later use

  // Define our base configuration parameters.

  configuration.addParameter("-base", CConfigurableObject::isInteger, NULL, "0");

  configuration.addParameter("-firmware",
			     validFirmwareFile, NULL, "");
}

/////////////////////////////////////////////////////////////////////////////////
///////////////// Utility functions for derived classes /////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
  Load a firmware file intothe FPGA.  Normally this will be used by:
  loadFirmware(vmusb, cget("-firmware")); .. that is loading the firmware that
  has been configured via the -firmware configuration parameter.

  @param controller  - Reference to a CVMUSB controller object.
  @param path Path to the firmware file.  This has to have been validated as existing
              and readable by the caller.  Note that if this value came from
	      -firmware, this has already been done by the configuration subsystem.
  @exception std::string If there is an error.
*/
void 
CXLM::loadFirmware(CVMUSB& controller, string path) throw(std::string)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  cerr << hex << "Loading firmware for XLM at " << base << endl << dec;

  // Prep the FPGA for loading.  Specifically:
  // 1. Set the load source to SRAMA
  // 2. Force a falling edge on the FGPA Reset.. while not modifying the state of the other
  //    components....and hold the FPGA reset.
  // 3. Force the FPGA off the bus.
  // 4. Request the bus to SRAM A.
  // With the exception of determining the state of the Interrupt/reset register so it can be manipulated,
  // these operations are done in an immediate list:

  // Get the state of the interrupt register.. well it's another crappy write only reigster so
  // let's assume everything is held reset and that we only want to start the fpga.

  uint32_t interruptRegister = 0; /* InterruptResetFPGA | InterruptResetDSP |
				     InterruptInterruptFPGA | InterruptInterruptDSP */

  // Build the list of operations:
  // Done in a block so the list is destroyed after it's executed:

  {
    CVMUSBReadoutList initList;
    initList.addWrite32(base + ForceOffBus, registerAmod, ForceOffBusForce); // Inhibit FPGA Bus access.
    initList.addWrite32(base + Interrupt,   registerAmod,  InterruptResetFPGA); // Hold FPGA reset.
    initList.addWrite32(base + FPGABootSrc, registerAmod, BootSrcSRAMA); // Set boot source
    addBusAccess(initList,  REQ_A, 0);                                    //  Request bus A.

    
    // run the list:

    size_t dummy;		// For read buffer.

    int status = controller.executeList(initList,
					&dummy, sizeof(dummy), &dummy);
    if (status != 0) {
      string reason = strerror(errno);
      string msg = "CXLM::loadFirmware - failed to execute initialization list: ";
      msg       += reason;

      throw msg;
    }

    // I should have bus a:

    uint32_t owner =0;
    controller.vmeRead32(base + BUSAOwner, registerAmod, &owner);
    cerr << "BUSA Owner is: " << owner << endl;

    // Open and read the entire fpga file into memory (can't be too large)



    uint32_t  bytesInFile = fileSize(path);
    uint8_t*  contents    = new uint8_t[bytesInFile];
    uint32_t* sramAImage  = new uint32_t[bytesInFile]; // Each byte becomes anSRAM Longword.
    memset(sramAImage, 0, bytesInFile * sizeof(uint32_t));

    // The remainder is in a try block so we can delete the file contents:

    try {
      // Read the file, convert it to an sram a image and load it into SRAM A:

      loadFile(path, contents, bytesInFile);	// Read the file into memory.

      // Skip the header:

      uint8_t* pc = contents;
      while (*pc != 0xff) {
	pc++;
	bytesInFile--;
      }

      remapBits(sramAImage, pc, bytesInFile);
      loadSRAMA(controller, sramAImage, bytesInFile*sizeof(uint32_t));
      
      // Release the SRAMA Bus, 
      // release the 'force'.
      // Remove the reset from the FPGA:
      
      cerr << "Firmware loaded in SRAMA\n";

      CVMUSBReadoutList  finalize;
      finalize.addWrite32(base + BusRequest, registerAmod, (uint32_t)0);	// Release bus request.
      finalize.addWrite32(base + ForceOffBus, registerAmod, (uint32_t)0); // Remove force
      finalize.addWrite32(base + Interrupt, registerAmod, (uint32_t)0);	// Release FPGA reset 
      status = controller.executeList(finalize,
				       &dummy, sizeof(dummy), &dummy);
				       
      if (status != 0) {
	string reason = strerror(errno);
	string message = "CXLM::loadFirmware failed to execute finalization list: ";
	message       += reason;
	throw message;
      }
    }
    catch (...) {
      delete []contents;
      delete []sramAImage;
      throw;			// Let some higher creature deal with this.
    }
    delete []contents;
    delete []sramAImage;
    cerr << "FPGA Should now be started\n";

  }
  

}

/*!
  Access some combination of the busses.  The assumption is that since this
  is a one shot, and since I'm ensured that I'll get the bus within 200ns, the host
  will not be able to turn the read around faster than that and therefore I don't need
  to check that I actually have the bus.

  Note this can be used to release the busses by not setting the appropriate 
  bus request bits.

  @param controller - VMUSB controller object.
  @param accessPattern - bitwise oro of one of the following:
  - CXLM::REQ_X   - Request the X (FPGA) bus.
  - CXLM::REQ_A   - Request the A (SRAM A) bus.
  - CXLM::REQ_B   - Request the B (SRAM B) bus.

*/
void
CXLM::accessBus(CVMUSB& controller, uint32_t accessPattern)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  controller.vmeWrite32(base + BusRequest, registerAmod, accessPattern);
}

/*!
  Adds a bus access request to a readout list.
  @param list - Reference to a VMUSBReadout list to which the appropriate instructions will be added.
  @param accessPattern - The requested access pattern, See accessBus above for a description of this.
  @param delay - Delay to insert after the bus request.  This should be the worst case time between
                 arbitration request and grant..determined by the FPGA firmware you are using.
		 delay is in 12.5ns units.
*/
void
CXLM::addBusAccess(CVMUSBReadoutList& list, uint32_t accessPattern,
		   uint8_t delay)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  list.addWrite32(base + BusRequest, registerAmod, accessPattern);
  if ( delay > 0) {		// Don't delay if none requested.
    list.addDelay(delay);
  }

}

/*! 
  Convenience function to return the base address of SRAM A
  @return uint32_t
  @retval -base + SRAMA

*/
uint32_t
CXLM::sramA()
{
  return m_pConfiguration->getUnsignedParameter("-base") + SRAMA;
}
/*!
  Convenience function to return the base address of SRAM B
  @return uint32_t
  @retval -base + SRAM
*/
uint32_t 
CXLM::sramB()
{
  return m_pConfiguration->getUnsignedParameter("-base") + SRAMB;
}
/*!
  Convenience function to return the base address of the FPGA register set.  These are the 
  registers that are maintained by the FPGA firmware, not the interface registers.
  @return uint32_t
  @retval -base + FPGA
*/
uint32_t
CXLM::FPGA()
{
  uint32_t b =    m_pConfiguration->getUnsignedParameter("-base");
  return b + FPGABase;
}
/*!
  Convenience function to return the interface register set base... though really low level access functions
  to those registers ought to be added to this base class with application level functions only in the 
  derived class
  @return uint32_t
  @return -base + Interface

*/
uint32_t
CXLM::Interface()
{
  return m_pConfiguration->getUnsignedParameter("-base")  + InterfaceBase;
}
//////////////////////////////////////////////////////////////////////////////////////////
//
// Private utility functions.
//

/**
 * Determines if a file is a valid firmware file.  At this point we just use access(2) to see if
 * the file exists and is readable by us.  This is a configuration validator and hence must
 * fit in with that call signature:
 *
 * @param name - Name of the configuration parameter that holds the file.
 * @param value - Path to the file
 * @param arg - Unused optional parameter 
 * @return bool
 * @retval true - Firmware file is valid.
 * @retval false - Firmware file is not valid.
 */
bool 
CXLM::validFirmwareFile(string name, string value, void* arg)
{
  int status = access(value.c_str(), R_OK);

  // TODO: Some day we should figure out a better way for parameter checkers to get error information
  //       back to the configuration subsystem.. e.g. errno would exactly describe what was wrong here.

  return (status == 0);
}
/**
 * Returns the size of a file.  The file must already exist and  be a valid target for 
 * stat(2). As this is used in the firmware load process, this has typically been assured by
 * a call to validFirmwareFile.
 * @param path - Absolute or relative path to the firmware file.
 * @return size_t
 * @retval Number of bytes in the file.  This includes 'holes' if the file is spares.
 * @throw std::string - if stat fails.
 */
uint32_t
CXLM::fileSize(string path) throw(std::string)
{
  struct stat fileInfo;
  int status = stat(path.c_str(), &fileInfo);
  if(status) {
    string msg = strerror(errno);
    string error = "CXLM::fileSize - Unable to stat firmware(?) file: ";
    error       += msg;
    throw error;
  }
  return static_cast<uint32_t>(fileInfo.st_size); // Limited to 4Gbyte firmware files should be ok ;-]
}
/**
 * loads a firmware file into memory.  By this time the file is known to exist and be readable
 * and the size has been determined so:
 * @param path - Absolute or relative path to the file.
 * @param contents - Pointer to a buffer that will hold the file.
 * @param bytes - Number of bytes in the file.
 * @throw std::string on any system call error.
 */
void
CXLM::loadFile(std::string path, void* contents, uint32_t size) throw(std::string)
{
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    string error = strerror(errno);
    string msg   = "CXLM::loadFile - Failed to open the file: ";
    msg         += error;
    throw msg;
  }

  // read can be partial... this can happen on signals or  just due to buffering;
  // therefore the loop below is the safe way to use read(2) to load a file.
  // TODO:  We need to make a library of such 'simple little things'... as I'm sure I've used
  //        loops like this in several places in my life.
  // 
  uint8_t* p = reinterpret_cast<uint8_t*>(contents); // qty read on each read(2) call are bytes.
  try {
    while (size > 0) {
      ssize_t bytesRead = read(fd, p, size);
      if (bytesRead < 0) {
	// only throw if errno is not EAGAIN or EINTR.
	
	int reason = errno;
	if ((reason != EAGAIN) && (reason != EINTR)) {
	  string error = strerror(reason);
	  string msg   = "CXLM::loadFile - read(2) failed on firmware file: ";
	  msg         += error;
	  throw msg;
	}
      }
      else {
	size -= bytesRead;
	p    += bytesRead;
      }
    }
  }
  catch (...) {
    // Close the file... and rethrow.
    
    close(fd);
    throw;
  }
  // close the file

  close(fd); 			// should _NEVER_ fail.
}
/**
 * Remap the bits of a firmware file into an SRAM image.  Each byte maps to some scattered set of
 * bits in an SRAM longword.  The mapping is defined by the table bitMap below.
 * @param sramImage - Buffer to hold the sramImage.
 * @param fileImage - Buffer holding the raw file contents.
 * @param bytes     - Number of bytes in fileImage.  It is up to the caller to ensure that the
 *                    size of sramImage is at least bytes*sizeof(uint32_t) big.
 */
void
CXLM::remapBits(void* sramImage, void* fileImage, uint32_t bytes)
{
  static const uint32_t bitMap[]  = {
    0x4, 0x8, 0x10, 0x400, 0x40000, 0x80000, 0x100000, 0x4000000
  };

  uint8_t*   src  = reinterpret_cast<uint8_t*>(fileImage);
  uint32_t*  dest = reinterpret_cast<uint32_t*>(sramImage);

  for(uint32_t i =0; i < bytes; i++) {
    uint32_t  lword = 0;		// build destination here.
    uint8_t   byte  = *src++;	// Source byte
    uint32_t  bit   = 1;
    for (int b = 0; b < 8; b++) { // remap the bits for a byte -> longword
      if (byte & bit) {
	lword |= bitMap[b];
      }
      bit = bit << 1;
    }
    *dest++ = lword;		// set a destination longword.
  }

}
/**
 * given a block of bytes, loads it into SRAMA.
 * - This is done 256bytes at a time.
 * - This block transfers are done.
 * - For now we don't trust Jan to do block writes larger than that quite correctly.
 * - We _do_ build a list of block writes and execute them all at once.
 * - Assumption: the image is an even multiple of uint32_t.
 *
 * @param controller - CVMUSB controller object reference.
 * @param image      - Bytes to load in the SRAMA
 * @param bytes      - sizeof the image in bytes.
 * @throw std::string - in the event of an error executing a VM-USB list.
 */
void
CXLM::loadSRAMA(CVMUSB& controller, void* image, uint32_t bytes) throw(std::string)
{
  static const size_t   blockSize = 256;
  static const size_t   vblockSize = blockSize;
  uint32_t              nBytes    = bytes;

  // for now load it one byte at a time... in 256 tansfer chunks:

  

  uint32_t*           p    = reinterpret_cast<uint32_t*>(image);
  static uint32_t    dest = sramA();

  cerr << hex << "LOADSRAMA - SRAM A base address is " << dest << endl << dec;

  if (bytes == 0) return;	// Stupid edge case but we'll handle it correctly.

  while (bytes > blockSize*sizeof(uint32_t)) {
    CVMUSBReadoutList  loadList;
    for (int i =0; i < blockSize; i++) {
      loadList.addWrite32(dest, sramaAmod,  *p++);
      dest += sizeof(uint32_t);
    }
    bytes -= blockSize*sizeof(uint32_t);
    // Write the block:

    size_t data;
    int status = controller.executeList(loadList,
					&data,
					sizeof(data), &data);
    if (status < 0) {
      string error = strerror(errno);
      string msg   = "CXLM::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }

  }
  // Handle any odd partial block:

  if (bytes > 0) {
    CVMUSBReadoutList loadList;
    while (bytes > 0) {
      loadList.addWrite32(dest, sramaAmod, *p++);
      dest += sizeof(uint32_t);
      bytes -= sizeof(uint32_t);
    }
    // Write the block:

    size_t readData;
    int status = controller. executeList(loadList, &readData, sizeof(size_t), &readData);
    if (status < 0) {
      string error = strerror(errno);
      string msg   = "CXLM::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }
  }

  return;

  /// Some tests for Jan.

  uint32_t* compareData = new uint32_t[blockSize];
  uint32_t src         = sramA();
  size_t   bytesRead;
  size_t   bytesLeft   = nBytes;

  while(bytesLeft > blockSize * sizeof(uint32_t)) {
    CVMUSBReadoutList verifyList;
    verifyList.addBlockRead32(src, blockTransferAmod,
			      blockSize);
    verifyList.dump(cerr);
    int stat = controller.executeList(verifyList,
				      compareData,
				      blockSize*sizeof(uint32_t),
				      &bytesRead);
    string msg = strerror(errno);
    cerr << "Status " << stat 
	 << " Read Size: " << blockSize*sizeof(uint32_t)
	 << "Actual read " << bytesRead << endl;
    if (stat != 0) {
      cerr << "Failure reason: " << msg << endl;
    }

    bytesLeft -= blockSize*sizeof(uint32_t);
    src       += blockSize*sizeof(uint32_t);
  }
  

    // Verify the load:

  return;			// For now for speed.

  cerr << "Verifying SRAMA contents\n";
  p        = reinterpret_cast<uint32_t*>(image);
  dest     = sramA();
  uint32_t* pSram = new uint32_t[vblockSize];
  size_t    xfered = 0;

  while (nBytes > vblockSize * sizeof(uint32_t)) {
    CVMUSBReadoutList vlist;
    uint32_t  daddr = dest;
    uint32_t* pr = pSram;
    for (int i =0; i < vblockSize; i++) {
      controller.vmeRead32(daddr,sramaAmod, pr);
      daddr += sizeof(uint32_t);
      pr++;
    }
    pr = pSram;

    for (int i =0; i < vblockSize; i++) {
      if (*p != *pr) {
	cerr << hex << "Mismatch at address " << dest << ": SB: " << *p << " Was: " << *pr  << endl <<dec;
      }

      p++;
      pr++;
      dest += sizeof(uint32_t);
    }

    nBytes -= vblockSize*sizeof(uint32_t);
    cout << '.';
    cout.flush();

  }
  cout << '\n';
  delete []pSram;


  cerr << "Verification complete\n";


}
