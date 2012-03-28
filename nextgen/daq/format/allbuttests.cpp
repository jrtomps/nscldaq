// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <config.h>
#include "CAllButPredicate.h"
#include "DataFormat.h"



class allbuttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(allbuttests);
  CPPUNIT_TEST(notinlist);
  CPPUNIT_TEST(inlist);
  //  CPPUNIT_TEST(sampledlast); /* Changed how sampling works */
  //  CPPUNIT_TEST(samplednotlast); /* Tests invalidated */
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* pCons;
  CRingBuffer* pProd;

public:
  void setUp() {
    CRingBuffer::create("pred");
    pCons = new CRingBuffer("pred", CRingBuffer::consumer);
    pProd = new CRingBuffer("pred", CRingBuffer::producer);
  }
  void tearDown() {
    delete pCons;
    delete pProd;   
    CRingBuffer::remove("pred");

  }
protected:
  void notinlist();
  void inlist();
  void sampledlast();
  void samplednotlast();
};

CPPUNIT_TEST_SUITE_REGISTRATION(allbuttests);

// Items not in the selection list should retuurn false;

void allbuttests::notinlist() {
  RingItemHeader h  = {sizeof(RingItemHeader), 1234};
  pProd->put(&h, sizeof(h));

  CAllButPredicate p;

  ASSERT(!p(*pCons));
}
//Unsampled item in the list should retuurn true:

void allbuttests::inlist()
{
  RingItemHeader h  = {sizeof(RingItemHeader), 1234};
  pProd->put(&h, sizeof(h));
  
  CAllButPredicate p;
  p.addExceptionType(1234);
  ASSERT(p(*pCons));
}
// A sampled item in the list that is at the end of the
// ring should give false.

void allbuttests::sampledlast()
{
  RingItemHeader h  = {sizeof(RingItemHeader), 1234};
  pProd->put(&h, sizeof(h));
  
  CAllButPredicate p;
  p.addExceptionType(1234, true);
  ASSERT(!p(*pCons));
}


// A sampled item in the list that is not at the end should give true.

void allbuttests::samplednotlast()
{  
  RingItemHeader h  = {sizeof(RingItemHeader), 1234};
  pProd->put(&h, sizeof(h));
  pProd->put(&h, sizeof(h));
  
  CAllButPredicate p;
  p.addExceptionType(1234, true);

  ASSERT(p(*pCons));
  EQ(sizeof(h), pCons->availableData());
 
  ASSERT(!p(*pCons));
  

}
