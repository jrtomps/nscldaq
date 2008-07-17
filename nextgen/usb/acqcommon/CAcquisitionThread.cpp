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
#include "CAcquisitionThread.h"
#include <DataBuffer.h>
#include <CControlQueues.h>
#include <CRunState.h> // else pick up the daq one by mistake.
#include <assert.h>
#include <time.h>
#include <string>
#include <Exception.h>
#include <iostream>

#include <string.h>
#include <errno.h>

using namespace std;


static const unsigned DRAINTIMEOUTS(5);	// # consecutive drain read timeouts before giving up.
static const unsigned USBTIMEOUT(10);



// buffer types:
//


bool                CAcquisitionThread::m_Running(false);
CAcquisitionThread* CAcquisitionThread::m_pTheInstance(0);

unsigned long CAcquisitionThread::m_tid; // Thread id of the running thread.


/*!
   Construct the the acquisition thread object.
   This is just data initialization its the start member that is
   important.
*/

CAcquisitionThread::CAcquisitionThread() :
  m_pDriver(0)
{

}
 
/*!
   Return the singleton instance of the acquisition thread object:

*/
CAcquisitionThread*
CAcquisitionThread::getInstance() 
{
  if (!m_pTheInstance) {
    m_pTheInstance =  new CAcquisitionThread;
  }
  return m_pTheInstance;
}

/*!
   Start the thread.
   - get an instance which, if necessary forces creation.
   - Set the adc/scaler lists.
   - Initiate the thread which gets the ball rolling.
   \param usb    :CVMUSB*
      Pointer to the vme interface object we use to deal with all this stuff.
   \param adcs   : std::vector<CReadoutModule*> 
      A vector consisting of the set of modules that will be read out in the
      event mode.  This is expected to be hooked to interrupt 
      80, IPL 6 and that will be used to fire off the list.
   \param scalers : std::vector<CReadoutModule*>
     A vector consisting of the set of scaler modules that will be read out
     every scalerPeriod seconds.

*/
void
CAcquisitionThread::Start()
{
  CRunState* pState = CRunState::getInstance();
  pState->setState(CRunState::Active);


  CAcquisitionThread* pThread = getInstance();




  // starting the thread will eventually get operator() called and that
  // will do all the rest of the work in thread context.

  pThread->start();
  m_tid = pThread->getId();
  
}

/*!
    Returns true if the thread is running.
*/
bool
CAcquisitionThread::isRunning()
{
  return m_Running;
}

/*!
   Wait for the thread to exit.
*/
void
CAcquisitionThread::waitExit()
{
  getInstance()->join();
}

/*!
   Set the object's device driver:
   \param pDriver - Pointer to an instance of a concrete
                    derivation of CUSBDeviceDriver
*/
void
CAcquisitionThread::setDriver(CUSBDeviceDriver* pDriver)
{
  m_pDriver = pDriver;
}


/*!
   Entry point for the thread.
*/
void
CAcquisitionThread::run()
{
  try {
    m_Running = true;		// Thread is off and running now.
    
    beginRun();			// Emit begin run buffer.
    
    startDaq();  		        // Setup and start data taking.
    try {
      
      mainLoop();			// Enter the main processing loop.
    }
    catch (...) {			// exceptions are used to exit the main loop.?
    }
    
    endRun();			// Emit end run buffer.
    
    m_Running = false;		// Exiting.

  }
  catch (string msg) {
    cerr << "CAcquisition thread caught a string exception: " << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "CAcquisition thread caught a char* exception: " << msg << endl;
    throw;
  }
  catch (CException& err) {
    cerr << "CAcquisitino thread caught a daq exception: "
	 << err.ReasonText() << " while " << err.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "CAcquisition thread caught some other exception type.\n";
    throw;
  }

}

/*!
  The main loop is simply one that loops:
  - Reading a buffer from the vm-usb and processing it if one comes in.
  - Checking for control commands and processing them if they come in.
*/
void
CAcquisitionThread::mainLoop()
{
  DataBuffer*     pBuffer   = gFreeBuffers.get();
  CControlQueues* pCommands = CControlQueues::getInstance(); 
  size_t          usbBufferSize = usbGetBufferSize();
  try {
    while (true) {
      
      // Event data from the VM-usb.
      
      size_t bytesRead;
      int status = usbReadBuffer(pBuffer->s_rawData,
				 usbBufferSize,
				 &bytesRead,
				 USBTIMEOUT);

      if (status == 0) {
	pBuffer->s_bufferSize = bytesRead;
	pBuffer->s_bufferType   = TYPE_EVENTS;
	processBuffer(pBuffer);	// Submitted to output thread so...
	pBuffer = gFreeBuffers.get(); // need a new one.
      } 
      else {
#ifdef REPORT_ERRORS
	cerr << "Bad status from usbread: " << strerror(errno) << endl;
#endif
      }
      // Commands from our command queue.
      
      CControlQueues::opCode request;
      bool   gotOne = pCommands->testRequest(request);
      if (gotOne) {
	processCommand(request);
      }
    }
  }
  catch (...) {
    gFreeBuffers.queue(pBuffer);	// Don't lose buffers!!
    throw;
  }
}
/*!
  Process a command from our command queue.  The command can be one of the
  following:
  ACQUIRE   - The commander wants to acquire the VM-USB we must temporarily
              shutdown data taking and drain the VMusb...then ack the message
              and wait paitently for a RELEASE before restarting.
  PAUSE     - Commander wants the run to pause.
  END       - The commander wants the run to end.
*/
void
CAcquisitionThread::processCommand(CControlQueues::opCode command)
{
  CControlQueues* queues = CControlQueues::getInstance();

  if (command == CControlQueues::ACQUIRE) {
    stopDaq();
    queues->Acknowledge();
    CControlQueues::opCode release  = queues->getRequest();
    assert(release == CControlQueues::RELEASE);
    queues->Acknowledge();
    usbToAutonomous();
  }
  else if (command == CControlQueues::END) {
    stopDaq();
    queues->Acknowledge();
    throw "Run ending";
  }
  else if (command == CControlQueues::PAUSE) {
    pauseDaq();

  }
  else {
    // bad error:

    assert(0);
  }
}
/*!
   Process a buffer of data from the VM-USB (not artificially generated
   buffers.  There are only two types of buffers that can come from the
   VM-USB:
   - Event data
   - Scalers
   These are distinguishable from the Scaler bit in the header word of the
   data from the VM-USB.
   \param buffer : DataBuffer*
      the buffer of data from the VM-USB.  The following fiels have been set:
      - s_storageSize  : number of bytes of physical storage in a buffer. 
      - s_bufferType.
      - s_buffersSize  : number of bytes read from the USB.
      - s_rawData      : the data read from the USB.

   The disposition of full buffers is done by another thread that is connected
   to us by a buffer queue.  Once we fill in the buffer type we just have to
   submit the buffer to the queue.. which can wake up the output thread and
   cause a scheduler pass... or not.

*/
void
CAcquisitionThread::processBuffer(DataBuffer* pBuffer)
{
  time_t acquiredTime;		// Need to generate  our own timestamps.
  time(&acquiredTime);
  pBuffer->s_timeStamp  = acquiredTime;

  // In this version, all stack ids are good.  The output thread will ensure that
  // stack 1 completions are scalers and all others are events.

  gFilledBuffers.queue(pBuffer);	// Send it on to be routed to spectrodaq in another thread.
}
/*!
    startDaq start data acquisition from a standing stop. To do this we need to:
    - Emit a begin run buffer.
    - Create VMUSBReadoutLists from the m_scalers and m_adcs
    - Download those lists into the VM-USB, list 2 will be used for event readout.
      list 1 for scalers.
    - Set the trigger for list 2 to be interrupt number 1 with vmeVector and
      vmeIPL as described in the static consts in this module
      \bug May want to make this parameterizable, but probably not necessary.
    - Set the buffersize to 13k (max). to get optimum throughput.
    - Setup scaler readout to happen every 10 seconds.
    - Initialize the hardware
    - Start USB data acuisition.

*/
void
CAcquisitionThread::startDaq()
{

  //  First do a bulk read just to flush any crap that's in the VM-USB
  // output fifo..as it appears to sometimes leave crap there.
  // ignore any error status, and use a short timeout:

  usbFlushGarbage();
  usbSetup();
  usbToAutonomous();


}
/*!
   Stop data taking this involves:
   - Forcing a scaler trigger (action register write)
   - Setting clearing the DAQ start bit (action register write)
   - draining data from the VMUSB:
*/
void
CAcquisitionThread::stopDaq()
{

  usbStopAutonomous();
  drainUsb();
}
/*!
  Pause the daq. This means doing a stopDaq() and fielding 
  requests until resume or stop was sent.

*/
void
CAcquisitionThread::pauseDaq()
{
  CControlQueues* queues = CControlQueues::getInstance();
  stopDaq();
  CRunState* pState = CRunState::getInstance();
  pState->setState(CRunState::Paused);
  queues->Acknowledge();

  while (1) {
    CControlQueues::opCode req = queues->getRequest();
    
    // Acceptable actions are to acquire the USB, 
    // End the run or resume the run.
    //

    if (req == CControlQueues::ACQUIRE) {
      queues->Acknowledge();
      CControlQueues::opCode release = queues->getRequest();
      assert (release == CControlQueues::RELEASE);
      queues->Acknowledge();
    }
    else if (req == CControlQueues::END) {
      queues->Acknowledge();
      pState->setState(CRunState::Idle);
      throw "Run Ending";
    }
    else if (req == CControlQueues::RESUME) {
      startDaq();
      queues->Acknowledge();
      pState->setState(CRunState::Active);
      return;
    }
    else {
      assert(0);
    }
  }
  
}

/*!
  Drain usb - We read buffers from the DAQ (with an extended timeout)
  until the buffer we get indicates it was the last one (data taking turned off).
  Each buffer is processed normally.
*/
void
CAcquisitionThread::drainUsb()
{
  bool done = false;
  DataBuffer* pBuffer = gFreeBuffers.get();
  int timeouts(0);
  size_t bytesRead;
  size_t bufferSize = usbGetBufferSize();

  cerr << "CAcquisitionThread::drainUsb...\n";
  do {
    int    status = usbReadBuffer(pBuffer->s_rawData,
				  bufferSize,
				  &bytesRead,
				  DRAINTIMEOUTS);

    if (status == 0) {
      pBuffer->s_bufferSize = bytesRead;
      pBuffer->s_bufferType   = TYPE_EVENTS;
      cerr << "Got a buffer, with type header: " << hex << pBuffer->s_rawData[0] << endl;
      if (isusbLastBuffer(pBuffer)) {
	cerr << "Done\n";
	done = true;
      }
      processBuffer(pBuffer);
      pBuffer = gFreeBuffers.get();
    }
    else {
      timeouts++;		// By the time debugged this is only failure.
      cerr << "Read timed out\n";
      if(timeouts >= DRAINTIMEOUTS) {
	usbGroinKick();
	done = true;
      }
    }
  } while (!done);


  gFreeBuffers.queue(pBuffer);
  cerr << "Done finished\n";
  
}
/*!
   Emit a begin run buffer to the output thread.
   the key info in this buffer is just its type and timestamp.
   No info at all is in the body, however since the buffer will be returned
   to the free list we have to get it from there to begin with:
*/
void
CAcquisitionThread::beginRun()
{
  DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_START;
  processBuffer(pBuffer);	// Rest gets taken care of there.
}
/*!
   Emit an end of run buffer.
   This is just like a begin run buffer except for the buffer type:
*/
void
CAcquisitionThread::endRun()
{
  DataBuffer* pBuffer   = gFreeBuffers.get();
  pBuffer->s_bufferSize = pBuffer->s_storageSize;
  pBuffer->s_bufferType = TYPE_STOP;
  processBuffer(pBuffer);
} 

// Below are a facade to the member functions of the driver object:

void 
CAcquisitionThread::usbToAutonomous()
{
  m_pDriver->usbToAutonomous();
}

int
CAcquisitionThread::usbReadBuffer(void* pBuffer,
				  size_t bytesToRead,
				  size_t* pBytesRead,
				  int     timeoutInSeconds)
{
  return m_pDriver->usbReadBuffer(pBuffer, bytesToRead, pBytesRead, timeoutInSeconds);
}

void 
CAcquisitionThread::usbSetup()
{
  m_pDriver->usbSetup();
}

size_t
CAcquisitionThread::usbGetBufferSize()
{
  m_pDriver->usbGetBufferSize();
}

void 
CAcquisitionThread::usbStopAutonomous()
{
  m_pDriver->usbStopAutonomous();
}


bool
CAcquisitionThread::isusbLastBuffer(DataBuffer* pBuffer)
{
  return m_pDriver->isusbLastBuffer(pBuffer);
}

void
CAcquisitionThread::usbGroinKick()
{
   m_pDriver->usbGroinKick();
}

void 
CAcquisitionThread::usbFlushGarbage()
{
  m_pDriver->usbFlushGarbage();
}
