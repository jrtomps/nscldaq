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
#include <vector>
#include <CPhysicsEventItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

#include "CFilter.h"
#include "CFatalException.h"


#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "CFilterMain.h"
#undef private
#undef protected

// A test suite 
class CFilterMainTest : public CppUnit::TestFixture
{


  public:
    CPPUNIT_TEST_SUITE( CFilterMainTest );
    CPPUNIT_TEST ( testBadSourceFail );
    CPPUNIT_TEST ( testBadSinkFail );
    CPPUNIT_TEST ( testSkipTransmitted );
    CPPUNIT_TEST ( testCountTransmitted );
    CPPUNIT_TEST ( testOneShot );

    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testBadSourceFail();
    void testNoSourceFail();
    void testBadSinkFail();
    void testSkipTransmitted();
    void testCountTransmitted();

    void testSetMembers();
    void testOneShot();

  private:
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFilterMainTest );

void CFilterMainTest::setUp()
{}

void CFilterMainTest::tearDown()
{}

void CFilterMainTest::testBadSourceFail()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--source=badproto://nofile"};

  // Ensure that this thing only throws a CFatalException
  CPPUNIT_ASSERT_THROW( CFilterMain app(argc,
                                        const_cast<char**>(argv)), 
      CFatalException ); 

}

void CFilterMainTest::testNoSourceFail()
{
  int argc = 3;
  const char* argv[] = {"Main",
                      "--sample=PHYSICS_EVENT",
                      "--sink=file://./dummy"};

  // Ensure that this thing only throws a CFatalException
  CPPUNIT_ASSERT_THROW( CFilterMain app(argc,
                                        const_cast<char**>(argv)), 
      CFatalException ); 

}
void CFilterMainTest::testBadSinkFail()
{
  int argc = 3;
  const char* argv[] = {"Main",
                      "--source=-",
                      "--sink=badproto://nofile"};
  // Ensure that this thing only throws a CFatalException
  CPPUNIT_ASSERT_THROW( CFilterMain app(argc, 
                                        const_cast<char**>(argv)), 
      CFatalException ); 

}

void CFilterMainTest::testSkipTransmitted()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--skip=5"};
  // Ensure that this thing only throws a CFatalException
  CFilterMain app(argc, const_cast<char**>(argv)); 
  CPPUNIT_ASSERT_EQUAL(5, app.m_mediator.m_nToSkip);
}

void CFilterMainTest::testCountTransmitted()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--count=5"};
  // Ensure that this thing only throws a CFatalException
  CFilterMain app(argc, const_cast<char**>(argv)); 
  CPPUNIT_ASSERT_EQUAL(5, app.m_mediator.m_nToProcess);
}

void CFilterMainTest::testOneShot()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--oneshot"};
  // Ensure that this thing only throws a CFatalException
  CFilterMain app(argc, const_cast<char**>(argv)); 
  CPPUNIT_ASSERT_EQUAL(true, app.m_mediator.m_isOneShot);
}

