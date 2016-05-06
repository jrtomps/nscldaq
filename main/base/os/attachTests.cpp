// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "daqshm.h"
#include <sys/mman.h>


class attachTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(attachTests);
  CPPUNIT_TEST(noShm);
  CPPUNIT_TEST(ok);
  CPPUNIT_TEST(size);
  CPPUNIT_TEST(mapped);
  CPPUNIT_TEST(doublemapped);
  CPPUNIT_TEST_SUITE_END();


private:
  static const char* shmName;
public:
  void setUp() {
    CDAQShm::create(shmName, 0x10000, 0);
  }
  void tearDown() {
    CDAQShm::remove(shmName);
  }
protected:
  void noShm();
  void ok();
  void size();
  void mapped();
  void doublemapped();
};

const char* attachTests::shmName="/testshm";
CPPUNIT_TEST_SUITE_REGISTRATION(attachTests);

void attachTests::noShm() {
  std::string noSuch(shmName);
  noSuch +=  "-nosuchmemory";
  
  void* pData = CDAQShm::attach(noSuch);
  EQ((void*)0, pData);
  EQ(CDAQShm::NonExistent, CDAQShm::lastError());
}


void attachTests::ok() {
  void* pData = CDAQShm::attach(shmName);
  ASSERT(pData);

}

void attachTests::size()
{
  ssize_t s = CDAQShm::size(shmName);
  EQ((ssize_t)0x10000, s);
}

void attachTests::mapped()
{
  void* pData = 0;
  pData = CDAQShm::attach(shmName);
  ASSERT(pData);
  int status = madvise(pData, 0x10000, MADV_SEQUENTIAL);
  EQ(0, status);
}

// Duplicate mapping gives the same va:
// (white box test)
//
void attachTests::doublemapped() 
{
  void* p1 = CDAQShm::attach(shmName);
  void* p2 = CDAQShm::attach(shmName);

  EQ(p1, p2);

  CDAQShm::detach(p1, shmName, 0x10000);
  ASSERT(!CDAQShm::detach(p1, shmName, 0x10000)); // p1/p2 are the same map.
}
