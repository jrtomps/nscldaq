// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CCAMACSubsystem.h>
#include "CFakeInterface.h"
#include "CFakeInterfaceCreator.h"
#include "CCAMACInterfaceFactory.h"
#include <iostream>
#include <sstream>
using namespace std;





class subsystem : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(subsystem);
  CPPUNIT_TEST(singleton);
  CPPUNIT_TEST(add);
  CPPUNIT_TEST(indexing);
  CPPUNIT_TEST(iteration);
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(createFromDesc);
  CPPUNIT_TEST(createFromFile);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAMACSubsystem* m_pSubsystem;

public:
  void setUp() {
    m_pSubsystem = CCAMACSubsystem::getInstance();
  }
  void tearDown() {
    CCAMACSubsystem::getInstance()->clearInterfaces();
  }
protected:
  void singleton();
  void add();
  void indexing();
  void iteration();
  void remove();
  void createFromDesc();
  void createFromFile();
};

CPPUNIT_TEST_SUITE_REGISTRATION(subsystem);

// Ensure the subsystem is a singleton:

void subsystem::singleton() {
  CCAMACSubsystem* p = CCAMACSubsystem::getInstance();

  EQ(m_pSubsystem, p);
}
// Test add/size function

void subsystem::add()
{
  CFakeInterface i1;
  CFakeInterface i2;

  EQMSG("Before add1", (size_t)0, m_pSubsystem->size());
  
  int i = m_pSubsystem->addInterface(i1);
  EQMSG("after add1 size", (size_t)1, m_pSubsystem->size());
  EQMSG("after add1 index", 0, i);

  i = m_pSubsystem->addInterface(i2);
  EQMSG("after add2 size", (size_t)2, m_pSubsystem->size());
  EQMSG("after add2 index", 1, i);


}

// Test indexing:

void subsystem::indexing()
{
  CFakeInterface i1;
  CFakeInterface i2;
  
  m_pSubsystem->addInterface(i1);
  m_pSubsystem->addInterface(i2);

  EQMSG("0", &i1, (CFakeInterface*)(*m_pSubsystem)[0]);
  EQMSG("1", &i2, (CFakeInterface*)(*m_pSubsystem)[1]);
}


// test iteration..

void subsystem::iteration()
{
  CFakeInterface i1;
  CFakeInterface i2;
  CFakeInterface i3;

  CFakeInterface* array[3] = {
    &i1, &i2, &i3
  };

  for (int i = 0; i < 3; i++) {
    m_pSubsystem->addInterface(*(array[i]));
  }

  CCAMACSubsystem::InterfaceIterator p = m_pSubsystem->begin();
  int num = 0;
  while (p != m_pSubsystem->end()) {
    EQMSG("Indexing", array[num], (CFakeInterface*)*p);
    p++; num++;
  }
  EQMSG("count", 3, num);
  
    
}

// Test removal:

void subsystem::remove()
{
  CFakeInterface i1;
  CFakeInterface i2;
  CFakeInterface i3;
  CFakeInterface* array[3] = {
    &i1, &i2, &i3
  };

  for (int i = 0; i < 3; i++) {
    m_pSubsystem->addInterface(*(array[i]));
  }

  CCAMACInterface* pRemoved = m_pSubsystem->removeInterface(1);
  EQMSG("right one gone", &i2, (CFakeInterface*)pRemoved);

  EQMSG("Size", (size_t)2, m_pSubsystem->size());

}

// Test  create from string description

void subsystem::createFromDesc()
{
  CCAMACInterfaceFactory* pFact = CCAMACInterfaceFactory::getInstance();
  CFakeInterfaceCreator*   c = new CFakeInterfaceCreator;
  pFact->addCreator("fake", c);

  size_t i  = m_pSubsystem->createInterface("fake more info");
  ASSERT(m_pSubsystem->size());


  CFakeInterface* pFake = dynamic_cast<CFakeInterface*>((*m_pSubsystem)[0]);
  ASSERT(pFake);

  EQ(string("more info"), string(pFake->getConfiguration()));
  


  delete pFake;
  delete pFact;
}


// This is the 'test file' for createInterfaces test.

static string description("fake first one# a comment\n#\n   fake a second one   \n");


void subsystem::createFromFile()
{
  istringstream file(description);

  CCAMACInterfaceFactory* pFact = CCAMACInterfaceFactory::getInstance();
  CFakeInterfaceCreator*  c     = new CFakeInterfaceCreator;
  pFact->addCreator("fake", c);

  // There will be a slight leak of interfaces.

  m_pSubsystem->createInterfaces(file);

  EQMSG("size", (size_t)2, m_pSubsystem->size());

  CCAMACInterface* p = (*m_pSubsystem)[0];
  CFakeInterface* pFake = dynamic_cast<CFakeInterface*>(p);
  ASSERT(pFake);

  EQMSG("config1", string("first one"), string(pFake->getConfiguration()));

  delete pFact;
  
}
