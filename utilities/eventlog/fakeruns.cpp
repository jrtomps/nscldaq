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

#include <string>
#include <CRingBuffer.h>
#include <CRingStateChangeItem.h>
#include <CRingTextItem.h>
#include <CRingScalerItem.h>
#include <CRingPhysicsEventCountItem.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <vector>
#include <string>

using namespace std;


static string whoami()
{
  uid_t id = getuid();
  struct passwd* pwd = getpwuid(id);
  string user(pwd->pw_name);

  return user;
  
}

static void usage()
{
  cerr << "Usage:\n";
  cerr << "   fakeruns run blocks scaler-period strings\n";
  exit(EXIT_FAILURE);
}


static void beginRun(unsigned runNumber, CRingBuffer& ring)
{
  char title[100];
  sprintf(title, "This is a test run %d", runNumber);
  CRingStateChangeItem item(BEGIN_RUN, runNumber, 0, time(NULL), title);

  item.commitToRing(ring);
}

static void endRun(unsigned runNumber, CRingBuffer& ring)
{
  char title[100];
  sprintf(title, "This is a test run %d", runNumber);
  CRingStateChangeItem item(END_RUN, runNumber,120, time(NULL), title);

  item.commitToRing(ring);
}

static void stringItem(unsigned strings, CRingBuffer& ring)
{
  vector<string> items;
  for (int i=0; i < strings; i++) {
    char item[1000];
    sprintf(item, "String number %d", i);
    items.push_back(string(item));
  }
  CRingTextItem thing(PACKET_TYPES, items);
  thing.commitToRing(ring);
}

static void event(CRingBuffer& ring)
{
  unsigned bodySize = (int)(100.0*drand48());

  CRingItem item(PHYSICS_EVENT);
  uint16_t* pBody = (uint16_t*)(item.getBodyCursor());
  for (int i=0; i < bodySize; i++) {
    *pBody++ = i;
  }
  item.setBodyCursor(pBody);
  item.commitToRing(ring);
}

static void scaler(CRingBuffer& ring)
{
  static int offset = 0;
  vector<uint32_t> scalers;
  for (int i=0; i < 31; i++) {
    scalers.push_back(i*32);
  }
  CRingScalerItem i(offset, offset+10, time(NULL), scalers);
  offset += 10;
  i.commitToRing(ring);

}

static void eventcount(CRingBuffer& ring, int count)
{
  static int offset = 10;
  CRingPhysicsEventCountItem item;
  item.setEventCount(count);
  item.setTimeOffset(offset);
  offset += 10;

  item.commitToRing(ring);
}

static void userItem(CRingBuffer& ring)
{
  CRingItem item(1234, 500);
  uint8_t*  p = reinterpret_cast<uint8_t*>(item.getBodyCursor());

  for (int i = 0; i < 256; i++) {
    *p++ = i;
  }
  item.setBodyCursor(p);
  item.commitToRing(ring);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//
// This file creates fake runs.
// usage:
//   fakeruns  run-number event-blocks scaler-period string-count
// 
// run-number - the run number to create.
//              Title string will be 'This is test run run-number"
// event-blocks - total number of event blocks to create.
//               events will be random length with counting pattern bodies.
//               max length 100 uint16_t's.
// scaler-period - After each scaler-period event blocks a 32 channel scaler block is written.
// string-count - Number of tests strings in the packet docs item created.
// 
// Data are written to the username ring.
//
/////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char**argv) 
{

  if (argc != 5) {
    usage();
  }
  
  unsigned run    = atoi(argv[1]);
  unsigned events = atoi(argv[2]);
  unsigned period = atoi(argv[3]);
  unsigned strings= atoi(argv[4]);

  string ringname = whoami();

  if(!CRingBuffer::isRing(ringname)) {
    CRingBuffer::create(ringname);
  }

  CRingBuffer ring(ringname, CRingBuffer::producer);

  // Begin run:

  beginRun(run, ring);

  userItem(ring);

  // The strings item:

  stringItem(strings, ring);

  // Events and scalers:

  for (int i =1; i < events; i++) {
    event(ring);
    if (i % period == 0) {
      sleep(2);
      eventcount(ring,i);
      scaler(ring);
    }
  }

  // End run

  endRun(run, ring);

  exit(0);

}
