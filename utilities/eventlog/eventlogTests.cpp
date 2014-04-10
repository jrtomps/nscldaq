// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <CRingBuffer.h>
#include <CRingStateChangeItem.h>
#include <string>
#include <iostream>

extern std::string uniqueName(std::string);

class EvlogTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EvlogTest);
  CPPUNIT_TEST(autorun);
  CPPUNIT_TEST(overriderun);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* pRing;
public:
  void setUp() {
    pRing = CRingBuffer::createAndProduce(uniqueName("evlog"));

  }
  void tearDown() {
    delete pRing;
    CRingBuffer::remove(uniqueName("evlog"));
  }
private: 
  pid_t startEventLog(std::string switches);
protected:
  void autorun();
  void overriderun();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EvlogTest);

/**
 * start event log - assuming it's in this directory.
 * @param tail - the command tail after the program invocation
 * @note cheat - use fork/system.
 */
pid_t
EvlogTest::startEventLog(std::string tail)
{
  pid_t pid = fork();
  if (!pid) {
    std::string command("./eventlog ");
    command += tail;
    system(command.c_str());
    exit(EXIT_SUCCESS);
  }
  sleep(2);			// too lazy to check for started file.

  return pid;
}



/**
 * test autorun file.
 * - Start event log with: --one-shot
 * - Send a begin run to the ring.
 * - Send an end run to the ring.
 * - Ensure there's an event file with the right name.
 * - unlink the event file.
 */
void EvlogTest::autorun() {
  std::string uri="tcp://localhost/";
  uri += uniqueName("evlog");
  std::string switches = ("--oneshot --source=");
  switches += uri;
  pid_t evlogPid = startEventLog(switches);


  // Create the run.

  CRingStateChangeItem begin(BEGIN_RUN, 123, 0, time(NULL), "This is a title");
  CRingStateChangeItem end(END_RUN, 123, 1, time(NULL), "This is a title");

  begin.commitToRing(*pRing);
  end.commitToRing(*pRing);

  // wait for eventlog to finish.

  int status;
  waitpid(evlogPid, &status, 0);

  // Check for the event file.

  const char* pFilename = "run-0123-00.evt";
  status = access(pFilename, F_OK);
  EQ(0, status);

  unlink(pFilename);
    

}
/**
 * test --run switch
 */
void
EvlogTest::overriderun()
{
  std::string uri="tcp://localhost/";
  uri += uniqueName("evlog");
  std::string switches = ("--oneshot --run=5 --source=");
  switches += uri;
  pid_t evlogPid = startEventLog(switches);

  // Create the run.

  CRingStateChangeItem begin(BEGIN_RUN, 123, 0, time(NULL), "This is a title");
  CRingStateChangeItem end(END_RUN, 123, 1, time(NULL), "This is a title");

  begin.commitToRing(*pRing);
  end.commitToRing(*pRing);

  // wait for eventlog to finish.

  int status;
  waitpid(evlogPid, &status, 0);

  // Check for the event file.

  const char* pFilename = "run-0005-00.evt";
  status = access(pFilename, F_OK);
  EQ(0, status);

  unlink(pFilename);
    
}
