/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


// #ifndef VMEMODULE_H
// #define VMEMODULE_H

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
  \class CVmeModule
  \file CVmeModule.cpp

  Implements a Vme module object. CVmeModules have access to
  memory which is mapped via the mmap(3) system service using 
  the CVME and CVMEptr objects. CVmeModules can write to and read 
  from these registers using poke and peek operations, respectively.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/

#include <config.h>

#include "VmeModule.h"

#include <string>


using namespace std;


/*

   The array below maps from address space selectors to CVME<uint16_t 
   selectors or, in the caes of WienerVME controllers, the address modifier.
   
*/
static const

CVME<uint16_t>::VmeSpace AmodTable[] = {
   CVME<uint16_t>::a16d16,
   CVME<uint16_t>::a24d16,
   CVME<uint16_t>::a24d32,
   CVME<uint16_t>::a32d32,
   CVME<uint16_t>::geo
};

/*
  \fn CVmeModule::CVmeModule(Space space, uint32_t base, uint32_t length)

  Operation type:
     Basic constructor

  Purpose: 
     Contructs an object of type CVmeModule

  \param Space space   - the vme device to which we are mapping
  \param uint32_t base   - the base address of the device
  \param uint32_t length - the length of the mapping (bytes)
  \param int nCrate    - VME crate number.
*/
CVmeModule::CVmeModule(CVmeModule::Space space, uint32_t base, 
		       uint32_t length, int nCrate)

{

  try {
    switch(space) {
    case a16d16:
      m_CVME = CVME<uint16_t>(CVME<uint16_t>::a16d16, base, length, nCrate);
      break;
    case a24d16:
      m_CVME = CVME<uint16_t>(CVME<uint16_t>::a24d16, base, length, nCrate);
      break;
    case a24d32:
      m_CVME = CVME<uint16_t>(CVME<uint16_t>::a24d32, base, length, nCrate);
      break;
    case a32d32:
      m_CVME = CVME<uint16_t>(CVME<uint16_t>::a32d32, base, length, nCrate);
      break;
    case geo:
      m_CVME =  CVME<uint16_t>(CVME<uint16_t>::geo, base, length);
      break;
    default:
      throw 1;			// Force the catch below to complain.
    }
  }
  catch(int i) {
    throw string("Invalid address space in CVMEModule constructor");
  }
}


/*
  \fn CVmeModule::CVmeModule(const CVmeModule& aCVmeModule)

  Operation Type:
     Copy contructor

  Purpose:
     Constructs this by copying the attributes of its parameter

  \param const CVmeModule& aCVmeModule - a CVmeModule from which 
                                         to contrust this
*/
CVmeModule::CVmeModule(const CVmeModule& aCVmeModule)
{
 
   CopyToMe(aCVmeModule);
}

/*
  \fn CVmeModule& CVmeModule::operator=(const CVmeModule& aCVmeModule)
  
  Operation Type:
     operator= Assignment operator

  Purpose:
     Assigns this object the same attributes as another CVmeModule object

  \param const CVmeModule& aCVmeModule - the CVmeModule to copy from
*/
CVmeModule&
CVmeModule::operator=(const CVmeModule& aCVmeModule)
{
  if(this == &aCVmeModule) return *this;
  
  CopyToMe(aCVmeModule);
  return *this;
}

/*
  \fn int CVmeModule::operator== (const CVmeModule& aCVmeModule)

  Operation Type:
     operator== Eqaulity operator

  Purpose:
     Determine if this object is equal to its parameter

  \param const CVmeModule& aCVmeModule - the CVmeModule with which to compare
*/
int
CVmeModule::operator== (const CVmeModule& aCVmeModule)
{

  return (m_CVME == aCVmeModule.m_CVME);

}

/*
  \fn uint8_t CVmeModule::peekb(uint32_t offset=0)

  Operation Type:
     Accessor

  Purpose:
     Reads byte from m_CVME at current offset + offset, which is
     defaulted to 0. Throws an exception if the read is out of bounds.

  \param uint32_t offset - the offset from which to read (or the current offset
                         if not specified.
*/
uint8_t
CVmeModule::peekb(uint32_t offset)
{

  volatile uint8_t* device = m_CVME.asChar();
   return device[offset];

}

/*
  \fn uint16_t CVmeModule::peekw(uint32_t offset=0)

  Operation Type:
     Accessor

  Purpose:
     Reads word from m_CVME at current offset + offset, which is
     defaulted to 0. Throws an exception if the read is out of bounds.

  \param uint32_t offset - the offset from which to read (or current offset
                         if unspecified.
*/
uint16_t
CVmeModule::peekw(uint32_t offset)
{

   volatile uint16_t* device = m_CVME.asShort();
   return device[offset];

}

/*
  \fn uint32_t CVmeModule::peekl(uint32_t offset=0)

  Operation Type:
     Accessor

  Purpose:
     Reads longword from m_CVME at current offset + offset, which is
     defaulted to 0. Throws an exception if the read is out of bounds.

  \param uint32_t offset - the offset from which to read (reads from current
                         offset if this is unspecified).
*/
uint32_t
CVmeModule::peekl(uint32_t offset)
{
   volatile uint32_t* device = m_CVME.asInt32();
   return device[offset];

}

/*
  \fn void CVmeModule::pokeb(uint8_t byte, uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Writes the specified byte to the specified offset in the module.
     Throws an exception if the specified offset is out of range.

  \param uint8_t byte  - the byte to write into the memory map
         uint32_t offset - the offset at which to write the byte.
*/
void
CVmeModule::pokeb(uint8_t byte, uint32_t nOffset)
{

    (m_CVME.asChar())[nOffset] = byte;

}

/*
  \fn void CVmeModule::pokew(uint16_t word, uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Writes the specified word to the specified offset in the module.
     Throws an exception if the specified offset is out of range.

  \param uint8_t word  - the word to write into the memory map
         uint32_t offset - the offset at which to write the word.
*/
void
CVmeModule::pokew(uint16_t word, uint32_t nOffset)
{

    (m_CVME.asShort())[nOffset] = word;

}

/*
  \fn void CVmeModule::pokel(uint8_t lword, uint32_t nOffset)

  Operation Type:
     Mutator

  Purpose:
     Writes the specified long word to the specified offset in the module.
     Throws an exception if the specified offset is out of range.

  \param uint8_t long word  - the long word to write into the memory map
         uint32_t offset - the offset at which to write the long word.
*/
void
CVmeModule::pokel(uint32_t lword, uint32_t nOffset)
{

   (m_CVME.asInt32())[nOffset] = lword;

}
/*!
    Utility function to copy an object to me.
*/
void
CVmeModule::CopyToMe(const CVmeModule& rModule)
{

  m_CVME = rModule.m_CVME;

}


/*!
    Read a block of longwords from the VME.  At present, we 
    assume that for mapped interfaces, transfers are faster if done
    under programmed control.  In future releases we may need to
    think about a threshold above which we ask the driver to 
    do the transfers for us.  The real motiviation for this
    function are the devices that do not have mapping...
    for them it's almost certain, that letting the driver do
    the block transfer is better than us doing it since
    the driver overhead will be amortized over the transfer count.

\param pBuffer - The target of the transfer.  This buffer must be at least
                 longs*sizeof(long) large.
\param nOffset - The longword offset into the region represented by
                 this class.
\param longs   - The number of longwords to transfer.

\return uint32_t
\retval number of longs transferred if everything worked or some device
        dependent result.
*/
uint32_t
CVmeModule::readl(void* pBuffer, uint32_t nOffset, size_t longs)
{
  uint32_t* pSource = (uint32_t*)m_CVME.asInt32() + nOffset;
  uint32_t* pDest   = (uint32_t*)pBuffer;
  for(uint32_t i =0; i < longs; i++) { // memcpy is not ensured to be long transfers.
    *pDest++ = *pSource++;
  }
  return longs;


}

/*!
    Read a block of words from the VME.  At present, we 
    assume that for mapped interfaces, transfers are faster if done
    under programmed control.  In future releases we may need to
    think about a threshold above which we ask the driver to 
    do the transfers for us.  The real motiviation for this
    function are the devices that do not have mapping...
    for them it's almost certain, that letting the driver do
    the block transfer is better than us doing it since
    the driver overhead will be amortized over the transfer count.

\param pBuffer - The target of the transfer.  This buffer must be at least
                 longs*sizeof(uint16_t) large.
\param nOffset - The word offset into the region represented by
                 this class.
\param words   - The number of words to transfer.

\return uint32_t
\retval number of words transferred if everything worked or some device
        dependent result.
*/
uint32_t
CVmeModule::readw(void* pBuffer, uint32_t nOffset, size_t words)
{

  uint16_t* pSource = (uint16_t*)m_CVME.asShort() + nOffset;
  uint16_t* pDest   = (uint16_t*)pBuffer;
  for(uint32_t i =0; i < words; i++) { // memcpy is not ensured to be word transfers.
    *pDest++ = *pSource++;
  }
  return words;

}

/*!
    Read a block of bytes from the VME.  At present, we 
    assume that for mapped interfaces, transfers are faster if done
    under programmed control.  In future releases we may need to
    think about a threshold above which we ask the driver to 
    do the transfers for us.  The real motiviation for this
    function are the devices that do not have mapping...
    for them it's almost certain, that letting the driver do
    the block transfer is better than us doing it since
    the driver overhead will be amortized over the transfer count.

\param pBuffer - The target of the transfer.  This buffer must be at least
                 longs bytes  large.
\param nOffset - The byte offset into the region represented by
                 this class.
\param bytes   - The number of bytes to transfer.

\return uint32_t
\retval number of bytes transferred if everything worked or some device
        dependent result.
*/
uint32_t
CVmeModule::readb(void* pBuffer, uint32_t nOffset, size_t bytes)
{

  uint8_t* pSource = (uint8_t*)m_CVME.asChar() + nOffset;
  uint8_t* pDest   = (uint8_t*)pBuffer;
  for(uint32_t i =0; i < bytes; i++) { // memcpy is not ensured to be long transfers.
    *pDest++ = *pSource++;
  }
  return bytes;



}



// #endif
