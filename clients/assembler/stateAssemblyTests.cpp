// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "StateTransitionAssemblyEvent.h"
#include "AssemblyEvent.h"
#include "AssembledEvent.h"
#include "StateTransitionFragment.h"
#include "AssembledStateTransitionEvent.h"
#include "NodeScoreboard.h"
#include <vector>
#include <stdint.h>
#include <buffer.h>
#include <buftypes.h>
#include <string.h>
#include <time.h>

using namespace std;



// Contents of the body of the state transition buffer:

#define TYPE BEGRUNBF
#define RUN  1234
#define CPU  0x5555
#define TITLE "This is my title"
#define ELAPSED 0

struct bftime bufferTime;

typedef struct _BeginBuffer {
  bheader header;
  ctlbody body;
  
} BeginBuffer;

BeginBuffer buffer;

class stateAssemblyTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(stateAssemblyTests);
  CPPUNIT_TEST(creation);
  CPPUNIT_TEST(complete);
  CPPUNIT_TEST(assemble);
  CPPUNIT_TEST_SUITE_END();


private:
  StateTransitionAssemblyEvent* m_pAssembly;
  StateTransitionFragment*      m_pInitialFragment;

public:
  void setUp() {
    vector<uint16_t> neededNodes;
    neededNodes.push_back(0x5555);
    neededNodes.push_back(0xaaaa);
    neededNodes.push_back(0x1234);
    NodeScoreboard::neededNodes(neededNodes);

    //  Initial fragment (comes from node 0x5555).
    
    initializeBuffer();
    m_pInitialFragment = new StateTransitionFragment(reinterpret_cast<uint16_t*>(&buffer));

    // Create the assembly:

    m_pAssembly = new StateTransitionAssemblyEvent(*m_pInitialFragment);

  }
  void tearDown() {
    delete m_pAssembly;
    memset(&buffer, 0, sizeof(buffer));
  }
protected:
  void creation();
private:
  void initializeBuffer();
  void complete();
  void assemble();
};


void stateAssemblyTests::initializeBuffer()
{
  buffer.header.nwds = sizeof(BeginBuffer)/sizeof(uint16_t);
  buffer.header.type        = TYPE;
  buffer.header.run         = RUN;
  buffer.header.seq         = 0;
  buffer.header.nevt        = 0;
  buffer.header.cpu         = CPU;
  buffer.header.buffmt      = 6;
  buffer.header.ssignature  = 0x0102;
  buffer.header.lsignature  = 0x01020304;
  buffer.header.nwdsHigh    = 0; 
  memset(buffer.body.title, 0, 80);
  strcpy(buffer.body.title, TITLE);
  buffer.body.sortim        = ELAPSED;
  
  time_t now = time(&now);
  struct tm* pTime = localtime(&now);

  bufferTime.month = buffer.body.tod.month = pTime->tm_mon+1;
  bufferTime.day   = buffer.body.tod.day   = pTime->tm_mday;
  bufferTime.year  = buffer.body.tod.year  = pTime->tm_year;
  bufferTime.hours = buffer.body.tod.hours = pTime->tm_hour;
  bufferTime.min   = buffer.body.tod.min   = pTime->tm_min;
  bufferTime.sec   = buffer.body.tod.sec   = pTime->tm_sec;
  bufferTime.tenths= buffer.body.tod.tenths= 0;	// Unix does not give us that.
  
}

CPPUNIT_TEST_SUITE_REGISTRATION(stateAssemblyTests);


// Creating should result in
// incomplete, not physics, .. in our case BERUNBF type.

void stateAssemblyTests::creation() {

  ASSERT(!m_pAssembly->isComplete());

  ASSERT(!m_pAssembly->isPhysics());

  EQMSG("Assembly type", static_cast<uint16_t>(BEGRUNBF), m_pAssembly->type());
}


/// Should be able to add more fragments until it gets complete:


void stateAssemblyTests::complete() {
  buffer.header.cpu =  0xaaaa;

  StateTransitionFragment* pFrag1 = 
    new StateTransitionFragment(reinterpret_cast<uint16_t*>(&buffer));

  buffer.header.cpu =  0x1234;
  StateTransitionFragment* pFrag2 = 
    new StateTransitionFragment(reinterpret_cast<uint16_t*>(&buffer));


  m_pAssembly->add(*pFrag1);
  m_pAssembly->add(*pFrag2);

  ASSERT(m_pAssembly->isComplete());


}

// Create assembled event:

void stateAssemblyTests::assemble()
{
  complete();			// build a complete event.

  AssembledEvent* pEvent = m_pAssembly->assembledEvent();

  // Must be an assembled State transition event:


  AssembledStateTransitionEvent* pStEvent = 
    dynamic_cast<AssembledStateTransitionEvent*>(pEvent);
  ASSERT(pStEvent);

  ///////////// Check stuff common to all assembled buffers:

  EQMSG("node", static_cast<unsigned short>(0), pStEvent->node());
  EQMSG("type", AssembledEvent::BeginRun,          pStEvent->type());

  /////////// Check stuff specific to state transition events.


  EQMSG("Title",   string(TITLE),     pStEvent->getTitle());
  EQMSG("Elapsed", static_cast<unsigned long>(ELAPSED),           
	           pStEvent->getElapsedTime());
  EQMSG("Run",     static_cast<unsigned short>(RUN),
	           pStEvent->getRunNumber());

  struct tm timestamp = pStEvent->getTimestamp();
  EQMSG("Time.month", static_cast<int>(bufferTime.month), 
                      timestamp.tm_mon);
  EQMSG("Time.day",   static_cast<int>(bufferTime.day),  
	              timestamp.tm_mday);
  EQMSG("Time.year",  static_cast<int>(bufferTime.year),  
                      timestamp.tm_year);
  EQMSG("Time.hours", static_cast<int>(bufferTime.hours), 
	              timestamp.tm_hour);
  EQMSG("Time.min",   static_cast<int>(bufferTime.min),   
	              timestamp.tm_min);
  EQMSG("Time.sec",   static_cast<int>(bufferTime.sec),   
	              timestamp.tm_sec);


}
