
#include <config.h>
#include "CAENcard_767.h"



#include <string>
#include <Iostream.h>
#include <CVMEInterface.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


struct CAENcard_767::CAENcrate_767 CAENcard_767::crate[VME_CRATE_SIZE + 1];

/*!
  \param slotNum  This specifies the slot in the VME crate in which the module resides.
    This value will be the first five bits of every 32-bit word in the output buffer.

  \param crateNum An optional value that the module will write into the header of each
     event/block of data.  May be useful to differential between different cards.
     The Default value is zero.

  If slotNum is not specified, or if it is outside the range of valid values then the
  function returns a "NULL" card.  If the card
  cannot be initialized (wrong version of card, no card in slot) then the function
  returns a card which will be recognized as invalid by the other functions.

  Once the card is verified to be in the slot, one of the supported types, and if it
  is not currently referenced by another instance then it is reset to clear any
  previous settings.  Also, the thresholds are also set to default values and the TDC
  is set to common-start mode.
 */
CAENcard_767::CAENcard_767(int slotNum, int crateNum)
{
  if(slotInit(slotNum, crateNum) < 0)
    slot = 0;
  ++(crate[slot].refCount);
};

/*!
  \param card The card to copy.

  The new CAENcard_767 object will not cause the module to be reset, as this only happens
  the first time that a CAENcard_767 object is created that refers to a given slot.  This
  means that any settings that you have set on the card in that slot will not be
  lost.  Also understand that the destination argument now points to the same card
  in the crate, and therefore any changes to either instance will be reflected in
  all other instances referring to the same card.
  */
CAENcard_767::CAENcard_767(const CAENcard_767& card)
{
  slot = card.slot;
  ++(crate[slot].refCount);
};

/*!
  \param card The CAENcard_767 value to assign to the destination argument.
  \return The value placed into the destination argument.

  The new CAENcard_767 object will not cause the module to be reset, as this only happens
  the first time that a CAENcard_767 object is created that refers to a give  //the value written doesn't matter, it is a dummy register.
  //the VME access is what triggers the reset.
n slot.  This
  means that any settings that you have set on the card in that slot will not be
  lost.  Also understand that the destination argument now points to the same card
  in the crate, and therefore any changes to either instance will be reflected in
  all other instances referring to the same card.
  */
CAENcard_767& CAENcard_767::operator=(const CAENcard_767& card)
{
  --(crate[slot].refCount);
  destruct();  //free the resources associated with the first argument
  slot = card.slot;
  ++(crate[slot].refCount);
  return(*this);
};

/*!
  \param slotNum  This specifies the slot in the VME crate in which the module resides.  This value will be the first five bits of every 32-bit word in the output buffer.
  \param crateNum An optional value that the module will write into the header of each event/block of data.  May be useful to differential between different cards.

  \return Failure to initialize the card is indicated by a return value less than zero.  Each failure returns a different value and are numbered sequentially through the function.

  The calling function must increment the value of
  CAENcard_767::crate[#slot].\link CAENcrate#refCount refCount\endlink after calling
  this function (if appropriate).  The memory map and file descriptor associated
  with this slot will be allocated and opened, respectively, if not already present.
  The status of the slot is then set to be CAEN_MODE_A32D16 if successful.
*/
int CAENcard_767::slotInit(int slotNum, int crateNum)
{
  unsigned int temp[2];

  //ensure that the slot and crate specified stay within bounds
//  crateNum = crateNum & 0xFF;  //not important enough to give an error for, just discard the extra bits
  slot = slotNum;
  if( slot == 0 )	//nothing to do... dummy card created by empty constructor
  {
    return(0);
  }

  if(slot > VME_CRATE_SIZE)
  {
    perror("Invalid slot number specified to slotInit(). ");
    return(-1);
  }

  //check to see if the card in this slot is already being referenced
  if(crate[slot].refCount > 0 && (crate[slot].status & CAEN_MODE_A32D16))
  {
    //set the crate number as requested by the calling program (defaults to zero)
//    setCrate(crateNum);

    return(0);
  }

  //the card is not initialized yet, so do it
  crate[slot].status = CAEN_MODE_UNINIT;

  try {
    crate[slot].fd = 
      CVMEInterface::Open(CVMEInterface::Geographical, crateNum);
  }
  catch (string& msg) {
    cerr << "Could not open vme crate: " << crateNum 
	 << " for geographical addressing: " << msg << endl;
    return -1;
  }


  try {
    crate[slot].mbuf = (unsigned short int*)
      CVMEInterface::Map(crate[slot].fd,
			 slot << 19,
			 CAEN_767_CARD_MMAP_LEN);
  }
  catch (string& msg) {
    cerr << "Could not map crate: " << crateNum << " slot: " << slot
	 << " : " << msg << endl;
    CVMEInterface::Close(crate[slot].fd);
    crate[slot].mbuf = 0;
    crate[slot].fd = NULL;
    return(-6);
 
  }

  if(  !( 0x0000 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID) &&
          0x0040 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 2) &&
	  /*  0x00E6 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 3) && */
          0x0000 == *(crate[slot].mbuf + CAEN_767_BOARD_ID) &&
          0x0000 == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 2) &&
          0x0002 == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 4) &&
          0x00FF == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 6) &&
          (short int)slot == ( 0x001F & (*(crate[slot].mbuf + CAEN_767_ADDR_GEO)) ) )  )
    {   //either an invalid board or no board is present in this slot
      printf( "\n767 Card %d is not inserted or is of an incompatable type!\n", slotNum);
      printf("  One of the following tests has failed\n");
      printf("    0x0000 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID) );
      printf("              0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 1) );
      printf("    0x0040 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 2) );
      printf("              0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 3) );
      printf("    0x00E6 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 4) );
      printf("              0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 5) );
      printf("    0x0000 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_BOARD_ID) );
      printf("    0x0000 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_BOARD_ID + 2) );
      printf("    0x0002 == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_BOARD_ID + 4) );
      printf("    0x00FF == 0x%4.4X\n", *(crate[slot].mbuf + CAEN_767_BOARD_ID + 6) );
      printf("    0x%4.4X == 0x%4.4X & 0x001F\n", (short int)slot, *(crate[slot].mbuf + CAEN_767_ADDR_GEO) );

      CVMEInterface::Unmap(crate[slot].fd,
			   (void*)crate[slot].mbuf,
			   CAEN_767_CARD_MMAP_LEN);
      CVMEInterface::Close(crate[slot].fd);

      crate[slot].mbuf = 0;
      crate[slot].fd = NULL;
      return(-7);
    }


  //card access and initialization was successful
  crate[slot].status = CAEN_MODE_GEO24;

  //disable the auto load of the user configuration at the next reset
  if(  ( writeOpcode(0x1900, 1000000) < 0 ) || ( opcodeWait(1000000) < 0 )  )
  {
    //failed to write the opcode...
    CVMEInterface::Unmap(crate[slot].fd,
			 (void*)crate[slot].mbuf,
			 CAEN_767_CARD_MMAP_LEN);
    CVMEInterface::Close(crate[slot].fd);

    crate[slot].mbuf = 0;
    crate[slot].fd = NULL;
    return(-10);
  }
  reset();

//end of slot initialization, begin address initialization

  //set the address in the registers based upon the slot number
  //the 32bit address becomes (slot<<24)
  *(crate[slot].mbuf + CAEN_767_ADDR_32) = slot;
  *(crate[slot].mbuf + CAEN_767_ADDR_24) = 0;
  //set "bit set 1" register to use address in registers for addressing
  *(crate[slot].mbuf + CAEN_767_BIT_SET) = 1<<4;

  //destroy GEO24 mmap and file descriptor

  CVMEInterface::Unmap(crate[slot].fd,
		       (void*)crate[slot].mbuf,
		       CAEN_767_CARD_MMAP_LEN);
  CVMEInterface::Close(crate[slot].fd);

  crate[slot].mbuf = 0;
  crate[slot].fd = NULL;

  // Create A32 map: Required to access the event memory buffer.

  try {
    crate[slot].fd = 
      CVMEInterface::Open(CVMEInterface::A32, crateNum);
  }
  catch (string& msg) {
    cerr << "Unable to open crate " << crateNum << " as A32\n";
    cerr << msg << endl;
    crate[slot].status = CAEN_MODE_UNINIT;
    return(-11);
  }

  //create A32D16 mmap

  try { 
    crate[slot].mbuf = (unsigned short int*) 
      CVMEInterface::Map(crate[slot].fd, 
			 slot << 24, 
			 CAEN_767_CARD_MMAP_LEN);
  }
  catch (string& msg) {
    cerr << "Unable to establish an a32 map to crate: " << crateNum
	 << " slot: " << slot << endl;
    cerr << msg << endl;
    CVMEInterface::Close(crate[slot].fd);
    crate[slot].mbuf = 0;
    crate[slot].fd = NULL;
    crate[slot].status = CAEN_MODE_UNINIT;
    return(-12);

  }

  crate[slot].status = CAEN_MODE_A32D16;

  //double check that the ROM is still readable
  if(  !( 0x0000 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID) &&
          0x0040 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 2) &&
       /* 0x00E6 == *(crate[slot].mbuf + CAEN_767_MANUFACT_ID + 4) && */
          0x0000 == *(crate[slot].mbuf + CAEN_767_BOARD_ID) &&
          0x0000 == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 2) &&
          0x0002 == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 4) &&
          0x00FF == *(crate[slot].mbuf + CAEN_767_BOARD_ID + 6) &&
          (short int)slot == ( 0x001F & (*(crate[slot].mbuf + CAEN_767_ADDR_GEO)) ) )  )
  {   //either an invalid board or no board is present in this slot
    printf( "\nIntegrity check failed after remapping card!\n" );

    CVMEInterface::Unmap(crate[slot].fd,
			 (void*)crate[slot].mbuf,
			 CAEN_767_CARD_MMAP_LEN);
    crate[slot].mbuf = 0;
    CVMEInterface::Close(crate[slot].fd);
    crate[slot].fd = NULL;
    return(-13);
  }

  //now set any desired default values (thresholds, range, operating mode, etc )
  if(cardType() == 767)
  {
    //load the default configuration (only problem is that it is "Stop Trigger Matching")
    if(  ( writeOpcode(0x1500, 1000000) < 0 ) || ( opcodeWait(1000000) < 0 )  )
    {
      //failed to write the opcode
      CVMEInterface::Unmap(crate[slot].fd,
			   (void*)crate[slot].mbuf, 
			   CAEN_767_CARD_MMAP_LEN);
      crate[slot].mbuf = 0;
      CVMEInterface::Close(crate[slot].fd);
      crate[slot].fd = NULL;
      return(-14);
    }
  }

  return(0);
};

/*! The reference count of the slot number is decreased and then the destruct()
    function is called.  If the reference count is less than one then the items
    in CAENcard_767::crate[#slot] are destroyed. (memory map unmapped, file descriptor
    closed, status set to uninitialized)
*/
CAENcard_767::~CAENcard_767()
{
  --(crate[slot].refCount);
  destruct();
};

/*! The calling function for this function must decrement value of
    CAENcard_767::crate[#slot].\link CAENcrate#refCount refCount\endlink before calling
    this function (if appropriate).  The memory map and file descriptor associated
    with this slot will be released and closed, respectively. The status of the
    slot is then set to be uninitialized.
*/
void CAENcard_767::destruct()
{
  int temp[2];
  if( crate[slot].refCount < 1)
  {
    crate[slot].refCount = 0;
    if(slot > 0 )
    {
      if (crate[slot].mbuf)
      {
	CVMEInterface::Unmap(crate[slot].fd,
			     (void*)crate[slot].mbuf, 
			     CAEN_767_CARD_MMAP_LEN);
      }
      crate[slot].mbuf = 0;
      if(crate[slot].fd != NULL)
      {
        CVMEInterface::Close(crate[slot].fd);
      }
      crate[slot].fd = NULL;
      crate[slot].status = CAEN_MODE_UNINIT;
    }
  }
};

int CAENcard_767::readOpcode(unsigned short int *value, int maxRetry)
{
  if( crate[slot].status )
  {
    int i;  //must declare outside of the loop so I can use the value after the loop is finished
    for( i = 0; i < maxRetry; ++i )
    {
      if( (*(crate[slot].mbuf + CAEN_767_OPCODE_STATUS)) & (1 << 0) )
      {
        //the "read okay" bit is set
        break;
      }
    }

    if( i == maxRetry )
    {
      //never broke the loop...
      return(-2);
    }

    //take the prescribed 10ms timeout before reading the value
    usleep(10000);

    //now read the register and return
    *value = *(crate[slot].mbuf + CAEN_767_OPCODE);
    return( i );
  }
  else
  {
    return(-1);
  }
};

int CAENcard_767::writeOpcode(unsigned short int value, int maxRetry)
{
  if( crate[slot].status )
  {
    int i;  //must declare outside of the loop so I can use the value after the loop is finished
    for( i = 0; i < maxRetry; ++i )
    {
      if( (*(crate[slot].mbuf + CAEN_767_OPCODE_STATUS)) & (1 << 1) )
      {
        //the "write okay" bit is set
        break;
      }
    }

    if( i == maxRetry )
    {
      //never broke the loop...
      return(-2);
    }

    //take the prescribed 10ms timeout before reading the value
    usleep(10000);

    //now read the register and return
    *(crate[slot].mbuf + CAEN_767_OPCODE) = value;

    return( i );
  }
  else
  {
    return(-1);
  }
};

int CAENcard_767::opcodeWait(int maxRetry)
{
  if( crate[slot].status )
  {
    int i;  //must declare outside of the loop so I can use the value after the loop is finished
    for( i = 0; i < maxRetry; ++i )
    {
      if( (*(crate[slot].mbuf + CAEN_767_OPCODE_STATUS)) & (1 << 1) )
      {
        //the "write okay" bit is set
        break;
      }
    }

    if( i == maxRetry )
    {
      //never broke the loop...
      return(-2);
    }

    //take the prescribed 10ms timeout before doing anything else
    usleep(10000);

    return( i );
  }
  else
  {
    return(-1);
  }
}

/*! \return \li If the call succeeds then 767 will be returned as an integer.
            \li If the call fails (if it is called on an uninitialized card) then -1 will be returned.
 */
int CAENcard_767::cardType()
{
  if( crate[slot].status )
  {
    //    return( *(crate[slot].mbuf + CAEN_767_BOARD_ID) );
    return(767);
  }
  else
  {
//    printf("Attempted to determine the card type of an uninitialized card.\n");
    return(-1);
  }
};

/*!
  No settings on the module are altered by this call.
*/
void CAENcard_767::clearData()
{
  if (crate[slot].status & CAEN_MODE_A32D16)
    *(crate[slot].mbuf + CAEN_767_CLEAR) = 1;
  //the value written doesn't matter, it is a dummy register.
  //the VME access is what triggers the data clear.
};


void CAENcard_767::reset()
{
  if( crate[slot].status )
  {
    //the value written doesn't matter, it is a dummy register.
    //the VME access is what triggers the reset.
    *(crate[slot].mbuf + CAEN_767_SS_RESET) = 1;
    sleep(2);
  }
};

/*!
  \return \li 1 indicates that there is data in the event buffer
          \li 0 indicates that the event buffer is empty
          \li -1 is returned when the card has not been initialized

  This function is called by all of the readEvent functions before they read any data.
*/
int CAENcard_767::dataPresent()
{
  if(crate[slot].status & CAEN_MODE_A32D16)
  {
    // Wait for data ready without a busy.

    unsigned short s1 = *(crate[slot].mbuf + CAEN_767_STATUS_1);

    return ((s1 & 5) == 1 );
  }
  else
  {
    return(-1);
  }
};

/*!
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
  if(n > 0)
  {
    n = 0;
    // read until it hits an invalid datum (should there be an option to stop at EOB?)
    temp = *(int*)(crate[slot].mbuf);
    while( (temp & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    {
      *(((int*)buf) + n) = temp;
      ++n;
      temp = *(int*)(crate[slot].mbuf);
    }
    n *= 4;  //convert the number of integers to the number of bytes
  }

  return(n);
};

/*!
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
    // read until it hits an invalid datum (should there be an option to stop at EOB?)
    temp.dword = *(int*)(crate[slot].mbuf);
//    while( (temp.dword & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    while( (temp.dword & CAEN_767_DATUM_TYPE) != CAEN_767_FOOTER )
    {
      wbuf[offset] = temp.word.high;
      wbuf[offset+1] = temp.word.low;
      ++n;
      temp.dword = *(int*)(crate[slot].mbuf);
    }
    if( (temp.dword & CAEN_767_DATUM_TYPE) == CAEN_767_FOOTER )
    {
      wbuf[offset] = temp.word.high;
      wbuf[offset + 1] = temp.word.low;
      ++n;
    }

    n *= 2;
  }
  return(n);
};

/*!
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
    temp.dword = *(int *)(crate[slot].mbuf);
    while( (temp.dword & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    {
      *wp = temp.word.high;
      wp++;
      *wp = temp.word.low;
      wp++;
      ++n;
      temp.dword = *(int *)(crate[slot].mbuf);
    }

    n *= 2;
  }
  return(n);
};

/*!
  \param dwbuf A DAQDWordBuffer object to put data in.
  \param offset The position that the data should be written to.  This is necessary
      to avoid overwriting other data in the DAQDWordBuffer.

  \return \li \> 0 indicates the number of 32-BIT DWORDS of data placed in dwbuf
          \li 0 indicates that no data was placed in the buffer
          \li -1 is returned if the card is not properly initialized

  Be careful about putting this function into a loop because it can return a negative
  value. Note that the standard readout skeleton does not provide you with a
  DAQDWordBuffer.
*/
int CAENcard_767::readEvent(DAQDWordBuffer& dwbuf, int offset)
{
  int temp, n = dataPresent();
  if(n > 0)
  {
    n = 0;
    temp = *(int*)(crate[slot].mbuf);
    while( (temp & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    {
      dwbuf[offset + n] = temp;
      ++n;
      temp = *(int*)(crate[slot].mbuf);
    }
  }
  return(n);
};

/*!
  \param wp A pointer to a DAQDWordBuffer object.

  \return \li \> 0 indicates the number of 32-BIT WORDS of data placed in buf
          \li 0 indicates that no data was placed in the buffer
          \li -1 is returned if the card is not properly initialized

  Be careful about putting this function into a loop because it can return a negative
  value. Note that the standard readout skeleton does not provide you with a
  DAQDWordBuffer. Also make sure that the pointer does not point to a location that
  already contains data.

  Under normal conditions the readEvent(DAQDWordBuffer& dwbuf, int offset) fuction is
  much easier and intuitive to use.
*/
int CAENcard_767::readEvent(DAQDWordBufferPtr& dwp)
{
  int temp, n = dataPresent();
  if(n > 0)
  {
    n = 0;
    temp = *(int *)(crate[slot].mbuf);
    while( (temp & CAEN_767_DATUM_TYPE) != CAEN_767_INVALID )
    {
      *dwp = temp;
      dwp++;
      ++n;
      temp = *(int *)(crate[slot].mbuf);
    }
  }
  return(n);
};

/*!
  Not likely to be of any use except to allow the cards to be used in an STL class.
*/
bool operator< (const CAENcard_767& card1, const CAENcard_767& card2)
{
  return card1.slot < card2.slot;
};


