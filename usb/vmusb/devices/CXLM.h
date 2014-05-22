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

#ifndef __CXLM_H
#define __CXLM_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/*!
   Provides base class support for XLM based readout targets.
   We do register (by chaining to our onAttach) two configuration parameters:
\verbatim
   parametr       Type       Default     Meaning
   -base          uint32_t   0           Base address of the module.
   -firmware      string     ""          Path to XLM firmware file.
\verbatim


*/
namespace XLM
{

class CXLM : public CReadoutHardware
{
protected:			// Data available to derived classes:

  CReadoutModule* m_pConfiguration;

public:				// 'constants'.
  // Bus access bits:

  static const uint32_t  REQ_X;		// FPGA bus.
  static const uint32_t  REQ_A;		// SRAM A bus
  static const uint32_t  REQ_B;		// SRAM B bus

public:				// Canonicals:
  CXLM();
  CXLM(const CXLM& rhs);
  virtual ~CXLM();
  CXLM& operator=(const CXLM& rhs);

 private:			// Unimplemented canonicals:
  int operator==(const CXLM& rhs) const;
  int operator!=(const CXLM& rhs) const;

  // Element of the standard readout hardware interface we implement:

public:
  virtual void onAttach(CReadoutModule& configuration);  

  // XLM support functions derived classes can use these:
protected:
  void loadFirmware(CVMUSB& controller,  std::string path) throw(std::string);
  void accessBus(CVMUSB& controller,  uint32_t accessPattern);

  void addBusAccess(CVMUSBReadoutList& list, uint32_t accessPattern, 
		    uint8_t delay);

  uint32_t sramA();		// Return base address of SRAM A
  uint32_t sramB();		// Return base address of SRAM B
  uint32_t FPGA();		// Return base address of FPGA 'registers'.
  uint32_t Interface();		/* Return the base address of the interface registers. */
};


///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE MEMBERS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Base addresses of 'unstructured' areas.

const uint32_t  SRAMA(0x000000);	//  Base address of static ram A
const uint32_t  SRAMB(0x200000);	//  Base address of static RAM B
const uint32_t  FPGABase (0x400000); //  Base address of FPGA `register' set.
const uint32_t  DSP  (0x600000); //  Base address of DSP interface.
const uint32_t  InterfaceBase(0x800000);

// Interface layout:

const uint32_t  BusRequest (0x800000); // Register for submitting bus requests.
const uint32_t  Interrupt  (0x800004); // Interrupt/reset register.
const uint32_t  FPGABootSrc(0x800008); // Select boot source for FPGA.
const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.
const uint32_t  BUSAOwner  (0x810000); // Shows who has bus A (SRAM A)
const uint32_t  BUSBOwner  (0x810004); // Shows who has bus B (SRAM B)
const uint32_t  BUSXOwner  (0x810008); // Shows who has bus X (FPGA).
const uint32_t  IRQSerial  (0x820048); // Write for IRQ id reads serial number.
const uint32_t  POLLISR    (0x820824); // 'mailbox' betweenFPGA and DSP.

// Note that the REQ_A/B/X definitions above define the bits in the bus request register
// therefore we don't define these bits here.

// Bits in Interrupt register; Note these are 'negative logic bits' in that to assert
// the reset, the bit shown below should be set to zero.

const uint32_t InterruptResetFPGA    (0x00000001);
const uint32_t InterruptResetDSP     (0x00000002);
const uint32_t InterruptInterruptFPGA(0x00010000);
const uint32_t InterruptInterruptDSP (0x00020000);

// Boot source register contents.  This defines where the FPGA loads its
// microcode from.  There are 4 ROM locations as well as the possibility
// to load the microcode in to SRAM A.  This is coded rather than bits.

const uint32_t BootSrcRom0 (0x00000000);	// Load from sector 0 of PROM.
const uint32_t BootSrcRom1 (0x00000001);	// Load from sector 1 of PROM.
const uint32_t BootSrcRom2 (0x00000002); // Load from sector 2 of PROM.
const uint32_t BootSrcRom3 (0x00000003); // Load from sector 3 of PROM.
const uint32_t BootSrcSRAMA(0x00010000);	// Load from SRAM A image.

// The ForceOffBus register only has a single bit.... the LSB.  When set, the FPGA
// and DSP are forced off the bus.  When clear they are allowed to arbitrate for the bus.
// XLM Manual 4.3.1.4 states that even when that bit is set the VME bus host must arbitrate
// for the bus via the BusRequest register.

const uint32_t ForceOffBusForce(0x00000001); // Force all but VME off bus.

// BUS*Owner values.  These describe who is actually the owner of the specific bus.

const uint32_t BusOwnerNone(0);	// Bus not owned.
const uint32_t BusOwnerVME (1);	// VME host owns bus.
const uint32_t BusOwnerFPGA(2);	// Bus owned by FPGA.
const uint32_t BusOwnerDSP (3);	// Bus owned by DSP.

///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE FUNCTIONS //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**! \fn loadFirmware(CVMUSB& ctlr, uint32_t baseAddr, 
                      uint32_t sramAddr, std::string fmwrpath)
*
* Implements the firmware loading algorithm for the  
*
*   \param controller the VM-USB device to communicate through
*   \param baseAddr   the base address of the target XLM
*   \param sramAddr   the address of the sram (SRAMA = 0x000000, SRAMB = 0x200000)
*   \param filename   path to the firmware file
*/ 
extern 
void loadFirmware(CVMUSB& controller, uint32_t baseAddr, uint32_t sramAddr, 
                  std::string filename);

/**! \fn accessBus(CVMUSB& ctlr, uint32_t baseAddr, uint32_t accessPattern)
*
* Obtain ownership of select internal busses in the XLM (A, B, and X)  
*
*   \param controller     the VM-USB device to communicate through
*   \param baseAddr       the base address of the target XLM
*   \param accessPattern  the bit-wise OR of CXLM::REQ_A, CXLM::REQ_B, 
*                         and CXLM::REQ_X
*/ 
extern 
void accessBus(CVMUSB& controller, uint32_t base, uint32_t accessPattern);

/**! \fn addBusAccess(CVMUSBReadoutList& ctlr, uint32_t baseAddr, 
                      uint32_t accessPattern, uint8_t delay)
*
* Add commands to a readout list that will acquire ownership of select internal
* busses in the XLM (A, B, and X)  
*
*   \param controller     the VM-USB device to communicate through
*   \param baseAddr       the base address of the target XLM
*   \param accessPattern  the bit-wise OR of CXLM::REQ_A, CXLM::REQ_B, 
*                         and CXLM::REQ_X
*   \param delay          delay length in units of 12.5 ns to delay proceeding 
*                         execution of subsequent VME commands in the stack 
*/ 
extern 
void addBusAccess(CVMUSBReadoutList& list, uint32_t base, uint32_t accessPattern, 
                  uint8_t delay);

///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE UTILITY FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace Utils 
{
  extern bool validFirmwareFile(std::string name, std::string value, void* arg);
  extern uint32_t fileSize(std::string path);
  extern void loadFile(std::string path, void* contents, uint32_t size);
  extern void loadSRAM(CVMUSB& controller, uint32_t dest_addr, void* image, uint32_t bytes);
  extern void remapBits(void* sramImage, void* fileImage,  uint32_t bytes);
}




} // end of namespace

#endif
