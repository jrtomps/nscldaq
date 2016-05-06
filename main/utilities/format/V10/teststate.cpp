// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#define proteted public
#include <V10/DataFormatV10.h>
#include <V10/CRingStateChangeItem.h>
#undef private
#undef protected

#include <V10/DataFormatV10.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace DAQ::V10;

class teststate : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(teststate);
  CPPUNIT_TEST(basiccons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST(copycons);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void basiccons();
  void fullcons();
  void castcons();
  void accessors();
  void copycons();
};

CPPUNIT_TEST_SUITE_REGISTRATION(teststate);

// Basic construction test.
// Make a state change item, and ensure it's properly initialized.
//
void teststate::basiccons() {
  CRingStateChangeItem item(END_RUN);
  EQ(END_RUN, item.type());
  EQ((uint32_t)0, item.m_pItem->s_runNumber);
  EQ((uint32_t)0, item.m_pItem->s_timeOffset);
  EQ((uint32_t)time(NULL), item.m_pItem->s_Timestamp);
  
}
//
// Do a full construction and test that out.
//
void teststate::fullcons()
{
  // One where the title fits just fine.

  CRingStateChangeItem item(PAUSE_RUN,
			    1234,
			    5678,
			    314159,
			    "This title fits");
  EQ(PAUSE_RUN, item.type());
  EQ((uint32_t)1234, item.getRunNumber());
  EQ((uint32_t)5678, item.getElapsedTime());
  EQ((time_t)(314159), item.getTimestamp());

  // The title does not fit in this one:

  string title("supercalifragalisticexpialidocious");
  while (title.size() <= TITLE_MAXSIZE) {
    title += title;
  }
  

  bool threw(false);
  try {
    CRingStateChangeItem(RESUME_RUN,
			 1234, 5678, 314159,
			 title);
  }
  catch (CRangeError& except) {
    threw = true;
  }
  ASSERT(threw);
  
  
}
// Test construction that effectively up-casts a CRingItem.
//
void
teststate::castcons()
{
  // Successful requires that the underlying ring item have the
  // right type.

  bool threw(false);
  CRingItem ok(BEGIN_RUN, sizeof(StateChangeItem) - sizeof(RingItemHeader));
  CRingItem bad(PHYSICS_EVENT);

  try {
    CRingStateChangeItem state(ok);
    size_t itemSize = state.getBodySize() + sizeof(RingItemHeader);
    EQ(sizeof(StateChangeItem), itemSize);
  }
  catch (bad_cast c) {
    threw = true;
  }
  ASSERT(!threw);



  threw = false;
  try {
    CRingStateChangeItem notstate(bad);
  } 
  catch (bad_cast c) {
    threw = true;
  }
  ASSERT(threw);
}


// Test the accessor functions.
void
teststate::accessors()
{
  CRingStateChangeItem empty(BEGIN_RUN);
  empty.setRunNumber(1234);
  EQ((uint32_t)1234, empty.getRunNumber());
  

  empty.setElapsedTime(45);
  EQ((uint32_t)45, empty.getElapsedTime());

  string title("this is my title");
  empty.setTitle(title);
  EQ(title, empty.getTitle());

  empty.setTimestamp(314159);
  EQ((time_t)(314159), empty.getTimestamp());
  
}
/**
 ** test copy construction.. need to wind up with the same sizes and contents as before.
 */
void teststate::copycons()
{
  std::string title("This is a test title");

  CRingStateChangeItem original(BEGIN_RUN,
				1234, 0, (time_t)(56789),
				title);
  CRingStateChangeItem copy(original); // Copy construction.

  EQ(original.getBodySize(), copy.getBodySize());
  _RingItem* porig = original.getItemPointer();
  _RingItem* pcopy = copy.getItemPointer();

  // headers must match 

  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

  // Contents must match:

  EQ(original.getRunNumber(),   copy.getRunNumber());
  EQ(original.getElapsedTime(), copy.getElapsedTime());
  EQ(original.getTitle(),         copy.getTitle());
  EQ(original.getTimestamp(),   copy.getTimestamp());
}
