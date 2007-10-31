// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "PhysicsAssemblyEvent.h"
#include "AssemblyEvent.h"
#include "AssembledEvent.h"
#include "AssembledPhysicsEvent.h"
#include "PhysicsFragment.h"
#include "NodeScoreboard.h"
#include <vector>
#include <stdint.h>
#include <buffer.h>
#include <buftypes.h>

#include <string>

using namespace std;


static const uint32_t INITIALSTAMP(0x12345678);

class physicsAssemblyTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physicsAssemblyTests);
  CPPUNIT_TEST(creation);
  CPPUNIT_TEST(complete);
  CPPUNIT_TEST(assembly);
  CPPUNIT_TEST_SUITE_END();


private:
  PhysicsAssemblyEvent*    m_pAssembly;
  PhysicsFragment*         m_pFirstFragment;
public:
  void setUp() {
    // Specify the needed nodes:

    vector<uint16_t> neededNodes;
    neededNodes.push_back(0x5555);
    neededNodes.push_back(0xaaaa);
    neededNodes.push_back(0xbbbb);

    NodeScoreboard::neededNodes(neededNodes);


    // Create the first physics fragment. 
    // A countinb pattern event from node 0x5555
    // with timestamp 0x12345678

    vector<uint16_t> firstBody;
    for (int i=0; i < 10; i++) {
      firstBody.push_back(i);
    }
    m_pFirstFragment = new PhysicsFragment(0x5555, firstBody, INITIALSTAMP);

    // Create the assembly:

    m_pAssembly = new PhysicsAssemblyEvent(m_pFirstFragment);

  }
  void tearDown() {
    delete m_pAssembly;		// Deletes the fragments too.
  }
protected:
  void creation();
  void complete();
  void assembly();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physicsAssemblyTests);

// Check that initial fragments have the right properties.

void physicsAssemblyTests::creation() {
  // Should not be complete..

  ASSERT(!m_pAssembly->isComplete());

  // Should be physics.

  ASSERT(m_pAssembly->isPhysics());

}

// Check that if I addd fragments from 0xaaaa, 0xbbbb
// I'll complete the assembly.
// Time stamps will be close to the initial 0x12345678

void physicsAssemblyTests::complete()
{
  // event bodies... the one from 0xaaaa will be an alternating 
  // 0xaaaa 0x5555 pattern.

  vector<uint16_t> bodyaaaa;
  for (int i =0; i < 5; i++) {
    bodyaaaa.push_back(0xaaaa);
    bodyaaaa.push_back(0x5555);
  }
  // the one from 0xbbbb will be alternating 0xbbbb and 0.

  vector<uint16_t> bodybbbb;
  for (int i = 0; i < 5; i++) {
    bodybbbb.push_back(0xbbbb);
    bodybbbb.push_back(0);
  }
  m_pAssembly->add(*(new PhysicsFragment(0xaaaa, bodyaaaa, INITIALSTAMP+1)));
  m_pAssembly->add(*(new PhysicsFragment(0xbbbb, bodybbbb, INITIALSTAMP-1)));

  ASSERT(m_pAssembly->isComplete());
}
// See if we can retrieve a good assembled event from the assembly.

void physicsAssemblyTests::assembly()
{
  complete();			// Create the completed event...

  AssembledEvent* pEvent = m_pAssembly->assembledEvent();
  ASSERT(pEvent);
  
  AssembledPhysicsEvent* pPhysics = dynamic_cast<AssembledPhysicsEvent*>(pEvent);
  ASSERT(pPhysics);

  uint16_t* pTheData = new uint16_t[pPhysics->size()];

  pPhysics->copyOut(pTheData);

  // Should be 3 packets in order in which they were inserted into the event.
  // each packet is a longword size, followed by the node id, followed by the
  // timestamp, followed by the body.  We won't check the sizes, just rely 
  // on the sizes to keep directing us to the right spot.

  struct __attribute__((__packed__)) packetHeader {
    uint32_t size;
    uint16_t node;
    uint32_t timestamp;
  } *pPacketHeader;

  int headerWords = sizeof(struct packetHeader)/sizeof(uint16_t);

  uint16_t* p = pTheData;
  pPacketHeader = reinterpret_cast<struct packetHeader*>(p);

  // first packet:

  EQMSG("1'st node", static_cast<uint16_t>(0x5555), pPacketHeader->node);
  EQMSG("1'st timestamp", static_cast<uint32_t>(INITIALSTAMP),
	pPacketHeader->timestamp);

  // 1'st packet body is a counting pattern

  uint32_t size = pPacketHeader->size - headerWords;
  p += headerWords;

  for (uint16_t i =0; i < size; i++) {
    EQMSG("1'st body", i, *p++);
  }

  // 2nd' packet (p points there):

  pPacketHeader = reinterpret_cast<struct packetHeader*>(p);
  
  EQMSG("2nd node", static_cast<uint16_t>(0xaaaa),
	pPacketHeader->node);
  EQMSG("2nd timestamp", static_cast<uint32_t>(INITIALSTAMP+1),
	pPacketHeader->timestamp);

  // 2nd packet body alternates 0xaaaa, 0x5555

  uint16_t patternaaaa[2] = {0xaaaa, 0x5555};
  p += headerWords;
  size = pPacketHeader->size - headerWords;
  for (int i=0; i <size; i++) {
    uint16_t sb = patternaaaa[i%2];
    EQMSG("2ND BODY", sb, *p++);
  }
  // 3'd packet :

  pPacketHeader = reinterpret_cast<struct packetHeader*>(p);
  EQMSG("3'd node", static_cast<uint16_t>(0xbbbb),
	pPacketHeader->node);
  EQMSG("3'd timestamp", static_cast<uint32_t>(INITIALSTAMP-1),
	pPacketHeader->timestamp);

  p += headerWords;
  size = pPacketHeader->size - headerWords;

  uint16_t patternbbbb[2] = {0xbbbb, 0};
  for (int i =0; i < size; i++) {
    EQMSG("3'd body", patternbbbb[i%2], *p++);
  }

  delete []pTheData;
  delete pPhysics;



}
