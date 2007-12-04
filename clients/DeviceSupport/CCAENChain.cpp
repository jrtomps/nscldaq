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

#define DMA_THRESHOLD 34*20*4	// Don't allow it to go DMA.

/*
  Revision history:
  $Log$
  Revision 8.3  2007/05/17 21:26:09  ron-fox
  Work on porting to spectrodq-lite.

  Revision 8.2  2005/06/24 11:30:36  ron-fox
  Bring the entire world onto the 8.2 line

  Revision 4.3  2004/12/07 15:20:21  ron-fox
  - Fix some CVS errors with the wiener driver.
  - Re create the autotools based build for the wiener driver stuff.
  - Actually check that we can compile the stuff selecting the wiener
    vme device

  Revision 4.2  2004/11/16 15:24:48  ron-fox
  - Port to the gnu 3.x compiler set.
  - Integrate buid of tests.
  - Integrate build of docos.

  Revision 1.2  2004/11/16 15:23:28  ron-fox
  - Port -> gcc/g++ 3.x
  - Support integrated test building.
  - Support integrated doxygen docu7mentation building.

  Revision 1.1  2003/12/03 18:45:45  ron-fox
  Update 767 documentation

  Revision 1.2  2003/09/19 19:52:45  ron-fox
  This is probably debugged.  Need to see what CAEN says about data corruption .

  Revision 1.1  2003/09/16 12:16:24  ron-fox
  Added support for CBLT readout of CAEN 32 channel adcs.  Note for SBS/bit3 the driver must be patched to support early termination of block transfers.

*/
#include <config.h>
#include "CCAENChain.h"
#include "CAENcard.h"
#include <CVMEInterface.h>
#ifdef HAVE_SBSVME_INTERFACE
#include <SBSBit3API.h>
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a chain.  The chain can consist either of a
   set of slots defined by [nFirstSlot, nLastSlot] or it can
   consist of a set of cards described by a vector of base addresses
   Note that chains must either be entirely geographically addressed
   or physically addressed.  This is a software restriction.
   
   \param nFirstSlot (int in):
      The first geographical address in the chain. If the
      geo is true, this represents a physical slot, otherwise a
      virtual slot that is programmed into the chain members.
   \param nLastSlot (int in):
      The last geographical address in the chain.  If the geo
      parameter is true, this represents a physical slot, otherwise
      a virtual slot that is programmed into the chain members.
   \param vBases (vector<unsigned long>& in):
      A set of base addresses for the modules in the chain.
      This is only important in the geo parameter is false.
      In that case, there must be nLastSlot - nFirstSlot + 1 entries
      in the vector that represent the base addresses at which
      the non-geographical modules live.
   \param nCrate (int in default = 0):
      Identifies the vme crate in which the chain lives.  Chains
      may not span VME crates.  This is a hardware restriction.
   \param geo (bool [in]):
      - true  - The modules are adressed geographicall, and the 
                nBases parameter is effectively ignored.
     -  false - The modules are addressed by the base addresses
                in vBases and the geographical addresses are just
		programmed into the modules as virtual slots.
   \throw string
      A descriptive error message should there be any problems 
      creating the chain.  The CAENmodules created up to that 
      point will be properly destroyed if an exception is thrown.
*/
CCAENChain::CCAENChain(int nFirstSlot, int nLastSlot,
		       vector<unsigned long>& vBases,
		       int nCrate, bool geo) throw (string)
      : m_nCBLTAddress(0),
	m_nCrate(nCrate),
	m_pHandle(0),
	m_nMaxBytes(0)
{

  // Require legitimate length of chain:

  int nModules = (nLastSlot - nFirstSlot + 1);
  if(nModules < 2) {
    throw string("Chains must have at least two modules!!");
  }
  // If not geo, require sufficient bases:

  if(!geo & (vBases.size() != nModules)) {
    throw string("Insufficient module base addresses in CAENChain");
  }
  

  // Create the modules in the chain.
  // 
  int i = 0;
  try {
    for(int slot = nFirstSlot; slot <= nLastSlot; slot++, i++) {
      m_vCards.push_back(new CAENcard(slot, nCrate, geo,
				      geo ?  0 : vBases[i]));
    }
  } 
  catch(...) {			// On any exception:
    FreeModules();		// Free the modules already created
    throw;			// and rethrow.
  }

  //  The base address of the chain is essentially the first
  //  slot number (shifted left by 16 bits to fall into the 
  //  range of address bits covered by the MCST/CBLT bits in the
  //  module's MCST/CBLT register.
  //

  m_nCBLTAddress = nFirstSlot << 24;
  for(i = 0; i < nModules; i++) {
    CAENcard::ChainMember where(CAENcard::IntermediateInChain);
    if(i == 0) where = CAENcard::FirstInChain;
    if(i == (nModules - 1))  where = CAENcard::LastInChain;
    m_vCards[i]->SetCBLTChainMembership(nFirstSlot, where);
  }

  // Calculate the  number of bytes that can be read:
  // In cblt seems like a module returns
  //  header data trailer invalid 
  //   Data can be 32 longs,
  //   header, trailer and invalid are both a single long.

  m_nMaxBytes = (nModules * 35 + 1) * sizeof(long);

  // The very last thing to do is open the VME crate on
  // CBLT addressing so that we can do a read(2) to read an event:

  try {
    m_pHandle = CVMEInterface::Open(CVMEInterface::CBLT, nCrate);
#ifdef HAVE_SBSVME_INTERFACE
    CSBSBit3VmeInterface::SetDMABlockTransfer(m_pHandle, true);
    CSBSBit3VmeInterface::SetDMAThreshold(m_pHandle, DMA_THRESHOLD);
#endif
  }
  catch (...) {
    FreeModules();
    throw;
  }
				   
  
}
/*!
   Destroy the  chain.  This involves:
   - Freeing all the modules.
   - closing the vme interface handle:
*/
CCAENChain::~CCAENChain() 
{
  FreeModules();
  CVMEInterface::Close(m_pHandle);
}
/*!
    Select a module from the chain.
    \param index (int in):
      Index of the module to select. 
    \return CAENCard* pointer to the module selected.
      The intent is that the user can then configure the module
      appropriately.
    \throw CRangeError
      If the index is out of range.
*/
CAENcard*
CCAENChain::operator[](int index) throw (CRangeError)
{
  if( (index >= 0) && (index < m_vCards.size())) {
    return m_vCards[index];
  }
  else {
    throw CRangeError(0, m_vCards.size(), index,
		      "Selecting a CAENcard from a chain");
  }

}
/*!
  Read an event from the chain.  This is done by issuing a
  read of m_nMaxBytes long.  The cards set themselves up in a 
  chain to Buserr on end of data so the actual number of bytes
  read will, in general, be fewer.

  \param pBuffer (void* out):
     User buffer into which the chain is read.
  \return int
     Number of words actually read.

   \note   This function does not detect buffer overruns.
           A good way to declare the user buffer is either:
	   \verbatim

	   CCAENChain  chain(...);
	   ...
	   long buffer[chain.getMaxBytes()/sizeof(long)];
	   ...
	   ReadEvent(buffer);

	   \endverbatim
	   
	   Or of course:
	   \verbatim

	   ...
	   long* pBuffer = new long[chain.getMaxByte()/sizeof(long)];
	   ...
	   ReadEvent(pBuffer);

	   \endverbatim

 */
int
CCAENChain::ReadEvent(void* pBuffer)
{

  int nRead = 0;
  try {
    nRead = CVMEInterface::Read(m_pHandle,
			m_nCBLTAddress,
			pBuffer, m_nMaxBytes);
  }
  catch (... ) {		//  Try to reset the adapter on error.
    // See if reopening fixes this (ugghhh).
    CVMEInterface::Close(m_pHandle);
    m_pHandle = CVMEInterface::Open(CVMEInterface::CBLT, m_nCrate);
#ifdef HAVE_SBSVME_INTERFACE
    CSBSBit3VmeInterface::SetDMABlockTransfer(m_pHandle, true);
    CSBSBit3VmeInterface::SetDMAThreshold(m_pHandle, DMA_THRESHOLD);    
#endif
    throw;			// But re-report the error.
  }

  return nRead/sizeof(unsigned short);
}
/*!
   Clear the data in all cards.
*/
void
CCAENChain::ClearData()
{
  int ncards = m_vCards.size();
  for(int i = 0; i < ncards; i++) {
    m_vCards[i]->clearData();
  }
}

/*!
  (utility).

  Frees the set of moudules that exist in the chain.  This
  is done both at destruction and for exceptions that occur in the
  constructor.
*/
void
CCAENChain::FreeModules()
{
  for(int i=0; i < m_vCards.size(); i++ ) {
    delete m_vCards[i];
  }
  m_vCards.erase(m_vCards.begin(), m_vCards.end());
}
/*!
   Turns on the EMPTY_PROG bit of the Bit2 control/status register on the right most module in the chain..
   When enabled, this bit implies that the last module in the chain will
   provide a header/trailer longword if it got a gate, if there is no data.
   This is intended to help unambiguously identify the end of data.

   The SBS PCI/VME interface has a deeply pipelined DMA engine.  It is not able
   to reliably deliver partial transfer counts for DMA that end in a BUSERR e.g.
   However, in most cases, doing a CBLT read will result in a partial transfer terminating 
   in a BUSERR.  

   If the modules in the chain all have  a common GATE, this ensures there are always
   a header/trailer from the last module in the chain, and that data can allow an
   unambiguous determination of the end of the chain data.  Typically one would use this
   by placing the data from the chain read in a packet.  Processing the data in the packet
   until the trailer from the last ADC in the chain, but using the packet word count
   to determine where the next chunk of data is.
*/
void 
CCAENChain::lastModuleEmptyEnable() 
{

  CAENcard* pCard = lastCard();
  pCard->emptyEnable();
}

/*!
   This turns off the EMPTY_PROG bit of the Bit2 stat of the right most module in the chain.
   See lastModuleEmptyEnable for an full discussion of  what that means.
   This is the default state (disabled).

*/
void 
CCAENChain::lastModuleEmptyDisable()
{
  CAENcard*  pCard = lastCard();
  pCard->emptyDisable();
}

/*
  Utility to return the last card in the chain (DRY).
*/
CAENcard*
CCAENChain::lastCard()
{
  // Get the pointer to the last module:

  CAENcard* pCard = m_vCards.back();
  return pCard;
}
