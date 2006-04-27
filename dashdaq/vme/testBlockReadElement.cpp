// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSimulatedVMEList.h>
#include <CBlockReadElement.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class blockReadElement : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(blockReadElement);
  CPPUNIT_TEST(singlelong);
  CPPUNIT_TEST(singleshort);
  CPPUNIT_TEST(singlebyte);
  CPPUNIT_TEST(multilong);
  CPPUNIT_TEST(multishort);
  CPPUNIT_TEST(multibyte);
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
  void singlelong();
  void singleshort();
  void singlebyte();

  void multilong();
  void multishort();
  void multibyte();
};

CPPUNIT_TEST_SUITE_REGISTRATION(blockReadElement);

void blockReadElement::singlelong() {
  m_pPio->write32(0x39, 0x500000, 0x12345678);
  CBlockReadElement<long> ele(0x39, 0x500000, 1);

  long result;
  ele(*m_pPio, *m_pList, &result);
  EQ(0x12345678l,result);
}

void blockReadElement::singleshort() {
  m_pPio->write16(0x39, 0x500000, 0xaaaa);
  CBlockReadElement<short> ele(0x39, 0x500000, 1);
  short result;
  ele(*m_pPio, *m_pList, &result);

  EQ((short)0xaaaa, result);
}

void blockReadElement::singlebyte()
{
  m_pPio->write8(0x39, 0x500000, 0x55);
  CBlockReadElement<char> ele(0x39, 0x500000, 1);
  char result;
  ele(*m_pPio, *m_pList, &result);

  EQ((char)0x55, result); 

}

void blockReadElement::multilong()
{
  for (int i =0; i < 0x200; i++) {
    m_pPio->write32(0x39, 0x500000+i*sizeof(long), (i%1) ? 0xaaaaaaaa : 0x55555555);
  }
  CBlockReadElement<long> ele(0x39, 0x500000, 0x200);
  long result[0x200];
  ele(*m_pPio, *m_pList, result);

  for (int i =0; i < 0x200; i++) {
    long expected = (i%1) ? 0xaaaaaaaa : 0x55555555;
    EQ(expected, result[i]);
  }
}
void blockReadElement::multishort()
{
  for(int i =0; i < 0x200; i++) {
    m_pPio->write16(0x39, 0x500000+i*sizeof(short), (i%1) ? 0xffff : 0);
  }
  CBlockReadElement<short> ele(0x39, 0x500000, 0x200);
  short result[0x200];
  ele(*m_pPio, *m_pList, result);

  for(int i =0; i < 0x200; i++) {
    short expected = (i%1) ? 0xffff : 0;
    EQ(expected, result[i]);
  }
}
void blockReadElement::multibyte()
{
  for (int i= 0; i < 0x100; i++) {
    m_pPio->write8(0x39, 0x500000+i, i);
  }
  CBlockReadElement<char> ele(0x39, 0x500000, 0x100);
  char result[0x100];
  ele(*m_pPio, *m_pList, result);

  for (int i =0; i < 0x100; i++) {
    EQ((char)i, result[i]);
  }
}
