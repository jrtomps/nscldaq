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


#include <cppunit/extensions/HelperMacros.h>
#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>
#include <CPhysicsEventItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

#include "CTransparentFilter.h"

#define private public
#define protected public
#include "CCompositeFilter.h"
#include "CMediator.h"
#undef private
#undef protected 

// A test suite 
class CCompositeFilterTest : public CppUnit::TestFixture
{
  private:
    // Define a test filter to return some testable results
    class CTestFilter : public CFilter {
      std::string m_history;

      public:
      std::string history() { return m_history; }

      virtual CTestFilter* clone() const { return new CTestFilter(*this);}

      virtual CRingItem* handleStateChangeItem(CRingStateChangeItem*) 
      { return new CRingStateChangeItem(BEGIN_RUN);}

      virtual CRingItem* handleScalerItem(CRingScalerItem* ) 
      { return new CRingScalerItem(200);}

      virtual CRingItem* handleTextItem(CRingTextItem*) 
      { 
        std::vector<std::string> str_vec;
        str_vec.push_back("0000");
        str_vec.push_back("1111");
        str_vec.push_back("2222");
        return new CRingTextItem(PACKET_TYPES,str_vec);
      }

      virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* ) 
      {return new CPhysicsEventItem(4096);}

      virtual CRingItem* 
        handlePhysicsEventCountItem(CRingPhysicsEventCountItem*) 
        { return new CRingPhysicsEventCountItem(static_cast<uint64_t>(4),
            static_cast<uint32_t>(1001));}

      virtual CRingItem* handleFragmentItem(CRingFragmentItem*)
      { 
        return new CRingFragmentItem(static_cast<uint64_t>(10101),
            static_cast<uint32_t>(1),
            static_cast<uint32_t>(2),
            reinterpret_cast<void*>(new char[2]),
            static_cast<uint32_t>(3));
      }

      virtual CRingItem* handleRingItem(CRingItem*) 
      { return new CRingItem(100);}

      virtual void initialize() 
      { m_history += "initialize";}
      virtual void finalize() 
      { m_history += "finalize";}
    };

  private:
    CFilter* m_filter;
    CCompositeFilter* m_composite;

  public:
    CCompositeFilterTest();

    CPPUNIT_TEST_SUITE( CCompositeFilterTest );
    CPPUNIT_TEST ( testConstructor );

    CPPUNIT_TEST ( testRegisterFilter );
    CPPUNIT_TEST ( testProcessTransparentFilter );

    CPPUNIT_TEST ( testTransparentStateChangeItem );
    CPPUNIT_TEST ( testTestStateChangeItem );
    CPPUNIT_TEST ( testTransparentScalerItem );
    CPPUNIT_TEST ( testTestScalerItem );
    CPPUNIT_TEST ( testTransparentTextItem );
    CPPUNIT_TEST ( testTestTextItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventItem );
    CPPUNIT_TEST ( testTestPhysicsEventItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventCountItem );
    CPPUNIT_TEST ( testTestPhysicsEventCountItem );
    CPPUNIT_TEST ( testTransparentFragmentItem );
    CPPUNIT_TEST ( testTestFragmentItem );
    CPPUNIT_TEST ( testTransparentGenericItem );
    CPPUNIT_TEST ( testTestGenericItem );
    CPPUNIT_TEST ( testInitialize0 );
    CPPUNIT_TEST ( testFinalize0 );

    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testRegisterFilter();

    void testProcessTransparentFilter();
    void testTransparentStateChangeItem();
    void testTestStateChangeItem();
    void testTestScalerItem();
    void testTransparentScalerItem();
    void testTestTextItem();
    void testTransparentTextItem();
    void testTransparentPhysicsEventItem();
    void testTestPhysicsEventItem();
    void testTransparentPhysicsEventCountItem();
    void testTestPhysicsEventCountItem();
    void testTransparentFragmentItem();
    void testTestFragmentItem();
    void testTransparentGenericItem();
    void testTestGenericItem();
    void testInitialize0();
    void testFinalize0();

//    void testTransparentMainLoop();

  private:
  template<class T>  CRingItem* setupAndRunFilter(CRingItem* item);
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CCompositeFilterTest );

CCompositeFilterTest::CCompositeFilterTest()
    : m_filter(0), m_composite(0)
{}

void CCompositeFilterTest::setUp()
{
  m_filter = new CTestFilter;
  m_composite = new CCompositeFilter;
  
}

void CCompositeFilterTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_composite;
  m_composite=0;

  delete m_filter;
  m_filter=0;
}


void CCompositeFilterTest::testConstructor()
{
  CPPUNIT_ASSERT(m_composite->begin() == m_composite->end());
}

void CCompositeFilterTest::testRegisterFilter()
{
  CPPUNIT_ASSERT(m_composite->begin() == m_composite->end()); 
  m_composite->registerFilter(m_filter);

  // Check that this is no longer empty
  CPPUNIT_ASSERT(m_composite->begin() != m_composite->end());
  // It should have only one filter
  CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1) , m_composite->size() );
  
}

void CCompositeFilterTest::testProcessTransparentFilter()
{
  // CReate a generic new item to pass in
  CRingItem* item = new CRingItem(100,100); 
  // Setup composite with transparent filter and handle the 
  // ring item with it (CCompositeFilter is just a transparent filter when
  // it has no filters registered)
  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );
  
  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

template<class T>
CRingItem* 
CCompositeFilterTest::setupAndRunFilter(CRingItem* item)
{
  // Create a transparent filter
  m_filter = new T;
  // Register it
  m_composite->registerFilter(m_filter);

  // Clone the composite to avoid double frees is tearDown
  CCompositeFilter* comp = m_composite->clone();

  // Set up the mediator
  CMediator mediator(0,comp,0);
  
  return mediator.handleItem(item);
}

void CCompositeFilterTest::testTransparentStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
} 

void CCompositeFilterTest::testTestStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  // Test filter should always return type BEGIN_RUN
  CPPUNIT_ASSERT( BEGIN_RUN == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

} 

// Create scaler item
void CCompositeFilterTest::testTransparentScalerItem()
{
  CRingItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create scaler item
void CCompositeFilterTest::testTestScalerItem()
{
  CRingItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  CRingScalerItem* new_sclr = static_cast<CRingScalerItem*>(new_item);
  CPPUNIT_ASSERT( 200 == new_sclr->getScalerCount() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}
  
// Text item
void CCompositeFilterTest::testTransparentTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);

  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);

  CPPUNIT_ASSERT (new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

}

// Text item
void CCompositeFilterTest::testTestTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);
  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  // Test filter should always return 200 scalers
  CRingTextItem* new_text = dynamic_cast<CRingTextItem*>(new_item);

  CPPUNIT_ASSERT( 3 == new_text->getStrings().size() );
  CPPUNIT_ASSERT( "0000" == new_text->getStrings()[0] );
  CPPUNIT_ASSERT( "1111" == new_text->getStrings()[1] );
  CPPUNIT_ASSERT( "2222" == new_text->getStrings()[2] );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// PhysicsEvent item
void CCompositeFilterTest::testTransparentPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);

  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEvent item
void CCompositeFilterTest::testTestPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  CPPUNIT_ASSERT( item != new_item );

  CPhysicsEventItem* new_evt = dynamic_cast<CPhysicsEventItem*>(new_item);

  CPPUNIT_ASSERT( 4096 == new_evt->getStorageSize() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTransparentPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item == new_item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTestPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item != new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(4) == new_cnt->getEventCount() );
  CPPUNIT_ASSERT( static_cast<uint32_t>(1001) == new_cnt->getTimeOffset() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// RingFragmentItem
void CCompositeFilterTest::testTransparentFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);

  CPPUNIT_ASSERT( new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// RingFragmentItem
void CCompositeFilterTest::testTestFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);

  CPPUNIT_ASSERT( new_item != item);

  CRingFragmentItem* new_frag = dynamic_cast<CRingFragmentItem*>(new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(10101) == new_frag->timestamp());
  CPPUNIT_ASSERT( static_cast<uint32_t>(1) == new_frag->source());
  CPPUNIT_ASSERT( static_cast<uint32_t>(2) == const_cast<CRingFragmentItem*>(new_frag)->payloadSize());
  CPPUNIT_ASSERT( static_cast<uint32_t>(3) == new_frag->barrierType());

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTransparentGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = setupAndRunFilter<CTransparentFilter>(item);
  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTestGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = setupAndRunFilter<CTestFilter>(item);
  CPPUNIT_ASSERT( item != new_item );
  // Test filter should always return type 100
  CPPUNIT_ASSERT( 100 == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}


void CCompositeFilterTest::testInitialize0()
{
  CTestFilter f;
  m_composite->registerFilter(&f); 
  m_composite->registerFilter(&f); 

  m_composite->initialize();
  CCompositeFilter::iterator it = m_composite->begin();
  CCompositeFilter::iterator itend = m_composite->end();
  while (it!=itend) {
    CTestFilter *tfilt = static_cast<CTestFilter*>(*it);
    CPPUNIT_ASSERT_EQUAL(std::string("initialize"), tfilt->history());
    ++it;
  }

}

void CCompositeFilterTest::testFinalize0()
{
  CTestFilter f;
  m_composite->registerFilter(&f); 
  m_composite->registerFilter(&f); 

  m_composite->finalize();
  CCompositeFilter::iterator it = m_composite->begin();
  CCompositeFilter::iterator itend = m_composite->end();
  while (it!=itend) {
    CTestFilter *tfilt = static_cast<CTestFilter*>(*it);
    CPPUNIT_ASSERT_EQUAL(std::string("finalize"), tfilt->history());
    ++it;
  }

}

