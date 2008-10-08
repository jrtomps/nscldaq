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

#ifndef CAENCARD_767_H
#define CAENCARD_767_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <VmeModule.h>



//the number of cards in a VME crate
#define VME_CRATE_SIZE   21

//define the operating modes of the card object
#define CAEN_MODE_UNINIT 0
#define CAEN_MODE_GEO24  1
#define CAEN_MODE_A32D16 2
#define CAEN_MODE_MCST   4
#define CAEN_MODE_CBLT   8

//define the address spaces used to access the board.

#define CAEN_GEO24       CVmeModule::geo
#define CAEN_A32D16      CVmeModule::a32d32 /* No real distinction with d16. */


//will be used to create a memory map for the card
#define CAEN_767_CARD_MMAP_LEN 0x00002000

//Constants used to determine the type of data being returned from the card
#define CAEN_767_DATUM_TYPE    0x00600000
#define CAEN_767_HEADER        0x00400000
#define CAEN_767_DATA          0x00000000
#define CAEN_767_FOOTER        0x00200000
#define CAEN_767_INVALID       0x00600000

//offsets (assuming short int units) for various registers
#define CAEN_767_ADDR_GEO      0x0002
#define CAEN_767_BIT_SET       0x0003
#define CAEN_767_BIT_CLEAR     0x0004
#define CAEN_767_INTERRUPT_LEVEL  0x0005
#define CAEN_767_INTERRUPT_VECTOR 0x0006
#define CAEN_767_STATUS_1      0x0007
#define CAEN_767_CONTROL_1     0x0008
#define CAEN_767_ADDR_32       0x0009
#define CAEN_767_ADDR_24       0x000A
#define CAEN_767_MCST_ADDR     0x000B
#define CAEN_767_SS_RESET      0x000C
#define CAEN_767_MCST_CONTROL  0x0010

#define CAEN_767_STATUS_2      0x0024
#define CAEN_767_CONTROL_2     0x0025
#define CAEN_767_EVENT_COUNTER 0x0026
#define CAEN_767_CLEAR_COUNTER 0x0027
#define CAEN_767_OPCODE_STATUS 0x0028
#define CAEN_767_OPCODE        0x0029
#define CAEN_767_CLEAR         0x002A
#define CAEN_767_TESTWORD_HIGH 0x002B
#define CAEN_767_TESTWORD_LOW  0x002C
#define CAEN_767_SOFT_TRIGGER  0x002D

#define CAEN_767_MANUFACT_ID   0x0813
#define CAEN_767_BOARD_ID      0x0819
#define CAEN_767_REVISION_ID   0x0827

// Bits in the edge info data:

#define CAEN_767_EDGE_RISING   0x0001
#define CAEN_767_EDGE_FALLING  0x0002
#define CAEN_767_EDGE_BOTH     (CAEN_767_EDGE_RISING | CAEN_767_EDGE_FALLING)


/*! \class CAENcard_767 CAENcard_767.h
    \brief Drivers for the CAEN V767A TDC module.

  So far no adverse effects have been noticed if the optimize option is used on the
  compiler, so optimize away!

  If you have a specific question or request email Ron Fox (fox@nscl.msu.edu)
  Original version of this was by Chris Maurice.  Main changes for this
  heavy modification is
  - Allow for cases where the module can only be addressed via base address.
  - Make this look much more like 'normal' daq code in the way commenting
    and Doxygen documentation is done.
  - Make the module indpendent of the VME bus interface by building it around
    a CVmeModule object rather than just a pointer to the mapped memory.
    We only optimize the readout for mapped/unmapped mode as the setup
    operations can run at a slower pace.
*/
class CAENcard_767 {

private:
  int slot;
  CVmeModule  *m_pSpace;


/*************************begin public section*********************************/
public:
  // Constructors and other canonical operations.
  
  CAENcard_767(int slotNum, int crateNum = 0,
	       bool fisGeo = true, unsigned long nBase=0);
  CAENcard_767(const CAENcard_767& card);
  CAENcard_767& operator=(const CAENcard_767& card);
  ~CAENcard_767();
  
  
  int cardType();		//!< Returns the type of the card in the slot.
  int mfgId();			//!< Return manufacturer id.
  void clearData();		//!< Clear event data.
  int dataPresent();		//!< Check for data present.
  
  
  int readEvent(void* buf);	//!< Read an event -> Ordinary buffer.

  
  int tempSetup();		//!< Setup the card.


  int SetRisingEdgeStart() ;
  int SetFallingEdgeStart(); 
  int SetRisingEdgeAll();
  int SetFallingEdgeAll();

  int getStartEdge();		//!< Get the edge of the start input.
  int getEdgeOdd();		//!< Get odd channel edge
  int getEdgeEven();		//!< Get even channel edge info.
  unsigned short getSr2();
  // Utilities:

protected:
  int slotInit(int slotNum, int crateNum,
	       bool fGeo, unsigned long base); //!< Initialize a slot.
  int readEdgeConfiguration(unsigned short* values);
  int readOpcode(unsigned short int *value, int maxRetry); //!< Opcode for seq read.
  int writeOpcode(unsigned short int value, int maxRetry); //!< Opcode for seq write.
  int opcodeWait(int maxRetry);           //!< Wait for sequencer ready.
  void reset();		            //!< MOdule soft reset. 
};

#endif
