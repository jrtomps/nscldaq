// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <StateException.h>
#include <CRingBuffer.h>
#include <ringbufint.h>
#include <testcommon.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>



#include "Asserts.h"


using namespace std;


#ifndef SHM_TESTFILE
#define SHM_TESTFILE "mgrtest"
#endif


class ManageTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ManageTests);
  CPPUNIT_TEST(attach);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    CRingBuffer::create(string(SHM_TESTFILE));
  }
  void tearDown() {
    shm_unlink(shmName(SHM_TESTFILE).c_str());
  }
protected:
  void attach();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ManageTests);

// Should be able to attach to the ring buffer as a manager.
// Data transfers should pop a state exception 
void ManageTests::attach() {
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::manager);
  

  // Should not be able to put:

  bool caught = false;
  try {
    char buffer[100];
    ring.put(buffer, sizeof(buffer));
  }
  catch (CStateException& state) {
    caught = true;
  }
  ASSERT(caught);

  caught = false;
  try {
    char buffer[100];
    ring.get(buffer, sizeof(buffer), 1);
  }
  catch (CStateException& state) {
    caught = true;
  }

  ASSERT(caught);
}
