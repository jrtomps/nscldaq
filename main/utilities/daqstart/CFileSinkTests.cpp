// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "grep.h"

#include "CFileSink.h"

#include <unistd.h>

#include <fstream>


using namespace std;

static const string Command("testing");
static const string LogName("testlog.log");
static const string EmptyLog("empty.log");

static const string testmsg1("This is a test message\n");
static const string testmsg2("Seond test messge\n");

static const streamsize MAXLINE(1024);

class FileSinkTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FileSinkTests);
  CPPUNIT_TEST(LogSink);
  CPPUNIT_TEST(CopyCompare);
  CPPUNIT_TEST(AppendTest);
  CPPUNIT_TEST(AssignTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CFileSink* m_pSink;

public:
  void setUp() {
    m_pSink = new CFileSink(Command, LogName);
  }
  void tearDown() {
    delete m_pSink;
    m_pSink = (CFileSink*)NULL;
    unlink(LogName.c_str());
}
  FileSinkTests() :
    m_pSink(0) {}

protected:
  void LogSink();
  void CopyCompare();
  void AppendTest();
  void AssignTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FileSinkTests);


static void
Check2Messages()
{
  // Check that both lines made it to the same file:

  
  ifstream log(LogName.c_str());
  ASSERT(log);

  char input[MAXLINE];
  log.getline(input, MAXLINE);
  string pattern = testmsg1;
  pattern[pattern.size()-1] = '$';
  ASSERT(grep(input, pattern));	// First string..

  log.getline(input, MAXLINE);
  pattern = testmsg2;
  pattern[pattern.size()-1] = '$';
  ASSERT(grep(input, pattern));	// Second string.

}

/*!
  Construct a file sink.  Log a message.  Destroy the sink.  
  Open the file and read it...ensure the message is the trailing 
  substring of the file.
  Teardown - destroy the file.
 */
void FileSinkTests::LogSink() {
  m_pSink->Log(testmsg1);
  delete m_pSink;
  m_pSink = (CFileSink*)NULL;	// Ensure teardown won't fail.

  // Read the file:

  ifstream log(LogName.c_str());
  ASSERT(log);

  char input[MAXLINE];

  log.getline(input, MAXLINE);

  string pattern = testmsg1;
  pattern[pattern.size()-1] = '$'; // End match instead of \n

  ASSERT(grep(input, pattern));

  
}
/*
Create a Sink.
Log a message
Clone the sink
1.	Test for equality between cloned and original sink.
Log message to cloned sink.
Destroy both sinks.
2. Check that both messages are in the log file.


 */
void 
FileSinkTests::CopyCompare()
{
  m_pSink->Log(testmsg1);

  CFileSink* pSink = (CFileSink*)m_pSink->clone(); // Functional duplicate...

  ASSERT(*m_pSink == *pSink);	// Should be equal...
  ASSERT(!(*m_pSink != *pSink)); // Should not be unequal.

  pSink->Log(testmsg2);		// Should log to same file.
  delete pSink;
  delete m_pSink;
  m_pSink = (CFileSink*)NULL;	// File closd by everyone..

  Check2Messages();
}
/*
Create a sink.  
Log a messageDestroy sink, 
create sink on same file
Log a messageCheck both messages in file.
 */
void
FileSinkTests::AppendTest()
{
  m_pSink->Log(testmsg1);
  delete m_pSink;
  m_pSink = new CFileSink(LogName, LogName); // Same file diff command.
  m_pSink->Log(testmsg2);
  delete m_pSink;
  m_pSink = (CFileSink*)NULL;

  Check2Messages();
}
/*
Create a sink, 
log a message
Create a sink on  a different file
Assign first sink to second sink
log on 'second' sink1. 
Messages should both be in same file
*/
void
FileSinkTests::AssignTest()
{
  m_pSink->Log(testmsg1);
  {
    CFileSink Sink("junk", EmptyLog);
    Sink = *m_pSink; 		// Now sink is same...
    ASSERT(Sink == *m_pSink);
    ASSERT(! (Sink != *m_pSink));
    
    Sink.Log(testmsg2);
  } // Sink gone.
  delete m_pSink;
  m_pSink = (CFileSink*)NULL;
  unlink(EmptyLog.c_str());

  Check2Messages();
  

  
}
