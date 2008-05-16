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

// A simple consumer for timing purposes.
// Creates the ring buffer "timing"
// Attaches to it and accepts data from the ring buffer until
// interrupted externally.
//
//
#include <string>
#include <CRingBuffer.h>

using namespace std;


int main(int argc, char**argv)
{
  string ringname("timing");

  CRingBuffer::create(ringname);
  CRingBuffer ring(ringname, CRingBuffer::consumer);
  ring.setPollInterval(1);

  char buffer[1024*1024]; 
  while(1) {
    ring.get(buffer, sizeof(buffer), sizeof(buffer));
  }
}
