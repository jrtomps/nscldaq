// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "grep.h"
#include <CSinkCreator.h>
#include <CFileSinkCreator.h>
#include <CFileSink.h>

#include <unistd.h>
#include <fstream>

using namespace std;

static const string Command("testing");
static const string Logfile("test.log");
static const string testmsg1("This is a test string\n");

static const int LINESIZE(1024);

class SinkCreatorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SinkCreatorTests);
  CPPUNIT_TEST(CreateTest);
  CPPUNIT_TEST(NameLegalTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void CreateTest();
  void NameLegalTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SinkCreatorTests);
/*

Make a CFileSinkCreator
Ask it to create a CFileSink.
Log a message to the object.
Destroy the sink1.	
Check existence of message.Destroy logfile

 */
void SinkCreatorTests::CreateTest() {
  CFileSinkCreator creator;
  CFileSink* pSink = (CFileSink*)creator.Create(Command, Logfile);

  pSink->Log(testmsg1);
  delete pSink;			//  Close the file.

  //  Ensure the string is in the file:

  ifstream in(Logfile.c_str());
  ASSERT(in);
  char line[LINESIZE];
  in.getline(line, LINESIZE);

  string pattern = testmsg1;
  pattern[pattern.size()-1] = '$'; // End match instead of \n
  
  ASSERT(grep(line, pattern));

  //


  unlink(Logfile.c_str());		// Destroy the file.

}
/*
Create a CFileSink
Creatorcheck a filename
1. Should be ok.
*/
void
SinkCreatorTests::NameLegalTest()
{
  CFileSinkCreator creator;

  ASSERT(creator.isNameLegal(Logfile));
}
