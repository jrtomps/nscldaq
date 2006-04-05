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



class FirstWord : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FirstWord);
  CPPUNIT_TEST(emptyString);
  CPPUNIT_TEST(leadingBlanks);
  CPPUNIT_TEST(singleWord);
  CPPUNIT_TEST(trailingSpace);
  CPPUNIT_TEST(doubleWord);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void emptyString();
  void leadingBlanks();
  void singleWord();
  void trailingSpace();
  void doubleWord();
};

 CPPUNIT_TEST_SUITE_REGISTRATION(FirstWord);


// Empty STrings yield an empty first word.

void FirstWord::emptyString() {
    string result = firstWord(string(""));
     EQ(string(""), result);
}
// Strings with leading whitespace of any sort
// yeild empty first words.

void FirstWord::leadingBlanks()
{
  string test1="   word1 (space)";
  string test2="\tWord1 (tab)";
  string result;

  result = firstWord(test1);
  EQMSG(test1, string(""), result);

  result = firstWord(test2);
  EQMSG(test2, string(""), result);
}
// Strings with only non-whitespace match the entire string.

void FirstWord::singleWord()
{
  string test = "oneword";
  string result = firstWord(test);
  EQ(test, result);
}

// Single word but trailing whitespace:

void FirstWord::trailingSpace()
{
  string test   = "oneword ";
  string result = firstWord(test);
  EQ(string("oneword"), result);
}
// Test string with two words

void FirstWord::doubleWord()
{
  string test1    = "word1  second";
  string test2    = "first\tsecond";
  string result;

  result = firstWord(test1);
  EQMSG("spaces", string("word1"), result);
  
  result = firstWord(test2);
  EQMSG("tabs", string("first"), result);
}
