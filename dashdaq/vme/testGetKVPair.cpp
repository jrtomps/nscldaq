// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <parseUtilities.h>

using namespace descriptionFile;

using namespace std;

class getKVTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(getKVTests);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(one);
  CPPUNIT_TEST(two);
  CPPUNIT_TEST(three);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void empty();
  void one();
  void two();
  void three();
};

CPPUNIT_TEST_SUITE_REGISTRATION(getKVTests);


// Feeding an empty string should give back a pair of empties
// and an empty line.
void getKVTests::empty() {
  string emptyline("");
  pair<string, string> result = getKeywordValue(emptyline);
  EQMSG("first", string(""), result.first);
  EQMSG("second", string(""), result.second);
  EQMSG("input", string(""), emptyline);
}

// Line with single keyword:

void getKVTests::one()
{
  string line("atest ");
  pair<string, string> result = getKeywordValue(line);
  line = stripLeadingBlanks(line);
  
  EQMSG("residual", string(""), line);
  EQMSG("first", string("atest"), result.first);
  EQMSG("second", string(""), result.second);
}

// Line with 2 keywords.
//
void getKVTests::two()
{
  string line("-crate 5    ");
  pair<string, string> result = getKeywordValue(line);
  line = stripLeadingBlanks(line);

  EQMSG("residual", string(""), line);
  EQMSG("first", string("-crate"), result.first);
  EQMSG("second", string("5"), result.second);

}
// Line with 3 words.

void getKVTests::three()
{
  string line("-crate 5 end");
  pair<string,string> result = getKeywordValue(line);
  line = stripLeadingBlanks(line);

  EQMSG("residual", string("end"), line);
  EQMSG("first", string("-crate"), result.first);
  EQMSG("second", string("5"), result.second);
}
