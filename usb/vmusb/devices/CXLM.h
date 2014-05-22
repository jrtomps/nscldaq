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

#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif


// Forward class definitions:

class CReadoutModule;
class CControlModule;
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
void loadFirmware(CVMUSB& controller, uint32_t baseAddr, uint32_t sramAddr, std::string filename);

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
void addBusAccess(CVMUSBReadoutList& list, uint32_t base, uint32_t accessPattern, uint8_t delay);

///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE UTILITY FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace Utils 
{
  static bool validFirmwareFile(std::string name, std::string value, void* arg);
  extern uint32_t fileSize(std::string path);
  extern void loadFile(std::string path, void* contents, uint32_t size);
  extern void loadSRAM(CVMUSB& controller, uint32_t dest_addr, void* image, uint32_t bytes);
  extern void remapBits(void* sramImage, void* fileImage,  uint32_t bytes);
}



///////////////////////////////////////////////////////////////////////////////
//////////////////////// SLOW CONTROLS FOR FIRMWARE LOADS /////////////////////
///////////////////////////////////////////////////////////////////////////////

class CXLMSlowControls : public CControlHardware
{
  public:
  virtual void clone( const CXLMSlowControls& rhs);

  virtual void onAttach (CControlModule& config);
  virtual void Initialize(CVMUSB& controller);
  virtual std::string Update(CVMUSB& vme);
  virtual std::string Set(CVMUSB& vme, std::string what, std::string value);
  virtual std::string Get(CVMUSB& vme, std::string what);

};

} // end of namespace

#endif
