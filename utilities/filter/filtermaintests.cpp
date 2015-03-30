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
#include <CRingBuffer.h>

#include "CFilter.h"
#include "CFatalException.h"

#include <unistd.h>
#include <wait.h>
#include <signal.h>

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "CFilterMain.h"
#include "COneShotMediator.h"
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
    CPPUNIT_TEST ( testMultiProducersOnRingIsFatal );

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
    void testMultiProducersOnRingIsFatal();

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
  CPPUNIT_ASSERT_EQUAL(5, app.m_mediator->m_nToSkip);
}

void CFilterMainTest::testCountTransmitted()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--count=5"};
  // Ensure that this thing only throws a CFatalException
  CFilterMain app(argc, const_cast<char**>(argv)); 
  CPPUNIT_ASSERT_EQUAL(5, app.m_mediator->m_nToProcess);
}

void CFilterMainTest::testOneShot()
{
  int argc = 2;
  const char* argv[] = {"Main",
                      "--oneshot"};
  // Ensure that this thing only throws a CFatalException
  CFilterMain app(argc, const_cast<char**>(argv)); 
  CPPUNIT_ASSERT(0 != &(dynamic_cast<COneShotMediator*>(app.m_mediator)->m_oneShot));
}

void CFilterMainTest::testMultiProducersOnRingIsFatal()
{
  std::cout << "\ntestMultiProducersOnRingIsFatal" << std::endl;
   CRingBuffer *ring = 0;
   std::string rname = "test";
   if (CRingBuffer::isRing(rname)) {
    ring = new CRingBuffer(rname,CRingBuffer::producer);
  } else {
    ring = CRingBuffer::createAndProduce(rname);
  }
   CPPUNIT_ASSERT(ring != 0);

   int argc = 2;
   std::string rsink="--sink=ring://localhost/";
   rsink += rname;
   const char* argv[] = {"Main", rsink.c_str() };

   // fork this to make a new process

   pid_t pid = fork();
   // if the pid is 0 then it is the child
   if (! pid) {
     // child process
     CPPUNIT_ASSERT_THROW( CFilterMain app(argc,const_cast<char**>(argv)),
         CFatalException );
   } else {
     // Parent process

     // give child 200 seconds to complete (this so it doesn't alarm while I am debugging)
     alarm(200);
     int status=0;
     if (waitpid(pid, &status, 0)==pid) {
       alarm(0);
     } else {
       alarm(0);
       kill(pid,SIGTERM);
     }
     delete ring;

   }

}