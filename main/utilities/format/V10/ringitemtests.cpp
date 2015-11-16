// Template for a test suite.
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <config.h>

#define private public
#define protected public
#include <V10/CRingItem.h>
#undef private
#undef protected

#include <V10/DataFormatV10.h>

using namespace DAQ::V10;
using namespace std;

class ritemtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ritemtests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(selectors);
  CPPUNIT_TEST(cursor);
//  CPPUNIT_TEST(toring);
//  CPPUNIT_TEST(fromring);
//  CPPUNIT_TEST(selection);
  CPPUNIT_TEST(copyconstruct);
  CPPUNIT_TEST(bodysize);
  CPPUNIT_TEST(fillBody_0);
  CPPUNIT_TEST(fillBody_1);
  CPPUNIT_TEST(fillBody_2);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();
  void copyconstruct();
  void selectors();
  void cursor();
//  void toring();
//  void fromring();
//  void selection();
//  void sampling();
  void bodysize();
  void fillBody_0();
  void fillBody_1();
  void fillBody_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ritemtests);

//
// There are two cases for construction...
// assuming the static ring buffer size is 8k.
// 
void ritemtests::construct() {
  CRingItem small(1);
  CRingItem big(1, CRingItemStaticBufferSize*2);

  // The tests below differentiate between small and large buffers.

  EQ(reinterpret_cast<uint8_t*>(small.m_pItem), 
     (uint8_t*)small.m_staticBuffer);
  ASSERT(reinterpret_cast<uint8_t*>(big.m_pItem) != big.m_staticBuffer);

  // Let's look at the member data for small too:

  EQ((void*)small.m_pItem->s_body, (void*)small.m_pCursor);
  EQ(CRingItemStaticBufferSize, small.m_storageSize);
  EQ(false, small.m_swapNeeded);

  // Let's look at the member data for big:
  
  EQ((uint8_t*)big.m_pItem->s_body, big.m_pCursor);
  EQ(CRingItemStaticBufferSize*2, big.m_storageSize);
  EQ(false, big.m_swapNeeded);
}

// Test that we've fixed the error in copy construction that did not
// get the cursor right:
//
void ritemtests::copyconstruct()
{
  // Make the source ring and put something in it:
  CRingItem source(100);
  uint8_t  *payload = reinterpret_cast<uint8_t*>(source.getBodyCursor());
  for (int i =0; i < 16; i++) {
    *payload++ = i;
  }
  source.setBodyCursor(payload);
  source.updateSize();

  CRingItem copy(source);

  EQ(source.getBodySize(), copy.getBodySize());
  _RingItem* porig = source.getItemPointer();
  _RingItem* pcopy = copy.getItemPointer();

  // headers must match 

  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

  // contents must match:

  uint8_t* pSource = reinterpret_cast<uint8_t*>(source.getBodyPointer());
  uint8_t* pCopy   = reinterpret_cast<uint8_t*>(copy.getBodyPointer());
  for (int i = 0; i < 16; i++) {
    EQ((int)*pSource, (int)*pCopy);
    pSource++;
    pCopy++;
  }


}

//
// Tests the various selector  member functions.
//
void
ritemtests::selectors()
{
  CRingItem item(5, 1024);
  EQ((size_t)1024, item.getStorageSize());
  EQ((size_t)0, item.getBodySize());
  EQ((void*)item.m_pCursor, item.getBodyPointer());
  EQ((void*)item.m_pCursor, item.getBodyCursor());
  EQ(item.m_pItem, item.getItemPointer());
  EQ((uint32_t)5, item.type());
  ASSERT(!item.mustSwap());
}
//
// Test the ability to manipulate the item by filling it in and
// updating the body cursor.
//
void
ritemtests::cursor()
{
  CRingItem item(0x1234);
  uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
  for (uint16_t  i =0; i < 16; i++) {
    *pData++  = i;
  }
  item.setBodyCursor(pData);

  EQ(16*sizeof(uint16_t), item.getBodySize());
}

////
//// Test the ability to write an item into a ring buffer.
////
//void
//ritemtests::toring()
//{
//  CRingBuffer::create("items");

//  try {
//    // Create fill an item

//    CRingItem item(0x1234);
//    uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
//    for (uint16_t  i =0; i < 16; i++) {
//      *pData++  = i;
//    }
//    item.setBodyCursor(pData);

//    CRingBuffer prod("items", CRingBuffer::producer);
//    CRingBuffer cons("items", CRingBuffer::consumer);

//    item.commitToRing(prod);

//    const size_t itemSize = item.getBodySize() + sizeof(RingItemHeader);
//    EQ(itemSize, cons.availableData());
//    uint8_t itemData[itemSize];
    
//    size_t got = cons.get(itemData, itemSize, itemSize, 0);
//    EQ(itemSize, got);

//    // Check contents:

//    RingItem* pItem = reinterpret_cast<RingItem*>(itemData);
//    EQ((uint32_t)0x1234, pItem->s_header.s_type);
//    EQ(itemSize, (size_t)pItem->s_header.s_size);
//    uint16_t* body = reinterpret_cast<uint16_t*>(pItem->s_body);
//    for (uint16_t i =0; i < 16; i++) {
//      EQ(i, body[i]);
//    }
//  }
//  catch(...) {
//    CRingBuffer::remove("items");
//    throw;
//  }
//  CRingBuffer::remove("items");


//}

////
//// Insert an item int oa ring and then see if it can be gotten out
//// we'll use a the all predicate with no exceptions in this test.
////
//void
//ritemtests::fromring()
//{
//  CRingBuffer::create("items");
  
//  try {
//    CRingItem item(0x1234);
//    uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
//    for (uint16_t  i =0; i < 16; i++) {
//      *pData++  = i;
//    }
//    item.setBodyCursor(pData);

//    CRingBuffer prod("items", CRingBuffer::producer);
//    CRingBuffer cons("items", CRingBuffer::consumer);

//    item.commitToRing(prod);

//    CAllButPredicate all;
//    CRingItem* pItem = CRingItem::getFromRing(cons, all);
//    ASSERT(pItem);

//    EQ((uint32_t)0x1234, pItem->type());
//    EQ((size_t)16*sizeof(uint16_t), pItem->getBodySize());
//    uint16_t* pBody = reinterpret_cast<uint16_t*>(pItem->getBodyPointer());
//    for (uint16_t i=0; i < 16; i++) {
//      EQ(i, pBody[i]);
//    }


//    delete pItem;
    
//  }
//  catch (...) {
//    CRingBuffer::remove("items");
//    throw;
  
//  }
//  CRingBuffer::remove("items");


//}
////
//// Lets' get data with some selectivity.
////   We'll insert an item with type 1 and another with type 2.
//// we'll request type 2 only with no sampling.. getting should get us that
//// item.
//void
//ritemtests::selection()
//{
//  CRingBuffer::create("items");

//  try {
//    CRingItem i1(1);
//    CRingItem i2(2);


//    CRingBuffer prod("items", CRingBuffer::producer);
//    CRingBuffer cons("items", CRingBuffer::consumer);

//    i1.commitToRing(prod);
//    i2.commitToRing(prod);

//    CDesiredTypesPredicate two;
//    two.addDesiredType(2);

//    CRingItem* pItem = CRingItem::getFromRing(cons, two);
//    EQ((uint32_t)2, pItem->type());

//  }
//  catch(...) {
//    CRingBuffer::remove("items");
//    throw;
//  }

//  CRingBuffer::remove("items");
//}

//// Let's get data with sampling.
//// insert two items of type 1, get with sampling on type 1. Should get
//// the second item.
////
//void
//ritemtests::sampling()
//{
//   CRingBuffer::create("items");

//  try {
//    CRingItem i1(1);
//    CRingItem i2(1);

//    // Distinguish i1 from i2 by putting data into it:

//    uint16_t* pData =  reinterpret_cast<uint16_t*>(i2.getBodyCursor());
//    *pData++ = 2;
//    i2.setBodyCursor(pData);

//    CRingBuffer prod("items", CRingBuffer::producer);
//    CRingBuffer cons("items", CRingBuffer::consumer);

//    i1.commitToRing(prod);
//    i2.commitToRing(prod);

//    CDesiredTypesPredicate s;
//    s.addDesiredType(1, true);

//    CRingItem* pItem = CRingItem::getFromRing(cons, s);
//    EQ(sizeof(uint16_t), pItem->getBodySize());


//  }
//  catch(...) {
//    CRingBuffer::remove("items");
//    throw;
//  }

//  CRingBuffer::remove("items");
//}

void 
ritemtests::bodysize()
{
  CRingItem i(1);
  i.updateSize();

  EQ(static_cast<size_t>(0), i.getBodySize());

  
  uint8_t* pBody = reinterpret_cast<uint8_t*>(i.getBodyCursor());
  i.setBodyCursor(pBody);
  EQ(static_cast<size_t>(0), i.getBodySize());

  *pBody++ = 1;
  *pBody++ = 2;
  i.setBodyCursor(pBody);
  EQ(static_cast<size_t>(2), i.getBodySize());
}


void ritemtests::fillBody_0()
{
    vector<uint16_t> data = {0, 1, 2, 3, 4, 5, 6};
    CRingItem item(1);

    item.fillBody(data);

    // body was updated appropriately
    CPPUNIT_ASSERT_EQUAL_MESSAGE("body size should match size of data passed in",
                         data.size()*sizeof(uint16_t), item.getBodySize());
}
void ritemtests::fillBody_1()
{
    vector<uint16_t> data = {0, 1, 2, 3, 4, 5, 6};
    CRingItem item(1);

    item.fillBody(data);

    // body contains data we expect
    uint16_t* pBody = reinterpret_cast<uint16_t*>(item.getBodyPointer());
    for (size_t i=0; i<data.size(); ++i) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "body should recieve data passed in",
                                      data.at(i), *pBody++ );
    }

}
void ritemtests::fillBody_2()
{
    vector<uint16_t> data = {0, 1, 2, 3, 4, 5, 6};
    CRingItem item(1);

    item.fillBody(data);

    // cursor is located after our new data
    uint16_t* pBody = reinterpret_cast<uint16_t*>(item.getBodyPointer());
    uint16_t* pCursor = reinterpret_cast<uint16_t*>(item.getBodyCursor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cursor should be located after newly inserted data",
                         ptrdiff_t(data.size()), pCursor-pBody);
}
