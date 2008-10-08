/*
#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2008.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#            Eric Kasten
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
*/

#ifndef CADC2530_H
#define CADC2530_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string>

#ifndef __CVMEINTERFACE_H
#include <CVMEInterface.h>
#endif

#include <VmeModule.h>		// Needed to access registers via peek/poke.

/**
* @class CADC2530 CADC2530.h
* @brief Driver for the Hytec 2530 ADC.
*
* This is a support class for the Hytec 2530 ADC.
*
* Note that for now, copy construction will involve the creation of a
* new memory map and is therefore discouraged.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class CADC2530 {
  // ------------------------------------------------------------
  // Public members
  public:
    CADC2530(int crateNum = 0, long nBase = 0); //!< 'normal' constructor.
    CADC2530(const CADC2530& card); //!< Copy Constructor.
    ~CADC2530();		//!< Destructor.

    static bool checkCard(int,long,unsigned short&,unsigned short&);
    static unsigned short volt2lld(double);
    static unsigned short volt2uld(double);

    CADC2530& operator=(const CADC2530&); //!< Assignment operator

    const std::string& toString();      //!< Stringify

    int readListEvents(void*,int&);      //!< Read multiple events into buffer.
    int readHistogramChannel(void*,int); //!< Read a histogram channel.

    unsigned short cardType();		//!< Return module device type.
    unsigned short manufacturerId();    //!< Return module Id of card.

    void clearHistogramMemory();        //!< Zero only histogram memory.
    void clearMemory();                 //!< Zero all histo/list memory.
    void resetCard();                   //!< Reset card to initial settings.

    void clearListAddress();            //!< Clear list memory address counter.
    unsigned int getListAddress();      //!< Get list memory address counter.

    void arm();                         //!< Arm the module.
    void disarm();                      //!< Disarm the module.
    bool isArmed();                     //!< Check if module is armed.

    void modeHistogram(bool);           //!< Set to histogram mode.
    void modeGate();                    //!< Set to list mode.

    void resetCSR();                    //!< Reset the CSR to zero.
    void fastClear();                   //!< Force fast clear all channels.

    bool isBusy();                      //!< Check if modules is busy.
    bool dataReady();                   //!< Check if data ready.
    bool hasInterrupt();                //!< Check CSR interrupt status.

    void enableInterrupt();             //!< Enable interrupt.
    void disableInterrupt();            //!< Disable interrupt.

    void enableGate();                  //!< Enable gate mode.
    void disableGate();                 //!< Disable gate mode.

    void enableZeroCnv();               //!< Enable zero conversion.
    void disableZeroCnv();              //!< Disable zero conversion.

    void enableCalibration();           //!< Enable calibration.
    void disableCalibration();          //!< Disable calibration.

    void enableSlidingScale();          //!< Enable sliding scale.
    void disableSlidingScale();         //!< Disable sliding scale.

    void setIPL(unsigned short);        //!< Set the interrupt priority level
    unsigned short getIPL();            //!< Get the interrupt priority level

    unsigned short getCSR();            //!< Get the CSR value.

    void dataReadyOnEvent();            //!< Data ready when at least 1 event.
    void dataReadyOnFullness();         //!< Data ready on fullness flags.

    void setLLD(unsigned short);        //!< Set the LLD value.
    unsigned short getLLD();            //!< Get the LLD value.
    void setULD(unsigned short);        //!< Set the ULD value.
    unsigned short getULD();            //!< Get the ULD value.

    void setInterruptVector(unsigned short); //!< Set the interrupt vector. 
    unsigned short getInterruptVector();     //!< Get the interrupt vector.
    void setInterruptMask(unsigned short);   //!< Set the interrupt mask.
    unsigned short getInterruptMask();       //!< Get the interrupt mask.

    void clearFullnessFlags();          //!< Clear the fullness flags to 0.
    unsigned short getFullnessFlags();  //!< Get the fullness flags.
    bool isChannelFull(unsigned short); //!< Check if histo. chan. is full.
    bool channelHasData(unsigned short); //!< Check if histo. channel has data.
    bool isListFull();                  //!< Check if list full.
    bool isListHalfFull();              //!< Check if list is half full.

    unsigned int getEventCounter();     //!< Get the event counter.
    void clearEventCounter();           //!< Clear the event counter. 

    // Calibration and test methods
    unsigned short getCTR();            //!< Get the CTR value.
    void resetCTR();                    //!< Reset the CTR to zero.
    void setCTRchannel(unsigned short); //!< Set the CTR channel.
    unsigned short getCTRchannel();     //!< Get the CTR channel.
    void enableAutoFastClear();         //!< Enable CTR auto fast clear. 
    void disableAutoFastClear();        //!< Disable CTR auto fast clear. 
    void enableMUXswitch();             //!< Enable CTR MUX switch.
    void disableMUXswitch();            //!< Disable CTR MUX switch.
    void enableCompensation();          //!< Enable CTR compensation.
    void disableCompensation();         //!< Disable CTR compensation.
    void setSSTR(unsigned short);       //!< Set the sliding scale test reg. 
    unsigned short getSSTR();           //!< Get the sliding scale test reg. 

  // ------------------------------------------------------------
  // Protected members
  protected:
    int    my_nCrate;		  //!< VME crate number housing the module.
    unsigned long my_nBase;	  //!< Base physical VME address in crate.
    unsigned long my_nMemOffset;  //!< Register vale for list/histogram offset.
    unsigned short my_nCardId;    //!< Card Id (filled in at MapCard)
    unsigned short my_nCardType;  //!< Type of card (filled in at MapCard)
    void*  my_nModFd;		  //!< File desc. for registers open on VME.
    void*  my_nMemFd;		  //!< File desc. for memory open on VME.
    bool my_eventmode;            //!< True when in event mode.
    unsigned int my_cureventpos;  //!< The current event long position.


    volatile unsigned short *my_pModule; //!< Pointer to modulre registers etc.
    volatile unsigned int *my_pMemory; //!< Pointer to list/histogram memory.


    void mapModule();           //!< Map register memory
    void mapMemory();           //!< Map list/histogram memory
    void destroyModule();       //!< Destroy register memory map
    void destroyMemory();       //!< Destroy list/histogram memory map

    void destroyCard();		//!< Destroy a card memory maps.
    void mapCard();		//!< Map the card's memory.

    void slotInit();		//!< Initialize a slot

    int readListEvent(void*,int); //!< Read event into a user buffer.

    unsigned long calcMemoryOffset(unsigned long); //!< Compute memory offset
    unsigned long setMemoryOffset(unsigned long); //!< Set the memory offset

    void setCSR(unsigned short);              //!< Set the CSR to a value.
    void setCSRbits(unsigned short,unsigned short); //!< Set bits in the CSR.
    void resetCSRbits(unsigned short);        //!< Reset bits in the CSR.

    void setCTR(unsigned short);              //!< Set the CTR to a value.
    void setCTRbits(unsigned short,unsigned short); //!< Set bits in the CTR.
    void resetCTRbits(unsigned short);        //!< Reset bits in the CTR.
};

#endif
