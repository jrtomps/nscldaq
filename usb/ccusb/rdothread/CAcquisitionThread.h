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

using namespace std;		// required for spectrodaq includes.
#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif

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

#ifndef __CCONTROLQUEUES_H
#include "CControlQueues.h"
#endif


// forward class definitions.

class CCCUSB;
class CReadoutModule;
class CCCUSBReadoutList;
struct DataBuffer;


/*!
   This is the thread that does the data acquisition.
   As coded this is a singleton class as well, however as a thread of execution,
   it gets started at the beginning of a run and politely requested to stop at
   the end of a run.
*/
class CAcquisitionThread : public DAQThread
{
private:
  static bool                   m_Running;	//!< thread is running.
  static DAQThreadId            m_tid;          //!< ID of thread when running.
  static CCCUSB*                m_pCamac;		//!< VME interface.

  static std::vector<CReadoutModule*>  m_Stacks;       //!< the stacks to run.

  //Singleton pattern stuff:


private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();

public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void start(CCCUSB* usb,
		    std::vector<CReadoutModule*> Stacks);
  static bool isRunning();
  static void waitExit();	/* Wait for this thread to exit (join). */

protected:
  virtual int operator()(int argc, char** argv);
private:
  void mainLoop();
  void processCommand(CControlQueues::opCode command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
  void stopDaq();
  void pauseDaq();
  void CCusbToAutonomous();
  void drainUsb();
  void beginRun();
  void endRun();

};

#endif
