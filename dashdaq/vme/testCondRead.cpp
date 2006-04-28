// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CSimulatedVMEList.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>


#include <CConditionalRead.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class testCondRead : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testCondRead);
  CPPUNIT_TEST(singleYes);
  CPPUNIT_TEST(singleNo);
  CPPUNIT_TEST(multiYes);
  CPPUNIT_TEST(multiNo);
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
  void singleYes();
  void singleNo();
  void multiYes();
  void multiNo();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testCondRead);

// Single term, conditional passed.

void testCondRead::singleYes() {
  // Set a mask in the list, and create a conditional read with
  // a single term that is satisfied by the mask.
  // put known data in the target of the read and do it.

  m_pList->setConditionMask(0xaaaa);
  m_pPio->write32(0x39, 0x500000, 0x12345678);

  vector<uint16_t> terms;
  terms.push_back(0x2);		// 2 & 0xaaaa = 2.
  CConditionalRead<long> ele(0x39, 0x500000, 1, terms);

  long data(0);
  void* p = ele(*m_pPio, *m_pList, &data);
  
  EQ((long)0x12345678, data);
}
// Single term does not make the condition true.
// This is identical to the previous case, but the
// term is one that ands to 0.

void testCondRead::singleNo()
{
  m_pList->setConditionMask(0xaaaa);
  m_pPio->write32(0x39, 0x500000, 0x12345678);

  vector<uint16_t> terms;
  terms.push_back(0x1);		// 1 & 0xaaaa = 0.
  CConditionalRead<long> ele(0x39, 0x500000, 1, terms);

  long data(0);
  void* p = ele(*m_pPio, *m_pList, &data);
  
  EQMSG("data", (long)0, data);
  EQMSG("pointer", &data, (long*)p);
}

// This is just like single yes, but there's more than one term and the
// first one doesn't match.
//
void testCondRead::multiYes()
{
  m_pList->setConditionMask(0xaaaa);
  m_pPio->write32(0x39, 0x500000, 0x12345678);

  vector<uint16_t> terms;
  terms.push_back(1);
  terms.push_back(0x2);		// 2 & 0xaaaa = 2.
  CConditionalRead<long> ele(0x39, 0x500000, 1, terms);

  long data(0);
  void* p = ele(*m_pPio, *m_pList, &data);
  
  EQ((long)0x12345678, data); 
}
void testCondRead::multiNo()
{
  // This is just like single no, but there's more than one term and none of them
  // trigger the condition.

  m_pList->setConditionMask(0xaaaa);
  m_pPio->write32(0x39, 0x500000, 0x12345678);

  vector<uint16_t> terms;
  terms.push_back(0x1);		// 1 & 0xaaaa = 0.
  terms.push_back(4);
  terms.push_back(0x10);
  CConditionalRead<long> ele(0x39, 0x500000, 1, terms);

  long data(0);
  void* p = ele(*m_pPio, *m_pList, &data);
  
  EQMSG("data", (long)0, data);
  EQMSG("pointer", &data, (long*)p);
}
