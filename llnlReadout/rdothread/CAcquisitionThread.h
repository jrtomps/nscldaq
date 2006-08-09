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
#include <spectrodaq>
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
class CAcquisitionThread : public DAQThread
{
private:
  static bool                   m_Running;	//!< thread is running.
  static DAQThreadId            m_tid;          //!< ID of thread when running.
  static CVMUSB*                m_pVme;		//!< VME interface.

  std::vector<CReadoutModule*>  m_adcs;
  std::vector<CReadoutModule*>  m_scalers;

  //Singleton pattern stuff:


private:
  static CAcquisitionThread*    m_pTheInstance;
  CAcquisitionThread();

public:
  static CAcquisitionThread*   getInstance();

  // Thread functions:

public:
  static void start(CVMUSB* usb,
		    std::vector<CReadoutModule*> adcs,
		    std::vector<CReadoutModule*> scalers);
  static void isRunning();

protected:
  virtual int operator()(int argc, char** argv);
private:
  void mainLoop();
  void processCommand(std::string command);
  void processBuffer(DataBuffer* pBuffer);
  void startDaq();
  void stopDaq();
  void VMusbToAutonomous();
  void drainUsb();
  void beginRun();
  void endRun();
  CVMUSBReadoutList* createList(std::vector<CReadoutModule*> modules);
  void InitializeHardware(std::vector<CReadoutModule*> modules);
};

#endif
