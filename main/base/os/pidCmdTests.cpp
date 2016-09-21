// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <vector>

#include "Asserts.h"

#include <os.h>


class pidCmdTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(pidCmdTest);
  CPPUNIT_TEST(checkArgs);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void checkArgs();
};

CPPUNIT_TEST_SUITE_REGISTRATION(pidCmdTest);


// Fork off a sleep 2 and ensure that we can get its args:


void pidCmdTest::checkArgs() {
  pid_t pid = fork();
  std::vector<std::string> cmd;
  if (pid) {
    // parent
    int status;
    sleep(2);               // Give child a chance to execl.
    cmd = Os::getProcessCommand(pid);
    waitpid(pid, &status, 0);
    
  } else {
    // child
    
    execlp("sleep", "sleep", "3", 0);   // Two seconds should suffice.
    exit(0);             // exit to satisfy wait.
  }
  // child exited before we got here so we're in parent now:
  
  EQ(size_t(2), cmd.size());
  EQ(std::string("sleep"), cmd[0]);
  EQ(std::string("3"), cmd[1]);
  
}
