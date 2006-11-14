/*
#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
*/

#ifndef CAENCARD_H
#define CAENCARD_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#ifndef __CVMEINTERFACE_H
#include <CVMEInterface.h>
#endif

#ifndef SPECTRODAQ_H
 // Hate to do this here, but I have no real choice:

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>
#endif


#ifndef HAVE_VME_MAPPING
#include <VmeModule.h>		// Needed to access registers via peek/poke.
#endif

//Constants used to determine the type of data being returned from the card

#define CAEN_DATUM_TYPE    0x07000000
#define CAEN_HEADER        0x02000000
#define CAEN_DATA          0x00000000
#define CAEN_FOOTER        0x04000000
#define CAEN_INVALID       0x06000000


/*! \class CAENcard CAENcard.h
    \brief Drivers for the CAEN V775 TDC, V785 ADC, and V792 QDC modules.

    This is a support class for the CAEN V775,785,792 digitizers.  
    the code is based on initial work by Chris Maurice.  It is
    also possible to place individual cards into a CAENchain.  Placing
    cards in a chain allows them to be read out using the Chain block 
    transfer readout.  For large systems this can be faster than using
    program controlled readout.  If you are using the modules in multi-event
    mode, it is your resopnsibility to re-assemble events after readout.

    The class supports construction for use either with or without 
    support for geographical addressing.

    Note that for now, copy construction will involve the creation of a
    new memory map and is therefore discouraged.

 */
class CAENcard {
 public:			// Data types
  typedef enum _ChainMember {	// Possible chain memberships
    NotInChain,
    FirstInChain,
    LastInChain,
    IntermediateInChain
  } ChainMember;

 protected:
  int    m_nSlot;		//!< vme virtual slot number.
  int    m_nCrate;		//!< VME crate number housing the module.
  bool   m_fGeo;		//!< True if geo addressing.
  unsigned long   m_nBase;	//!< Base physical VME address in crate.
#ifdef HAVE_VME_MAPPING
  volatile unsigned short* m_pModule;	//!< Pointer to module data (not prom).
#else
  CVmeModule* m_pModule;
#endif
  long   m_nCardType;          //!< Type of card (filled in at MapCard)
  void*  m_nFd;			//!< File desc. open on VME.
  int    m_nSerialno;		//!< Serial number of card.
  int    m_nHardwareRev;	//!< Hardware revision level.
  bool   m_fSmallThresholds;	//!< True if small threshold mode.
  int    m_nInvalidTrailers;	//!< How many invalid trailers.
  int    m_nChancountHigh;	//!< How many channel counts high
  int    m_nChancountLow;	//!< How many channel counts low.
  int    m_nEvents;		//!< How many events read.
  unsigned short m_nFirmware;    //!< Firmware rev level.




/*************************begin public section*********************************/
public:
  //
  // Constructors and canonical operations.
  //
  
  CAENcard(int slotNum = -1, int crateNum = 0, 
	   bool fGeo=true, long nBase = 0); //!< 'normal' constructor.
  CAENcard(const CAENcard& card); //!< Copy Constructor.
  CAENcard& operator=(const CAENcard& card); //!< Assignment involves mapping.
  ~CAENcard();		//!< Destructor.

  // selectors:
  int getPhysicalCrate() const {
    return m_nCrate;
  }
  int getSlot() const {
    return m_nSlot;
  }
  bool isGeo() const {
    return m_fGeo;
  }
  unsigned long getBase() const {
    return m_nBase;
  }
  int getSerial() const {
    return  m_nSerialno;
  }
  int getHardwareRev() const {
    return m_nHardwareRev;
  }

  //   Operations on the card.

    int  cardType();		//!< Return module Id of card.
  int  getFirmware();		//!< return hardware firmware rev.
    int  getCrate();		//!< Gets the module's crate number.
    void setCrate(int crateNum);  //!< Set the crate id register.
    void setThreshold(int ch, int threshold); //!< Changes the card threshold.
    void setThresholdVoltage(int ch, double voltage);//!< Set 785 adc threshold from volts.
    void keepUnderThresholdData(); //!< Allow retentionof under-thresholds.
    void discardUnderThresholdData(); //!< Discards under threshold chans.
    void keepOverflowData();	//!< Keep channels above overflow value.
    void discardOverflowData();	//!< Discard channels below overflow value.
    void keepInvalidData();	//!< Keep conversions in the midst of reset.
    void discardInvalidData();	//!< Discard data in midst of reset.
    void commonStart();		//!< If TDC, use in common start mode.
    void commonStop();		//!< IF TDC use in common stop mode.
    void setRange(int range);	//!< If TDC Set range (from 140 to 1200 ns).
    void setPedestalCurrent(int ped);//!< IF Qdc set pedestal curent.
    void cardOff();		//! Disable card.
    void cardOn();		//!< Enable card.
    void channelOff(int ch);	//!< Disable a channel.
    void channelOn(int ch);	//!< Enable a channel.
    void resetEventCounter();	//!< Set event number -> 0.
    void clearData();		//!< Clear any buffered event data.
    void reset();		//!< Soft power up up reset.
    bool dataPresent();		//!< TRUE if data is ready to be read.
    bool gdataPresent();
    bool Busy();
    bool gBusy();
    bool MEBFull();
    bool MEBEmpty();
    int readEvent(void* buf);	//!< Read event into 'ordinary' buffer.
    int readEvent(DAQWordBuffer& wbuf, int offset);//!< Read event to daq word buffer.
    int readEvent(DAQWordBufferPtr& wp); //!< Read event into daq word buffer
    int readEvent(DAQDWordBuffer& dwbuf, int offset);//!< Read event into daq long buf.
    int readEvent(DAQDWordBufferPtr& dwp); //!< Read event into daq long buf.
    void setIped(int value); //!< IF qdc set pedestal current.
    int  getIped();	//! If qdc set pedestal current.
    void setFastClearWindow(int n);  //!< Set fast clear window width.
    void enableSmallThresholds();   //!< Enable small thresholds.
    void disableSmallThresholds();  //!< Disable small thresholds.
  void SetCBLTChainMembership(int cbltaddr,
			      ChainMember where); //!< set up cblt.

  void ClearStatistics();	//!< Clear the statistics counters.
  int  InvalidTrailerCount();
  int  ChannelsTooBigCount();
  int  ChannelsTooSmallCount();
  int  EventCount();

  //Utility functions:
protected:
  void  DestroyCard();		//!< Destroy a card memory map.
  void  slotInit();		//!< Bring a card into being.
  void  MapCard();		//!< Map the card's memory.
private:
   void Bitset1(short mask);    //!< Set a mask in the bit set 1 register.
   void Bitclear1(short mask);  //!< Set a mask in the bit clear 1 register.
   void Bitset2(short mask);    //!< Set a mask i the bit set 2 register.
   void Bitclear2(short mask);  //!< set a mask in the bit clear 2 register.
   void ReadBufferBlock(int* pDest, Int_t size); // Read block from data buf.
};

#endif
