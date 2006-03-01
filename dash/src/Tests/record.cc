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

/*===================================================================*/
// Make use of cppunit for testing if available.
#ifdef CPPUNIT
class RecordTest;
typedef CppUnit::TestCaller<RecordTest> my_TestCaller_t; 

/**
* @class RecordTest
* @brief RecordTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the Record class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RecordTest : public CppUnit::TestFixture  {
  public:
    // Set up this test
    void setUp() { 
      Record::initHeader(ctrl_record);
      Record::initHeader(test_record);
      Record::initHeader(tgt_record);
    }

    // Tear down this test
    void tearDown() { }

    // Use the macros to decare the suite
    CPPUNIT_TEST_SUITE(RecordTest);
      CPPUNIT_TEST(initTest);
      CPPUNIT_TEST(encodeTest);
      CPPUNIT_TEST(decodeTest);
      CPPUNIT_TEST(copyTest);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void initTest() 
    * @brief Test record header initialization.
    *
    * Test if the Record::initHeader() method correctly
    * initializes the header.
    *
    * @param None
    * @return None
    */                                                             
    void initTest() {
      Record::initHeader(ctrl_record);
      CPPUNIT_ASSERT(ctrl_record.version == Record::current_version);
      CPPUNIT_ASSERT(ctrl_record.record_size == Record::encode_buffer_size);
      CPPUNIT_ASSERT(ctrl_record.record_type == Record::type_physics);
      CPPUNIT_ASSERT(ctrl_record.status_code == Record::status_ok);
      CPPUNIT_ASSERT(ctrl_record.byte_order == 0x01020304);
      CPPUNIT_ASSERT(ctrl_record.extended_header_size == 0);
      CPPUNIT_ASSERT(ctrl_record.data_size == 0);
    }

    /*==============================================================*/
    /** @fn void copyTest() 
    * @brief Test if record header copying works.
    *
    * Test if record header copying works correctly.
    *
    * @param None
    * @return None
    */                                                             
    void copyTest() {
      Record::initHeader(test_record);
      Record::initHeader(tgt_record);

      test_record.record_size = 12345;
      test_record.record_type = Record::type_physics;
      test_record.status_code = Record::status_trunc;
      test_record.extended_header_size = 513;
      test_record.data_size = 12345 - 513 - Record::encode_buffer_size;
     
      ctrl_record.version = test_record.version;
      ctrl_record.record_size = test_record.record_size;
      ctrl_record.record_type = test_record.record_type;
      ctrl_record.status_code = test_record.status_code;
      ctrl_record.extended_header_size = test_record.extended_header_size;
      ctrl_record.data_size = test_record.data_size;

      Record::copyHeader(tgt_record,test_record);

      // Encoding should not change values in header struct
      CPPUNIT_ASSERT(tgt_record.version == ctrl_record.version);
      CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
      CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
      CPPUNIT_ASSERT(tgt_record.byte_order == ctrl_record.byte_order);
      CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
      CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
      CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);
    }

    /*==============================================================*/
    /** @fn void encodeTest() 
    * @brief Test if record header encoding works.
    *
    * Test if record header encoding work correctly.
    *
    * @param None
    * @return None
    */                                                             
    void encodeTest() {
      ubyte buf[Record::encode_buffer_size * 2]; 
      int sml = Record::encode_buffer_size - 2;
      int lrg = Record::encode_buffer_size + 2;
      Record::initHeader(test_record);

      test_record.record_size = 12345;
      test_record.record_type = Record::type_physics;
      test_record.status_code = Record::status_trunc;
      test_record.extended_header_size = 513;
      test_record.data_size = 12345 - 513 - Record::encode_buffer_size;
     
      ctrl_record.version = test_record.version;
      ctrl_record.record_size = test_record.record_size;
      ctrl_record.record_type = test_record.record_type;
      ctrl_record.status_code = test_record.status_code;
      ctrl_record.extended_header_size = test_record.extended_header_size;
      ctrl_record.data_size = test_record.data_size;

      // Buffer too small test (should return < 0)
      CPPUNIT_ASSERT(Record::encodeHeader(buf,sml,test_record) < 0);

      // Buffer too large test (should be ok)
      CPPUNIT_ASSERT(Record::encodeHeader(buf,lrg,test_record) == Record::encode_buffer_size);

      // Buffer just right (should be ok);
      CPPUNIT_ASSERT(Record::encodeHeader(buf,Record::encode_buffer_size,test_record) == Record::encode_buffer_size);

      // Encoding should not change values in header struct
      CPPUNIT_ASSERT(test_record.version == ctrl_record.version);
      CPPUNIT_ASSERT(test_record.record_size == ctrl_record.record_size);
      CPPUNIT_ASSERT(test_record.record_type == ctrl_record.record_type);
      CPPUNIT_ASSERT(test_record.byte_order == ctrl_record.byte_order);
      CPPUNIT_ASSERT(test_record.status_code == ctrl_record.status_code);
      CPPUNIT_ASSERT(test_record.extended_header_size == ctrl_record.extended_header_size);
      CPPUNIT_ASSERT(test_record.data_size == ctrl_record.data_size);
    }

    /*==============================================================*/
    /** @fn void decodeTest() 
    * @brief Test if record header decoding works.
    *
    * Test if record header decoding works.
    *
    * @param None
    * @return None
    */                                                             
    void decodeTest() {
      ubyte buf[Record::encode_buffer_size * 2]; 
      int sml = Record::encode_buffer_size - 2;
      int lrg = Record::encode_buffer_size + 2;

      Record::initHeader(test_record);
      Record::initHeader(ctrl_record);
      Record::initHeader(tgt_record);

      test_record.record_size = 12345;
      test_record.record_type = 101;
      test_record.status_code = Record::status_trunc;
      test_record.extended_header_size = 513;
      test_record.data_size = 12345 - 513 - Record::encode_buffer_size;

      ctrl_record.version = test_record.version;
      ctrl_record.record_size = test_record.record_size;
      ctrl_record.record_type = test_record.record_type;
      ctrl_record.status_code = test_record.status_code;
      ctrl_record.extended_header_size = test_record.extended_header_size;
      ctrl_record.data_size = test_record.data_size;
    
      // Encode the test header 
      CPPUNIT_ASSERT(Record::encodeHeader(buf,Record::encode_buffer_size,test_record) == Record::encode_buffer_size);

      // Buffer too small test (should return < 0)
      CPPUNIT_ASSERT(Record::decodeHeader(buf,sml,tgt_record) < 0);

      // Buffer too large test (should be ok)
      CPPUNIT_ASSERT(Record::decodeHeader(buf,lrg,tgt_record) == Record::encode_buffer_size);

      // Buffer just right (should be ok);
      Record::initHeader(tgt_record);
      CPPUNIT_ASSERT(Record::decodeHeader(buf,Record::encode_buffer_size,tgt_record) == Record::encode_buffer_size);

      // Byte order should neither be encoded nor decoded.
      record_header_t *hdr = (record_header_t*)buf; 
      CPPUNIT_ASSERT(ctrl_record.byte_order == hdr->byte_order);
      CPPUNIT_ASSERT(tgt_record.byte_order == hdr->byte_order);

      // Value tests
      CPPUNIT_ASSERT(tgt_record.version == ctrl_record.version);
      CPPUNIT_ASSERT(tgt_record.record_size == ctrl_record.record_size);
      CPPUNIT_ASSERT(tgt_record.record_type == ctrl_record.record_type);
      CPPUNIT_ASSERT(tgt_record.status_code == ctrl_record.status_code);
      CPPUNIT_ASSERT(tgt_record.extended_header_size == ctrl_record.extended_header_size);
      CPPUNIT_ASSERT(tgt_record.data_size == ctrl_record.data_size);
    }

  private:
    record_header_t ctrl_record;
    record_header_t test_record;
    record_header_t tgt_record;
};

// Get the singleton registry
static CppUnit::TestFactoryRegistry &test_registry = CppUnit::TestFactoryRegistry::getRegistry();

// Register the test with the registry
CPPUNIT_TEST_SUITE_REGISTRATION(RecordTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class RecTest : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"There are %d arguments\n",argc);
    fprintf(stdout,"The current milliseconds is: %llu\n",System.currentTimeMillis());
    fprintf(stdout,"Record ----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

RecTest rrec;
