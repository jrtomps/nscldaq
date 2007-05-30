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

#include <dshnet/daqhwyapi.h>
#include <dshnet/daqhwynet.h>


using namespace daqhwyapi;
using namespace daqhwynet;

#include <spdaq/spdaqlite.h>
using namespace spdaq;

#include <stdio.h>

static const int BUFFER_SIZE(4096); // words.

class stdinToSpdaqlite : public Main {
private:
  int tagOfBuffer(unsigned short* pBuffer);
public:
  virtual void main(int argc, char** argv);
};

/*
  Figure out the tag associated with the buffer.
  It's going to be 2 for data buffer, 3 for anything else.
*/
int tagofBufer(unsigned short* pBuffer) {
  return (pBuffer[1] == 1) ? 2 : 3;
}


/* The main loop just reads data from stdin and shoots it a buffer
   at a time over to the input stage of spdaq-lite.
*/

void
stdinToSpdaqlite::main(int argc, char** argv)
{
  DAQDataStore& dataStore = DAQDataStore::instance();
  dataStore.setSourcePort(2700); // Hard coded for now.
  int fd = fileno(stin);	// Locate the fd for stdin.
  unsigned short* pRawBuffer[BUFFER_SIZE];

  while (1) {
    int read = read(fd, pRawBuffer, BUFFER_SIZE*sizeof(unsigned short));
    if (read <= 0) {
      // EOF or error finishes us off.

      return;
    }

    DAQWordBuffer buffer(BUFFER_SIZE);
    buffer.CopyIn(pRawBuffer, 0, BUFFER_SIZE);

    buffer.SetTag(tagOfBuffer(pRawBuffer));
    buffer.Route();

  }
}


stdinToSpdaqLite app;
