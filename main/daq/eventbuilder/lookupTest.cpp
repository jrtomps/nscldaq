// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <os.h>
#include <CPortManager.h>
#include "CEventOrderClient.h"
#include <ErrnoException.h>

class lookupTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(lookupTests);
  CPPUNIT_TEST(noserver);
  CPPUNIT_TEST(server);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void noserver();
  void server();
};

CPPUNIT_TEST_SUITE_REGISTRATION(lookupTests);

//
// Lookup should throw CErrnoException with ENOENT if there is no
// server registered.

void lookupTests::noserver() {
  bool thrown           = false;
  bool correctException = false;
  try {
    uint16_t port = CEventOrderClient::Lookup("localhost");
  }
  catch(CErrnoException& e) {
    thrown = true;
    correctException = (ENOENT == e.ReasonCode());
  }
  catch (...) {
  }
  ASSERT(thrown);
  ASSERT(correctException);
}
//
// Lookup should be ok if there is a server registered to us:
//
void lookupTests::server() {
  // Register a server:

  CPortManager manager;
  int p = manager.allocatePort("ORDERER"); // Change if this changes in CEventOrderClient.cpp
  uint16_t evbPort;
  bool thrown = false;
  try {
    evbPort = CEventOrderClient::Lookup("localhost");
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);
  EQ(p, (int)evbPort);
}
