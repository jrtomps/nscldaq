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




class physicsAssemblyTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physicsAssemblyTests);
  CPPUNIT_TEST(creation);
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
    m_pFirstFragment = new PhysicsFragment(0x5555, firstBody, 0x12345678);

    // Create the assembly:

    m_pAssembly = new PhysicsAssemblyEvent(m_pFirstFragment);

  }
  void tearDown() {
    delete m_pAssembly;		// Deletes the fragments too.
  }
protected:
  void creation();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physicsAssemblyTests);

void physicsAssemblyTests::creation() {
  // Should not be complete..

  ASSERT(!m_pAssembly->isComplete());

  // Should be physics.

  ASSERT(m_pAssembly->isPhysics());

}
