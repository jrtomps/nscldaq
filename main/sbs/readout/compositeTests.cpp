// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CObject.h"

// Include the object under test and give ourselves
// access to the internals if needed.

#define private public
#include "CComposite.h"
#undef private

// Minimal leaf node:

class CLeaf : public CObject
{
public:
  virtual bool isComposite() const { return false; }
};


class compositeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(compositeTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(insertleaves);
  CPPUNIT_TEST(deleteleaves);
  CPPUNIT_TEST(insertcomposite);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();
  void insertleaves();
  void deleteleaves();
  void insertcomposite();
};

CPPUNIT_TEST_SUITE_REGISTRATION(compositeTests);


// Construction should result in:
//  size = 0
//  begin == end
//  leafCount == 0
//  fullCount == 0
//  isComposite == true
//
void compositeTests::construct() {
  CComposite c;

  EQ(static_cast<size_t>(0), c.size());
  ASSERT (c.end() == c.begin());
  EQ(static_cast<size_t>(0), c.leafCount());
  EQ(static_cast<size_t>(0), c.fullCount());
  EQ(true, c.isComposite());

}
// Inserting leaves:
// - Leaves should be inserted at the end.
// - inserting leaves should bump all counts up by one.
//

void compositeTests::insertleaves()
{
  CComposite c;
  CLeaf      l1, l2, l3;

  c.addItem(&l1);
  EQ(static_cast<size_t>(1), c.size());
  ASSERT (c.end() != c.begin());
  EQ(static_cast<size_t>(1), c.leafCount());
  EQ(static_cast<size_t>(1), c.fullCount());
  CComposite::objectIterator p = c.end();
  p--;
  EQ(&l1, reinterpret_cast<CLeaf*>(*p));

  c.addItem(&l2);
  c.addItem(&l3);

  CLeaf* pLeaves[] = {&l1, &l2, &l3};
  p = c.begin();
  for (int i = 0; i < c.size(); i++) {
    EQ(pLeaves[i], reinterpret_cast<CLeaf*>(*p));
    p++;
  }
  
}
// Deleting leaves:
//   Deleting a leaf removes it from the collection
//   Deleting a leaf decrements all counts.
// Deleting nonexistent leaves:
//   changes nothing.
//
void compositeTests::deleteleaves()
{
  CComposite c;
  CLeaf      l1, l2, l3;
  CLeaf      notInserted;
  CLeaf*     pLeaves[] = {&l1, &l2, &l3};

  for (int i = 0; i < 3; i++) {
    c.addItem(pLeaves[i]);
  }

  // Removing nonexistent does nothing:

  c.deleteItem(&notInserted);
  EQ(static_cast<size_t>(3), c.size());
  EQ(static_cast<size_t>(3), c.leafCount());
  EQ(static_cast<size_t>(3), c.fullCount());
  CComposite::objectIterator p = c.begin();
  for (int i=0; i < 3; i++) {
    EQ(pLeaves[i], reinterpret_cast<CLeaf*>(*p));
    p++;
  }

  // Delete an existing leaf.. from the middle too:

  c.deleteItem(&l2);
  EQ(static_cast<size_t>(2), c.size());
  EQ(static_cast<size_t>(2), c.leafCount());
  EQ(static_cast<size_t>(2), c.fullCount());

  p = c.begin();
  EQ(&l1, reinterpret_cast<CLeaf*>(*p));
  p++;
  EQ(&l3, reinterpret_cast<CLeaf*>(*p));
  
  
}
// Inserting composites make a difference between
// the leaf and all counts.. Other than that there's no
// difference whatsoever.
//
void compositeTests::insertcomposite()
{
  CComposite c;
  CComposite c1;
  CLeaf      l1, l2, l3;
  c1.addItem(&l3);
  c.addItem(&l1);
  c.addItem(&l2);
  c.addItem(&c1);

  EQ(static_cast<size_t>(3), c.leafCount());
  EQ(static_cast<size_t>(4), c.fullCount());
}
