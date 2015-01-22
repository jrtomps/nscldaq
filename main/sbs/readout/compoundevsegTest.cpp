// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <RangeError.h>

#include "CEventSegment.h"
#include "CCompoundEventSegment.h"

using namespace std;



class compoundevTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(compoundevTest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(add);
  CPPUNIT_TEST(Delete);
  CPPUNIT_TEST(visitors);
  CPPUNIT_TEST(readover);
  CPPUNIT_TEST_SUITE_END();


private:
// concrete Event segment class that will be used in testing:
//
class TestSegment : public CEventSegment
{
public:
  bool isInitialized;
  bool isCleared;
  bool isDisabled;
  bool isRead;

  static uint16_t next;

  TestSegment() : isInitialized(false),
		  isCleared(false),
		  isDisabled(false),
		  isRead(false)
  {}

  virtual size_t read(void* pBuffer, size_t maxwords) {
    uint16_t* p = reinterpret_cast<uint16_t*>(pBuffer);
    *p = next++;
    isRead = true;
    return 1;
  }
  virtual void initialize() { isInitialized = true;
                             next = 0; }
  virtual void clear() { isCleared = true; }
  virtual void disable() { isDisabled = true; }
};


public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();
  void add();
  void Delete();
  void visitors();
  void readover();
};

CPPUNIT_TEST_SUITE_REGISTRATION(compoundevTest);

uint16_t compoundevTest::TestSegment::next;


// Construction should give the usual answers:
// size = 0,
// begin == end()
// isComposite == true
void compoundevTest::construct() {
  CCompoundEventSegment seg;

  EQ(static_cast<size_t>(0), seg.size());
  ASSERT(seg.begin() == seg.end());
  ASSERT(seg.isComposite());
}
//
// add should be a perfunctory test.  We should be able to create a hierarchy
// get the right answers for the counts functions and
// be able to iterate to get the correct top level entries:

void compoundevTest::add()
{
  TestSegment a,b,c,d,e;
  CCompoundEventSegment top, bottom;

  CEventSegment* pTop[]    = {&a, &bottom, &b};
  CEventSegment* pBottom[] = {&c, &d, &e};

  for (int i=0; i < 3; i++) {
    top.AddEventSegment(pTop[i]);
    bottom.AddEventSegment(pBottom[i]);
  }

  EQ(static_cast<size_t>(3), top.size());
  EQ(static_cast<size_t>(5), top.leafCount());
  EQ(static_cast<size_t>(6), top.fullCount());

  CCompoundEventSegment::EventSegmentIterator p = top.begin();
  CCompoundEventSegment::EventSegmentIterator q = bottom.begin();

  for (int i = 0; i < 3; i++) {
    EQ(dynamic_cast<CEventSegment*>(pTop[i]), dynamic_cast<CEventSegment*>(*p));
    EQ(dynamic_cast<CEventSegment*>(pBottom[i]), dynamic_cast<CEventSegment*>(*q));

    p++;
    q++;
  }

     
}

// Delete should similarly be a perfunctory test
// as it is handled mostly by delegation to the base class:
//
void compoundevTest::Delete()
{
  CCompoundEventSegment   seg;
  TestSegment             s1, s2,s3,s4;
  CEventSegment* pSeg[] = {&s1, &s2, &s3, &s4};
  for (int i = 0; i < 4; i++) {
    seg.AddEventSegment(pSeg[i]);
  }

  seg.DeleteEventSegment(&s2);

  EQ(static_cast<size_t>(3), seg.size());
  EQ(static_cast<size_t>(3), seg.leafCount());
  EQ(static_cast<size_t>(3), seg.fullCount());

  CEventSegment* pShouldBe[] =  {&s1, &s3, &s4};

  CCompoundEventSegment::EventSegmentIterator p = seg.begin();

  for (int i=0; i < 3;i ++) {
    EQ(pShouldBe[i], reinterpret_cast<CEventSegment*>(*p));
    p++;
  }
  ASSERT(p == seg.end());

}


void compoundevTest::visitors()
{
  TestSegment a,b,c,d,e;
  CCompoundEventSegment top, bottom;

  CEventSegment* pTop[]    = {&a, &bottom, &b};
  CEventSegment* pBottom[] = {&c, &d, &e};

  for (int i=0; i < 3; i++) {
    top.AddEventSegment(pTop[i]);
    bottom.AddEventSegment(pBottom[i]);
  }

  TestSegment* pLeaves[] = {&a, &b, &c, &d, &e};

  top.initialize();
  for (int i=0; i < 5; i++) {
    ASSERT(pLeaves[i]->isInitialized);
  }

  top.clear();
  for (int i=0; i < 5; i++) {
    ASSERT(pLeaves[i]->isCleared);
  }

  top.disable();
  for (int i=0; i < 5; i++) {
    ASSERT(pLeaves[i]->isDisabled);
  }
  
  uint16_t buffer[100];
  size_t nRead = top.read(buffer, 100);
  EQ(static_cast<size_t>(5), nRead);
  for (int i=0; i < 5; i++) {
    ASSERT(pLeaves[i]->isRead);
  }
  for (uint16_t i = 0; i < 5; i++) {
    EQ(i, buffer[i]);
  }


}
//
// An attempt to read too much will throw an exception:
//
void compoundevTest::readover()
{
  TestSegment a,b,c,d,e;
  CCompoundEventSegment top, bottom;

  CEventSegment* pTop[]    = {&a, &bottom, &b};
  CEventSegment* pBottom[] = {&c, &d, &e};

  for (int i=0; i < 3; i++) {
    top.AddEventSegment(pTop[i]);
    bottom.AddEventSegment(pBottom[i]);
  }

  TestSegment* pLeaves[] = {&a, &b, &c, &d, &e};

  top.initialize();

  uint16_t buffer[100];		// ensure we don't cause harm..
  bool     thrown(false);
  try {
    size_t n = top.read(buffer, 2);
  }
  catch (CRangeError& e) {
    thrown = true;
  }
  ASSERT(thrown);
}
