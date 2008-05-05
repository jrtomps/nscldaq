// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "grep.h"

#include <CLocalMonitoredProgram.h>
#include <CSink.h>
#include <CSinkFactory.h>
#include <CFileSink.h>

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fstream>

using namespace std;

class LocalMonitorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LocalMonitorTest);
  CPPUNIT_TEST(Construction);
  CPPUNIT_TEST(OutputTest);
  CPPUNIT_TEST(echotest);
  CPPUNIT_TEST(badtest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void Construction();
  void OutputTest();
  void echotest();
  void badtest();
};

static char* argv[]= {
  "echo",
  "this",
  "is",
  "a",
  "test"
};

static char* badargv[] = {
  "ehco",
  "this",
  "is",
  "a",
  "test"
};
static const int argc = sizeof(argv)/sizeof(char*);
static const string msg1("This is a test message");
static const string msg2("This is another test message");
static const string output("out.log");
static const string error("err.log");

static const int MAXLINE(1024);

CPPUNIT_TEST_SUITE_REGISTRATION(LocalMonitorTest);

static CLocalMonitoredProgram*
CreateProgram()
{
  CLocalMonitoredProgram* program = new CLocalMonitoredProgram(argc, argv);
  // Stdout:

  CSink* pOut = new CFileSink(argv[0], output);

  ASSERT(pOut);
  ASSERT(!program->AttachOutput(pOut));
  EQMSG("Reattach out", pOut, program->AttachOutput(pOut));

  // Stderr:

  CSink *pError = new CFileSink(argv[0], error);

  ASSERT(pError);
  ASSERT(!program->AttachError(pError));
  EQMSG("Reattach error", pError, program->AttachError(pError));

  return program;
}

static void
DestroyProgram(CMonitoredProgram& program)
{
  // Detach

  CSink* pOut = program.AttachOutput((CSink*)NULL);
  CSink* pError = program.AttachOutput((CSink*)NULL);

  // Delete

  delete &program;
  delete pOut;
  delete pError;
}

/*
Construct a CLocalMonitoredProgram
1.	Check Value of Getname
2.	Check value of GetNargs
3.	Check value of Getargs

 */
void LocalMonitorTest::Construction() {
  CLocalMonitoredProgram program(argc, argv);

  string echo("echo");
  string name(program.GetName());

  EQMSG("Name", echo, name);	// Check name...
  EQMSG("Nargs", argc, program.GetNargs()); // Check parameter count.

  char** pArgs = program.GetArgs();
  for(int i =0; i < argc; i++) {
    char msg[100];
    sprintf(msg, "Argument %d: ", i);
    string sb(argv[i]);
    string is(pArgs[i]);

    EQMSG(msg, sb, is);
  }
}
/*
Construct a CLocalMonitoredProgram
Attach a CFileSink to Stdout and Stderr
Invoke Stdout with a test message1
Invoke Stderr with a test mesage2
Destroy the CLocalMonitoredProgram
Destroy sinks.
1.	Ensure stdout file contains msg 1
2.	Ensure stderr file contains msg 2
 */
void LocalMonitorTest::OutputTest()
{
  CLocalMonitoredProgram& program(*CreateProgram());

  // Log:

  program.StdOut(msg1+"\n");
  program.StdErr(msg2+"\n");

  // Shutdown the stuff.

  DestroyProgram(program);


  // Examine the output log file.

  {
    ifstream fout(output.c_str());
    ASSERT(fout);
    char data[MAXLINE];
    fout.getline(data, MAXLINE);
    string pattern = msg1+"$";
    ASSERT(grep(string(data), pattern));
  } // Close the file too.
  

  // Examine the error log file.

  {
    ifstream ferr(error.c_str());
    ASSERT(ferr);
    char data[MAXLINE];
    ferr.getline(data, MAXLINE);
    string pattern = msg2+"$";
    ASSERT(grep(string(data), pattern));    
  }

  unlink(output.c_str());
  unlink(error.c_str());


}
/*
instantiate a CLocalMonitoredProgram:echo this is a test
Run it monitoring output and status
1.	Check final status.
2.	Check logfile contents.
 */
void
LocalMonitorTest::echotest()
{
  CMonitoredProgram* pProgram = CreateProgram();

  pProgram->Start();
  while(!pProgram->PollStatus()) {
    pProgram->PollIO(10);
  }
  pProgram->PollIO(10);

  // Check final status:

  int status = pProgram->GetFinalStatus();
  ASSERT(status == 0);

  DestroyProgram(*pProgram);

  // Examine the log file: 

  ifstream fout (output.c_str());
  ASSERT(fout);
  char data[MAXLINE];
  fout.getline(data, MAXLINE);
  string pattern;
  for(int i =1; i < argc; i++) {
    pattern += argv[i];
    pattern += ' ';
  }
  pattern[pattern.size()-1] = '$';
  ASSERT(grep(string(data), pattern));
 
  unlink(output.c_str());
  unlink(error.c_str());


  
}
/*
 */
void
LocalMonitorTest::badtest()
{
  CLocalMonitoredProgram program(argc, badargv);
  CSink* pOut  = new CFileSink(badargv[0], output);
  CSink* pErr  = new CFileSink(badargv[0], error);

  program.AttachOutput(pOut);
  program.AttachError(pErr);

  program.Start();

  while(!program.PollStatus()) {
    program.PollIO(10);
  }
  program.PollIO(10);

  delete pOut;
  delete pErr;

  // Ensure we have an Unable to run string
  // in the error log:

  ifstream errin(error.c_str());
  char msg[MAXLINE];
  errin.getline(msg, MAXLINE);

  ASSERT(grep(msg, "Unable to run"));
  
  // destroy the logs.
  
  unlink(error.c_str());
  unlink(output.c_str());

}
