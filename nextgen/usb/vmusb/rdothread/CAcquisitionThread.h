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

#ifndef __THREAD_H
#include <Thread.h>
#ifndef __THREAD_H
#define __THREAD_H
#endif
#endif

// forward class definitions.

class CVMUSB;
class CReadoutModule;
class CVMUSBReadoutList;
struct DataBuffer;


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
  static CVMUSB*                m_pVme;		//!< VME interface.

  static std::vector<CReadoutModule*>  m_Stacks;       //!< the stacks to run.

  bool                          m_haveScalerStack;

  //Singleton pattern stuff:


private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();

public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void start(CVMUSB* usb);
  static bool isRunning();
  static void waitExit();	/* Wait for this thread to exit (join). */
  virtual void run();		/* Adapt between nextgen  spectrodaq thread model. */

protected:
  virtual int operator()();
private:
  void mainLoop();
  void processCommand(CControlQueues::opCode command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
  void stopDaq();
  void pauseDaq();
  void VMusbToAutonomous();
  void drainUsb();
  void beginRun();
  void endRun();
  void bootToTheHead();

};

#endif
