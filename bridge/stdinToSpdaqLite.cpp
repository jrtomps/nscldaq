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

#include <dshapi/daqhwyapi.h>
#include <dshnet/daqhwynet.h>


using namespace daqhwyapi;
using namespace daqhwynet;

#include <spdaq/spdaqlite.h>
using namespace spdaq;

#include <netdb.h>
#include <netinet/in.h>


#include <stdio.h>

static const int BUFFER_SIZE(4096); // words.

class stdinToSpdaqLite : public Main {
private:
  int tagOfBuffer(unsigned short* pBuffer);
public:
  virtual void main(int argc, char** argv);
};

/*
  Figure out the tag associated with the buffer.
  It's going to be 2 for data buffer, 3 for anything else.
*/
int 
stdinToSpdaqLite::tagOfBuffer(unsigned short* pBuffer) {
  return (pBuffer[1] == 1) ? 2 : 3;
}


/* The main loop just reads data from stdin and shoots it a buffer
   at a time over to the input stage of spdaq-lite.
*/

void
stdinToSpdaqLite::main(int argc, char** argv)
{
  DAQDataStore& dataStore = DAQDataStore::instance();

  int port;
  struct servent* serviceInfo = getservbyname("sdlite-buff",
					      "tcp");
  if (serviceInfo) {
    port = ntohs(serviceInfo->s_port);
  } 
  else {
    port = 2701;
  }

					      
  
  dataStore.setSourcePort(port);

  int fd = fileno(stdin);	// Locate the fd for stdin.
  unsigned short pRawBuffer[BUFFER_SIZE];

  while (1) {
    int nRead = read(fd, pRawBuffer, BUFFER_SIZE*sizeof(unsigned short));
    if (nRead <= 0) {
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
