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
#include <CDataSink.h>
#include <CDataSinkFactory.h>

#include <V12/CRingStateChangeItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/DataFormat.h>
#include <RingIOV12.h>
#include <ByteBuffer.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <os.h>

#include <vector>
#include <string>

using namespace std;

using namespace DAQ;
using namespace DAQ::V12;


static void usage()
{
  cerr << "Usage:\n";
  cerr << "   fakeruns run blocks scaler-period strings\n";
  exit(EXIT_FAILURE);
}


static void beginRun(unsigned runNumber, CDataSink& ring)
{
  char title[100];
  sprintf(title, "This is a test run %d", runNumber);
  CRingStateChangeItem item(BEGIN_RUN, runNumber, 0, time(NULL), title);

  ring << CRawRingItem(item);
}

static void endRun(unsigned runNumber, CDataSink& ring)
{
  char title[100];
  sprintf(title, "This is a test run %d", runNumber);
  CRingStateChangeItem item(END_RUN, runNumber,120, time(NULL), title);

  ring << CRawRingItem(item);

}

static void stringItem(unsigned strings, CDataSink& ring)
{
  vector<string> items;
  for (int i=0; i < strings; i++) {
    char item[1000];
    sprintf(item, "String number %d", i);
    items.push_back(string(item));
  }
  CRingTextItem thing(PACKET_TYPES, items);

  ring << CRawRingItem(thing);

}

static void event(CDataSink& ring)
{
  unsigned bodySize = (int)(100.0*drand48());

  CPhysicsEventItem item;

  auto& body = item.getBody();
  body.reserve(100*sizeof(uint16_t));
  for (uint16_t i=0; i<100; ++i) {
      body << i;
  }

  ring << item;
}

static void scaler(CDataSink& ring)
{
  static int offset = 0;
  vector<uint32_t> scalers;
  for (int i=0; i < 32; i++) {
    scalers.push_back(i*32);
  }
  CRingScalerItem i(offset, offset+10, time(NULL), scalers);
  offset += 10;

  ring << CRawRingItem(i);
}

static void eventcount(CDataSink& ring, int count)
{
  static int offset = 10;
  CRingPhysicsEventCountItem item;
  item.setEventCount(count);
  item.setTimeOffset(offset);
  offset += 10;

  ring << CRawRingItem(item);
}

static void userItem(CDataSink& ring)
{
  CRawRingItem item;
  item.setType(1234);

  for (int i = 0; i < 256; i++) {
    item.getBody() << char(i);
  }

  ring << CRawRingItem(item);
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

  string ringname = "tcp://localhost/";
  ringname += Os::whoami();

  auto pSink = CDataSinkFactory().makeSink(ringname);

  // Begin run:

  beginRun(run, *pSink);

  userItem(*pSink);

  // The strings item:

  stringItem(strings, *pSink);

  // Events and scalers:

  for (int i =1; i < events; i++) {
    event(*pSink);
    if (i % period == 0) {
      sleep(2);
      eventcount(*pSink,i);
      scaler(*pSink);
    }
  }

  // End run

  endRun(run, *pSink);

  delete pSink;

  exit(0);

}

