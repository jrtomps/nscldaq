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

#ifndef __CGAURDEDOBJECT_H
#define __CGAURDEDOBJECT_H
using namespace std;		// Needed for spectrodaq includes.

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif


/*!
  CGaurded object implements monitor like functionality.  This is intended
  to be used as a based class.  To implement a monitor, derive from this class
  and have the critical functions call Enter and Leave as needed to thread-safe
  their critical regions.
*/
class CGaurdedObject
{
private:
  DAQThreadMutex  m_Mutex;	// The gaurd is a pthreads mutex.
public:
  void Enter();			//!< Enter a critical region.
  void Leave();			//!< Leave a critical region.
  DAQThreadMutex& mutex();	//!< Return the underlying mutex object.
};


#endif
