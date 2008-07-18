#ifndef __CVMUSBDEVICEDRIVER_H
#define __CVMUSBDEVICEDRIVER_H
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

#ifndef __CACQUISTIONTHREAD_H
#include <CAcquisitionThread.h>
#endif

class CVMUSB;
struct DataBuffer;
/*!
   This file provides a VM-USB specific device driver module 
   for the acquisition thread.
   It is a concrete implementation of the CUSBDeviceDriver class.
   
*/
class CVMUSBDeviceDriver : public CUSBDeviceDriver
{
private:
  CVMUSB*  m_pInterface;

  // Constructors and canonicals in a minimal sort of way:
public:
  CVMUSBDeviceDriver(CVMUSB* pInterface);
  
  // Users may want the m_pInterface:

public:
  CVMUSB* getInterface();

  // to be a concreate implementation of CUSBDeviceDriver,
  // we must implement:
  
  virtual void usbToAutonomous();                             // start autonomous data taking.
  virtual int  usbReadBuffer(void* pBuffer, 
			     size_t bytesToRead, 
			     size_t* bytesRead,
			     int timeoutInSeconds); // Read block from usb device.
  virtual void usbSetup(); 	                                 // Load stacks and setup USB device mode.
  virtual size_t usbGetBufferSize();                             // Get the correct data buffers size for the dev.
  virtual void usbStopAutonomous(); 	// Stop autonomous data taking.
  virtual bool isusbLastBuffer(DataBuffer* pBuffer);  // True if last buffer of run.
  virtual void  usbGroinKick();  // Called if can't get last buffer at stop.
  virtual void usbFlushGarbage(); 

 private:
  void usbReadTrash(int milliseconds);
  
};



#endif
