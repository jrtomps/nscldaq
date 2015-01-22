// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CScaler.h"

#define private public
#include "CScalerBank.h"
#undef private

using namespace std;

// needed due to the ABC nature of CScaler

class testScaler : public CScaler
{
public:
  bool initialized;
  bool cleared;
  bool disabled;

  static uint32_t nextValue;
public:
  testScaler() :
    initialized(false),
    cleared(false),
    disabled(false) {}



  virtual void initialize() {
    initialized = true;
  }
  virtual void clear() {
    nextValue = 0;
    cleared   = true;
  }

  virtual void disable() {
    disabled = true;
  }

  virtual vector<uint32_t> read() {
    vector<uint32_t> r;
    r.push_back(nextValue++);
    return r;
  }
};

uint32_t testScaler::nextValue(0);

class scalerbankTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(scalerbankTests);
  CPPUNIT_TEST(construction);
  CPPUNIT_TEST(add);
  CPPUNIT_TEST(Delete);
  CPPUNIT_TEST(visit);
  CPPUNIT_TEST(initialize);
  CPPUNIT_TEST(clear);
  CPPUNIT_TEST(disable);
  CPPUNIT_TEST(read);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construction();
  void add();
  void Delete();
  void visit();
  void initialize();
  void clear();
  void disable();
  void read();
};

CPPUNIT_TEST_SUITE_REGISTRATION(scalerbankTests);

//  construction should leave:
//  size 0
//  leafCount 0
//  fullCount 0
//  isComposite true
//  begin() == end()
//
void scalerbankTests::construction() {
  CScalerBank bank;

  EQ(static_cast<size_t>(0), bank.size());
  EQ(static_cast<size_t>(0), bank.leafCount());
  EQ(static_cast<size_t>(0), bank.fullCount());
  EQ(true, bank.isComposite());

  ASSERT(bank.begin() == bank.end());

    
}
// Adding elements is delegated so its test is practially
// a formality.  Construct a hierarchy..ensure that
// counts are correct and that the iterator allows us to see
// the top level of the hierarchy
//
void scalerbankTests::add()
{
  testScaler s1, s2, s3, s4, s5;
  CScalerBank top;
  CScalerBank sub;

  CScaler* ptop[] = {&s1, &s3, &sub, &s5};
  CScaler* psub[]  = {&s2, &s4};

  for (int i =0; i < 4; i++) {
    top.AddScalerModule(ptop[i]);
  }
  for (int i=0; i < 2; i++) {
    sub.AddScalerModule(psub[i]);
  }

  // size sb 4 
  // leafcount sb  5
  // fullcount sb 6
  // top iteration give ptop pointers


  EQ(static_cast<size_t>(4), top.size());
  EQ(static_cast<size_t>(5), top.leafCount());
  EQ(static_cast<size_t>(6), top.fullCount());

  CScalerBank::ScalerIterator p = top.begin();
  int i = 0;
  while (p != top.end()) {
    EQ(reinterpret_cast<CScaler*>(ptop[i]), reinterpret_cast<CScaler*>(*p));

    p++;
    i++;
  }


}
//
// Deleting elements is delegated so this test is practically a formality.
// Construct a flat set. delete an element and see that counts and
// contents are ok:
// Naming convention is modified for the fact that delete is a reserved
// word.
void scalerbankTests::Delete()
{
  CScalerBank bank;
  testScaler     s1,s2,s3,s4;

  CScaler* pScalers[]={&s1, &s2, &s3, &s4};
  for (int i =0; i < 4; i++) {
    bank.AddScalerModule(pScalers[i]);
  }

  bank.DeleteScaler(&s2);

  EQ(static_cast<size_t>(3), bank.size());
  EQ(static_cast<size_t>(3), bank.leafCount());
  EQ(static_cast<size_t>(3), bank.fullCount());

  CScaler* pShouldBe[] = {&s1, &s3, &s4};

  CScalerBank::ScalerIterator p = bank.begin();
  for (int i=0; i < 3;i ++) {
    EQ(pShouldBe[i], reinterpret_cast<CScaler*>(*p));
    p++;
  }
  ASSERT(p == bank.end());


}
// Visiting elements.  The visitor I create will count top level elements.  It should
// wind up giving us the same thig as size()
//
class counter : public CScalerBank::CVisitor
{
private:
  size_t m_count;
public:
  counter() : m_count(0) {}
  virtual void operator()(CScaler* pScaler) {
    m_count++;
  }
  size_t count() const { return m_count; }
};

void scalerbankTests::visit()
{
  counter c;
  CScalerBank b;
  testScaler  a,bb,cc,d;
  CScaler* pScalers[] = {&a, &bb, &cc, &d};
  for (int i =0; i < 4;i ++) {
    b.AddScalerModule(pScalers[i]);
  }
  b.visit(c);
  EQ(c.count(), b.size());
}

// Initialize the collection should initialize all elements regardless
// of depth in the hierarchy due to the recursion done by the
// scaler bank class.

void scalerbankTests::initialize()
{
  testScaler s1, s2, s3, s4, s5;
  CScalerBank top;
  CScalerBank bottom;

  CScaler* pTop[] = {&s1, &bottom, &s2};
  CScaler* pBottom[] = {&s3, &s4, &s5};
  testScaler* pAll[] = {&s1, &s2, &s3, &s4, &s5};


  for (int i=0; i < 3; i++) {
    bottom.AddScalerModule(pBottom[i]);
    top.AddScalerModule(pTop[i]);
  }

  top.initialize();

  for (int i=0; i < 5; i++) {
    ASSERT(pAll[i]->initialized);
  }
}
// Same as initialize, but should be cleared.
void scalerbankTests::clear()
{
  testScaler s1, s2, s3, s4, s5;
  CScalerBank top;
  CScalerBank bottom;

  CScaler* pTop[] = {&s1, &bottom, &s2};
  CScaler* pBottom[] = {&s3, &s4, &s5};
  testScaler* pAll[] = {&s1, &s2, &s3, &s4, &s5};


  for (int i=0; i < 3; i++) {
    bottom.AddScalerModule(pBottom[i]);
    top.AddScalerModule(pTop[i]);
  }

  top.clear();

  for (int i=0; i < 5; i++) {
    ASSERT(pAll[i]->cleared);
  }
}

// same as above but should be disabled:

void scalerbankTests::disable()
{
  testScaler s1, s2, s3, s4, s5;
  CScalerBank top;
  CScalerBank bottom;

  CScaler* pTop[] = {&s1, &bottom, &s2};
  CScaler* pBottom[] = {&s3, &s4, &s5};
  testScaler* pAll[] = {&s1, &s2, &s3, &s4, &s5};


  for (int i=0; i < 3; i++) {
    bottom.AddScalerModule(pBottom[i]);
    top.AddScalerModule(pTop[i]);
  }

  top.disable();

  for (int i=0; i < 5; i++) {
    ASSERT(pAll[i]->disabled);
  }
}
// Test reads.
// Our test class delivers a scaler per object with a counting pattern.
//
void scalerbankTests::read()
{
  testScaler s1, s2, s3, s4, s5;
  CScalerBank top;
  CScalerBank bottom;

  CScaler* pTop[] = {&s1, &bottom, &s2};
  CScaler* pBottom[] = {&s3, &s4, &s5};
  testScaler* pAll[] = {&s1, &s2, &s3, &s4, &s5};


  for (int i=0; i < 3; i++) {
    bottom.AddScalerModule(pBottom[i]);
    top.AddScalerModule(pTop[i]);
  }

  top.clear();
  vector<uint32_t> values = top.read();

  EQ(static_cast<size_t>(5), values.size());
  for (uint32_t i=0; i < 5; i++) {
    EQ(i, values[i]);
  }
}
