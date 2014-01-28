/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";
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
