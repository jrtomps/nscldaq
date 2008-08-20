#ifndef __CVMUSBOUTPUTTHREAD_H
#define __CVMUSBOUTPUTTHREAD_H
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

#ifndef __COUTPUTTHREAD_H
#include <COutputThread.h>
#endif

/*!
  This class implements the pure virtual member functions that are needed
  to make an output thread the output thread for a VM-USB module.
*/
class CVMUSBOutputThread : public COutputThread
{
public:
  virtual unsigned bodySize(DataBuffer& buffer);
  virtual unsigned eventCount(DataBuffer& buffer) ;
  virtual unsigned headerSize(DataBuffer& buffer) ;
  virtual uint32_t eventSize(uint16_t* pEvent)    ;
  virtual void     processEvent(time_t when, 
				uint32_t size, 
				uint16_t* pEvent)   ;

};

#endif
