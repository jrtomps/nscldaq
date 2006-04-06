// Template for a test suite.
#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "parseUtilities.h"

#include "parseUtilities.h"
using namespace descriptionFile;


class GetLine : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(GetLine);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(noEol);
  CPPUNIT_TEST(endingEol);
  CPPUNIT_TEST(embeddedEol);
  CPPUNIT_TEST(onlyEol);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void emptyFile();
  void noEol();
  void endingEol();
  void embeddedEol();
  void onlyEol();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GetLine);


// An empty File should yield an empty string 
// and an stream.eof() true.

void GetLine::emptyFile() {
  istringstream   test(string(""));
  string result = getLine(test);
  EQMSG("return value", string(""), result);
  EQMSG("eof state:",   true, test.eof());

}
// The input 'file' does not have an eol but does
// have text... should get the text in one whack.
//
void GetLine::noEol()
{
  string contents = "This has no eol";
  istringstream test(contents);
  string result = getLine(test);
  EQMSG("first", contents, result);

  result = getLine(test);
  EQMSG("Empty", string(""), result);
  EQMSG("eof",   true, test.eof());

}
// The input 'file' ends in a single eol...should get that line
// without the trailng eol, and then o more
// 
void GetLine::endingEol()
{
  string line = "This will have an ending eol";
  string contents = line + string("\n");
  istringstream test(contents);
  
  string result = getLine(test);
  EQMSG("first", line, result);

  result  = getLine(test);
  EQMSG("second", string(""), result);
  EQMSG("eof", true, test.eof());

}
// Input file has an embedded eol should get two lines then eof.

void GetLine::embeddedEol()
{
  string line1 = "This is the first line";
  string line2 = "This is the last line";
  string eol   = "\n";
  string contents = line1 + eol + line2;
  istringstream test(contents);

  string result = getLine(test);
  EQMSG("first", line1, result);
  
  result = getLine(test);
  EQMSG("second", line2, result);

  result  = getLine(test);
  EQMSG("third", string(""), result);
  EQMSG("eof", true, test.eof());

}
// A file containing only an eol should
// give: An empty line with no eof state.
// another empty with eof state.

void GetLine::onlyEol()
{
  string contents="\n";
  istringstream test(contents);

  string result = getLine(test);
  EQMSG("first data", string(""), result);
  EQMSG("first eof", false, test.eof());

  result = getLine(test);
  EQMSG("Second data", string(""), result);
  EQMSG("second eof", true, test.eof());
}
