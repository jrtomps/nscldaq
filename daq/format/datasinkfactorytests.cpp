#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include <URIFormatException.h>
#include <string>
#include <fcntl.h>
#include <errno.h>

#define private public

#include "CDataSinkFactory.h"
#include <CFileDataSink.h>

// A test suite 
class CDataSinkFactoryTest : public CppUnit::TestFixture
{
    public:
    CDataSinkFactoryTest();

    CPPUNIT_TEST_SUITE( CDataSinkFactoryTest );
    CPPUNIT_TEST ( testStdout );
    CPPUNIT_TEST ( testStdoutDash );
    CPPUNIT_TEST ( testFailOnStdin );
    CPPUNIT_TEST ( testRingSink );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testStdout();
    void testStdoutDash();
    void testFailOnStdin();
    void testRingSink();

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
  CFileDataSink* sink = dynamic_cast<CFileDataSink*>(factory.makeSink("-"));
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
    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
    CPPUNIT_FAIL(errmsg.str().c_str());
  } catch (CURIFormatException& exc) {
    CPPUNIT_FAIL(exc.ReasonText());
  }

  if (sink!=0) { delete sink; }
}

void CDataSinkFactoryTest::testRingSink()
{
  CDataSinkFactory factory;
  CDataSink* sink;
  try {
    sink = factory.makeSink("ring://localhost/myring");
  } catch (int err) {
    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
    CPPUNIT_FAIL( errmsg.str().c_str() );
  } catch (CURIFormatException& exc) {
    CPPUNIT_FAIL( exc.ReasonText() );
  }


  if (sink!=0) { delete sink; }
}
