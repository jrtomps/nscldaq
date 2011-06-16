/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";

using namespace std;

/*=========================================================================*/
// DAQKeyRing.cc 
//
// Author:
//		Eric Kasten
//		NSCL
//		Michigan State University
//		East Lansing, MI 48824-1321
//		mailto:kasten@nscl.msu.edu
//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef MAINEXTERNS_H
#include <mainexterns.h>
#endif

#ifndef DAQKEYRING_H
#include <DAQKeyRing.h>
#endif

#ifndef DAQMAIN_H
#include <DAQMain.h>
#endif

#ifndef DAQSTATUS_H
#include <DAQStatus.h>
#endif

#define KEYRINGTMP  "/tmp/keyring-%d-%d.XXXXXX"

DAQKeyRing *_DAQKeyRing = NULL; 

/*===================================================================*/
// DAQKeyRing::DAQKeyRing       
//                                   
// Constructor.                     
//                                 
DAQKeyRing::DAQKeyRing() 
{
  SetType(DAQTYPEID(DAQKeyRing)); 
  last = 0;
  _DAQKeyRing = this;
}

/*===================================================================*/
// DAQKeyRing::~DAQKeyRing       
//                                   
// Destructor.                     
//                                 
DAQKeyRing::~DAQKeyRing()
{
  _DAQKeyRing = NULL;
  last = 0;
}

/*===================================================================*/
// DAQKeyRing::GetKey
//                                   
// Get a new IPC key.                     
//                                 
int DAQKeyRing::GetKey()
{
  int fd;
  key_t key;
  char ftmplt[256];
  char proj;

  last++;  // To try to guarantee uniqness

  sprintf(ftmplt,KEYRINGTMP,last,getpid());

  if ((fd = mkstemp(ftmplt)) < 0) {
    LOG_AND_THROW(os_exception_factory.CreateBaseSystemException(DAQCSTR("DAQKeyRing::GetKey() Failed to create temp file")));
    return(-1);
  }

  proj = (getpid()+last)&0x007f;

  if ((key = ftok(ftmplt,proj)) < 0) {
    close(fd);			// No resource leaks if spectrodaq can keep running.
    LOG(("DAQKeyRing::GetKey() Using file=\"%s\" project=0x%02x",ftmplt,proj));
    LOG_AND_THROW(os_exception_factory.CreateBaseSystemException(DAQCSTR("DAQKeyRing::GetKey() Failed to create IPC key")));
    return(-1);
  }

  close(fd); 
  unlink(ftmplt);

  return(key);
}
