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

#ifndef __CACQUISITIONTHREAD_H
#define __CACQUISITIONTHREAD_H

using namespace std;	


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __THREAD_H
#include <Thread.h>
#endif

#ifndef __CCONTROLQUEUES_H
#include "CControlQueues.h"
#endif


// forward class definitions.

struct DataBuffer;


/*!
  This is device driver class.  Concrete derivations from this class
  are attached to the acquisition thread object to provide device specific
  functionality.  The CAquisitioThread object contains a facade that these
  get plugged into:
*/
class CUSBDeviceDriver
{
public:
  virtual void usbToAutonomous() = 0;                              // start autonomous data taking.
  virtual int  usbReadBuffer(void* pBuffer, 
		     size_t bytesToRead, 
		     size_t* bytesRead,
		     int timeoutInSeconds) =  0; // Read block from usb device.
  virtual void usbSetup() = 0;	                                 // Load stacks and setup USB device mode.
  virtual size_t usbGetBufferSize() = 0;                            // Get the correct data buffers size for the dev.
  virtual void usbStopAutonomous() = 0;	// Stop autonomous data taking.
  virtual bool isusbLastBuffer(DataBuffer* pBuffer) = 0; // True if last buffer of run.
  virtual void  usbGroinKick() = 0; // Called if can't get last buffer at stop.
  virtual void usbFlushGarbage() = 0;
};




/*!
   This is the thread that does the data acquisition.
   As coded this is a singleton class as well, however as a thread of execution,
   it gets started at the beginning of a run and politely requested to stop at
   the end of a run.
*/
class CAcquisitionThread : public Thread
{

private:
  static bool                   m_Running;	//!< thread is running.
  static unsigned long          m_tid;          //!< ID of thread when running.


  CUSBDeviceDriver*            m_pDriver; // Contains USB device specific functions.

  //Singleton pattern stuff:


private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();


public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void Start();
  static bool isRunning();
  static void waitExit();	/* Wait for this thread to exit (join). */

public:
  void setDriver(CUSBDeviceDriver* pDriver);

protected:
  virtual void run();



private:
  void mainLoop();
  void processCommand(CControlQueues::opCode command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
  void stopDaq();
  void pauseDaq();
  void drainUsb();
  void beginRun();
  void endRun();

  // The following are callouts for interface specific stuff;
  // they will call into the device specific object.

protected:

   void usbToAutonomous() ;                              // start autonomous data taking.
   int  usbReadBuffer(void* pBuffer, 
			     size_t bytesToRead, 
			     size_t* bytesRead,
			     int timeoutInSeconds); // Read block from usb device.
   void usbSetup();	                                 // Load stacks and setup USB device mode.
   size_t usbGetBufferSize();                            // Get the correct data buffers size for the dev.
   void usbStopAutonomous() ;	// Stop autonomous data taking.
   bool isusbLastBuffer(DataBuffer* pBuffer) ; // True if last buffer of run.
   void usbGroinKick() ; // Called if can't get last buffer at stop.
   void usbFlushGarbage() ;
};

#endif
