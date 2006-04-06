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


class StripTrailing : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StripTrailing);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(noTrailing);
  CPPUNIT_TEST(trailingBlanks);
  CPPUNIT_TEST(trailingTabs);
  CPPUNIT_TEST(trailingMixed);
  CPPUNIT_TEST(onlyWhitespace);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void empty();
  void noTrailing();
  void trailingBlanks();
  void trailingTabs();
  void trailingMixed();
  void onlyWhitespace();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StripTrailing);

// Empty strings should not change.

void StripTrailing::empty() {
  string test    = "";
  string result  = stripTrailingBlanks(test);
  EQ(test, result);
}
// Strings with no trailing whitespace should not change.

void StripTrailing::noTrailing()
{
  string test   = "No trailing whitespace";
  string result = stripTrailingBlanks(test);
  EQ(test, result);
}

// strings that only have " " trailing.

void StripTrailing::trailingBlanks()
{
  string test  = "This string has trailing spaces    ";
  string result= stripTrailingBlanks(test);
  EQ(string("This string has trailing spaces"), result);
}
// Strings that only have \t trailing.

void StripTrailing::trailingTabs()
{
  string test   = "This string has trailing tabs\t\t\t";
  string result =  stripTrailingBlanks(test);
  EQ(string("This string has trailing tabs"), result);
}
// Strings that have mixed trailing tabs/blanks.

void StripTrailing::trailingMixed()
{
  string test = "This string has mixed trailing whitespace \t  \t\t  ";
  string result = stripTrailingBlanks(test);
  EQ(string("This string has mixed trailing whitespace"), result);
}
// Strings that are only whitespace should result in an empty string.

void StripTrailing::onlyWhitespace()
{
  string test = "   \t\t  ";
  string result = stripTrailingBlanks(test);
  EQ(string(""), result);
}
