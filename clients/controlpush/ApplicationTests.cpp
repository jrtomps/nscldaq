// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CChannel.h>
#include <CNoUnitsException.h>
#include <CAllOk.h>
#include <CSGBuilder.h>
#include <CSingleUpdater.h>
#include <CLookupVisitor.h>
#include <CBuildChannelData.h>
#include <CChannelList.h>
#include <CApplication.h>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class ApplicationTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ApplicationTests);
  CPPUNIT_TEST(PortValidation);	// CApplication::ValidatePort test.
  CPPUNIT_TEST(IntervalValidation); // CApplication;:ValidateInterval.
  CPPUNIT_TEST(SetTest);	// CApplication::GenerateSet.
  CPPUNIT_TEST(ReadTest);	// Read channels -> m_Channels.
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void PortValidation();
  void IntervalValidation();
  void SetTest();
  void ReadTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ApplicationTests);
//
// Ports are valid in the range of 0 - 65535 inclusive...
// 1. Test 0.         (should be ok).
// 2. Test 65535      (should be ok).
// 3. Test -1         (should fail).
// 4. Test 65536      (should fail).
void

ApplicationTests::PortValidation() 
{
  bool good = true;

  CApplication app;

  // Try the endpoints of the range (should be inclusive..)

  try {
    app.ValidatePort(0);
  }
  catch (string msg) {
    good = false;
    cerr << " Failure message was: " << msg << endl;
  }
  ASSERT(good);
  good = true;
  try {
    app.ValidatePort(65535);
  }
  catch (string msg) {
    good = false;
    cerr << "Failure message was: " << msg << endl;
  }
  ASSERT(good);

  // now try the 'first' bad values outside the range.

  good = false;
  try {
    app.ValidatePort(-1);
  }
  catch (string msg) {
    good = true;
    cerr << "OK! message thrown was: " << msg << endl;
  }
  ASSERT(good);

  good = false;
  try {
    app.ValidatePort(65536);
  }
  catch (string msg) {
    good = true;
    cerr << "OK! message thrown was: " << msg << endl;
  }
  ASSERT(good);
}
// Validate the interval paramteer... it just must be > MININTERVAL which
// at this instant is 5.0.
void
ApplicationTests::IntervalValidation()
{
  bool good=true;
  CApplication app;

  try {
    app.ValidateInterval(5);
    good=  true;
  }
  catch (string msg) {
    cerr << "FAIL! message: " << msg << endl;
    good=false;
  }
  ASSERT(good);

  try {
    app.ValidateInterval(4);
    good = false;
  }
  catch (string msg) {
    cerr << "OK! Message caught: " << msg << endl;
    good = true;
  }
  ASSERT(good);

}
//
// Test simple sets and sets that need escapig.
//
void
ApplicationTests::SetTest()
{
  string name("EPICS_DATA");
  string index("Test");
  string easyvalue("a b c d e");
  string hardvalue("a\" b $c [d]");

  // Try the easy value we should get out of this:
  //   set EPICS_DATA(Test) "a b c d e"

  CApplication app;
  string answer = app.GenerateSet(name, index, easyvalue);
  EQ(string("set EPICS_DATA(Test) \"a b c d e\"\n"), answer);

  // Try the hard valud.. we should get:
  //    set EPICS_DATA(Test) "a\" b \$c \[d]"

  answer = app.GenerateSet(name, index, hardvalue);
  EQ (string("set EPICS_DATA(Test) \"a\\\" b \\$c \\[d]\"\n"), answer);
}
//  After comments and blank lines, the file Channels.dat has the
// names shown below:

const string Names[]={
  "Z001DV", "Z002DH",
  "STRCHAN50", "STRCHAN40",
  "FLTCHAN59", "FLTCHAN67",
  "FLTCHAN70","P#3:42502#f"};
const int nNames = (sizeof(Names)/sizeof(string));

void
ApplicationTests::ReadTest()
{
  try {
    ifstream infile("Channels.dat");
    CApplication app;
    
    app.ReadChannelFile(infile);

    EQ(nNames, app.m_Channels.size());
    
    CChannelList::ChannelIterator p = app.m_Channels.begin();
    int i=0;
    while(p != app.m_Channels.end()) {
      EQ(Names[i], (*p)->GetName());
      
      i++;
      p++;
    }
  } catch (string msg) {
    cerr << "String exception: " << msg << endl;
  }
  catch(char * msg) {
    cerr << " Char* exception: " << msg << endl;
  }
  catch(CException e) {
    cerr << " CException caught: " << e.ReasonText() << endl; 
  }

}
