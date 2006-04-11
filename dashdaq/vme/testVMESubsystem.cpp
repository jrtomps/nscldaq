// Template for a test suite.
#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CVMESubsystem.h"
#include "nullVMEInterface.h"
#include "nullVMEInterfaceCreator.h"
#include "CVMEInterfaceFactory.h"
#include <RangeError.h>

class testSubsystem : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testSubsystem);
  CPPUNIT_TEST(install);
  CPPUNIT_TEST(iterate);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(replace);
  CPPUNIT_TEST(index);
  CPPUNIT_TEST(goodDescriptionString);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem* m_pSubsystem;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
  }
  void tearDown() {
    delete m_pSubsystem;
  }
protected:
  void install();
  void iterate();
  void empty();
  void replace();
  void index();
  void goodDescriptionString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testSubsystem);

void testSubsystem::install() {
  EQMSG("before", static_cast<size_t>(0), m_pSubsystem->size());
  
  nullVMEInterface interface;
  int i = m_pSubsystem->installInterface(interface);
  EQMSG("after: crate #", 0, i);
  EQMSG("after: size   ", static_cast<size_t>(1), m_pSubsystem->size());


}
void testSubsystem::iterate()
{
  nullVMEInterface interface1;
  nullVMEInterface* piface2 = new nullVMEInterface("dynamic");

  int first  = m_pSubsystem->installInterface(interface1);
  int second = m_pSubsystem->installInterface(*piface2, true);

  EQMSG("first", 0, first);
  EQMSG("second",1, second);

  CVMESubsystem::InterfaceIterator i = m_pSubsystem->begin();
  EQMSG("notdelete1", false, i->s_mustDelete);
  EQMSG("1stptr",    static_cast<CVMEInterface*>(&interface1), i->s_pInterface);

  i++;
  EQMSG("delete2", true, i->s_mustDelete);
  EQMSG("2ndptr",  static_cast<CVMEInterface*>(piface2), i->s_pInterface);

  i++;
  ASSERT(m_pSubsystem->end() == i);
	           
}
void testSubsystem::empty()
{
  EQMSG("size", static_cast<size_t>(0), m_pSubsystem->size());
  ASSERT(m_pSubsystem->begin() == m_pSubsystem->end());
}

void testSubsystem::replace()
{
  nullVMEInterface interface1("initial");
  nullVMEInterface interface2("final");
  nullVMEInterface additional("forget");

  m_pSubsystem->installInterface(interface1);
  m_pSubsystem->installInterface(additional);

  // Try to replace a good index:

  CVMEInterface* old                 = m_pSubsystem->replaceInterface(0, interface2);
  CVMESubsystem::InterfaceIterator i = m_pSubsystem->begin();
  EQMSG("correct old", static_cast<CVMEInterface*>(&interface1), old);
  EQMSG("correct new", static_cast<CVMEInterface*>(&interface2), 
	i->s_pInterface);

  // Try to replace a bad index.
	
  bool threw = false;
  try {
    m_pSubsystem->replaceInterface(2, interface1);
  }
  catch (CRangeError& r) {
    threw = true;
  }
  ASSERT(threw);

}
void testSubsystem::index()
{
  nullVMEInterface interface1("first");
  nullVMEInterface interface2("second");
  nullVMEInterface interface3("last");
  m_pSubsystem->installInterface(interface1);
  m_pSubsystem->installInterface(interface2);
  m_pSubsystem->installInterface(interface3);

  CVMEInterface*     p = &((*m_pSubsystem)[0]);
  nullVMEInterface* pI = static_cast<nullVMEInterface*>(p);
  EQ(string("first"), pI->getType());

  p    = &((*m_pSubsystem)[1]);
  pI   =  static_cast<nullVMEInterface*>(p);
  EQ(string("second"), pI->getType());

  
  p    = &((*m_pSubsystem)[2]);
  pI   =  static_cast<nullVMEInterface*>(p);
  EQ(string("last"), pI->getType());

  // indexing range error:

  bool thrown(false);

  try {
    p = &((*m_pSubsystem)[3]);
  }
  catch (CRangeError& r) {
    thrown = true;
  }
  ASSERT(thrown);
}

// Add a null interface creator and ask the subsystem
// to process a description string that creates one.
//
void testSubsystem::goodDescriptionString()
{
  nullVMEInterfaceCreator creator;
  CVMEInterfaceFactory::addCreator("null", creator);

  int i = m_pSubsystem->processDescription(string("null some configuration"));
  EQ(0, i);

  CVMEInterfaceFactory::clearRegistry(); // Clean up!
}
