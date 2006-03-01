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

#define MYBUFSIZ 2048

/*===================================================================*/
// Make use of cppunit for testing if available.
#ifdef CPPUNIT
class BufferedRecordReaderTest;
typedef CppUnit::TestCaller<BufferedRecordReaderTest> my_TestCaller_t; 

/**
* @class BufferedRecordReaderTest
* @brief BufferedRecordReaderTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the BufferedRecordReader class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedRecordReaderTest : public CppUnit::TestFixture  {
  public:
    // Set up this test
    void setUp() { 
      memset(readbuf,'\0',MYBUFSIZ);
      for (int i = 0; i < MYBUFSIZ; i++) {
        writebuf[i] = (ubyte)(i&0x00ff);
        ctrlbuf[i] = writebuf[i];
      }
      Record::initHeader(ctrl_record);
      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
    }

    // Tear down this test
    void tearDown() { }

    // Use the macros to decare the suite
    CPPUNIT_TEST_SUITE(BufferedRecordReaderTest);
      CPPUNIT_TEST(initTest);
      CPPUNIT_TEST(readyTest);
      CPPUNIT_TEST(skipTest);
      CPPUNIT_TEST(readTest);
      CPPUNIT_TEST(readextTest);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void initTest() 
    * @brief Test for correct initialization with an FdInputStream.
    *
    * Test for correct initialization with an FdInputStream.
    *
    * @param None
    * @return None
    */                                                             
    void initTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("initTest()");

      FdInputStream fis(pfd[0]);
      BufferedRecordReader reader(fis);

      CPPUNIT_ASSERT(reader.getFD() >= 0); // Open
      CPPUNIT_ASSERT(reader.eof() == false); // And not at eof

      reader.close();
      CPPUNIT_ASSERT(reader.getFD() < 0); // Not open

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void readyTest() 
    * @brief Test the ready() method.
    *
    * Test the ready() method.
    *
    * @param None
    * @return None
    */                                                             
    void readyTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("readyTest()");

      FdInputStream fis(pfd[0]);
      BufferedRecordReader reader(fis);
      FILE *fp = fdopen(pfd[1],"w");

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // Not ready

      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
      ubyte hdrbuf[Record::encode_buffer_size];

      test_record.record_size = Record::encode_buffer_size + 256;
      test_record.record_type = 101;
      test_record.status_code = Record::status_ok;
      test_record.extended_header_size = 0;
      test_record.data_size = 256;
     
      ctrl_record.record_size = test_record.record_size;
      ctrl_record.record_type = test_record.record_type;
      ctrl_record.status_code = test_record.status_code;
      ctrl_record.extended_header_size = test_record.extended_header_size;
      ctrl_record.data_size = test_record.data_size;
    
      Record::encodeHeader(hdrbuf,Record::encode_buffer_size,test_record);
       
      CPPUNIT_ASSERT(::fwrite(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
      ::fflush(fp);
      CPPUNIT_ASSERT(::fwrite(writebuf,1,256,fp) == 256); 
      ::fflush(fp);

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == true); // Should be ready

      ByteArray rdata;
      int rc = reader.readRecord(tgt_record,rdata);

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // No longer ready
      CPPUNIT_ASSERT(rc == (Record::encode_buffer_size+256)); 
     
      // Should get back what we wrote
      CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
      CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
      CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
      CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
      CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
      CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);
      CPPUNIT_ASSERT(rdata.length == 256);

      // Read what we wrote
      CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements,256) == 0); 

      reader.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void skipTest() 
    * @brief Test for correct skip operation.
    *
    * Test for correct skip operation on BufferedRecordReaders.
    *
    * @param None
    * @return None
    */                                                             
    void skipTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("skipTest()");

      FdInputStream fis(pfd[0]);
      BufferedRecordReader reader(fis);
      FILE *fp = fdopen(pfd[1],"w");

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // Not ready

      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
      ubyte hdrbuf[Record::encode_buffer_size];

      test_record.record_size = Record::encode_buffer_size + 256;
      test_record.record_type = 5;
      test_record.status_code = Record::status_ok;
      test_record.extended_header_size = 0;
      test_record.data_size = 256;
     
      Record::encodeHeader(hdrbuf,Record::encode_buffer_size,test_record);
       
      CPPUNIT_ASSERT(::fwrite(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
      ::fflush(fp);
      CPPUNIT_ASSERT(::fwrite(writebuf,1,256,fp) == 256); 
      ::fflush(fp);

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == true); // Should be ready

      int rc = reader.skip();

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // No longer ready
      CPPUNIT_ASSERT(rc == (Record::encode_buffer_size+256)); 
     
      reader.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void readTest() 
    * @brief Test the readRecord() method.
    *
    * Test the readRecord() method.
    *
    * @param None
    * @return None
    */                                                             
    void readTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("readTest()");

      FdInputStream fis(pfd[0]);
      BufferedRecordReader reader(fis);
      FILE *fp = fdopen(pfd[1],"w");

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // Not ready

      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
      ubyte hdrbuf[Record::encode_buffer_size];

      for (int i = 0; i < 1024; i++) {
        test_record.record_size = Record::encode_buffer_size + i;
        test_record.record_type = i;
        test_record.status_code = Record::status_ok;
        test_record.extended_header_size = 0;
        test_record.data_size = i;
     
        ctrl_record.record_size = test_record.record_size;
        ctrl_record.record_type = test_record.record_type;
        ctrl_record.status_code = test_record.status_code;
        ctrl_record.extended_header_size = test_record.extended_header_size;
        ctrl_record.data_size = test_record.data_size;
    
        Record::encodeHeader(hdrbuf,Record::encode_buffer_size,test_record);
       
        CPPUNIT_ASSERT(::fwrite(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ::fflush(fp);
        CPPUNIT_ASSERT(::fwrite(writebuf,1,i,fp) == i); 
        ::fflush(fp);

        CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
        CPPUNIT_ASSERT(reader.ready() == true); // Should be ready

        ByteArray rdata;
        Record::initHeader(tgt_record);
        int rc = reader.readRecord(tgt_record,rdata);

        CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
        CPPUNIT_ASSERT(reader.ready() == false); // No longer ready
        CPPUNIT_ASSERT(rc == (Record::encode_buffer_size+i)); 
     
        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);
        CPPUNIT_ASSERT(rdata.length == i);

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements,i) == 0); 
      }

      reader.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void readextTest() 
    * @brief Test the readRecord() method with extended headers.
    *
    * Test the readRecord() method with extended headers.
    *
    * @param None
    * @return None
    */                                                             
    void readextTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("readextTest()");

      FdInputStream fis(pfd[0]);
      BufferedRecordReader reader(fis);
      FILE *fp = fdopen(pfd[1],"w");

      CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
      CPPUNIT_ASSERT(reader.ready() == false); // Not ready

      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
      ubyte hdrbuf[Record::encode_buffer_size];
      ubyte exthdr[512];
      for (int i = 0; i < 512; i++) exthdr[i] = (ubyte)i;

      int extlen = 0;
      for (int i = 0; i < 1024; i += 2) {
        test_record.record_size = Record::encode_buffer_size + i + extlen;
        test_record.record_type = i;
        test_record.status_code = Record::status_ok;
        test_record.extended_header_size = extlen;
        test_record.data_size = i;
 
        ctrl_record.record_size = test_record.record_size;
        ctrl_record.record_type = test_record.record_type;
        ctrl_record.status_code = test_record.status_code;
        ctrl_record.extended_header_size = test_record.extended_header_size;
        ctrl_record.data_size = test_record.data_size;
    
        Record::encodeHeader(hdrbuf,Record::encode_buffer_size,test_record);
       
        CPPUNIT_ASSERT(::fwrite(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ::fflush(fp);
        CPPUNIT_ASSERT(::fwrite(exthdr,1,extlen,fp) == extlen); 
        ::fflush(fp);
        CPPUNIT_ASSERT(::fwrite(writebuf,1,i,fp) == i); 
        ::fflush(fp);

        CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
        reader.ready(); // Helps move data along the pipe 
        CPPUNIT_ASSERT(reader.ready() == true); // Should be ready

        ByteArray rdata;
        Record::initHeader(tgt_record);
        int rc = reader.readRecord(tgt_record,rdata);

        CPPUNIT_ASSERT(reader.eof() == false); // Not at eof
        CPPUNIT_ASSERT(reader.ready() == false); // No longer ready
        CPPUNIT_ASSERT(rc == (Record::encode_buffer_size+i+extlen)); 
     
        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);
        CPPUNIT_ASSERT(rdata.length == (i+extlen));

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(exthdr,rdata.elements,extlen) == 0); 
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements+extlen,i) == 0); 
        extlen++; 
      }

      reader.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

  private:
    record_header_t test_record; 
    record_header_t ctrl_record; 
    record_header_t tgt_record; 
    ubyte writebuf[MYBUFSIZ];
    ubyte readbuf[MYBUFSIZ];
    ubyte ctrlbuf[MYBUFSIZ];
};

// Get the singleton registry
static CppUnit::TestFactoryRegistry &test_registry = CppUnit::TestFactoryRegistry::getRegistry();

// Register the test with the registry
CPPUNIT_TEST_SUITE_REGISTRATION(BufferedRecordReaderTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class BRRTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"BufferedRecordReader ----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

BRRTest brr;
