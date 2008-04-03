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

/*
**  daqinterface.h:
**     This file defines the interfaces into the data acquisition system which
**     will be used for the pilot project readout.
**
**
*/
#ifndef __DAQINTERFACE_H
#define __DAQINTERFACE_H



#ifndef __CRTL_TIME_H
#include <time.h>
#define __CRTL_TIME_H
#endif

#ifndef _DAQTYPES_H
#include "daqdatatypes.h"
#endif


#ifndef __CAMACMAP_H
#include <camacmap.h>
#endif


#ifdef __cplusplus
extern "C" {
#else
  typedef int bool;		/* C has no bool type. */
#endif

#define DAQ_BUFFERSIZE 4096	/* Default Size of event buffers (Words). */
#define DAQ_HEADERSIZE 16	/* Size of buffer header (words). */
#define DAQ_EVENTS      2	/* Routing tag for physics buffers. */
#define DAQ_STATE       3       /* Routing tag for nonphysics buffers */



  /*  
  ** Buffer manager interface functions: 
  */

void* daq_AttachBufferManager();        /* Connect to buffer mgr.            */
void* daq_GetBuffer(void* pMgr);        /* Get a data buffer for event data  */
void  daq_SubmitBuffer(void *pMgr,
		       void* pBuffer); /* Submit the buffer for routing. */
void  daq_ReleaseBuffer(void* pMgr,
			void* pBuffer);	/* Release buffer w/o routing    */
void  daq_DetachBufferManager(void* pMgr);

  /*
  ** Control path interface functions: 
  */
typedef enum __RunControlCmd {
                                rctl_Begin,      /* Begin run.           */
                                rctl_End,        /* End run              */
                                rctl_Pause,      /* Pause run.           */
                                rctl_Resume,     /* Resume paused run    */
				rctl_Scaler,     /* Take scaler buffer   */
				rctl_Snapshot,   /* Take Snapshot Buffer */
				rctl_Exit,       /* Exit program         */
				rctl_NoOp        /* Do Nothing. */
                              } RunControlCmd;

void* daq_OpenControlPath();	       /* Get access to system ctl functions */
void  daq_CloseControlPath(void* pCtl); /* Deaccess system control functions */
int   daq_CheckForCommand(void* pCtl);  /* TRUE if control command avail.    */
RunControlCmd daq_GetCommand(void* pCtl); /* Get control command w/block    */

   /*
   **   Miscellaneous:
   */

typedef enum _DaqPriorities {
                              pri_EventReadout,
                              pri_UnSampledConsumer,
                              pri_SampledConsumer,
                              pri_Controller,
                              pri_NormalProcess
                            } DaqPriorities; 

void daq_SetPriority(DaqPriorities nPriority);

unsigned daq_GetScalerCount();
void     daq_SetScalerCount(unsigned nScalers);
char*    daq_GetTitle();
void     daq_SetTitle(const char* pString);
unsigned daq_GetRunNumber();
void     daq_SetRunNumber(unsigned nNext);
void     daq_IncrementRunNumber();
time_t   daq_GetScalerReadoutInterval();
void     daq_SetScalerReadoutInterval(unsigned nSeconds);

void     daq_setCPUNumber(unsigned short node);
unsigned short daq_getCPUNumber();

void     daq_StartRun();
void     daq_EndRun();
void     daq_PauseRun();
void     daq_ResumeRun();

bool   daq_IsActive();
bool   daq_IsHalted();
bool   daq_IsPaused();

/*
** Control/get the size of the event buffer:
*/
void     daq_SetBufferSize(int newSize);
int      daq_GetBufferSize();

/*
 *  Determine if jumbo buffer formatting is required:
*/

bool daq_isJumboBuffer();

#ifdef __cplusplus
}
#endif

#endif

