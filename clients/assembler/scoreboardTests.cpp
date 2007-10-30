// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <stdint.h>
#include <vector>
#include "Asserts.h"
#include "NodeScoreboard.h"
#include "InvalidNodeException.h"
#include <RangeError.h>

using namespace std;


class sbTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sbTests);
  CPPUNIT_TEST(initiallyIncomplete);
  CPPUNIT_TEST(canComplete);
  CPPUNIT_TEST(canClear);
  CPPUNIT_TEST(detectInvalidNode);
  CPPUNIT_TEST(detectTooManyNodes);
  CPPUNIT_TEST_SUITE_END();


private:
  NodeScoreboard*   m_pScoreBoard;
public:
  void setUp() {
    vector<uint16_t> nodes;
    nodes.push_back(0x80);
    nodes.push_back(0x85);
    m_pScoreBoard = new NodeScoreboard;
    NodeScoreboard::neededNodes(nodes);
  }
  void tearDown() {
    delete m_pScoreBoard;
    m_pScoreBoard = 0;
  }
protected:
  void initiallyIncomplete();
  void canComplete();
  void canClear();
  void detectInvalidNode();
  void detectTooManyNodes();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sbTests);

// After construction, the scoreboard is incomplete.

void sbTests::initiallyIncomplete() {
  ASSERT(!m_pScoreBoard->isComplete());
}

// Can complete the scoreaboard with the require nodes:

void sbTests::canComplete() {
  m_pScoreBoard->addNode(0x85);
  
  // not yet complete:

  ASSERT(!m_pScoreBoard->isComplete());

  m_pScoreBoard->addNode(0x80);

  ASSERT(m_pScoreBoard->isComplete());
}

// Can clear the scoreboard.

void sbTests::canClear()
{
  canComplete();		// Scoreboard is now complete...
  m_pScoreBoard->clear();	//  now incomplete.
  ASSERT(!m_pScoreBoard->isComplete());

  // Should be able to complete again:

  canComplete();
}

// Can detect addition of invalid node:

void sbTests::detectInvalidNode()
{
  bool caught = false;
  uint16_t      badNode=(0xffff);
  try {
    m_pScoreBoard->addNode(static_cast<uint16_t>(12));	// Not in the list.
  }
  catch (InvalidNodeException except) {
    caught = true;
    badNode = except.getNode();
  }
  ASSERT(caught);
  EQ(static_cast<uint16_t>(12), badNode);

}

// Can catch too many nodes being added to the scoreboard.

void sbTests::detectTooManyNodes()
{
  vector<uint16_t> nodes;

  for (uint16_t i=0; i < 33; i++) {
    nodes.push_back(i);
  }

  // now all the bits in the bitmask are in use so this will kill it:

  bool caught = false;

  try {
    NodeScoreboard::neededNodes(nodes);
  }
  catch (CRangeError error) {
    caught = true;
  }
  ASSERT(caught);
}
