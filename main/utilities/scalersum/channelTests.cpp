// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CIncrementalChannel.h"

class testIncChan : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testIncChan);
  CPPUNIT_TEST(initially0);
  CPPUNIT_TEST(inc1);
  CPPUNIT_TEST(accumulate);
  CPPUNIT_TEST(mask);
  CPPUNIT_TEST_SUITE_END();


private:
CIncrementalChannel* m_pChannel;
public:
  void setUp() {
    m_pChannel = new CIncrementalChannel;
  }
  void tearDown() {
    delete m_pChannel;
  }
protected:
  void initially0();
  void inc1();
  void accumulate();
  void mask();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testIncChan);

void testIncChan::initially0() {
  uint64_t value = *m_pChannel;
  EQ(uint64_t(0), value);
}

// An increment

void testIncChan::inc1()
{
  m_pChannel->update(1234);
  uint64_t value = *m_pChannel;
  EQ(uint64_t(1234), value);
}

// summing works.

void testIncChan::accumulate()
{
  m_pChannel->update(1234);
  m_pChannel->update(4321);
  
  uint64_t value = *m_pChannel;
  EQ(uint64_t(1234+4321), value);
}

// channel width honored:

void testIncChan::mask()
{
  m_pChannel->update(1234 | 0x1000000, 24);   // Should mask the bit off.
  
  uint64_t value = *m_pChannel;
  EQ(uint64_t(1234), value);
}

// Tests for cumulative channels:

#include "CCumulativeChannel.h"

class testCumChan : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testCumChan);
  CPPUNIT_TEST(initially0);
  CPPUNIT_TEST(inc1);
  CPPUNIT_TEST(multiple);
  CPPUNIT_TEST(wrap32);
  CPPUNIT_TEST(inc24);
  CPPUNIT_TEST(wrap24);
  CPPUNIT_TEST_SUITE_END();


private:
CCumulativeChannel* m_pChannel;
public:
  void setUp() {
    m_pChannel = new CCumulativeChannel;
  }
  void tearDown() {
    delete m_pChannel;
  }
protected:
  void initially0();
  void inc1();
  void multiple();
  void wrap32();
  void inc24();
  void wrap24();
};


CPPUNIT_TEST_SUITE_REGISTRATION(testCumChan);

void testCumChan::initially0() {
  uint64_t value = *m_pChannel;
  
  EQ(uint64_t(0), value);
}

void testCumChan::inc1()
{
  m_pChannel->update(1234);
  uint64_t value = *m_pChannel;
  EQ(uint64_t(1234), value);
}

void testCumChan::multiple()
{
  m_pChannel->update(1234);
  m_pChannel->update(5432);      // Bigger so no wrap.
  
  uint64_t value = *m_pChannel;
  EQ(uint64_t(5432), value);
}

void testCumChan::wrap32()
{
  m_pChannel->update(4321);
  m_pChannel->update(1234);    // Looks like a 32 bit wrap.
  
  uint64_t value = *m_pChannel;
  uint64_t sb(0x100000000L);
  sb += 1234;
  EQ(sb, value);
  
}

void testCumChan::inc24()
{
  m_pChannel->update(1234 | 0x1000000, 24);
  uint64_t value = *m_pChannel;
  EQ(uint64_t(1234), value);
}

void testCumChan::wrap24()
{
  m_pChannel->update(4321, 24);
  m_pChannel->update(1234, 24);   // 24 bit wrap.
  uint64_t value = *m_pChannel;
  
  EQ(uint64_t(0x1000000 + 1234), value);
}