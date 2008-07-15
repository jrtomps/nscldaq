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


#ifndef __CMUTEX_H
#include <CMutex.h>
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
  CMutex  m_Mutex;	        
public:
  void Enter();			//!< Enter a critical region.
  void Leave();			//!< Leave a critical region.
  CMutex& mutex();	        //!< Return the underlying mutex object.
};


#endif
