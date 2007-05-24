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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <Iostream.h>
#include <Iomanip.h>
#include <buftypes.h>
#include <buffer.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <CopyrightNotice.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

static const int NOTRANSFERDELAY(10*1000); // Delay time before write retry if EAGAIN.

// How big a buffer do we want?

#define BUFLEN 0

int Write(int fd, void* pBuffer, size_t nLength)
{
  char* p = (char*) pBuffer;
  size_t nBytes = 0;
  size_t nResid = nLength;
  while(nBytes != nLength){
    int nWritten = write(fd, p, nResid);
    if(nWritten < 0) {
      if(errno == EAGAIN) {	// Blocked.
	if(nBytes == 0) {	// Nothing written yet
	  return nLength;	// Don't transfer to client.
	} else {		// Something written
	                        // so write the rest. 
	  usleep(NOTRANSFERDELAY);
	}
      }
      else {			// Not due to blocking...
	return -1;		// Real errors -> exit.
      }
    }
    else if(nWritten == 0) {
      return -1;		// should not happen!
    }
    else {
      // If control is here, nBytes > 1
      
      nBytes += nWritten;
      p      += nWritten;
      nResid -= nWritten;
    }
  }
  return nBytes;		// All bytes written.
}

//=====================================================================
//
// CopyOut - Copy a bunch of data out of a word buffer.
//
void CopyOut(void* pDest, DAQWordBufferPtr pSrc, unsigned int nWds)
{
  pSrc.CopyOut(pDest, 0, nWds);	//  This should be more efficient...
}


/*===================================================================*/
class DAQBuff : public DAQROCNode {
  int operator()(int argc,char **argv) {

    DAQWordBuffer bbuf(BUFLEN);
    DAQURL sinkurl("TCP://localhost:2700/");
    long sinkid;

    // Print out copyright information:

    CopyrightNotice::Notice(cerr, argv[0], "2.0", "2002");
    CopyrightNotice::AuthorCredit(cerr, argv[0],
				  "Ron Fox", "Eric Kasten", NULL);

    // In case we're running spectrodaq on something other than
    // the default URL.
    if (argc > 1) {
      cerr << "Using URL: " << argv[1] << endl;
      sinkurl = argv[1];
    } 

    // Tag this buffer so we know what type of buffer to receive.
    bbuf.SetTag(2);

    // Add a sink for this tag

    sinkid = daq_link_mgr.AddSink(sinkurl,2, ALLBITS_MASK, COS_UNRELIABLE);
    if(sinkid <= 0) {
      cerr << "Failed to add unreliable sink\n";
      exit(-1);
    }
    cerr << "Added unreliable sink " << sinkid << endl;
    sinkid = daq_link_mgr.AddSink(sinkurl,3, ALLBITS_MASK, COS_RELIABLE);


    // If the sinkid == 0, then the AddSink failed.
    cerr << "Added Reliable Sink Id " << sinkid << endl;
    if (sinkid <= 0) {
      cerr << "Failed to add a sink." << endl;
      exit(-1);
    }

    // receive buffers and put them out to stdout (normally a pipe to
    // SpecTcl.
    // 

    // Set stdout to non blocking mode:

    int fd = fileno(stdout);
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1) {
      cerr << "Failed to read initial stdout flag set " << strerror(errno) << endl;
      exit(-1);
    }

    flags |= O_NONBLOCK;
    int stat = fcntl(fd, F_SETFL, flags);
    if(stat == -1) {
      cerr << "Failed to set stdout to nonblocking mode " << strerror(errno) << endl;
    }

    while(1) {



      // Accept a buffer (with wait).
      SetProcessTitle("spectcldaq - Accepting");
      do {
	bbuf.SetTag(2);
	bbuf.Accept();
      } while(bbuf.GetLen() == 0);
      SetProcessTitle("spectcldaq - Copying");

      DAQWordBufferPtr pBuf = &bbuf;
      
      int nLength = bbuf.GetLen();
      short* pLocalBuffer = new short[nLength];

      CopyOut(pLocalBuffer, pBuf, bbuf.GetLen());

      bbuf.Release();		// Implicitly sets buffers size to 0
				// avoiding Spectrodaq deadlocks.

      // From now on we can operate on the local copy of the buffer.
      
      {
	char buffer[100];
	sprintf(buffer, "spectcldaq - piping fd=%d", fileno(stdout));
	DAQString process(buffer);
	SetProcessTitle(process);
      }
      if(Write(fileno(stdout), pLocalBuffer, nLength*sizeof(short)) <= 0)
	 exit(0);
      delete []pLocalBuffer;
    }

  //sleep(10);
  // Delete the sink.
    daq_link_mgr.DeleteSink(sinkid);
  } 
};

DAQBuff mydaq;




