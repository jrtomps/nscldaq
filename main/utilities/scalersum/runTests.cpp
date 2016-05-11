// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CRun.h"


class testRun : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testRun);
  CPPUNIT_TEST(getrun);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(noscalers);
  CPPUNIT_TEST(createIncremental);
  CPPUNIT_TEST(createCumulative);
  CPPUNIT_TEST(makemany1source);
  CPPUNIT_TEST(makemany2sources);
  CPPUNIT_TEST_SUITE_END();


private:
  CRun* m_pRun;
public:
  void setUp() {
    m_pRun = new CRun(1234);
  }
  void tearDown() {
    delete m_pRun;
  }
protected:
  void getrun();
  void empty();
  void noscalers();
  void createIncremental();
  void createCumulative();
  void makemany1source();
  void makemany2sources();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testRun);

void testRun::getrun() {
  EQ(unsigned(1234), m_pRun->getRun());
}


void testRun::empty() {
  std::vector<unsigned> srcs = m_pRun->sources();
  EQ(size_t(0), srcs.size());
}

void testRun::noscalers()
{
  std::vector<uint64_t> scls = m_pRun->sums(0);        // Should be empty vector.
  EQ(size_t(0), scls.size());
}

void testRun::createIncremental()
{
  m_pRun->update(1, 0, 4321, true);
  m_pRun->update(1, 0, 1234, true);
  
  // If this made an incremental scaler the result should be 1234+ 4321.
  
  std::vector<unsigned> sources= m_pRun->sources();
  EQ(size_t(1), sources.size());
  EQ(unsigned(1), sources[0]);
  
  std::vector<uint64_t> sums = m_pRun->sums(1);
  EQ(size_t(1), sums.size());
  EQ(uint64_t(1234 + 4321), sums[0]);
}

void testRun::createCumulative()
{
  m_pRun->update(1, 0, 4321, false);
  m_pRun->update(1, 0, 1234, false);    // 32 bit wrap.
  
  uint64_t result(1234);
  result += 0x100000000;
  
  std::vector<uint64_t> sums = m_pRun->sums(1);
  EQ(size_t(1), sums.size());
  EQ(result, sums[0]);
}

void testRun::makemany1source()
{
  for (int i =0; i < 32; i++) {
    m_pRun->update(1, i, i, true);
  }
  std::vector<uint64_t> sums = m_pRun->sums(1);
  
  EQ(size_t(32), sums.size());
  for (int i = 0; i < 32; i++) {
    EQ(uint64_t(i), sums[i]);
  }
}

void testRun::makemany2sources()
{
  for (int i =0; i < 32; i++) {
    m_pRun->update(1, i, i, true);
    m_pRun->update(2, i, 32-i, false);
  }
  
  std::vector<unsigned> srcs = m_pRun->sources();
  EQ(size_t(2), srcs.size());
  EQ(unsigned(1), srcs[0]);
  EQ(unsigned(2), srcs[1]);
  
  // Check the data:
  
  std::vector<uint64_t> sums = m_pRun->sums(1);           // Source 1:
  EQ(size_t(32), sums.size());
  for (int i = 0; i < 32; i++) {
    EQ(uint64_t(i), sums[i]);
  }
  
  sums = m_pRun->sums(2);                          // Source 2.
  EQ(size_t(32), sums.size());
  for (int i = 0; i < 32; i++) {
    EQ(uint64_t(32 - i) , sums[i]);
  }
}