// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#define protected public
#include <CRingTextItem.h>
#undef private
#undef protected

#include <DataFormat.h>

#include <string>
#include <vector>

#include <string.h>

using namespace std;

class texttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(texttests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void simplecons();
  void fullcons();
  void castcons();
  void accessors();
};

CPPUNIT_TEST_SUITE_REGISTRATION(texttests);

// Test simple construction

void texttests::simplecons() {
  // Make some strings:

  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  CRingTextItem item(PACKET_TYPES, strings);

  EQ((uint32_t)0, item.m_pItem->s_timeOffset);
  EQ((uint32_t)4, item.m_pItem->s_stringCount);

  // Check the contents:

  const char* p = item.m_pItem->s_strings;
  EQ(s1, string(p));
  p  += strlen(p) + 1;
  EQ(s2, string(p));
  p += strlen(p) +1;
  EQ(s3, string(p));
  p += strlen(p) + 1;
  EQ(s4, string(p));

}

// Test the full constructor.

void texttests::fullcons()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  CRingTextItem item(PACKET_TYPES, strings,
		     1234,
		     5678);
  
  EQ((uint32_t)1234, item.m_pItem->s_timeOffset);
  EQ((uint32_t)4, item.m_pItem->s_stringCount);
  EQ((time_t)5678, item.m_pItem->s_timestamp);



   // Check the contents:

  const char* p = item.m_pItem->s_strings;
  EQ(s1, string(p));
  p  += strlen(p) + 1;
  EQ(s2, string(p));
  p += strlen(p) +1;
  EQ(s3, string(p));
  p += strlen(p) + 1;
  EQ(s4, string(p));
}
// test the 'casting' constructor.

void texttests::castcons()
{
  // Create a RingItem that is actually a text ring item:

  CRingItem ritem(PACKET_TYPES);
  
  pTextItem pText = reinterpret_cast<pTextItem>(ritem.getItemPointer());
  pText->s_timeOffset = 1234;
  pText->s_timestamp  = 4321;
  pText->s_stringCount= 4;
  char* p = pText->s_strings;
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  strcpy(p, s1.c_str());
  p += strlen(p)+1;
  strcpy(p, s2.c_str());
  p += strlen(p)+1;
  strcpy(p, s3.c_str());
  p += strlen(p)+1;
  strcpy(p, s4.c_str());
  p += strlen(p)+1;
  ritem.setBodyCursor(p);

  bool thrown = false;

  try {
    CRingTextItem item(ritem);
    pText = reinterpret_cast<pTextItem>(item.getItemPointer());
    EQ((uint32_t)1234, pText->s_timeOffset);
    EQ((time_t)4321, pText->s_timestamp);
    EQ((uint32_t)4,    pText->s_stringCount);
    
    // Check the contents:
    
    char* p = item.m_pItem->s_strings;
    EQ(s1, string(p));
    p  += strlen(p) + 1;
    EQ(s2, string(p));
    p += strlen(p) +1;
    EQ(s3, string(p));
    p += strlen(p) + 1;
    EQ(s4, string(p));
    
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);


  // Should fail:

  CRingItem bad(BEGIN_RUN);
  thrown = false;
  try {
    CRingTextItem item(bad);
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(thrown);
}


// test the accessor functions:

void texttests::accessors()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);


  CRingTextItem item(PACKET_TYPES, strings );

  vector<string> content = item.getStrings();
 ASSERT(strings ==  content);

  //  The simpler ones.

 item.setTimeOffset(1234);
 EQ((uint32_t)1234, item.getTimeOffset());

 item.setTimestamp(66776);
 EQ((time_t)66776, item.getTimestamp());

}
