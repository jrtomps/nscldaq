// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/mman.h>
#include "daqshm.h"



class detachTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(detachTests);
  CPPUNIT_TEST(notattached);
  CPPUNIT_TEST(detach);
  CPPUNIT_TEST_SUITE_END();


private:
  static const char* shmName;
public:
  void setUp() {
  }
  void tearDown() {
    CDAQShm::remove(shmName);
  }
protected:
  void notattached();
  void detach();
};

const char* detachTests::shmName = "/testshmem";

CPPUNIT_TEST_SUITE_REGISTRATION(detachTests);

void detachTests::notattached() {
  ASSERT(CDAQShm::detach(this, shmName, 0x4000));
  EQ(CDAQShm::NotAttached, CDAQShm::lastError());
}

// Ensure that detaching makes the address invalid.
//
void detachTests::detach()
{
  CDAQShm::create(shmName, 0x4000,0);
  void *pData = CDAQShm::attach(shmName);
  ASSERT(!CDAQShm::detach(pData, shmName, 0x4000));

  EQ(-1, madvise(pData, 0x4000, MADV_SEQUENTIAL));
     
}
