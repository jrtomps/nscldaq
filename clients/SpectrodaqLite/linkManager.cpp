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

DAQLinkMgr daq_link_mgr;	//  For the rest of the world.

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
DAQLinkMgr::AddSink(DAQURL& url, int mask1, int mask2, bool deliveryType)
{
  PacketRange range = maskToRanges(mask1, mask2);

  return DAQClientUtils::addSink(url, range, deliveryType);
}


/*!
  Delete a sink
  \param id - the id of the sink gotten from an AddSink.
    this just wraps DAQClientUtils::delSink.

*/
void
DAQLinkMgr::DeleteSink(int sinkid)
{
  DAQClientUtils::delSink(sinkid);
}


// map an old style tag mask pair  into a PacketRange.
// This is simplified to only work with contiguous bit ranges.
//
PacketRange
DAQLinkMgr::maskToRanges(int mask, int careBits)
{
  PacketRange result;

  int lowTag = mask & careBits;

  result.addRange(lowTag, mask);
  return result;

}
