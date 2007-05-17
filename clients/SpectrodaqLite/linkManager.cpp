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
#include <spectrodaq.h>

linkManager daq_link_mgr;	//  For the rest of the world.

/*!
   Add a sink -- wraps DAQClentUtils::addSink
   \param DAQURL  - Url pointing to the system to which the link will be formed.
   \param mask1,mask2    - Masks describing which tag ranges will be accepted.
   \param deliveryType - true (COS_UNRELIABLE) for sampled, 
                         false (COS_RELIABLE) for unsampled.
		      
   \return int
   \retval -1 if the sink could not be created.
   \retval sinkid if the sink could be created.

*/
int
linkManager::AddSink(DAQURL& url, int mask1, int mask2, bool deliveryType)
{
  PacketRange range = maskToRanges(mask1 & mask2);

  return DAQClientUtils::addSink(url, range, deliveryType);
}


/*!
  Delete a sink
  \param id - the id of the sink gotten from an AddSink.
    this just wraps DAQClientUtils::delSink.

*/
void
linkManager::DeleteSink(int sinkid)
{
  DAQClientUtils::delSink(sinkid);
}


// map an old style tag mask into a PacketRange.
//
PacketRange
linkManager::maskToRanges(int mask)
{
  PacketRange result;

  int bitnum = 0;
  while (mask) {
    if (mask & bitnum) {
      int lowbit = bitnum;	// range started...
      int hibit  = bitnum;
      mask = mask >> 1;
      bitnum++;
      while (mask & 1) {	// Hunt for range ending.
	hibit++;
	mask = mask >> 1;
	bitnum++;
      }
      // lowbit and hibit are the bit ranges of a contiguous range of bits.
      
      uint32_t lowRange = (1UL << lowbit);
      uint32_t hiRange  = (1UL << hibit);
      result.addRange(lowRange, hiRange);
    }
    mask = mask >> 1;
    bitnum++;
  }

  return result;
}
