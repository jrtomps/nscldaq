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
#include "CAENcard_767.h"



#include <string>
#include <Iostream.h>
#include <CVMEInterface.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



/*!
  Construct a CAEN V767 object. 
  \param slotNum - Slot in which the module is installed, or a slot address to be
     programmed into the module in case geographical addressing is not allowed.

  \param crateNum Physical VME crate in which the module is inserted.  Note that
     this is also programmed a sthe crate number field for the data.
  \param fisGeo - If true, the slot number determines the base addressing via
     the CAEN/CERN P3 connector, if Fals, then the nBase parameter is the
     base address of the module.
  \param nBase - Base address of the module when fisGeo is false.

  

  Once the card is verified to be in the slot, one of the supported types, and if it
  is not currently referenced by another instance then it is reset to clear any
  previous settings.  Also, the thresholds are also set to default values and the
   TDC   is set to common-start mode.

   \throw string - If any thing prevented the module form getting properly 
      establihsed.  See the slotInit function for more info.
 */
CAENcard_767::CAENcard_767(int slotNum, int crateNum,
			   bool fisGeo, unsigned long nBase) :
  slot(0), 
  m_pSpace(0)
  
{
  if(slotInit(slotNum, crateNum, fisGeo, nBase) < 0) {
    throw string("CAENcard_767 construction failed!");
  }

};

/*!
  Copy constructor.  Creates a functional equivalent of the card parameter.
  Copy construction is a heavyweight mechanism to get pass by value semantics in 
  function calls and other cases where temporaries are needed.  
  It is recommended that code avoid the use of copy construction if it is speed
  critical.

  \param card The card to copy.


  */
CAENcard_767::CAENcard_767(const CAENcard_767& card) :
  slot(card.slot),
  m_pSpace(card.m_pSpace)
{

};

/*!
  Assignment from another card.

  \param card The CAENcard_767 value to assign to the destination argument.
  \return CAENcard_767&
  \retval reference to *this... allowing operator chaining.

  */
CAENcard_767& 
CAENcard_767::operator=(const CAENcard_767& card)
{
  if(this != &card) {
    slot     = card.slot;
    m_pSpace = card.m_pSpace;
  }
  return *this;

};

/*!
  This function does the work of initializing a module.

  \param slotNum  This specifies the slot in the VME crate in which the module 
        resides.  This value will be the first five bits of every 32-bit word 
	in the output buffer.
  \param crateNum Specifies which VME crate we are plugged into.  This is also
        part of the data stream.  Defaults to zero.
  \param fisGeo  True if the module is to be addressed via geographical means,
        false if the base parameter is to be used to set a base address for the 
	module.
  \param nBase  If fisGeo is false, this is the base addressof th emodule.


  \return int
  \retval Failure to initialize the card is indicated by a return value less
       than zero.  Each failure returns a different value and are numbered 
       sequentially through the function.


*/
int CAENcard_767::slotInit(int slotNum, int crateNum, 
			   bool fisGeo, unsigned long nBase)
{

  //ensure that the slot and crate specified stay within bounds

  
  slot = slotNum;


  if((slot > VME_CRATE_SIZE) || (slot <= 0))
  {
    perror("Invalid slot number specified to slotInit(). ");
    return(-1);
  }



  // Create the VME module... this may be a GEO or an extended address
  // modifier range, depending on the base parameter.

  CVmeModule::Space space;
  unsigned long     base;
  
  // Based on the geo flag, figure out which space, and what the base is.

  if (fisGeo) {
    space = CAEN_GEO24;
    base  = slot << 19;
  }
  else {
    space = CAEN_A32D16;
    base  = nBase;
  }

  try {
    m_pSpace = new CVmeModule(space, base, CAEN_767_CARD_MMAP_LEN, crateNum);
  } 
  catch (string& msg) {
    cerr << "Could not map crate: " << crateNum << " slot: " << slot
	 << " : " << msg << endl;
    return(-6);  
  }
   


  if(  !( (0x0040e6 == mfgId())                   &&
	  (767    == cardType())                  &&
          (((short int)slot ==  (0x001F & m_pSpace->peekw(CAEN_767_ADDR_GEO))) ||
	   !fisGeo))) {

      printf( "\n767 Card %d is not inserted or is of an incompatable type!\n", slotNum);
      printf("  One of the following tests has failed\n");
      printf("    0x0040e6 = 0x%6.6X\n", mfgId());
      printf("    767    == %d\n",  cardType());
      printf("    0x%4.4X == 0x%4.4X & 0x001F\n", slot,
                                          m_pSpace->peekw(CAEN_767_ADDR_GEO));

      delete m_pSpace;
      m_pSpace = NULL;
      return(-7);
  }
  // If the card is not being accessed geographically we program the slot:

  reset();


  if(!fisGeo) {
     m_pSpace->pokew(slot,CAEN_767_ADDR_GEO);
  }

  //disable the auto load of the user configuration at the next reset

  if(  ( writeOpcode(0x1900, 1000000) < 0 ) || ( opcodeWait(1000000) < 0 )  )
  {
    delete m_pSpace;
    m_pSpace = NULL;
    return(-10);
  }

  // end of slot initialization, begin address initialization.. This is only needed
  // if access is geographical.... as otherwise we're already mapped the way we
  // want to be!

  if(fisGeo) {
    m_pSpace->pokew(slot, CAEN_767_ADDR_32); // Set the new module address...
    m_pSpace->pokew(0,    CAEN_767_ADDR_24);
    m_pSpace->pokew(1<<4, CAEN_767_BIT_SET); // Enable address relocation.

    delete m_pSpace;
    m_pSpace = NULL;
    try {
      m_pSpace = new CVmeModule(CAEN_A32D16, slot << 24,
				CAEN_767_CARD_MMAP_LEN, crateNum);
    }
    catch (string& msg) {
      cerr << "Unable to create relocated address for CAEN-767 " 
	   << msg << endl;
      return -11;
    }

  }



  // load the default configuration (only problem is that it is "Stop Trigger 
  // Matching")

  if(cardType() == 767)   {
    if(  ( writeOpcode(0x1500, 1000000) < 0 ) || ( opcodeWait(1000000) < 0 )  )
    {
      delete m_pSpace;
      m_pSpace = NULL;
      return(-14);
    }
  }
  sleep(5);

  return(0);
};

/*!
  The destructor just needs to release the address space object.

 */
CAENcard_767::~CAENcard_767()
{
  m_pSpace->pokew(1 << 4, CAEN_767_BIT_CLEAR); // Turn off any relocation
  delete m_pSpace;
};

/*!
   Do a read from the sequencer.
   \param value    - buffer to hold the returned value.
   \param maxRetry - Maximum number of times to retry the check for
                     info ready from the sequencer

   \return int
   \retval >0  - Number tries actually used.
   \retval <0  - Failure:

*/
int CAENcard_767::readOpcode(unsigned short int *value, int maxRetry)
{
  int i;  
  for( i = 0; i < maxRetry; ++i ) {
    if( m_pSpace->peekw(CAEN_767_OPCODE_STATUS) & (1 << 0) ) {
      //the "read okay" bit is set
      break;
    }
  }
  
  if( i >= maxRetry )  {
    //never broke the loop...
    return(-1);
  }
  
  //take the prescribed 10ms timeout before reading the value
  
  usleep(10000);
  
  //now read the register and return
  *value = m_pSpace->peekw(CAEN_767_OPCODE);
  return( i );
}

/*!
  Do a write to the sequencer.
  \param value    - the value to write.
  \param maxRetry - # times to loop while waiting for the sequencer to allow
                    a write.
   \return int
   \retval >=0   Number of times needed to loop before sequencer was ready.
   \retval <0    Failure.
 */
int 
CAENcard_767::writeOpcode(unsigned short int value, int maxRetry)
{
  int i = opcodeWait(maxRetry);

  if (i >= 0) {
    
    //take the prescribed 10ms timeout before reading the value
    
    usleep(10000);
    
    //now write the register and return
    
    m_pSpace->pokew(value, CAEN_767_OPCODE);
  }
  return( i );

};
/*!
  Wait for the opcode register to be writable.
  \param maxRetry  Number of retries to allow.

   \return int
   \retval >=0   Number of times needed to loop before sequencer was ready.
   \retval <0    Failure.  

 */
int CAENcard_767::opcodeWait(int maxRetry)
{
  int i;  
  for( i = 0; i < maxRetry; ++i ) {
    if( m_pSpace->peekw(CAEN_767_OPCODE_STATUS) & (1 << 1) ) {
      //the "write okay" bit is set
      break;
    }
  }
  
  if( i == maxRetry ) {
    //never broke the loop...
    return(-2);
  }
  
  //take the prescribed 10ms timeout before doing anything else
  
  usleep(10000);
  
  return( i );
  
}

/*! 
  Returns the board id.   This should be 0x2ff which translates to 767.

 */
int CAENcard_767::cardType()
{
  return (( (int)m_pSpace->peekw(CAEN_767_BOARD_ID) << 24)   |
	  ( (int)m_pSpace->peekw(CAEN_767_BOARD_ID+2) << 16) |
	  ( (int)m_pSpace->peekw(CAEN_767_BOARD_ID+4) << 8)  |
	  ( (int)m_pSpace->peekw(CAEN_767_BOARD_ID+6)));
};

/*!
  Clear the data buffer for the module.
*/
void CAENcard_767::clearData()
{
  m_pSpace->pokew(1, CAEN_767_CLEAR);

};


void CAENcard_767::reset()
{
  m_pSpace->pokew(1, CAEN_767_SS_RESET);
  sleep(2);

};

/*!
  Determines if there is any data in the output fifo of the module.
  This function is called by all of the readEvent functions before they read 
  any data.  Your code can also loop on this a while to see if the module
  has data to contribute to an event.

  \return int
  \retval  1 indicates that there is data in the event buffer
  \retval  0 indicates that the event buffer is empty

*/
int CAENcard_767::dataPresent()
{

  unsigned short s1 = m_pSpace->peekw(CAEN_767_STATUS_1);
  
  return ((s1 & 5) == 1 );

};

/*!
  Read an event into an ordinary memory buffer.

  \param buf A pointer to local memory that has already been allocated.
      Should be at least 34 * 4 = 136 bytes to hold the header, footer, and 32
      channels of data.

  \return \li \> 0 indicates the number of BYTES of data placed in buf
          \li 0 indicates that no data was placed in the buffer
          \li -1 is returned if the card is not properly initialized

  Be careful about putting this function into a loop because it can return a negative
  value.
*/
int CAENcard_767::readEvent(void* buf)
{
  int temp, n = dataPresent();
  if(n > 0) {
    n = 0;
    // read until it hits an invalid datum (should there be an option to stop at EOB?)
    temp = m_pSpace->peekl(0);
    while( (temp & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )  {
      *(((int*)buf) + n) = temp;
      ++n;
      temp = m_pSpace->peekl(0);
    }
    n *= 4;  //convert the number of integers to the number of bytes
  }
  
  return(n);
};

/*!
  Read an event into an offset within a spectrodaq word buffer.

  \param wbuf A DAQWordBuffer object to put data in. When using the standard readout
      skeleton this object is created for you.
  \param offset The position that the data should be written to.  This is necessary
      to avoid overwriting other data in the DAQWordBuffer.

  \return \li \> 0 indicates the number of 16-BIT WORDS of data placed in wbuf
          \li 0 indicates that no data was placed in the buffer
          \li -1 is returned if the card is not properly initialized

  Be careful about putting this function into a loop because it can return a negative
  value.
*/
int CAENcard_767::readEvent(DAQWordBuffer& wbuf, int offset)
{
  int n = dataPresent();
  union{			// Assume little endian longword order.
    int dword;
    struct{
      short int low;
      short int high;
    } word;
  } temp;

  if(n > 0)  {
    n = 0;
    // read until it hits an invalid datum (should there be an option to stop at EOB?)
    temp.dword = m_pSpace->peekl(0);
    
    uint16_t* buf = wbuf.GetPtr();
    while( (temp.dword & CAEN_767_DATUM_TYPE) != CAEN_767_FOOTER ) {
      buf[offset] = temp.word.high;
      buf[offset+1] = temp.word.low;
      ++n;
      temp.dword = m_pSpace->peekl(0);
    }
    if( (temp.dword & CAEN_767_DATUM_TYPE) == CAEN_767_FOOTER ) {
      buf[offset] = temp.word.high;
      buf[offset + 1] = temp.word.low;
      ++n;
    }
    
    n *= 2;
  }
  return(n);
};

/*!
  Read an even into a spectrodaq word buffer by way of a 'pointer'.
  \param wp A DAQWordBufferPtr object.

  \return \li \> 0 indicates the number of 16-BIT WORDS of data placed in buf
          \li 0 indicates that no data was placed in the buffer
          \li -1 is returned if the card is not properly initialized

  Be careful about putting this function into a loop because it can return a negative
  value. Also make sure that the pointer does not point to a location that already
  contains data.

  Under normal conditions the readEvent(DAQWordBuffer& wbuf, int offset) fuction is
  much easier and intuitive to use.
*/
int CAENcard_767::readEvent(DAQWordBufferPtr& wp)
{
  int n = dataPresent();
  union{
    int dword;
    struct{
      short int low;
      short int high;
    } word;
  } temp;

  if(n > 0)
  {
    n = 0;
    temp.dword = m_pSpace->peekl(0);
    while( (temp.dword & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    {
      *wp = temp.word.high;
      wp++;
      *wp = temp.word.low;
      wp++;
      ++n;
      temp.dword = m_pSpace->peekl(0);
    }

    n *= 2;
  }
  return(n);
};


/*!
  set up the card. At one point this was considered a temp
  The current setup is common start, all channels enabled
*/

int
CAENcard_767::tempSetup()
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
  opcodeWait(1000000);
  
  return(0);
}
/*!
   Enable hits to be considered as starts  on the rising
edge of the input signals.
*/

int 
CAENcard_767::SetRisingEdgeStart()
{
  if(writeOpcode(0x6400, 1000000) < 0) {
    return -1;
  }
  opcodeWait(1000000);
  return 0;
}
/*!
   Enable hits to be considered as starts on the falling edge
of the input signal.
*/

int
CAENcard_767::SetFallingEdgeStart()
{
  if(writeOpcode(0x6500, 100000) < 0) {
    return -1;
  }
  opcodeWait(100000);
  return 0;
}
/*!
   Enable input rising edges to be considered channel hits.
*/
int
CAENcard_767::SetRisingEdgeAll()
{
  if(writeOpcode(0x6000, 1000000) < 0) {
    return -1;
  }
  opcodeWait(1000000);
  return 0;
}
/*!
   Enable input falling edges to be considered channel hits.
*/
int
CAENcard_767::SetFallingEdgeAll()
{
  if(writeOpcode(0x6100, 1000000) < 0) {
    return -1;
  }
  opcodeWait(1000000);
  return 0;
}
/*!
   Return the edge information for the start trigger:
   Note that negative is bad.
*/
int
CAENcard_767::getStartEdge() {
  unsigned short edges[3];
  int stat = readEdgeConfiguration(edges);
  if (stat < 0) {
    return stat;
  }
  return edges[2];
}
/*!
   Return the edge information for the even channels
*/
int
CAENcard_767::getEdgeEven()
{
  unsigned short edges[3];
  int stat = readEdgeConfiguration(edges);
  if (stat < 0) {
    return stat;
  }
  return edges[0];
}
/*!
  Return edge information for the odd channels.
*/
int 
CAENcard_767::getEdgeOdd()
{
  unsigned short edges[3];
  int stat = readEdgeConfiguration(edges);
  if (stat < 0) {
    return stat;
  }
  return edges[1];  
}
/*!
   Return the value of status register 2.
*/
unsigned short CAENcard_767::getSr2()
{
  return m_pSpace->peekw(CAEN_767_STATUS_2);
}
/*!
   Return the manufacturer's id.
*/
int
CAENcard_767::mfgId()
{
  return ((m_pSpace->peekw(CAEN_767_MANUFACT_ID) <<   16)  |
          (m_pSpace->peekw(CAEN_767_MANUFACT_ID+2) << 8)  |
	  (m_pSpace->peekw(CAEN_767_MANUFACT_ID+4)));
}


/*!
  Read the three words of edge information.
*/
int
CAENcard_767::readEdgeConfiguration(unsigned short* values)
{
  if (writeOpcode(0x6700, 10000) < 0) {
    return -1;
  }
  opcodeWait(1000000);
  if (readOpcode(values, 1000) < 0) { // Even channels..
    return -2;
  }
  *values &= CAEN_767_EDGE_BOTH;
  values++;
  if (readOpcode(values, 1000) < 0) { // Odd channels..
    return -3;
  } 
  *values &= CAEN_767_EDGE_BOTH;
  values++;
  if (readOpcode(values, 1000) < 0) { // Start channel...
    return -4;
  }
  *values &= CAEN_767_EDGE_BOTH;
  return 0;
}
