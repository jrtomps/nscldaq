#ifndef CDataSinkFactoryTEST_H
#define CDataSinkFactoryTEST_H

#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include <CFileDataSink.h>
#include <URIFormatException.h>
#include <string>
#include <fcntl.h>
#include <errno.h>

#define private public

#include "CDataSinkFactory.h"


// A test suite 
class CDataSinkFactoryTest : public CppUnit::TestFixture
{
    public:
    CDataSinkFactoryTest();

    CPPUNIT_TEST_SUITE( CDataSinkFactoryTest );
    CPPUNIT_TEST ( testStdout );
    CPPUNIT_TEST ( testStdoutDash );
    CPPUNIT_TEST ( testFailOnStdin );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testStdout();
    void testStdoutDash();
    void testFailOnStdin();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CDataSinkFactoryTest );

CDataSinkFactoryTest::CDataSinkFactoryTest()
{}


void CDataSinkFactoryTest::setUp()
{}

void CDataSinkFactoryTest::tearDown()
{}

void CDataSinkFactoryTest::testStdout()
{
  CDataSinkFactory factory;
  CFileDataSink* sink = 
      dynamic_cast<CFileDataSink*>(factory.makeSink("file:///stdout"));
  CPPUNIT_ASSERT ( 0 != sink );
  CPPUNIT_ASSERT_EQUAL( STDOUT_FILENO, sink->m_fd );

  delete sink;
}

void CDataSinkFactoryTest::testStdoutDash()
{
  CDataSinkFactory factory;
  CFileDataSink* sink = 
      dynamic_cast<CFileDataSink*>(factory.makeSink("-"));
  CPPUNIT_ASSERT ( 0 != sink );
  CPPUNIT_ASSERT_EQUAL( STDOUT_FILENO, sink->m_fd );
  delete sink;
}

void CDataSinkFactoryTest::testFailOnStdin()
{
  CDataSinkFactory factory;
  CDataSink* sink;
  try {
    sink = factory.makeSink("file:///stdin");
  } catch (int err) {
    std::cout << "errno = " << err << std::endl;
  } catch (CURIFormatException& exc) {
    std::cout << exc.ReasonText() << std::endl;
  }

  if (sink!=0) { delete sink; }
}
