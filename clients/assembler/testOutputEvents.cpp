// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <vector>

#include "OutputPhysicsEvents.h"
#include "PhysicsFragment.h"
#include "PhysicsAssemblyEvent.h"
#include "StateTransitionFragment.h"
#include "StateTransitionAssemblyEvent.h"
#include "NodeScoreboard.h"


#include <buffer.h>
#include <buftypes.h>


using namespace std;


class testOutputEvents : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testOutputEvents);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(insert1);
  CPPUNIT_TEST(insert2);
  CPPUNIT_TEST(findtype1);
  CPPUNIT_TEST(findtype2);
  CPPUNIT_TEST(timematch1);
  CPPUNIT_TEST(timematch2);
  CPPUNIT_TEST(removeprior);
  CPPUNIT_TEST(countprior);
  CPPUNIT_TEST(countrange);
  CPPUNIT_TEST_SUITE_END();


private:
  OutputPhysicsEvents*  m_pAssemblers;
public:
  void setUp() {
    vector<uint16_t> nodes;
    nodes.push_back(0x1234);
    nodes.push_back(0x4321);
    nodes.push_back(0x5555);

    NodeScoreboard::neededNodes(nodes);
    m_pAssemblers = new OutputPhysicsEvents;
  }
  void tearDown() {
    delete m_pAssemblers;
    m_pAssemblers = 0;
  }
protected:
  void construct();
  void insert1();
  void insert2();
  void findtype1();
  void findtype2();
  void timematch1();
  void timematch2();
  void removeprior();
  void countprior();
  void countrange();
private:
  void  setupFindType();
  void  setupWindowMatch();
};

#define TYPE  BEGRUNBF
#define RUN   1234
#define CPU   0x5555
#define TITLE "This is my title"
#define ELAPSED 0

typedef struct _BeginBuffer {
  bheader header;
  ctlbody body;
  
} BeginBuffer;


static vector<uint16_t>
makeBody()
{
  vector<uint16_t> result;
  for (uint16_t i=0; i < 10; i++) {
    result.push_back(i);
  }
  return result;
}

static void
makeBeginBuffer(void* pData)
{
  BeginBuffer& buffer(*(static_cast<BeginBuffer*>(pData)));

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

  buffer.body.tod.month =  pTime->tm_mon+1;
  buffer.body.tod.day   =  pTime->tm_mday;
  buffer.body.tod.year  =  pTime->tm_year;
  buffer.body.tod.hours =  pTime->tm_hour;
  buffer.body.tod.min   =  pTime->tm_min;
  buffer.body.tod.sec   =  pTime->tm_sec;
  buffer.body.tod.tenths=  0;	// Unix does not give us that.
    
}


CPPUNIT_TEST_SUITE_REGISTRATION(testOutputEvents);


// create the initial stockage of events for find1/find2:
//   We add in order:
//     a DATABF assembler,  (node 0x1243 timestamp 100)
//     a BEGRUNBF assembler.(node 0x5555 timestamp)
//     a DATABF assembler.  (node 0x4321, timestamp 0x300).
//
void
testOutputEvents::setupFindType()
{
  static uint16_t startBuffer[4096];

  vector<uint16_t> body = makeBody();
  AssemblyEvent* pAssembly = new PhysicsAssemblyEvent(new PhysicsFragment(0x1234, body, 100));
  m_pAssemblers->add(*pAssembly);

  makeBeginBuffer(startBuffer);
  pAssembly = new StateTransitionAssemblyEvent(*(new StateTransitionFragment(startBuffer)));
  m_pAssemblers->add(*pAssembly);


  pAssembly = new PhysicsAssemblyEvent(new PhysicsFragment(0x4321, body, 300));
  m_pAssemblers->add(*pAssembly);
}


// puts a bunch of physics events in the pot separated by 1000 clicks.

void testOutputEvents::setupWindowMatch()
{
  vector<uint16_t> body = makeBody();
  
  for (int i = 0; i < 10000; i += 1000) {
    AssemblyEvent* pAssembly = new PhysicsAssemblyEvent(new PhysicsFragment(0x1234, body, i));
    m_pAssemblers->add(*pAssembly);
  }
}

////////////////////////////// The tests ////////////////////////////////////////////

// Just after construction:
// size  == 0
// begin == end
//

void testOutputEvents::construct() 
{
  EQMSG("Size", static_cast<size_t>(0), m_pAssemblers->size());
  ASSERT(m_pAssemblers->begin() == m_pAssemblers->end());
}
// When a single event is inserted:
// size == 1
// begin != end
// begin++ == end
// *begin == ptr to the item inserted.
//
// Physics event fragments/assemblers are the simplest to do so that's what we'll do.
//
void
testOutputEvents::insert1()
{
  PhysicsFragment*      pFragment;
  AssemblyEvent*        pAssembly;

  vector<uint16_t> body = makeBody();

  pFragment = new PhysicsFragment(0x1234, body, 0x87654321);
  pAssembly = new PhysicsAssemblyEvent(pFragment);

  m_pAssemblers->add(*pAssembly);

  // Now we can test th eassertinos:

  EQMSG("Size", static_cast<size_t>(1), m_pAssemblers->size());
  
  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  OutputPhysicsEvents::iterator e = m_pAssemblers->end();

  if (i == e) {
    FAIL("begin == end");
  }
  
  EQMSG("Pointer matching", pAssembly, *i);

  i++;
  ASSERT(i == e);
  
}
// After inserting 2 assemblers
// size ==2
// Assemblers can be pulled out in order.
//
void
testOutputEvents::insert2()
{
  PhysicsFragment* pFrag1;
  PhysicsFragment* pFrag2;
  AssemblyEvent*   pAss1;
  AssemblyEvent*   pAss2;

  vector<uint16_t> body = makeBody();

  pFrag1 = new PhysicsFragment(0x1234, body, 0x100);
  pFrag2 = new PhysicsFragment(0x4321, body, 0x200);

  pAss1  = new PhysicsAssemblyEvent(pFrag1);
  pAss2  = new PhysicsAssemblyEvent(pFrag2);

  m_pAssemblers->add(*pAss1);
  m_pAssemblers->add(*pAss2);

  EQMSG("Size", static_cast<size_t>(2), m_pAssemblers->size());

  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  EQMSG("First is first", pAss1, *i);
  i++;
  EQMSG("Second is second", pAss2, *i);
  i++;

  ASSERT(i == m_pAssemblers->end());
}
//  searching in our hoked up scheme by type:
//  Search for DATABF should give us the first entry.
//  Search for BEGRUNBF should give us the second entry.
void
testOutputEvents::findtype1()
{
  setupFindType();		// Stock the assembler list.

  // find the first physics data...

  OutputPhysicsEvents::iterator i = m_pAssemblers->findByType(DATABF);
  ASSERT(i != m_pAssemblers->end()); // Should find one.

  AssemblyEvent* pEvent = *i;
  ASSERT(pEvent->isPhysics());
  EQMSG("DATABF type", static_cast<uint16_t>(DATABF), pEvent->type());
  EQMSG("DATABF stamp", static_cast<uint32_t>(100),   pEvent->timestamp());

  // Find the first BEGRUNBF:

  i = m_pAssemblers->findByType(BEGRUNBF);
  ASSERT(i != m_pAssemblers->end());
  pEvent = *i;
  ASSERT(!pEvent->isPhysics());
  EQMSG("BEGRUNBF type", static_cast<uint16_t>(BEGRUNBF), pEvent->type());
  EQMSG("DATABF stamp", static_cast<uint32_t>(0), pEvent->timestamp());

}
//  Searching for the second DATABF by selecting where the search starts.

void
testOutputEvents::findtype2()
{
  setupFindType();
 
  //
  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  i++;				// Now we start searching with the second event.
  i = m_pAssemblers->findByType(DATABF, i, m_pAssemblers->end());

  ASSERT(i != m_pAssemblers->end());
  AssemblyEvent* pEvent = *i;
  ASSERT(pEvent->isPhysics());
  EQMSG("DATABF type", static_cast<uint16_t>(DATABF), pEvent->type());
  EQMSG("DATABF stamp", static_cast<uint32_t>(300), pEvent->timestamp());
}

// Test the ability to find fragments with simple time windows.
// A bit of whiteboxedness, all searches reduce to the guy with iterators so we won't
// explicitly test that.
// simple time windows are those that don't wrap 0.

void
testOutputEvents::timematch1()
{
  setupWindowMatch();

  // Windows are inclusive of endpoints so.. this should match the guy at t=0:

  OutputPhysicsEvents::iterator i; 

  i = m_pAssemblers->findPhysByWindow(0, 100);
  ASSERT(i != m_pAssemblers->end());
  AssemblyEvent* pEvent = *i;
  EQMSG("Found timestamp", static_cast<uint32_t>(0), pEvent->timestamp());
  
  i = m_pAssemblers->findPhysByWindow(500, 1500);
  ASSERT(i != m_pAssemblers->end());
  pEvent = *i;
  EQMSG("found 500,1500", static_cast<uint32_t>(1000), pEvent->timestamp());

  // This should fail to match.

  i = m_pAssemblers->findPhysByWindow(20000, 30000);
  ASSERT(i == m_pAssemblers->end());
  
}



// Try matches with 'wrapping' intevals.  This is also going to involve
// removing one of the test events at some point.

void
testOutputEvents::timematch2()
{
  setupWindowMatch();

  OutputPhysicsEvents::iterator i;

  i = m_pAssemblers->findPhysByWindow(0xffffff00, 100);	// Includes 0.
  ASSERT(i != m_pAssemblers->end());
  AssemblyEvent* pEvent  = *i;
  EQMSG("found large,100", static_cast<uint32_t>(0), pEvent->timestamp());
  
  // get rid of the first event, and search the same range.  that should fail.

  pEvent = m_pAssemblers->removeItem(i);

  i = m_pAssemblers->findPhysByWindow(0xffffff00, 100);
  ASSERT(i == m_pAssemblers->end());

  
}
// setup for window, remove the first 5 items...check the list
// check that the first one left is at timestamp 5000.
//
void
testOutputEvents::removeprior()
{
  setupWindowMatch();

  // Calculate the iterator:

  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  for (int j =0; j < 5; j++, i++) 
    ;

  OutputPhysicsEvents::AssemblyList removed = m_pAssemblers->removePrior(i);
  
  EQMSG("removed size", static_cast<size_t>(5), removed.size());
  for (int j =0; j < removed.size(); j++) {
    AssemblyEvent* pEvent = removed.front();
    removed.pop_front();
    EQMSG("Removed timestamp", static_cast<uint32_t>(j*1000), pEvent->timestamp());
  }
  // Now check the first one...

  //should have a timestamp of 5000.

  AssemblyEvent* pEvent= *i;
  EQMSG("Remaining timestamp", static_cast<uint32_t>(5000), pEvent->timestamp());
  

}
// As for removeprior but just count.

void
testOutputEvents::countprior()
{
  setupWindowMatch();

  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  for (int  j = 0; j < 5; j++, i++)
    ;

  ASSERT(m_pAssemblers->countPrior(i) == 5);
}
// count elements in a rang.

void
testOutputEvents::countrange()
{
  setupWindowMatch();

  OutputPhysicsEvents::iterator i = m_pAssemblers->begin();
  for (int j =0; j < 4; j++, i++)
    ;

  ASSERT(m_pAssemblers->countRange(m_pAssemblers->begin(), i) == 4);
  ASSERT(m_pAssemblers->countRange(i, m_pAssemblers->end()) == 6);
}
