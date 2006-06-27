// Template for a test suite.
#include <config.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "parseUtilities.h"
using namespace descriptionFile;



class StripLeadingBlanks : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StripLeadingBlanks);
  CPPUNIT_TEST(noLeading);
  CPPUNIT_TEST(leadingSpace);
  CPPUNIT_TEST(leadingTabs);
  CPPUNIT_TEST(leadingMixed);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(allBlank);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void noLeading();
  void leadingSpace();
  void leadingTabs();
  void leadingMixed();
  void empty();
  void allBlank();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StripLeadingBlanks);

// A string with no leading blanks is itself.

void StripLeadingBlanks::noLeading() {
  string test   = "a test string";
  string result = stripLeadingBlanks(test);
  EQ(test, result);
}
// String with one or more  ' ' chars.

void StripLeadingBlanks::leadingSpace()
{
  string test1    = " one space";
  string test2    = "  two spaces";
  string test3    = "     several spaces";
  string result;
  
  result = stripLeadingBlanks(test1);
  EQMSG(test1, string("one space"), result);

  result = stripLeadingBlanks(test2);
  EQMSG(test2, string("two spaces"), result);

  result = stripLeadingBlanks(test3);
  EQMSG(test3, string("several spaces"), result);
}

void StripLeadingBlanks::leadingTabs()
{
  string test1 = "\tone tab";
  string test2 = "\t\tTwo tabs";
  string result;

  result = stripLeadingBlanks(test1);
  EQMSG(test1, string("one tab"), result);

  result = stripLeadingBlanks(test2);
  EQMSG(test2, string("Two tabs"), result);
}

void StripLeadingBlanks::leadingMixed()
{
  string test1  = " \tspace tab";
  string test2  = "\t tab space";
  string result;

  result = stripLeadingBlanks(test1);
  EQMSG(test1, string("space tab"), result);

  result = stripLeadingBlanks(test2);
  EQMSG(test2, string("tab space"), result);
}

void StripLeadingBlanks::empty()
{
  string result = stripLeadingBlanks(string(""));
  EQ(string(""), result);
}
void StripLeadingBlanks::allBlank()
{
  string result = stripLeadingBlanks(string("    \t"));
  EQ(string(""), result);
}
