// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSimulatedVMEList.h>
#include <CSBSVMEInterface.h>
#include <CVMEPio.h>
#include <iostream>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


class StartupMessage
{
public:
  StartupMessage(const char* message) {
    cerr << message << endl;
  }
};

StartupMessage msg1("The Simulated list tests require an SBS interface that");
StartupMessage msg2(" is connected to a VME crate with memory at 0x500000");
StartupMessage msg3("  If this condition is not met, tests will fail");

class TestSimAccess : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestSimAccess);
  CPPUNIT_TEST(countField);
  CPPUNIT_TEST(pattern);
  CPPUNIT_TEST_SUITE_END();


private:
  CSBSVMEInterface*  m_pInterface;
  CVMEPio*           m_pIo;
  CSimulatedVMEList* m_pList;
public:
  void setUp() {
    m_pInterface = new CSBSVMEInterface(0);
    m_pIo        = m_pInterface->createPioDevice();
    m_pList      = new CSimulatedVMEList(*m_pIo);
  }
  void tearDown() {
    delete m_pList;
    delete m_pIo;
    delete m_pInterface;
  }
protected:
  void countField();
  void pattern();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSimAccess);

void TestSimAccess::countField() {
  uint8_t shift = (uint8_t)3;
  uint32_t mask = 0xfe;

  m_pList->setCountExtractionParameters(shift, mask);
  EQMSG("shift", shift, m_pList->getCountRightShift());
  EQMSG("mask",  mask,  m_pList->getCountMask());
}

void TestSimAccess::pattern()
{
  uint16_t mask = 0xaaaa;
  m_pList->setConditionMask(mask);
  EQ(mask, m_pList->getConditionMask());
}
