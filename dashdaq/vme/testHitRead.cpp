// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include <CSimulatedVMEList.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>

#include <CHitRegisterRead.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



class testHitRead : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testHitRead);
  CPPUNIT_TEST(theTest);
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
  void theTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testHitRead);

void testHitRead::theTest() {
  // Set a pattern in 0x500000,
  // Read it with a hit register read and check that it made it into thelist.

  m_pPio->write16(0x39, 0x500000, 0xaaaa);
  CHitRegisterRead ele(0x39, 0x500000);
  short buffer;
  ele(*m_pPio, *m_pList, &buffer);

  EQMSG("buffer", (short)(0xaaaa), buffer);
  EQMSG("list",   (uint16_t)(0xaaaa), m_pList->getConditionMask());
}
