//
//  This is a cppunit test for spectcldaq.  
#include <config.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>




#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  pid_t  pid;
public:
  void setUp() {
  }
  void tearDown() {
    kill(pid, 9);		// Kill spectcldaq.
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

//
// - Start up spectcldaq (in the current directory)
//   on the other end of a pipe.
// - system() the injector.
// - Read the pipe:
// Should have:
//    Exactly 8192 bytes.
//    Wordwise counting pattern.
//
void Testname::aTest() {

  // Make the pipe we'll read and spectcldaq writes:

  int pipefds[2];
  ASSERT(pipe(pipefds) == 0);
  
  int readfd  = pipefds[0];
  int writefd = pipefds[1];

  // Fork off spectcldaq:

  pid = fork();
  ASSERT(pid >= 0);

  if(pid == 0) { 		// Child process
    // Dup the writefd -> stdout, so spectcldaq writes to the pipe.

    dup2(writefd, fileno(stdout));
    execl("./spectcldaq", "tcp://localhost:2602", NULL);
    
  }
  else {			// parent process.
    //  Inject the data:

    sleep(1);			// SpecTcldaq needs to start up.
    int stat = system("./injector");
    ASSERT(stat >= 0);

    // Now the pipe should have our data:

    short buffer[4096];
    ssize_t nTotal= 0;
    char*   pBuffer = (char*)buffer;
    ssize_t residual = sizeof(buffer);

    while(residual) {
      int nRead = read(readfd, pBuffer, residual);
      if(nRead <= 0) break;
      pBuffer  += nRead;
      residual -= nRead;
    }
    ASSERT(residual == 0);


    for(int i=0; i < 4096; i++) {
      EQ((short)i, buffer[i]);		// Must be a counting pattern.
    }
  }
}
