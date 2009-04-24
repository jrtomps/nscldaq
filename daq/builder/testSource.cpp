// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <unistd.h>

// Become CFragmentSource's very best friend...

#define private public
#define protected public
#include "CFragmentSource.h"
#undef private
#undef protected

// here's a null driver:

class NULLFragmentSource : public CFragmentSource 
{
public:
  unsigned int mtimes;
  bool     notlocked;
protected:
  void attempt() {
    notlocked = m_lock.trylock();
    if (notlocked) m_lock.unlock();
  }
  virtual bool     operator()()                 ;
  virtual CBuilderConstant::EventType      provideType()                ;
  virtual void     discardFragment()            ;
  virtual void*    addNextFragment(void *pData) ;
  virtual uint64_t provideTimestamp()           ;
  virtual bool     dataPresent()                ;
public:
  NULLFragmentSource() : mtimes(0) {}

};


// Since no data is required, the source thread will return false:

bool
NULLFragmentSource::operator()() {
  mtimes++;
  return false;
}


// NULL Source never has any event:

CBuilderConstant::EventType NULLFragmentSource::provideType()
{
  attempt();

  return CBuilderConstant::NOEVENT;
}

void NULLFragmentSource::discardFragment() {
  attempt();
}
void* NULLFragmentSource::addNextFragment(void* pData) {
  attempt();
  return pData;
}
uint64_t NULLFragmentSource::provideTimestamp() {
  attempt();
  return static_cast<uint64_t>(1234);
}
bool NULLFragmentSource::dataPresent() {
  attempt();
  return false;
}



class sourceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sourceTest);
  CPPUNIT_TEST(enterleave);
  CPPUNIT_TEST(threadstart);
  CPPUNIT_TEST(nexttype);
  CPPUNIT_TEST(discard);
  CPPUNIT_TEST(timestamp);
  CPPUNIT_TEST(havedata);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void enterleave();
  void threadstart();
  void nexttype();
  void discard();
  void timestamp();
  void havedata();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sourceTest);

// Enter should lock and leave should unlock.

void sourceTest::enterleave() {
  NULLFragmentSource src;

  src.enter();
  ASSERT(!src.m_lock.trylock()); // Can't lock w/o blocking.
  src.leave();
  ASSERT(src.m_lock.trylock());
  src.m_lock.unlock();
}

// Ensure the fragment reader thread will actually run:

void sourceTest::threadstart()
{
  NULLFragmentSource src;
  src.start();			// Schedule the thread.

  usleep(10000);			// Wait a nice ms.
  EQ(1U, src.mtimes);
}
// getNextType both with and without locking.  Should return
// CBuilderConstant::NOEVENT regardless of how.

void
sourceTest::nexttype()
{
  NULLFragmentSource src;
  CBuilderConstant::EventType type = src.getNextType(); // nolock.
  EQ(CBuilderConstant::NOEVENT, type);
  ASSERT(src.notlocked);

  type = src.getNextType(true);
  ASSERT(!src.notlocked);
}

void
sourceTest::discard()
{
  NULLFragmentSource src;
  src.discardNext();
  ASSERT(src.notlocked);
  src.discardNext(true);
  ASSERT(!src.notlocked);
}

void
sourceTest::timestamp()
{
  NULLFragmentSource src;
  uint64_t stamp = src.getNextTimestamp();
  ASSERT(src.notlocked);
  EQ((uint64_t)(1234), stamp);

  stamp = src.getNextTimestamp(true);
  ASSERT(!src.notlocked);
}

void
sourceTest::havedata()
{
  NULLFragmentSource src;
  bool data = src.hasData();
  ASSERT(src.notlocked);
  ASSERT(!data);

  data = src.hasData(true);
  ASSERT(!src.notlocked);
}
