// Template for a test suite.

#include <config.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSimulatedVMEList.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>
#include <CCountFieldElement.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



class testCountFieldElement : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testCountFieldElement);
  CPPUNIT_TEST(testall);
  CPPUNIT_TEST_SUITE_END();


private:
  CSBSVMEInterface* m_pInterface;
  CVMEPio*             m_pPio;
  CSimulatedVMEList*   m_pList;

public:
  void setUp() {
    m_pInterface = new CSBSVMEInterface(0); // VME crate 0.
    m_pPio       = m_pInterface->createPioDevice();
    m_pList      = new CSimulatedVMEList(*m_pPio);
  }
  void tearDown() {
    delete m_pList;
    delete m_pPio;
    delete m_pInterface;
  }
protected:
  void testall();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testCountFieldElement);

void testCountFieldElement::testall() {
  CCountFieldElement ele(7, 0xaa);
  ele(*m_pPio, *m_pList, (void*)0);

  EQMSG("shift", (uint8_t)7, m_pList->getCountRightShift());
  EQMSG("mask",  (uint32_t)0xaa, m_pList->getCountMask());
  
}
