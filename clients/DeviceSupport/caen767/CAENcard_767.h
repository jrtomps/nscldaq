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



#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

//the number of cards in a VME crate
#define VME_CRATE_SIZE   21

//define the operating modes of the card object
#define CAEN_MODE_UNINIT 0
#define CAEN_MODE_GEO24  1
#define CAEN_MODE_A32D16 2
#define CAEN_MODE_MCST   4
#define CAEN_MODE_CBLT   8

//define the drivers used (different Address Modifiers are used in each)
//am = 0x2f
#define CAEN_GEO24       "/dev/vmegeo24"
//am = 0x0d
#define CAEN_A32D16      "/dev/vme32d16"
//the next two are only used in CAENchain.cpp
//am = 0x09
#define CAEN_MCST        "/dev/vmemca32"
//am = 0x0b
#define CAEN_CBLT        "/dev/vmecba32"

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



/*! \class CAENcard_767 CAENcard_767.h
    \brief Drivers for the CAEN V767A TDC module.

  So far no adverse effects have been noticed if the optimize option is used on the
  compiler, so optimize away!

  If you have a specific question or request email Chris Maurice at <maurice@nscl.msu.edu> and I will do my best to help.
 */
class CAENcard_767 {
  //! Compares the slot numbers to determine if the first is less.
  friend bool operator< (const CAENcard_767&, const CAENcard_767&);

  protected:
    //! This is the only member varaiable that is created with each instance of the class.
    /*! This variable is used as a reference into the static CAENcrate instance crate.  It
        holds the reference counted information needed to access the card. <br>
        The value is not zero indexed, so a value of 1 refers to the card in the physical
        slot number 1.  A value of zero is reserved for uninitialized cards.
    */
    int slot;

    //! An array of this structure holds all of the information on the current instances of CAENcard_767s.
    /*! \struct CAENcrate
        This struct is only referenced to create the crate member array. There is no
        good reason why you should ever have a need to creat CAENcrate objects. Doxygen wouldn't
        parse things right if I declared the variable in the same line as the struct so now
        CAENcrate_767 gets this extra documentation.
    */
    struct CAENcrate_767{
      volatile unsigned short int *mbuf;  //!< Pointer to a virtual memory map of the card. (note that it is a short int and that offsets must take this into account)
      unsigned int status;                //!< Used to specify what level of initialization the slot is currently at.
      int refCount;                       //!< The number of instances currently referring to the given slot.
      void* fd;                             //!< The file descriptor used to memory map the card and in various \c ioctl calls.
    };

    //! This member holds all of the information on the current instances of CAENcard_767s
    /*! The array holds a reference counted list of all of the currently initialized cards.
        index zero, aka. crate[0], is used to specify that the instance of a CAENcard_767
        has not been initialized to a slot (ie. a NULL CAENcard_767). Only one array is
        created no matter how many of the instances of the class are created, so any
        instances that are created with the same slot number will reference the the same
        physical card and the same entry in the crate array.
    */
    static CAENcrate_767 crate[VME_CRATE_SIZE + 1];

    //! Called by the constructor to initialize a card in a given slot. (should probably be broken into multiple functions...)
    int slotInit(int slotNum, int crateNum);

    //! Called by the destructor to free the resources associated with an allocated card.
    void destruct();

    //! A return value greater than or equal to zero indicates success
    int readOpcode(unsigned short int *value, int maxRetry);
    int writeOpcode(unsigned short int value, int maxRetry);
    int opcodeWait(int maxRetry);

    //! Clears all data from the buffer and clears all settings (they are set to default values by the card)
    //will destroy the address registers... unsafe for users
    void reset();

/*************************begin public section*********************************/
  public:

    //! Standard constructor. SlotNum is required to create a valid card, crateNum is not.
    CAENcard_767(int slotNum = 0, int crateNum = 0);

    //! Another constructor. Allows a card to be created using another card.
    CAENcard_767(const CAENcard_767& card);

    //! The card being assigned to has the destructor called on it and the value of the other argument assigned to it.
    CAENcard_767& operator=(const CAENcard_767& card);

    //! Reference counted destructor. The cards are not de-allocated until there are no references to them.
    ~CAENcard_767();

    //! Returns the module number of the card in the slot.
    int cardType();

    //! Clears all data from the event buffer and usually the event counter, too (if not in "count all triggers" mode).
    void clearData();

    //! A simple test to see if there is data in the event buffer.
    int dataPresent();

    //! Reads one event from the event buffer (if data is present) and returns the number of BYTES read into the buffer.
    int readEvent(void* buf);
    //! Reads one event from the event buffer (if data is present) and returns the number of 16-BIT WORDS read into the buffer.
    int readEvent(DAQWordBuffer& wbuf, int offset);
    //! Reads one event from the event buffer (if data is present) and returns the number of 16-BIT WORDS read into the buffer.
    int readEvent(DAQWordBufferPtr& wp);
    //! Reads one event from the event buffer (if data is present) and returns the number of 32-BIT DWORDS read into the buffer.
    int readEvent(DAQDWordBuffer& dwbuf, int offset);
    //! Reads one event from the event buffer (if data is present) and returns the number of 32-BIT DWORDS read into the buffer.
    int readEvent(DAQDWordBufferPtr& dwp);

    // Set up the card -- this is a temporary function that will be removed later
    // The current setup is common start, all channels enabled
    int tempSetup()
    {
      //set to common start mode ("Start Gating")
      // -- saves all conversions while the start signal is high.  the trigger signal is unused
      if( writeOpcode(0x1200, 1000000) < 0 )
      {
        return(-1);
      }

      //set data-ready mode to be event-ready
      if( writeOpcode(0x7000, 1000000) < 0 )
      {
        return(-2);
      }

      //enable subtraction of start time for start gating mode
      if( writeOpcode(0x4300, 1000000) < 0)
      {
	return(-3);
      }
      
      ////disables all channels
      //if( writeOpcode(0x2400, 1000000) < 0)
      // {
      //return(-4);
      //}
      //enable individual channels
      //if( writeOpcode(0x2000, 1000000) < 0)
      //{  
      //  return(-5);
      //}
      
      //if( writeOpcode(0x2001, 1000000) < 0)
      //{
      //	return(-6);
      //}

      opcodeWait(1000000);

      return(0);
    }

    int SetRisingEdgeStart() {
      if(writeOpcode(0x6400, 1000000) < 0) {
	return -1;
      }
      opcodeWait(1000000);
      return 0;
    }
    int SetFallingEdgeStart() {
      if(writeOpcode(0x6500, 100000) < 0) {
	return -1;
      }
      opcodeWait(100000);
      return 0;
    }
    
    int SetRisingEdgeAll() {
      if(writeOpcode(0x6000, 1000000) < 0) {
	return -1;
      }
      opcodeWait(1000000);
      return 0;
    }

    int SetFallingEdgeAll() {
      if(writeOpcode(0x6100, 1000000) < 0) {
	return -1;
      }
      opcodeWait(1000000);
      return 0;
    }
    unsigned short getSr2() {
      return *((crate[slot].mbuf + CAEN_767_STATUS_2));
    }

};

#endif
