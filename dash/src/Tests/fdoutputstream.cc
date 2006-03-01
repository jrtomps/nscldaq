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
#include <signal.h>

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
class FdOutputStreamTest;
typedef CppUnit::TestCaller<FdOutputStreamTest> my_TestCaller_t; 

/**
* @class FdOutputStreamTest
* @brief FdOutputStreamTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the FdOutputStream class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdOutputStreamTest : public CppUnit::TestFixture  {
  public:
    // Set up this test
    void setUp() { 
      memset(readbuf,'\0',MYBUFSIZ);
      for (int i = 0; i < MYBUFSIZ; i++) {
        writebuf[i] = (ubyte)(i&0x00ff);
        ctrlbuf[i] = writebuf[i];
      }
    }

    // Tear down this test
    void tearDown() { }

    // Use the macros to decare the suite
    CPPUNIT_TEST_SUITE(FdOutputStreamTest);
      CPPUNIT_TEST(opencloseTest);
      CPPUNIT_TEST_EXCEPTION(reopenTestThrows,daqhwyapi::Exception);
      CPPUNIT_TEST_EXCEPTION(writeeofTestThrows,daqhwyapi::Exception);
      CPPUNIT_TEST(writereadBufTest);
      CPPUNIT_TEST(writereadBufOsetTest);
      CPPUNIT_TEST(writereadByteTest);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void opencloseTest() 
    * @brief Test for correct open/close operation.
    *
    * Test for correct open/close operation on FdOutputStreams.
    *
    * @param None
    * @return None
    */                                                             
    void opencloseTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("opencloseTest()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("opencloseTest()");

      FdOutputStream strm;
      CPPUNIT_ASSERT(strm.getFD() < 0); // Not open

      strm.open(pfd[1]);
      CPPUNIT_ASSERT(strm.getFD() >= 0); // Open

      FdOutputStream strm2(pfd2[1]);  // Construct and open
      CPPUNIT_ASSERT(strm2.getFD() >= 0); // Open

      strm.close();
      CPPUNIT_ASSERT(strm.getFD() < 0); // Not open

      strm2.close();
      CPPUNIT_ASSERT(strm2.getFD() < 0); // Not open

      ::close(pfd[0]); // Close the read end
      ::close(pfd2[0]); // Close the read end
      ::close(pfd[1]);
      ::close(pfd2[1]);
    }

    /*==============================================================*/
    /** @fn void reopenTestThrows() 
    * @brief Test that opening when already open throws an exception.
    *
    * Test that opening an already open FdOutputStream throws an
    * exception.
    *
    * @param None
    * @return None
    * @throw daqhwyapi::Exception If this test succeeds.
    */                                                             
    void reopenTestThrows() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("reopenTestThrows()");
      FdOutputStream strm;
      strm.open(pfd[1]);
      CPPUNIT_ASSERT(strm.getFD() >= 0); // Open
      strm.open(1); // Should throw an exception 
      strm.close();
      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writeeofTestThrows() 
    * @brief Test for exception when writing to a closed fd.
    *
    * Test for an IOException when writing when the read end has
    * closed its file descriptor.
    *
    * @param None
    * @return None
    */                                                             
    void writeeofTestThrows() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("opencloseTest()");

      FdOutputStream strm;
      strm.open(pfd[1]);
      ::close(pfd[0]); // Close the read end

      int rc = 0; 
      for (int i = 0; i < 10000; i++) {
        rc = strm.write(writebuf,MYBUFSIZ); 
        if (rc < 0) break;
        strm.flush();  // Just in case
      } 

      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writereadBufOsetTest() 
    * @brief Test for correct write and read operation.
    *
    * Test for correct write and read operation on FdOutputStreams
    * when reading with an offset.
    *
    * @param None
    * @return None
    */                                                             
    void writereadBufOsetTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadBufOsetTest()");

      FdOutputStream strm;
      strm.open(pfd[1]);

      int chunksiz = MYBUFSIZ / 10;
      int wbsiz = chunksiz * 10;
      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < wbsiz) {
          for (int j = 0; j < 10; j++) {
            int wrc = strm.write(writebuf,cnt,chunksiz);

            CPPUNIT_ASSERT(wrc > 0);

            strm.flush(); // Flush the write buffer
            int rrc = ::read(pfd[0],readbuf+cnt,wrc);

            CPPUNIT_ASSERT(rrc >= 0); // Read some data w/o error
            CPPUNIT_ASSERT(rrc == wrc); // Read as much as we wrote

            cnt += wrc;
          }

          // Read what we wrote
          CPPUNIT_ASSERT(memcmp(ctrlbuf,readbuf,wbsiz) == 0); 
        }
      }

      strm.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writereadBufTest() 
    * @brief Test for correct write and read operation.
    *
    * Test for correct write and read operation on FdOutputStreams.
    *
    * @param None
    * @return None
    */                                                             
    void writereadBufTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadBufTest()");

      FdOutputStream strm;
      strm.open(pfd[1]);

      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < MYBUFSIZ) {
          int wrc = strm.write(writebuf,cnt,MYBUFSIZ-cnt);

          CPPUNIT_ASSERT(wrc > 0);

          strm.flush(); // Flush the write buffer
          int rrc = ::read(pfd[0],readbuf+cnt,wrc);

          CPPUNIT_ASSERT(rrc >= 0); // Read some data w/o error
          CPPUNIT_ASSERT(rrc == wrc); // Read as much as we wrote

          cnt += wrc;
        }

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,readbuf,MYBUFSIZ) == 0); 
      }

      strm.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writereadByteTest() 
    * @brief Test for correct write and read operation.
    *
    * Test for correct write and read operation on FdOutputStreams
    * when reading a byte at a time.
    *
    * @param None
    * @return None
    */                                                             
    void writereadByteTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadByteTest()");

      FdOutputStream strm;
      strm.open(pfd[1]);

      int chunksiz = MYBUFSIZ / 10;
      int wbsiz = chunksiz * 10;
      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < wbsiz) {
          for (int j = 0; j < 10; j++) {
            for (int k = 0; k < chunksiz; k++) { 
              int wrc = strm.write((int)writebuf[cnt+k]);
              CPPUNIT_ASSERT(wrc == 1);
            }

            strm.flush(); // Flush the write buffer
            int rcnt = 0;
            while (rcnt < chunksiz) {
              int rrc = ::read(pfd[0],readbuf+cnt+rcnt,chunksiz-rcnt);
              CPPUNIT_ASSERT(rrc > 0);
              rcnt += rrc;
            }

            cnt += chunksiz;
          }
        }
        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,readbuf,wbsiz) == 0); 
      }

      strm.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

  private:
    ubyte writebuf[MYBUFSIZ];
    ubyte readbuf[MYBUFSIZ];
    ubyte ctrlbuf[MYBUFSIZ];
};

// Get the singleton registry
static CppUnit::TestFactoryRegistry &test_registry = CppUnit::TestFactoryRegistry::getRegistry();

// Register the test with the registry
CPPUNIT_TEST_SUITE_REGISTRATION(FdOutputStreamTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class FDOSTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"FdOutputStream ----------------------------\n");
    ::signal(SIGPIPE,SIG_IGN);

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

FDOSTest fdos;
