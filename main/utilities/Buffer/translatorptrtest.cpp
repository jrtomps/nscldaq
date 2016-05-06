
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <algorithm>
#include "ByteBuffer.h"

#define private public
#define protected public
#include <BufferPtr.h>
#undef protected
#undef private

using namespace std;

using namespace DAQ::Buffer;

class translatorptrtest : public CppUnit::TestFixture {
private:
  ByteBuffer m_buffer;

  public:
  CPPUNIT_TEST_SUITE(translatorptrtest);
  CPPUNIT_TEST(construct_0);
  CPPUNIT_TEST(copyConstruct_0);
  CPPUNIT_TEST(copyConstruct_1);
  CPPUNIT_TEST(copyConstruct_2);
  CPPUNIT_TEST(copyConstruct_3);
  CPPUNIT_TEST(moveConstruct_0);
  CPPUNIT_TEST(assign_0);
  CPPUNIT_TEST(assign_1);
  CPPUNIT_TEST(assign_2);
  CPPUNIT_TEST(assign_3);
  CPPUNIT_TEST(dereference_0);
  CPPUNIT_TEST(index_0);
  CPPUNIT_TEST(postincrement_0);
  CPPUNIT_TEST(postincrement_1);
  CPPUNIT_TEST(postdecrement_0);
  CPPUNIT_TEST(postdecrement_1);
  CPPUNIT_TEST(preincrement_0);
  CPPUNIT_TEST(predecrement_0);
  CPPUNIT_TEST(add_0);
  CPPUNIT_TEST(add_1);
  CPPUNIT_TEST(add_2);
  CPPUNIT_TEST(sub_0);
  CPPUNIT_TEST(sub_1);
  CPPUNIT_TEST(sub_2);
  CPPUNIT_TEST(sub_3);
  CPPUNIT_TEST(addequal_0);
  CPPUNIT_TEST(addequal_1);
  CPPUNIT_TEST(subequal_0);
  CPPUNIT_TEST(subequal_1);
  CPPUNIT_TEST(lessThan_0);
  CPPUNIT_TEST(lessThan_1);
  CPPUNIT_TEST(greaterThan_0);
  CPPUNIT_TEST(greaterThan_1);
  CPPUNIT_TEST(equal_0);
  CPPUNIT_TEST(equal_1);
  CPPUNIT_TEST(inequal_0);
  CPPUNIT_TEST(inequal_1);
  CPPUNIT_TEST(lessThanEq_0);
  CPPUNIT_TEST(lessThanEq_1);
  CPPUNIT_TEST(greaterThanEq_0);
  CPPUNIT_TEST(greaterThanEq_1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
    m_buffer = {0, 1, 2, 3 ,4, 5, 6, 7, 8};
  }
  void tearDown() {
  }

  void construct_0() {
    BufferPtr<uint8_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_MESSAGE("Construction points to ptr == buffer.begin()",
                             m_buffer.begin() == ptr.getBaseIterator());
  }

  void copyConstruct_0 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin());

    BufferPtr<uint8_t> ptr2(ptr);
    CPPUNIT_ASSERT_MESSAGE("Copy construct copies base iterator",
                            ptr.getBaseIterator() == ptr2.getBaseIterator());
  }

  void copyConstruct_1 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin(), DAQ::BO::CByteSwapper(true));

    BufferPtr<uint8_t> ptr2(ptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Copy construct maintains swapping status",
                                 true, ptr2.getSwapper().isSwappingBytes());
  }

  void copyConstruct_2 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin());

    BufferPtr<uint64_t> ptr2(ptr);
    CPPUNIT_ASSERT_MESSAGE("Generalized copy copies base iterator",
                           ptr.getBaseIterator() == ptr2.getBaseIterator());
  }

  void copyConstruct_3 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin(), DAQ::BO::CByteSwapper(true));

    BufferPtr<uint64_t> ptr2(ptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Generalized copy maintains swapping status",
                                 true, ptr2.getSwapper().isSwappingBytes());
  }

  void moveConstruct_0 () {

    BufferPtr<uint64_t> ptr2(BufferPtr<uint8_t>(m_buffer.begin(), DAQ::BO::CByteSwapper(true)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Generalized copy maintains swapping status",
                                 true, ptr2.getSwapper().isSwappingBytes());
  }

  void assign_0() {
    BufferPtr<uint8_t> ptr(m_buffer.begin());

    BufferPtr<uint8_t> ptr2 = ptr;
    CPPUNIT_ASSERT_MESSAGE("Assignment copies base iterator",
                            ptr.getBaseIterator() == ptr2.getBaseIterator());
  }

  void assign_1 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin(), DAQ::BO::CByteSwapper(true));

    BufferPtr<uint8_t> ptr2 = ptr;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Assignment includes swapping status",
                                 true, ptr2.getSwapper().isSwappingBytes());
  }

  void assign_2 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin());

    BufferPtr<uint64_t> ptr2 = ptr;
    CPPUNIT_ASSERT_MESSAGE("Generalized assignment handles base iterator",
                           ptr.getBaseIterator() == ptr2.getBaseIterator());
  }

  void assign_3 () {
    BufferPtr<uint8_t> ptr(m_buffer.begin(), DAQ::BO::CByteSwapper(true));

    BufferPtr<uint64_t> ptr2 = ptr;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Generalized assign includes swapping status",
                                 true, ptr2.getSwapper().isSwappingBytes());
  }

  void dereference_0() {
    m_buffer[0] = 255;
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dereference without swap should return lower 16-bits",
                                 uint16_t(0x1ff), *ptr);
  }

  void index_0() {
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Index operator works",
                                 uint16_t(0x0504), ptr[2]);
  }

  void postincrement_0() {
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr++;
    CPPUNIT_ASSERT_MESSAGE("Post increment operator works",
                             (oldPos+2) == ptr.getBaseIterator());
  }
  void postincrement_1() {
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    auto lastPtr = ptr++;
    CPPUNIT_ASSERT_MESSAGE("Post increment operator returns original position",
                           lastPtr == (ptr.getBaseIterator()-sizeof(uint16_t)));
  }

  void postdecrement_0() {
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr--;

    // note that this does not throw
    CPPUNIT_ASSERT_MESSAGE("Post decrement operator works",
                             (oldPos-2) == ptr.getBaseIterator());
  }

  void postdecrement_1() {
    BufferPtr<uint16_t> ptr(m_buffer.begin());

    auto lastPos = ptr--;

    // note that this does not throw
    CPPUNIT_ASSERT_MESSAGE("Post decrement operator works",
                           lastPos == (ptr.getBaseIterator()+sizeof(uint16_t)));
  }

  void preincrement_0() {
    BufferPtr<uint32_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    auto newPtr = ++ptr;

    CPPUNIT_ASSERT_MESSAGE("Pre increment operator works",
                           (oldPos+sizeof(uint32_t)) == newPtr.getBaseIterator());
  }

  void predecrement_0() {
    BufferPtr<uint32_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    auto newPtr = --ptr;

    CPPUNIT_ASSERT_MESSAGE("Pre decrement operator works",
                           (oldPos-sizeof(uint32_t)) == newPtr.getBaseIterator());
  }
  void add_0() {
    BufferPtr<int8_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr + 5;

    CPPUNIT_ASSERT_MESSAGE("Addition does not change the original ptr",
                           oldPos == ptr.getBaseIterator());

  }

  void add_1() {
    BufferPtr<int8_t> ptr(m_buffer.begin());

    auto newPtr = ptr + 5;

    CPPUNIT_ASSERT_MESSAGE("Addition returns a meaningful new value",
                           newPtr == (ptr.getBaseIterator()+5));

  }

  void add_2() {
    BufferPtr<int8_t> ptr(m_buffer.begin(), true);

    auto newPtr = ptr + 5;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Addition returns ptr with same swapping semantics",
                           true, newPtr.getSwapper().isSwappingBytes());

  }

  void sub_0() {
    BufferPtr<int8_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr - 5;

    CPPUNIT_ASSERT_MESSAGE("Subtraction does not change the original ptr",
                           oldPos == ptr.getBaseIterator());

  }

  void sub_1() {
    BufferPtr<int8_t> ptr(m_buffer.begin());

    auto newPtr = ptr - 5;

    CPPUNIT_ASSERT_MESSAGE("Subtracttion returns a meaningful new value",
                           newPtr == (ptr.getBaseIterator()-5));

  }

  void sub_2() {

    using diff_type = typename BufferPtr<int16_t>::difference_type;

    BufferPtr<int16_t> ptr(m_buffer.begin());

    auto newPtr = ptr - 2;

    CPPUNIT_ASSERT_MESSAGE("Subtract two translatorptrs returns a correct value",
                           diff_type(2) == (ptr-newPtr));

  }

  void sub_3() {
    BufferPtr<int8_t> ptr(m_buffer.begin(), true);

    auto newPtr = ptr + 5;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Subtraction returns ptr with same swapping semantics",
                           true, newPtr.getSwapper().isSwappingBytes());

  }

  void addequal_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr += 2;

    CPPUNIT_ASSERT_MESSAGE("+= changes the original ptr",
                           (oldPos+2*sizeof(int32_t)) == ptr.getBaseIterator());

  }

  void addequal_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin(), true);

    ptr += 2;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("+= maintains swapping",
                           true, ptr.getSwapper().isSwappingBytes());

  }

  void subequal_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());

    auto oldPos = ptr.getBaseIterator();
    ptr -= 2;

    CPPUNIT_ASSERT_MESSAGE("-= changes the original ptr",
                           (oldPos-2*sizeof(int32_t)) == ptr.getBaseIterator());

  }

  void subequal_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin(), true);

    ptr -= 2;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("-= maintains swapping",
                           true, ptr.getSwapper().isSwappingBytes());

  }


  void lessThan_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int32_t> ptr2(m_buffer.begin()+2);

    CPPUNIT_ASSERT_MESSAGE("< operator makes sense",
                           ptr < ptr2);
  }

  void lessThan_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int8_t> ptr2(m_buffer.begin()+1);

    CPPUNIT_ASSERT_MESSAGE("Generalized < operator makes sense",
                           ptr < ptr2);
  }

  void greaterThan_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int32_t> ptr2(m_buffer.begin()+2);

    CPPUNIT_ASSERT_MESSAGE("> operator makes sense",
                           ptr2 > ptr);
  }

  void greaterThan_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int64_t> ptr2(m_buffer.begin()+1);

    CPPUNIT_ASSERT_MESSAGE("Generalized > operator makes sense",
                           ptr2 > ptr);
  }

  void equal_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_MESSAGE("equality operator makes sense",
                           ptr == ptr);
  }

  void equal_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int8_t>  ptr2(m_buffer.begin());

    CPPUNIT_ASSERT_MESSAGE("Generalized equality operator makes sense",
                           ptr2 == ptr);
  }

  void inequal_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int32_t>  ptr2(m_buffer.begin() + 1);

    CPPUNIT_ASSERT_MESSAGE("Inequality operator makes sense",
                           ptr2 != ptr);
  }

  void inequal_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int64_t> ptr2(m_buffer.begin() + 1);

    CPPUNIT_ASSERT_MESSAGE("Inequality operator makes sense",
                           ptr2 != ptr);
  }

  void greaterThanEq_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_MESSAGE(">= operator treats equality properly",
                           ptr >= ptr);
  }

  void greaterThanEq_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int32_t> ptr2(m_buffer.begin()+1);

    CPPUNIT_ASSERT_MESSAGE(">= treats greater than properly",
                           ptr2 >= ptr);
  }

  void lessThanEq_0 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());

    CPPUNIT_ASSERT_MESSAGE("<= operator treats equality properly",
                           ptr <= ptr);
  }

  void lessThanEq_1 () {
    BufferPtr<int32_t> ptr(m_buffer.begin());
    BufferPtr<int32_t> ptr2(m_buffer.begin()+1);

    CPPUNIT_ASSERT_MESSAGE("<= treats greater than properly",
                           ptr <= ptr2);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(translatorptrtest);

