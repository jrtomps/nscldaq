#ifndef __ASSEMBLEDSCALEREVENT_H
#define __ASSEMBLEDSCALEREVENT_H
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

#ifndef __ASSEMBLEDEVENT_H
#include "AssembledEvent.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT
#include <stdint.h>
#ifndef __CRT_STDINT
#define __CRT_STDINT
#endif
#endif


/*!
   This assembled event holds scaler data...either snapshot or 
   non-snapshot (note that we have never implemented snapshot 
   scaler events for the unix nscldaq and nobody has ever asked
   us to do that so knock on wood.. nonentheless, this 
   class can have either type 2 (non-snapshot) or type 3
   data (snapshots).

   For those who actually bother to read this stuff, a snapshot
   scaler is a scaler event that does not flush out partial event
   buffers.  This can be useful to get a higher rate of scalers to
   display than to the recorded event file, but without a pile
   of partial event buffers in the recorded event file.  
   Snapshot scaler buffers get totalled into the non-snapshot
   scaler buffers... if we ever implement them that is.
*/
class AssembledScalerEvent : public AssembledEvent
{
private:
  unsigned long         m_startTime;  
  unsigned long         m_stopTime;
  std::vector<uint32_t> m_scalers;

public:
  AssembledScalerEvent(unsigned short node,
		       unsigned long startTime,
		       unsigned long endTime,
		       AssembledEvent::BufferType type = AssembledEvent::Scaler);

  // Access the times:

  unsigned long getStartTime() const;
  unsigned long getEndTime()   const;

  // Put/get scalers in the beast:

  void addScalers(void* pScalers, size_t count);
  void addScalers(std::vector<uint32_t> scalers);

  std::vector<uint32_t> getScalers() const;
  uint32_t& operator[](unsigned int index);

  // delegations to the m_scalers vector:

  size_t size() const;
  std::vector<uint32_t>::iterator begin();
  std::vector<uint32_t>::iterator end();
 
};



#endif


