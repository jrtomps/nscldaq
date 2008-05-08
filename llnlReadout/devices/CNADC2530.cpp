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
#include "CNADC2530.h"
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <stdio.h>

using namespace std;

#define Const(name) static const int name = 

////////////////////////////////////////////////////////////////////////
//
//                  Constants:

//  Register offsets for the NADC2530 module
//  (I've only bothered with the registers I may need).
//
Const(REG_ID)        0;		// Manufacturer's id.
Const(REG_TYPE)      2;		// Module model number.
Const(REG_CSR)       4;		// Control and status register.
Const(REG_MEM)       6;		// Memory offset for list mode memory.
Const(REG_LISTWL)    8;           // Low order number of words in list memory.
Const(REG_LISTWH)    0xa;         // High order number of words in list memory.
Const(REG_VECTOR)    0xc;         // Interrupt vector
Const(REG_FULLNESS)  0x10;
Const(REG_EVENTSREQ) 0x12;        // Requested event count
Const(REG_IRQMASK)   0x12;        // or could be interrupt mask depending on csr
Const(REG_LLD)       0x14;        // Lower level discriminator value.
Const(REG_HLD)       0x16;        // Upper level discrminator value.
Const(REG_EVENTSL)   0x18;        // Low 16 bits of number of events acquired.
Const(REG_EVENTSH)   0x1a;        // High 8 bits of number of events acquired.

// Identification Register values:

Const(ID_VALUE)      0x8063;      // Manufacturer code.
Const(TYPE_VALUE)    2530;        // Model number.

// CSR bits (we're really only going to set them).

Const(CSR_HE)       0x8000;       // 1 Enables histogram mode 0 list mode.
Const(CSR_GE)       0x4000;       // 1 Enables gate mode 0 auto trigger.
Const(CSR_ZE)       0x2000;       // 1 Disables zero supresssion 0 Enables zero supression.
Const(CSR_IT)       0x1000;       // 1 Interrupt on event count 0 Interrupt on full/half full.
Const(CSR_IPLMASK)  0x0e00;       // Interrupt priority level mask.
Const(CSR_IPLSHIFT) 9;            // Lowest bit position for IPL mask.
Const(CSR_ARM)      0x0100;       // 1 ARM Cleared automatically when full in list mode.
Const(CSR_IE)       0x0080;      // 1 Enable ineterrupts 0 disable.
Const(CSR_IS)       0x0040;       // 1 if an interrupt condition is set.
Const(CSR_FFCLR)    0x0010;       // Force a fast clear
Const(CSR_SC)       0x0008;       // 1 Set in calibration mode.
Const(CSR_iDR)      0x0004;       // 1 Data ready when event count hit else on fullness flags.
Const(CSR_SS)       0x0002;       // 1 Disables sliding scale (should never be set!).
Const(CSR_RESET)    0x0001;       // Resets the csr.


// Address modifiers we will use to access the module:

Const(initamod)  CVMUSBReadoutList::a24UserData;
Const(readamod)   CVMUSBReadoutList::a32UserBlock;

// Parameter limits:

// The register base addresss is in A24 space so it must  live between
// 0x00000000 and 0x00ffffff.
//

static CConfigurableObject::limit zero(0);
static CConfigurableObject::limit maxCsr(0x00ffffff);
static CConfigurableObject::Limits CsrLimits(zero, maxCsr);


// The IPL must be in the range [0..7]

static CConfigurableObject::limit maxIPL(7);
static CConfigurableObject::Limits IPLLimits(zero, maxIPL);

// The 16 bit vector must be in the range [0..7].

static CConfigurableObject::limit maxVector(0xffff);
static CConfigurableObject::Limits VectorLimits(zero, maxVector);

// For now we won't accept more than 


// For now we won't accept more than 10 events at a time...

static CConfigurableObject::limit minEvents(1);
static CConfigurableObject::limit maxEvents(10);
static CConfigurableObject::Limits EventLimits(minEvents, maxEvents);


// Limits on the thresholds...these are set in V/mv or V and converted so these
// are floating point limits.


CConfigurableObject::flimit fzero(0.0);
CConfigurableObject::flimit lldhigh(819.2); // some slop for fp compares
CConfigurableObject::flimit hldhigh(8.192); // some slop for fp compares.

static CConfigurableObject::FloatingLimits lldLimits(fzero, lldhigh);
static CConfigurableObject::FloatingLimits hldLimits(fzero, hldhigh);

///////////////////////////////////////////////////////////////
//   Constructors and other canonical methods

/*!
  Construction is largely a no-op.  The realwork is done
  in the onAttach method when the object gets a configuration
  and sets up the parameters it recognizes.
*/
CNADC2530::CNADC2530() : 
  m_pConfiguration(0)
{
}
/*!
  Copy construction snapshots  the configuration

*/
CNADC2530::CNADC2530(const CNADC2530& rhs) :
  m_pConfiguration(0),
  m_csr(rhs.m_csr),
  m_csrValue(rhs.m_csrValue),
  m_eventBase(rhs.m_eventBase),
  m_eventCount(rhs.m_eventCount)
{
 if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/*!
   
Destructor leaks for now... if copy constructed.
*/
CNADC2530::~CNADC2530()
{
}

/*!
  Assignment is also bogus for now due to danger of recursion loops.
*/
CNADC2530&
CNADC2530::operator=(const CNADC2530& rhs)
{
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Implementing the base class interface:
//


/*!
  Attach a configuration to this object.  We create the
  configuration parameters, set them to default values, and provide limits where
  appropriate.  
  
  See the class header for a description of the configuration parameters.

  \param configuration - reference to the configuration of the module. 
*/
void
CNADC2530::onAttach(CReadoutModule& configuration) 
{
  // Save our configuration so we can query it later.

  m_pConfiguration = &configuration;

  // Now setup  our configuration parameter set.

  // The module has two address spaces.  The register space is defined by jumpers on the
  // board and is in A24 space, while the data memory is in A32 space and programmed.
  // the -csr parameter must match the jumpers, the -memory parameter is used to 
  // program the data memory addresss.  Only the top 11 bits are meaningful.
  // the rest will be forced -> 0.

  m_pConfiguration->addParameter("-csr", CConfigurableObject::isInteger, 
				 static_cast<void*>(&CsrLimits), "0");
  m_pConfiguration->addParameter("-memory", CConfigurableObject::isInteger,
				 NULL, "0");

  //  Next set of parameters determine how the module interrupts.  This includes
  // the values of the IPn and IE bits in the CSR as well as the
  // value programmed in the vector register.
  //
  // -ipl - The interrupt priority level, the value programmed inthe Ipn bits
  //        If nonzero, IE is also set which enables the interrpts.
  // -vector - Determines the value set in the Vector register, which is the status/Id 
  //           response of the module to the interrupt acknowledge cycle.
  // 

  m_pConfiguration->addParameter("-ipl", CConfigurableObject::isInteger,
				 static_cast<void*>(&IPLLimits), "0");
  m_pConfiguration->addParameter("-vector", CConfigurableObject::isInteger,
				 static_cast<void*>(&VectorLimits), "0");

  //  Set up the discriminator values.
  // -lld  - The low level discriminator.
  // -hld  - The high level discriminator.
  //  These are floating parameters, so that they can be set in 'sensible' units.

  m_pConfiguration->addParameter("-lld", CConfigurableObject::isFloat,
				 static_cast<void*>(&lldLimits), "0.0");
  m_pConfiguration->addParameter("-hld", CConfigurableObject::isFloat,
				 static_cast<void*>(&hldLimits), "8.191");

  // The module is a multi-event adc, in interrupt mode, we can determine how many events
  // are accepted before the module interrutps.  This allows for somewhat faster processing
  // by amortizing interrupt overhead across several events.
  // -events determines how many events are read per interrupt.  It's up to the
  // analysis software to deal with the fact that multiple events can occur in one VM-USB superevent.
  //

  m_pConfiguration->addParameter("-events", CConfigurableObject::isInteger,
				 static_cast<void*>(&EventLimits), "1");


}

/*!
   When the system is about ready to start data taking, the Initialize member is called
   to set up the module.  The module is set up in accordance with the
   configuration parameters defined  in onAttach. 
   Once thec csr address has been extracted, we'll verify that the module is, in fact
   a hytec adc by reading the ID and TYPE registers... comparing them
   with ID_VALUE and TYPE_VALUE respectively.

   If the -csr parameter does not specify the address of a hytec module CSR page,
   a descriptive string exception is thrown which should abort the begin of run.

   \param  controller - VM-USB controller object that allows us to manipulate the module.
*/
void
CNADC2530::Initialize(CVMUSB& controller)
{
  // Get the register page and list mode data addresses.

  uint32_t csr  = m_pConfiguration->getIntegerParameter("-csr");
  uint32_t list = m_pConfiguration->getIntegerParameter("-memory");
  uint16_t listRegister = (uint16_t)((list >> 16) & 0xffe0); // Value for list register.

  // Check the validity of the -csr. If valid, program the memory base.
  // read failures are either VM-USB problem or, potentially I'm pointed into
  // VME addresses with no responders.
  //
  uint16_t   id, model;
  int status  = controller.vmeRead16(csr + REG_ID, initamod, &id);
  if (status) {
    throw "Could not read ID register of NADC2530";
  }
  status = controller.vmeRead16(csr + REG_TYPE, initamod, &model);
  if (status) {
    throw "Could not read TYPE register of NADC 2530";
  }
  if ((id != ID_VALUE) || (model != TYPE_VALUE)) {
    char message[1024];
    sprintf(message,"Module at 0x%08x has an invalid ID (was %x sb %x) or Model (was %d sb %d)",
	    csr, id, ID_VALUE, model, TYPE_VALUE);
    throw string(message);
  }

  // Disable the module and set its counters to something reasonable.

  controller.vmeWrite16(csr + REG_CSR, initamod, CSR_RESET);
  controller.vmeWrite16(csr + REG_MEM, initamod, listRegister);
  controller.vmeWrite16(csr + REG_LISTWL, initamod, 0);	// offset where data will be put.
  controller.vmeWrite16(csr + REG_LISTWH, initamod, 0);
  controller.vmeWrite16(csr + REG_EVENTSL, initamod, 0); // Events acquired counter.
  controller.vmeWrite16(csr + REG_EVENTSH, initamod, 0);



  // Set up all values but the CSR

  int vector = m_pConfiguration->getIntegerParameter("-vector");
  int events = m_pConfiguration->getIntegerParameter("-events");
  double lld = m_pConfiguration->getFloatParameter("-lld");
  double hld = m_pConfiguration->getFloatParameter("-hld");

  controller.vmeWrite16(csr + REG_VECTOR, initamod, vector);
  controller.vmeWrite16(csr + REG_EVENTSREQ, initamod, events);
  controller.vmeWrite16(csr + REG_LLD, initamod, lldToRegister(lld));
  controller.vmeWrite16(csr + REG_HLD, initamod, hldToRegister(hld));
  controller.vmeWrite16(csr + REG_FULLNESS, initamod, 0);


  // Figure out the CSR value based on the IPL etc, and set it.. which enables the module.
  // 

  uint16_t csrValue = CSR_GE | CSR_ZE | CSR_IT | CSR_ARM;
  int ipl = m_pConfiguration->getIntegerParameter("-ipl");
  if (ipl) {
    csrValue |= CSR_IE;
    csrValue |= (ipl << CSR_IPLSHIFT) & CSR_IPLMASK;
  }
  controller.vmeWrite16(csr + REG_CSR, initamod, csrValue);

  // Save a few values that will be useful later on:

  m_csr       = csr;
  m_csrValue  = csrValue;	// What to set the CSR to after an event.
  m_eventBase = list;		// Where the events will live.
  m_eventCount= events;         // Determines how many events are read on a trigger.

}
/*!
   Add the VME transactions needed to read the module. Since I'm lazy, the following
   data elements have been saved so that I don't have to recompute them from the configuration:
   - m_csr        - Address of the register page.
   - m_csrValue   - Value to program into the CSR to re-arm the module.
   - m_eventBase  - Base address of the list mode data.
   - m_eventCount - Number of events in each trigger.

   \param list  - The CVMUSBReadoutList that will be loaded into the VM-USB to 
                  read out this 'stack'.

*/
void
CNADC2530::addReadoutList(CVMUSBReadoutList& list)
{

  // Disarm for the durationof the read:

  list.addWrite16(m_csr + REG_CSR, initamod, m_csr & (~CSR_ARM));


  // The number of events between interrupts is used to determine the number of longs to 
  // read from the module.  There's a header long, trailer long and 8 channels for each
  // event (not zero suppression mode yet).

  int transfers = m_eventCount*(8+2);

  // The version of the firmware I have at this time for the VM-USB appears to
  // delete the last word of a block transfer.  This is probably a hold-over
  // from how Jan fixed the extra word inserted by a BERR end of transfer..
  // If this is still the case, add an extra longword to the transfer.
  // This means that now we'll get an extra word but that's probably better
  // than missing the trailer >sigh<

#ifndef VM_USB_BLKREADOK
  transfers++;
#endif

  list.addBlockRead32(m_eventBase, readamod, transfers);
  
  // We're going to zero the offset regiseter so the next set of events is also
  // at the bottom of the memory, and then reset the CSR with the m_csrValue:

  list.addWrite16(m_csr + REG_LISTWL, initamod, 0);
  list.addWrite16(m_csr + REG_LISTWH, initamod, 0);

  list.addWrite16(m_csr + REG_CSR, initamod, m_csr);
}

/*!

  Clone the object... produce an exact duplicate of this.
  This is effectively a virtual copy constructor.

*/
CReadoutHardware*
CNADC2530::clone() const
{
  return new CNADC2530(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////
//  Private utility functions.
//

/* 
  Convert the low level threshold value (millivolts) to 
  the value to program in the lld register.
  Note that the configuration subsystem has range checked the value.
*/
uint16_t
CNADC2530::lldToRegister(double lld)
{
  double volts = lld/1000.0;	// Millivolts -> volts.

  // This is taken from section 5.11 of the hytec manual.

  int intermediate = (int)((volts/3.2764)/(0.25/4095));
  return (uint16_t)((intermediate << 2) & 0x3ffc);
}
/*
  Convert the high level threshold value (volts) to the value to program in the hld register.
  Note that the configuration subsystem has already range checked the value.
*/
uint16_t
CNADC2530::hldToRegister(double hld)
{
  // hld is already in volts:

  int intermediate = (int)(((hld/3.2764) - 2)/(0.5/4095));
  return (uint16_t)((intermediate << 2) & 0x3ffc);
}
