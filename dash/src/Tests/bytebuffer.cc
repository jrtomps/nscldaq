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
class ByteBufferTest;
typedef CppUnit::TestCaller<ByteBufferTest> my_TestCaller_t; 

/**
* @class ByteBufferTest
* @brief ByteBufferTest CPPUnit test fixture.
*
* This class implements several methods for using CPPUnit to
* unit test the functionality of the ByteBuffer.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class ByteBufferTest : public CppUnit::TestFixture  {
  public:
    // Set up this test
    void setUp() { 
      bytelen = sizeof(short) * 1024;
      indat = new short[1024];
      outdat = new short[1024*2];
      for (int i = 0; i < 1024; i++) indat[i] = i;
      memset((void*)outdat,'\0',bytelen);
    }

    // Tear down this test
    void tearDown() { 
      if (indat != NULL) delete[] indat;
      indat = NULL;
      if (outdat != NULL) delete[] outdat;
      outdat = NULL;
    }

    // Use the macros to decare the suite
    CPPUNIT_TEST_SUITE(ByteBufferTest);
      CPPUNIT_TEST_EXCEPTION(limitTestThrows,daqhwyapi::Exception);
      CPPUNIT_TEST(truncateTest);
      CPPUNIT_TEST(consumeTest);
      CPPUNIT_TEST(relativePutGet);
    CPPUNIT_TEST_SUITE_END();

    /*==============================================================*/
    /** @fn void limitTestThrows() 
    * @brief Test ByteBuffer limit controls.
    *
    * Check if the ByteBuffer put operator will throw an exception
    * if a put that exceeds the buffer limit is executed.
    *
    * @param None
    * @return None
    * @throws daqhwyapi::Exception If this test functions correctly.
    */                                                             
    void limitTestThrows() {
      ByteBuffer buf;
      buf.limit(200);
      buf.put((ubyte*)indat,bytelen); // Should throw an exception 
    }

    /*==============================================================*/
    /** @fn void truncateTest() 
    * @brief Test if the truncation method works.
    *
    * Test if the ByteBuffer truncate method correctly truncates
    * the buffer.
    *
    * @param None
    * @return None
    */                                                             
    void truncateTest() {
      ByteBuffer buf;
      buf.limit(bytelen*2);
      int l = bytelen * 2;
      buf.put((ubyte*)indat,bytelen); 
      CPPUNIT_ASSERT(buf.capacity() == bytelen);
      CPPUNIT_ASSERT(buf.position() == bytelen);
      buf.position(20);
      buf.truncate(256);
      CPPUNIT_ASSERT(buf.capacity() == 256);
      CPPUNIT_ASSERT(buf.position() == 20);
      buf.rewind();
      l = bytelen;
      ubyte *dat = buf.array((ubyte*)outdat,l);
      CPPUNIT_ASSERT(l == 256);
      CPPUNIT_ASSERT(memcmp((void*)indat,(void*)dat,256) == 0);
    }

    /*==============================================================*/
    /** @fn void consumeTest() 
    * @brief Test if the consume method works.
    *
    * Test if the ByteBuffer consume method correctly consumes
    * bytes from the beginning of the buffer.
    *
    * @param None
    * @return None
    */                                                             
    void consumeTest() {
      ByteBuffer buf;
      buf.limit(bytelen*2);
      int l = bytelen * 2;
      buf.put((ubyte*)indat,bytelen); 
      CPPUNIT_ASSERT(buf.capacity() == bytelen);
      CPPUNIT_ASSERT(buf.position() == bytelen);
      buf.position(50);
      CPPUNIT_ASSERT(buf.position() == 50);
      buf.consume(256);
      CPPUNIT_ASSERT(buf.capacity() == (bytelen-256));
      CPPUNIT_ASSERT(buf.position() == 0);
      buf.rewind();
      l = bytelen;
      ubyte *dat = buf.array((ubyte*)outdat,l);
      ubyte *p = (ubyte*)indat;
      p += 256;
      CPPUNIT_ASSERT(l == (bytelen-256));
      CPPUNIT_ASSERT(memcmp((void*)p,(void*)dat,(bytelen-256)) == 0);
    }

    /*==============================================================*/
    /** @fn void relativePutGet() 
    * @brief Test ByteBuffer relative put/get methods.
    *
    * Test ByteBuffer relative put/get methods.  That is, make
    * we get out what we put into the buffer correctly.
    *
    * @param None
    * @return None
    */                                                             
    void relativePutGet() {
      ByteBuffer buf;
      buf.limit(bytelen*2);
      int l = bytelen * 2;
      buf.put((ubyte*)indat,bytelen); 
      ubyte *dat = buf.array((ubyte*)outdat,l);
      CPPUNIT_ASSERT(l == bytelen);
      CPPUNIT_ASSERT(memcmp((void*)indat,(void*)dat,bytelen) == 0);
      CPPUNIT_ASSERT(memcmp((void*)indat,(void*)outdat,bytelen) == 0);
      CPPUNIT_ASSERT(buf.capacity() == bytelen);

      memset((void*)outdat,'\0',bytelen);
      buf.rewind();
      CPPUNIT_ASSERT(buf.position() == 0);

      l = 20;
      dat = buf.get((ubyte*)outdat,l);
      CPPUNIT_ASSERT(l == 20);
      CPPUNIT_ASSERT(memcmp((void*)indat,(void*)dat,20) == 0);
      CPPUNIT_ASSERT(buf.capacity() == bytelen);
      CPPUNIT_ASSERT(memcmp((void*)indat,(void*)dat,20) == 0);
      CPPUNIT_ASSERT(buf.position() == 20);
    }

  private:
    short *indat;
    short *outdat;
    int bytelen;
};

// Get the singleton registry
static CppUnit::TestFactoryRegistry &test_registry = CppUnit::TestFactoryRegistry::getRegistry();

// Register the test with the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ByteBufferTest);

#endif // CPPUNIT

/*===================================================================*/
// Main line
class Bbuffers : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stdout,"There are %d arguments\n",argc);
    fprintf(stdout,"The current milliseconds is: %llu\n",System.currentTimeMillis());
    fprintf(stdout,"ByteBuffer----------------------------\n");

#ifdef CPPUNIT
    // Run cppunit tests if available
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(test_registry.makeTest());
    runner.run();
#endif
  }
};

Bbuffers bybuffers;
