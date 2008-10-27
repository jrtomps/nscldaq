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
#include "CCUSBUnpacker.h"
#include <Event.h>
#include <Analyzer.h>
#include <BufferDecoder.h>
#include <TCLAnalyzer.h>
#include "ParamMapCommand.h"

using namespace std;


/*
  The table below is the number of bits set in a 4 bit mask.
  e.g. bitCount[5] = 2 because 5 = 101b.

  this is used to speed up the couting of bits in a 16 bit mask by
  turning it in to 4 table lookups rather than 16 bit checks.

*/

static int bitCount[16] = {
  0,                            // 0000
  1,                            // 0001
  1,                            // 0010
  2,                            // 0011
  1,                            // 0100
  2,                            // 0101
  2,                            // 0110
  3,                            // 0111
  1,                            // 1000
  2,                            // 1001
  2,                            // 1010
  3,                            // 1011
  2,                            // 1100
  3,                            // 1101
  3,                            // 1110
  4 };                          // 1111


///////////////////////////////////////////////////////////////////////////////////
/*!
    Analyze an event.
    \param pEvent    - Pointer to the raw event.  We assume that buffer byte order
                        signatures are present that will allow us to determine the
			byte ordering.
    \param rEvent    - The parameter 'array' into which this data will be unpacked.
    \param rAnalyzer - The analyzer that's in charge of this buffer.  This is
                       a CTCLAnalyzer to which we need to give the size of the event
    \param rDecoder  - The buffer decoder that understands the outer structure of the
                       event buffer.  We're going to use it to get byte ordering to
                       construct a translating pointer so we can reference the
                       data correctly even if there is a byte ordering mis-match
                       between this system and the one that created the data.
*/
Bool_t
CCUSBUnpacker::operator()(const Address_t pEvent,
			    CEvent&         rEvent,
			    CAnalyzer&      rAnalyzer,
			    CBufferDecoder& rDecoder)
{

  // Get all the book-keeping out of the way and create a pointer
  // p that points to the event body, and nWords, a remaining word count.
  //

  TranslatorPointer<UShort_t> p(*(rDecoder.getBufferTranslator()), pEvent);
  CTclAnalyzer&            rAna((CTclAnalyzer&)rAnalyzer);
  UShort_t               nWords = *p++;   	// Word count and pointer to body.
  rAna.SetEventSize((nWords+1)*sizeof(UShort_t));

  
  // Note in the CCUSB the word count is not self inclusive!


  CParamMapCommand* pMap   = CParamMapCommand::getInstance();
  int               module = 0;


  // Get the data
  //  The CCUSB puts in an extra word after the end of a QStop transfer.
  //  (the data when Q was not set).   I've arranged that the 
  //  max transfer count be 17 not 16 so that there's always
  //  that extra word, and therefore we know to unconditionally
  //  skip it.

  while (nWords > 0) {

    const CParamMapCommand::ParameterMap* moduleInfo= pMap->getModuleMap(module);

    CParamMapCommand::ModuleType type = moduleInfo->s_type;
    const vector<int>* moduleMap = &(moduleInfo->s_parameterIds); 

    if (module > pMap->getMapSize()) {
      return kfFALSE;		// bad event!
    }

   
    if (type == CParamMapCommand::Phillips) {

      int hitPattern         = *p++;
      nWords--;
      int hits               = bitsInMask(hitPattern);
      // For now we'll just decode the event and at each step check the moduleMap..could tune this.
      
      while (hits) {
	
	UShort_t word     = *p++;
	hits--;
	nWords--;
	
	UShort_t channel  = (word >> 12) & 0xf;
	UShort_t value    = (word & 0xfff);
	
	if (moduleMap  && (channel < moduleMap->size())) {
	  int param  = (*moduleMap)[channel];
	  if (param >= 0) {
	    rEvent[param] = value;
	  }
	}
      }
      // Skip the extra word at the end of the q-stop.
      p++;
      nWords--;
      // On to the next module.
      module++;
    }
    
    if (type  == CParamMapCommand::Ortec) {
      for (int param =0; param < 8; param++)
	{     
	  UShort_t word     = *p++;    
	  nWords--;
	  UShort_t value    = ( (word >> 5) & 0x7ff);
	  rEvent[param] = value;
	}      
      p++;
      module++;
    }
  }
  
  // Must return true to histogram.
  
  return kfTRUE;
}


////////////////////////////////////////////////////////////////////////////////////
/*
   Returns the number of bits in 16 bit mask that is passed into us.
   We use the bitCount array to speed this up to 4 table lookups from
   16 bit tests.
   Parameters:
      mask   - The mask in which to count bits.
*/
int
CCUSBUnpacker::bitsInMask(UShort_t mask)
{
  int count = 0;

  // The loop below is faster than a counted loop because
  // - there's no increment, and we have to shift the mask anyway.
  // - We'll short cut out of the loop in there are upper nybbles that don't
  //   have bits set.
  //
  while(mask) {
    count += bitCount[mask & 0xf];  // Count lowest 4 bits.
    mask   = mask >> 4;		    // Next 4 bits.
  }
  return count;
}
