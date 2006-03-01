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
class BufferedRecordWriterTest;
typedef CppUnit::TestCaller<BufferedRecordWriterTest> my_TestCaller_t; 

/**
* @class BufferedRecordWriterTest
* @brief BufferedRecordWriterTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the BufferedRecordWriter class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedRecordWriterTest : public CppUnit::TestFixture  {
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
    CPPUNIT_TEST_SUITE(BufferedRecordWriterTest);
      CPPUNIT_TEST(initTest);
      CPPUNIT_TEST(writeTest);
      CPPUNIT_TEST(write2Test);
      CPPUNIT_TEST(write3Test);
      CPPUNIT_TEST(writeextTest);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void initTest() 
    * @brief Test for correct initialization with an FdOutputStream.
    *
    * Test for correct initialization with an FdOutputStream.
    *
    * @param None
    * @return None
    */                                                             
    void initTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("initTest()");

      FdOutputStream fos(pfd[1]);
      BufferedRecordWriter writer(fos);

      CPPUNIT_ASSERT(writer.getFD() >= 0); // Open

      writer.close();
      CPPUNIT_ASSERT(writer.getFD() < 0); // Not open

      ::close(pfd[0]);
      ::close(pfd[1]);
    }


    /*==============================================================*/
    /** @fn void writeTest() 
    * @brief Test the writeRecord() method.
    *
    * Test the writeRecord() method.
    *
    * @param None
    * @return None
    */                                                             
    void writeTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writeTest()");

      FdOutputStream fos(pfd[1]);
      BufferedRecordWriter writer(fos);
      FILE *fp = fdopen(pfd[0],"r");

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
    

        CPPUNIT_ASSERT(writer.write(writebuf,i) == i);
        CPPUNIT_ASSERT(writer.writeRecord(test_record) == ctrl_record.record_size);
        writer.flush();
      
        CPPUNIT_ASSERT(::fread(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ByteArray rdata(i);
        CPPUNIT_ASSERT(::fread(rdata.elements,1,i,fp) == i); 

        Record::initHeader(tgt_record);
        Record::decodeHeader(hdrbuf,Record::encode_buffer_size,tgt_record);

        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements,i) == 0); 
      }

      writer.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void write2Test() 
    * @brief Test writes that use an offset.
    *
    * Test the partial writes on a BufferedRecordWriter.  That is,
    * use the write() method that writes using an offset and length.
    *
    * @param None
    * @return None
    */                                                             
    void write2Test() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("write2Test()");

      FdOutputStream fos(pfd[1]);
      BufferedRecordWriter writer(fos);
      FILE *fp = fdopen(pfd[0],"r");

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
    
        // Write the buffer in parts
        int oset = 0;
        int l = i;
        while (l > 0) { 
          int wcnt = ((i/10) > 0) ? (i/10) : 1;
          wcnt = (l >= wcnt) ? wcnt : l;
          int rc = 0;
          CPPUNIT_ASSERT((rc = writer.write(writebuf,oset,wcnt)) == wcnt);
          l -= rc;
          oset += rc;
        }

        CPPUNIT_ASSERT(writer.writeRecord(test_record) == ctrl_record.record_size);
        writer.flush();
      
        CPPUNIT_ASSERT(::fread(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ByteArray rdata(i);
        CPPUNIT_ASSERT(::fread(rdata.elements,1,i,fp) == i); 

        Record::initHeader(tgt_record);
        Record::decodeHeader(hdrbuf,Record::encode_buffer_size,tgt_record);

        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements,i) == 0); 
      }

      writer.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void write3Test() 
    * @brief Test writes that write one byte at a time.
    *
    * Test the the write() method that writes one byte at a time
    *
    * @param None
    * @return None
    */                                                             
    void write3Test() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("write3Test()");

      FdOutputStream fos(pfd[1]);
      BufferedRecordWriter writer(fos);
      FILE *fp = fdopen(pfd[0],"r");

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
    
        // Write the buffer in parts
        int oset = 0;
        int l = i;
        while (l > 0) { 
          int rc = 0;
          CPPUNIT_ASSERT((rc = writer.write((int)(writebuf[oset]))) == 1);
          l -= rc;
          oset += rc;
        }

        CPPUNIT_ASSERT(writer.writeRecord(test_record) == ctrl_record.record_size);
        writer.flush();
      
        CPPUNIT_ASSERT(::fread(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ByteArray rdata(i);
        CPPUNIT_ASSERT(::fread(rdata.elements,1,i,fp) == i); 

        Record::initHeader(tgt_record);
        Record::decodeHeader(hdrbuf,Record::encode_buffer_size,tgt_record);

        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements,i) == 0); 
      }

      writer.close();

      ::close(pfd[0]);
      ::close(pfd[1]);
    }

    /*==============================================================*/
    /** @fn void writeextTest() 
    * @brief Test the writeRecord() method with extended headers.
    *
    * Test the writeRecord() method with extended headers.
    *
    * @param None
    * @return None
    */                                                             
    void writeextTest() {
      int pfd[2];
      if (::pipe(pfd) < 0) perror("writeextTest()");

      FdOutputStream fos(pfd[1]);
      BufferedRecordWriter writer(fos);
      FILE *fp = fdopen(pfd[0],"r");

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

        CPPUNIT_ASSERT(writer.write(exthdr,extlen) == extlen);
        CPPUNIT_ASSERT(writer.write(writebuf,i) == i);
        CPPUNIT_ASSERT(writer.writeRecord(test_record) == ctrl_record.record_size);
        writer.flush();
      
        CPPUNIT_ASSERT(::fread(hdrbuf,1,Record::encode_buffer_size,fp) == Record::encode_buffer_size); 
        ByteArray rdata(i+extlen);
        CPPUNIT_ASSERT(::fread(rdata.elements,1,(i+extlen),fp) == (i+extlen)); 

        Record::initHeader(tgt_record);
        Record::decodeHeader(hdrbuf,Record::encode_buffer_size,tgt_record);
    
        // Should get back what we wrote
        CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
        CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
        CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
        CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
        CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
        CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);

        // Read what we wrote
        CPPUNIT_ASSERT(memcmp(exthdr,rdata.elements,extlen) == 0); 
        CPPUNIT_ASSERT(memcmp(ctrlbuf,rdata.elements+extlen,i) == 0); 
        extlen++; 
      }

      writer.close();

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
CPPUNIT_TEST_SUITE_REGISTRATION(BufferedRecordWriterTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class BRRTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"BufferedRecordWriter ----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

BRRTest brr;
