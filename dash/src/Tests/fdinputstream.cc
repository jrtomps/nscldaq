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
class FdInputStreamTest;
typedef CppUnit::TestCaller<FdInputStreamTest> my_TestCaller_t; 

/**
* @class FdInputStreamTest
* @brief FdInputStreamTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the FdInputStream class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdInputStreamTest : public CppUnit::TestFixture  {
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
    CPPUNIT_TEST_SUITE(FdInputStreamTest);
      CPPUNIT_TEST(opencloseTest);
      CPPUNIT_TEST_EXCEPTION(reopenTestThrows,daqhwyapi::Exception);
      CPPUNIT_TEST(readyavailTest);
      CPPUNIT_TEST(skipTest);
      CPPUNIT_TEST(writereadBufTest);
      CPPUNIT_TEST(writereadBufOsetTest);
      CPPUNIT_TEST(writereadByteTest);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void opencloseTest() 
    * @brief Test for correct open/close operation.
    *
    * Test for correct open/close operation on FdInputStreams.
    *
    * @param None
    * @return None
    */                                                             
    void opencloseTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("opencloseTest()");
      int pfd2[2];
      if (::pipe(pfd2) < 0) perror("opencloseTest()");

      FdInputStream strm;
      CPPUNIT_ASSERT(strm.getFD() < 0); // Not open

      strm.open(pfd[0]);
      CPPUNIT_ASSERT(strm.getFD() >= 0); // Open
      CPPUNIT_ASSERT(strm.eof() == false); // And not at eof

      FdInputStream strm2(pfd2[0]);  // Construct and open
      CPPUNIT_ASSERT(strm2.getFD() >= 0); // Open
      CPPUNIT_ASSERT(strm2.eof() == false); // And not at eof

      strm.close();
      CPPUNIT_ASSERT(strm.getFD() < 0); // Not open
      strm2.close();
      CPPUNIT_ASSERT(strm2.getFD() < 0); // Not open

      ::close(pfd[0]);
      ::close(pfd[1]);
      ::close(pfd2[0]);
      ::close(pfd2[1]);
    }

    /*==============================================================*/
    /** @fn void reopenTestThrows() 
    * @brief Test that opening when already open throws an exception.
    *
    * Test that opening an already open FdInputStream throws an
    * exception.
    *
    * @param None
    * @return None
    * @throw daqhwyapi::Exception If this test succeeds.
    */                                                             
    void reopenTestThrows() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("reopenTestThrows()");
      FdInputStream strm;
      strm.open(pfd[0]);
      CPPUNIT_ASSERT(strm.getFD() >= 0); // Open
      strm.open(0); // Should throw an exception 
      strm.close();
      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void readyavailTest() 
    * @brief Test the ready() and available() methods.
    *
    * Test the ready() and available() methods.
    *
    * @param None
    * @return None
    */                                                             
    void readyavailTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("readyavailTest()");
      FdInputStream strm(pfd[0]);  // Construct and open
      FILE *fp = fdopen(pfd[1],"w");

      CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
      CPPUNIT_ASSERT(strm.ready() == false); // Not ready
      CPPUNIT_ASSERT(strm.available() == 0); // No data yet
      
      CPPUNIT_ASSERT(::fwrite(writebuf,1,256,fp) == 256); 
      ::fflush(fp);

      CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
      CPPUNIT_ASSERT(strm.ready() == true); // Should be ready
      CPPUNIT_ASSERT(strm.available() > 0); // Should have data

      strm.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writereadBufOsetTest() 
    * @brief Test for correct write and read operation.
    *
    * Test for correct write and read operation on FdInputStreams
    * when reading with an offset.
    *
    * @param None
    * @return None
    */                                                             
    void writereadBufOsetTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadBufOsetTest()");

      FdInputStream strm;
      strm.open(pfd[0]);

      int chunksiz = MYBUFSIZ / 10;
      int wbsiz = chunksiz * 10;
      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < wbsiz) {
          for (int j = 0; j < 10; j++) {
            int wrc = ::write(pfd[1],writebuf+cnt,chunksiz);

            CPPUNIT_ASSERT(wrc > 0);
            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == true); // Ready
            CPPUNIT_ASSERT(strm.available() > 0); // Data available

            int rrc = strm.read(readbuf,cnt,wrc);

            CPPUNIT_ASSERT(rrc >= 0); // Read some data w/o error
            CPPUNIT_ASSERT(rrc == wrc); // Read as much as we wrote
            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == false); // Not ready
            CPPUNIT_ASSERT(strm.available() == 0); // Nothing available

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
    * Test for correct write and read operation on FdInputStreams.
    *
    * @param None
    * @return None
    */                                                             
    void writereadBufTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadBufTest()");

      FdInputStream strm;
      strm.open(pfd[0]);

      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < MYBUFSIZ) {
          int wrc = ::write(pfd[1],writebuf+cnt,MYBUFSIZ-cnt);

          CPPUNIT_ASSERT(wrc > 0);
          CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
          CPPUNIT_ASSERT(strm.ready() == true); // Ready
          CPPUNIT_ASSERT(strm.available() > 0); // Data available

          int rrc = strm.read(readbuf,cnt,wrc);

          CPPUNIT_ASSERT(rrc >= 0); // Read some data w/o error
          CPPUNIT_ASSERT(rrc == wrc); // Read as much as we wrote
          CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
          CPPUNIT_ASSERT(strm.ready() == false); // Not ready
          CPPUNIT_ASSERT(strm.available() == 0); // Nothing available

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
    * Test for correct write and read operation on FdInputStreams
    * when reading a byte at a time.
    *
    * @param None
    * @return None
    */                                                             
    void writereadByteTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writereadByteTest()");

      FdInputStream strm;
      strm.open(pfd[0]);

      int chunksiz = MYBUFSIZ / 10;
      int wbsiz = chunksiz * 10;
      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < wbsiz) {
          for (int j = 0; j < 10; j++) {
            int wrc = ::write(pfd[1],writebuf+cnt,chunksiz);

            CPPUNIT_ASSERT(wrc == chunksiz);
            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == true); // Ready
            CPPUNIT_ASSERT(strm.available() > 0); // Data available

            for (int k = 0; k < wrc; k++) { 
              CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
              CPPUNIT_ASSERT(strm.ready() == true); // Ready
              CPPUNIT_ASSERT(strm.available() > 0); // Data available
              readbuf[cnt+k] = (ubyte)(strm.read());
            }

            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == false); // Not ready
            CPPUNIT_ASSERT(strm.available() == 0); // Nothing available

            cnt += wrc;
          }
        }
        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,readbuf,wbsiz) == 0); 
      }

      strm.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void skipTest() 
    * @brief Test for correct skip operation.
    *
    * Test for correct skip operation on FdInputStreams.
    *
    * @param None
    * @return None
    */                                                             
    void skipTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("skipTest()");

      FdInputStream strm;
      strm.open(pfd[0]);

      int chunksiz = MYBUFSIZ / 10;
      int wbsiz = chunksiz * 10;
      for (int i = 0; i < 1000; i++) {
        int cnt = 0;
        memset(readbuf,'\0',MYBUFSIZ);
        while (cnt < wbsiz) {
          for (int j = 0; j < 10; j++) {
            int wrc = ::write(pfd[1],writebuf+cnt,chunksiz);

            CPPUNIT_ASSERT(wrc == chunksiz);
            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == true); // Ready
            CPPUNIT_ASSERT(strm.available() > 0); // Data available

            int rrc = -1;
            if ((j%2) == 0) rrc = strm.read(readbuf,wrc);
            else rrc = strm.skip(wrc);

            CPPUNIT_ASSERT(rrc > 0); // Read/skip some data w/o error
            CPPUNIT_ASSERT(rrc == wrc); // Read/skip as much as we wrote
            CPPUNIT_ASSERT(strm.eof() == false); // Not at eof
            CPPUNIT_ASSERT(strm.ready() == false); // Not ready
            CPPUNIT_ASSERT(strm.available() == 0); // Nothing available

            // Read what we wrote
            if ((j%2) == 0) {
              CPPUNIT_ASSERT(memcmp(ctrlbuf+cnt,readbuf,wrc) == 0); 
            }

            cnt += wrc;
          }
        }
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
CPPUNIT_TEST_SUITE_REGISTRATION(FdInputStreamTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class FDISTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"FdInputStream ----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

FDISTest fdis;
