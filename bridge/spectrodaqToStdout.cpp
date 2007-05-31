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

// This is quite similar to spectcldaq.. however the
// data are gotten without sampling.  The idea is that
// the output would be piped into stdinToSpdaqLite 
// to form a bridge between the two.
//

using namespace std;
#include <spectrodaq.h>
#include <string>
#include <iostream>
#include <stdio.h>



static const int BUFFER_LENGTH(4096); // words

class DAQBuff : public DAQROCNode
{
  string m_Url;

public:
  DAQBuff(const char* url) :
    m_Url(url) {}

private:
  virtual int operator()(int argc, char** argv) {
    DAQWordBuffer spectrodaqBuffer(BUFFER_LENGTH);
    DAQURL        sinkURL(m_Url.c_str());

    // Form the sink for buffers from spectrodaq:

    long sinkid = daq_link_mgr.AddSink(sinkURL,
				       2,
				       2, 
				       COS_RELIABLE);
    if(sinkid < 0) {
      cerr << "Failed to add the sink for " << m_Url << endl;
      exit(-1);
    }

    int fd = fileno(stdout);

    // Process the buffers:

    while (1) {
      
      do {
	spectrodaqBuffer.SetTag(2);
	spectrodaqBuffer.SetMask(2);
	spectrodaqBuffer.Accept();
      } while (spectrodaqBuffer.GetLen() == 0);
      
      int nLength = spectrodaqBuffer.GetLen();
      short* pRegularBuffer = new short[nLength];
      spectrodaqBuffer.CopyOut(pRegularBuffer, 0, nLength);
      spectrodaqBuffer.Release();

      write(fd, pRegularBuffer, nLength);
      delete []pRegularBuffer;
    }
    return 0;
  }
};

DAQBuff application("tcp://localhost:2602");
