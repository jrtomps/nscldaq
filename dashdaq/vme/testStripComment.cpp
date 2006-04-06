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

class testComment : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testComment);
  CPPUNIT_TEST(noComment);
  CPPUNIT_TEST(lineCommentAndStuff);
  CPPUNIT_TEST(lineCommentNoStuff);
  CPPUNIT_TEST(onlyComment);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void noComment();
  void lineCommentAndStuff();
  void lineCommentNoStuff();
  void onlyComment();
  void empty();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testComment);

// lines with no comment don't get altered:

void testComment::noComment() {
  string test    = "This line has no comment";
  string result = stripComment(test);
  EQ(test, result);
}
// A line with a comment and trailing stuff.

void testComment::lineCommentAndStuff()
{
  string test    = "This line has a #comment with stuff after it";
  string result  = stripComment(test);
  EQ(string("This line has a "), result);
}
// A line with a trailing hash only.

void testComment::lineCommentNoStuff()
{
  string test   = "This line has a trailing #";
  string result = stripComment(test);
  EQ(string("This line has a trailing "), result);
}
// Cases where there's only a comment:

void testComment::onlyComment()
{
  string test1 = "# This line only has a comment";
  string test2 = "#";

  string result = stripComment(test1);
  EQMSG(test1, string(""), result);
  
  result = stripComment(test2);
  EQMSG(test2, string(""), result);
}
// Empty string remains empty:

void testComment::empty()
{
  string result = stripComment(string(""));
  EQ(string(""), result);
}
