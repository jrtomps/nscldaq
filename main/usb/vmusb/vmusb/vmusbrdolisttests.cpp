// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#include "Asserts.h"

#define private public
#define protected public
#include <CVMUSBReadoutList.h>
#undef protected
#undef private

using namespace std;

// If your memory is in a different place.. change the three lines
// below:

class VMUSBRdoListTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VMUSBRdoListTests);
  CPPUNIT_TEST (clear_0); 
  CPPUNIT_TEST (size_0); 
  CPPUNIT_TEST (append_0); 
  CPPUNIT_TEST (addRegisterRead_0);
  CPPUNIT_TEST (addRegisterWrite_0);
  CPPUNIT_TEST (addWrite32_0);
  CPPUNIT_TEST (addWrite16_0);
  CPPUNIT_TEST (addWrite8_0);
  CPPUNIT_TEST (addRead32_0);
  CPPUNIT_TEST (addRead16_0);
  CPPUNIT_TEST (addRead8_0);
  CPPUNIT_TEST (addBlockRead32_0);
  CPPUNIT_TEST (addFifoRead32_0);
  CPPUNIT_TEST (addFifoRead16_0);
  CPPUNIT_TEST (addBlockWrite32_0);
  CPPUNIT_TEST (addBlockCountRead8_0);
  CPPUNIT_TEST (addBlockCountRead16_0);
  CPPUNIT_TEST (addBlockCountRead32_0);
  CPPUNIT_TEST (addMaskedCountBlockRead32_0);
  CPPUNIT_TEST (addMaskedCountFifoRead32_0);
  CPPUNIT_TEST (addDelay_0);
  CPPUNIT_TEST (addMarker_0);
  CPPUNIT_TEST_SUITE_END();

  private:
  CVMUSBReadoutList* m_pList;

  public:
  void setUp() {
    m_pList = new CVMUSBReadoutList();
  }
  void tearDown() {
    delete m_pList;
  }
  private:
  void clear_0(); 
  void size_0(); 
  void append_0(); 
  void addRegisterRead_0();
  void addRegisterWrite_0();
  void addWrite32_0();
  void addWrite16_0();
  void addWrite8_0();
  void addRead32_0();
  void addRead16_0();
  void addRead8_0();
  void addBlockRead32_0();
  void addFifoRead32_0();
  void addFifoRead16_0();
  void addBlockWrite32_0();
  void addBlockCountRead8_0();
  void addBlockCountRead16_0();
  void addBlockCountRead32_0();
  void addMaskedCountBlockRead32_0();
  void addMaskedCountFifoRead32_0();
  void addDelay_0();
  void addMarker_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(VMUSBRdoListTests);

template<class T>
void print_vectors(const std::vector<T>& expected, const std::vector<T>& actual) {
  std::cout.flags(std::ios::hex);

  std::copy(expected.begin(), expected.end(), std::ostream_iterator<T>(std::cout,"\n"));
  std::cout << "---" << std::endl;
  std::copy(actual.begin(), actual.end(), std::ostream_iterator<T>(std::cout,"\n"));

  std::cout.flags(std::ios::dec);
}

void VMUSBRdoListTests::clear_0() {
  m_pList->m_list.push_back(0);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_pList->get().size());

  m_pList->clear();
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), m_pList->get().size());
}

void VMUSBRdoListTests::size_0() {
  // make sure that the vector is empty
  m_pList->m_list.clear();

  // add an element
  m_pList->m_list.push_back(0);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_pList->size());
}

void VMUSBRdoListTests::append_0() {
  // add an element
  m_pList->m_list.push_back(0);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_pList->size());

  // generate a second list
  std::vector<uint32_t> rawData(2,2);
  CVMUSBReadoutList list2(rawData);
  m_pList->append(list2);

  // create a list that contains what we expect should be the result
  std::vector<uint32_t> expected(3);
  expected[0] = 0;
  expected[1] = 2;
  expected[2] = 2;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addRegisterRead_0() {

  m_pList->addRegisterRead(0x4);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<12)|(1<<8)); // self bit, nowrite bit
  expected[1] = 0x4;

  CPPUNIT_ASSERT(expected == m_pList->get());
 
}

void VMUSBRdoListTests::addRegisterWrite_0() {

  m_pList->addRegisterWrite(0x4, 24);

  std::vector<uint32_t> expected(3);
  expected[0] = (1<<12); // self bit, nowrite bit
  expected[1] = 0x4;
  expected[2] = 24;

  CPPUNIT_ASSERT(expected == m_pList->get());
 
}

void VMUSBRdoListTests::addWrite32_0() {

  m_pList->addWrite32(0x8023f, 0x09, 244);

  std::vector<uint32_t> expected(3);
  expected[0] = 0x09; 
  expected[1] = 0x8023c;
  expected[2] = 244;

  CPPUNIT_ASSERT(expected == m_pList->get());
 
}

void VMUSBRdoListTests::addWrite16_0() {

  m_pList->addWrite16(0x80230, 0x09, 244);

  std::vector<uint32_t> expected(3);
  expected[0] = 0x09; 
  expected[1] = 0x80231;
  expected[2] = 244;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addWrite8_0() {

  m_pList->addWrite8(0x80230, 0x09, 244);

  std::vector<uint32_t> expected(3);
  expected[0] = ((2<<6)|0x09); // 2 is for DS1  
  expected[1] = 0x80231; // address has bit1 set for LWORD*
  expected[2] = ((244<<8)|244); // we put duplicate data in either lane
                                // to make sure it gets picked up properly

  CPPUNIT_ASSERT(expected == m_pList->get());
}



void VMUSBRdoListTests::addRead32_0() {

  m_pList->addRead32(0x8023f, 0x09);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<8)|0x09);  // amod + nw=1
  expected[1] = 0x8023c; // address has bit1 set for LWORD*
                         // to make sure it gets picked up properly

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addRead16_0() {

  m_pList->addRead16(0x80230, 0x09);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<8)|0x09);  // amod + nw=1
  expected[1] = 0x80231; // address has bit1 set for LWORD*
                         // to make sure it gets picked up properly

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addRead8_0() {

  m_pList->addRead8(0x80230, 0x09);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<8)|(2<<6)|0x09);  // amod + nw=1 + DS1
  expected[1] = 0x80231; // address has bit1 set for LWORD*
                         // to make sure it gets picked up properly

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addBlockRead32_0() 
{
  // needs an alignment transfer 
  m_pList->addBlockRead32(0xff82ff, 0x0b,255); 

  std::vector<uint32_t> expected(2);
  expected[0] = ((255<<24)|(1<<8)|0x0b); // 255 transfers to align, 
  expected[1] = 0xff82ff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addFifoRead32_0() 
{
  // needs an alignment transfer 
  m_pList->addFifoRead32(0xff82ff, 0x0b,255);  

  std::vector<uint32_t> expected(2);
  expected[0] = ((255<<24)|(1<<10)|(1<<8)|0x0b); // 255 transfers to align, 
                                                 // nowrite, no auto increment blt
                                                 // amod=0x0b
  expected[1] = 0xff82ff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addFifoRead16_0() {
  
  // needs an alignment transfer 
  m_pList->addFifoRead16(0xff82ff, 0x0b,255);  

  std::vector<uint32_t> expected(2);
  expected[0] = ((255<<24)|(1<<10)|(1<<8)|0x0b); // 255 transfers to align, 
                                                 // nowrite, no auto increment blt
                                                 // amod=0x0b
  expected[1] = 0xff82ff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addBlockWrite32_0() 
{
  uint32_t data[5] = {0, 1, 2, 3, 4};
  m_pList->addBlockWrite32(0xffa23, 0x0b, data, 5);
  

  std::vector<uint32_t> expected(7);
  expected[0] = ((0x5<<24)|0x0b);  
  expected[1] = 0xffa23;
  expected[2] = 0;
  expected[3] = 1;
  expected[4] = 2;
  expected[5] = 3;
  expected[6] = 4;

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addBlockCountRead8_0()
{
  m_pList->addBlockCountRead8(0xffffff,0xa0a0,0x09);

  std::vector<uint32_t> expected(3);
  expected[0] = ((1<<18)|(1<<8)|(1<<6)|0x09); // ND, nw=1, DS0, amod=0x09
  expected[1] = 0xa0a0;
  expected[2] = 0xffffff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addBlockCountRead16_0()
{
  m_pList->addBlockCountRead16(0xffffff,0xa0a0,0x09);

  std::vector<uint32_t> expected(3);
  expected[0] = ((1<<18)|(1<<8)|0x09); // ND, nw=1, amod=0x09
  expected[1] = 0xa0a0;
  expected[2] = 0xffffff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addBlockCountRead32_0()
{
  m_pList->addBlockCountRead32(0xffffff,0xa0a0,0x09);

  std::vector<uint32_t> expected(3);
  expected[0] = ((1<<18)|(1<<8)|0x09); // ND, nw=1, amod=0x09
  expected[1] = 0xa0a0;
  expected[2] = 0xfffffc;

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addMaskedCountBlockRead32_0()
{
  m_pList->addMaskedCountBlockRead32(0xffffff,0x0b);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<24)|(1<<8)|0x0b); // 1 transfer, nw=1, amod=0x0b
  expected[1] = 0xffffff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addMaskedCountFifoRead32_0()
{
  m_pList->addMaskedCountFifoRead32(0xffffff,0x0b);

  std::vector<uint32_t> expected(2);
  expected[0] = ((1<<24)|(1<<10)|(1<<8)|0x0b); // 1 transfer, nw=1, NA, amod=0x0b
  expected[1] = 0xffffff;

  CPPUNIT_ASSERT(expected == m_pList->get());
}

void VMUSBRdoListTests::addDelay_0() 
{
  m_pList->addDelay(10);

  std::vector<uint32_t> expected(1);
  expected[0] = ((1<<15)|10);  // DLY, amount

  CPPUNIT_ASSERT(expected == m_pList->get());
}


void VMUSBRdoListTests::addMarker_0() 
{
  m_pList->addMarker(0xabcd);

  std::vector<uint32_t> expected(2);
  expected[0] = (1<<13);  // MRK 
  expected[1] = 0xabcd;

  CPPUNIT_ASSERT(expected == m_pList->get());
}
