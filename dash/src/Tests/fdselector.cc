/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef CPPUNIT
#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestResult.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#endif

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif

using namespace daqhwyapi;

#define MYBUFSIZ 1024

/*===================================================================*/
// Make use of cppunit for testing if available.
#ifdef CPPUNIT
class FdSelectorTest;
typedef CppUnit::TestCaller<FdSelectorTest> my_TestCaller_t; 

/**
* @class FdSelectorTest
* @brief FdSelectorTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the FdSelector class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdSelectorTest : public CppUnit::TestFixture  {
  public:
    // Set up this test
    void setUp() { 
      memset(readbuf,'\0',MYBUFSIZ);
      for (int i = 0; i < MYBUFSIZ; i++) {
        writebuf[i] = (ubyte)(i&0x00ff);
      }
    }

    // Tear down this test
    void tearDown() { }

    // Use the macros to decare the suite
    CPPUNIT_TEST_SUITE(FdSelectorTest);
      CPPUNIT_TEST(addremoveTest);
      CPPUNIT_TEST(pollTest);
      CPPUNIT_TEST(select1Test);
      CPPUNIT_TEST(select2Test);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void addremoveTest() 
    * @brief Test for correct add and remove operation.
    *
    * Test for correct adding and removing of file descriptors to
    * an FdSelector.
    *
    * @param None
    * @return None
    */                                                             
    void addremoveTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("addremoveTest()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("addremoveTest()");

      FdSelector fdsel;
      fdsel.addReadFd(pfd[0]);
      fdsel.addReadFd(pfd2[0]);
 
      CPPUNIT_ASSERT(fdsel.poll() == 0); // Nothing yet
      CPPUNIT_ASSERT(::write(pfd2[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event

      CPPUNIT_ASSERT(fdsel.isReadable(pfd2[0]) == true); // is readable
      CPPUNIT_ASSERT(fdsel.isWritable(pfd2[0]) == false); // not writable
      CPPUNIT_ASSERT(fdsel.hasException(pfd2[0]) == false); // not an exception

      CPPUNIT_ASSERT(::read(pfd2[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 0); // No events

      CPPUNIT_ASSERT(fdsel.isReadable(pfd2[0]) == false); // is readable
      CPPUNIT_ASSERT(fdsel.isWritable(pfd2[0]) == false); // not writable
      CPPUNIT_ASSERT(fdsel.hasException(pfd2[0]) == false); // not an exception

      fdsel.removeReadFd(pfd2[0]);
      CPPUNIT_ASSERT(::write(pfd2[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 0); // No events
      CPPUNIT_ASSERT(::read(pfd2[0],readbuf,100) == 100);

      CPPUNIT_ASSERT(::write(pfd[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event

      CPPUNIT_ASSERT(fdsel.isReadable(pfd[0]) == true); // is readable
      CPPUNIT_ASSERT(fdsel.isWritable(pfd[0]) == false); // not writable
      CPPUNIT_ASSERT(fdsel.hasException(pfd[0]) == false); // not an exception

      CPPUNIT_ASSERT(fdsel.isReadable(pfd2[0]) == false); // not readable
      CPPUNIT_ASSERT(fdsel.isWritable(pfd2[0]) == false); // not writable
      CPPUNIT_ASSERT(fdsel.hasException(pfd2[0]) == false); // not an exception

      CPPUNIT_ASSERT(::read(pfd[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 0); // No events

      fdsel.removeReadFd(pfd[0]);

      ::close(pfd[0]);
      ::close(pfd[1]);
      ::close(pfd2[0]);
      ::close(pfd2[1]);
    }

    /*==============================================================*/
    /** @fn void pollTest() 
    * @brief Test for correct poll() operation.
    *
    * Test for correct poll() operation.
    *
    * @param None
    * @return None
    */                                                             
    void pollTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("pollTest()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("pollTest()");

      FdSelector fdsel;
      fdsel.addReadFd(pfd[0]);
      fdsel.addReadFd(pfd2[0]);
 
      CPPUNIT_ASSERT(fdsel.poll() == 0); // Nothing yet
      CPPUNIT_ASSERT(::write(pfd2[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event

      CPPUNIT_ASSERT(::write(pfd[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 2); // Two events

      CPPUNIT_ASSERT(::read(pfd[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event

      CPPUNIT_ASSERT(::read(pfd2[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 0); // No events

      fdsel.removeReadFd(pfd[0]);
      fdsel.removeReadFd(pfd2[0]);

      ::close(pfd[0]);
      ::close(pfd[1]);
      ::close(pfd2[0]);
      ::close(pfd2[1]);
    }

    /*==============================================================*/
    /** @fn void select2Test() 
    * @brief Test for correct select() operation.
    *
    * Test for correct select() operation.
    *
    * @param None
    * @return None
    */                                                             
    void select2Test() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("select2Test()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("select2Test()");

      FdSelector fdsel;
      fdsel.addReadFd(pfd[0]);
      fdsel.addReadFd(pfd2[0]);
 
      CPPUNIT_ASSERT(fdsel.poll() == 0); // Nothing yet
      CPPUNIT_ASSERT(::write(pfd2[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event
      CPPUNIT_ASSERT(fdsel.select() == 1); // One event

      CPPUNIT_ASSERT(::write(pfd[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 2); // Two events
      CPPUNIT_ASSERT(fdsel.select() == 2); // Two events

      CPPUNIT_ASSERT(::read(pfd[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.poll() == 1); // One event
      CPPUNIT_ASSERT(fdsel.select() == 1); // One event

      CPPUNIT_ASSERT(::read(pfd2[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.select(0) == 0); // No events

      fdsel.removeReadFd(pfd[0]);
      fdsel.removeReadFd(pfd2[0]);

      ::close(pfd[0]);
      ::close(pfd[1]);
      ::close(pfd2[0]);
      ::close(pfd2[1]);
    }

    /*==============================================================*/
    /** @fn void select1Test() 
    * @brief Test for correct select(int) operation.
    *
    * Test for correct select(int) operation.
    *
    * @param None
    * @return None
    */                                                             
    void select1Test() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("select1Test()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("select1Test()");

      FdSelector fdsel;
      fdsel.addReadFd(pfd[0]);
      fdsel.addReadFd(pfd2[0]);
 
      CPPUNIT_ASSERT(fdsel.select(2) == 0); // Nothing yet
      CPPUNIT_ASSERT(::write(pfd2[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.select(2) == 1); // One event

      CPPUNIT_ASSERT(::write(pfd[1],writebuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.select(2) == 2); // Two events

      CPPUNIT_ASSERT(::read(pfd[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.select(2) == 1); // One event

      CPPUNIT_ASSERT(::read(pfd2[0],readbuf,100) == 100);
      CPPUNIT_ASSERT(fdsel.select(2) == 0); // No events

      fdsel.removeReadFd(pfd[0]);
      fdsel.removeReadFd(pfd2[0]);

      ::close(pfd[0]);
      ::close(pfd[1]);
      ::close(pfd2[0]);
      ::close(pfd2[1]);
    }


  private:
    ubyte writebuf[MYBUFSIZ];
    ubyte readbuf[MYBUFSIZ];
};

// Get the singleton registry
static CppUnit::TestFactoryRegistry &test_registry = CppUnit::TestFactoryRegistry::getRegistry();

// Register the test with the registry
CPPUNIT_TEST_SUITE_REGISTRATION(FdSelectorTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class FDSelTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"FdSelector ----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

FDSelTest fdsel;
