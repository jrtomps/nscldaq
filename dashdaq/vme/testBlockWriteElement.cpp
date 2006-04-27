// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CSimulatedVMEList.h>
#include <CBlockWriteElement.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


class blockWriteElement : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(blockWriteElement);
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

CPPUNIT_TEST_SUITE_REGISTRATION(blockWriteElement);

void blockWriteElement::singlelong() {
  vector<long> data;
  unsigned long         result;
  data.push_back(0x12345678);
  CBlockWriteElement<long> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  result = m_pPio->read32(0x39, 0x500000);
  EQ((unsigned long)0x12345678, result);
}

void blockWriteElement::singleshort()
{
  vector<short>  data;
  unsigned short result;
  data.push_back(0xaaaa);
  CBlockWriteElement<short> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  result = m_pPio->read16(0x39, 0x500000);
  EQ((unsigned short)0xaaaa, result);
}

void blockWriteElement::singlebyte()
{
  vector<char>  data;
  unsigned char result;
  data.push_back(0xff);
  CBlockWriteElement<char> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  result = m_pPio->read8(0x39, 0x500000);
  EQ((unsigned char)0xff, result);
}


void blockWriteElement::multilong()
{
  vector<long>  data;
  unsigned long result;

  for (int i=0; i < 0x200; i++) {
    data.push_back( (i%1) ? 0xaaaaaaaa : 0x55555555);
  }
  CBlockWriteElement<long> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  for(int i =0; i < 0x200; i++) {
    result = m_pPio->read32(0x39, 0x500000+i*sizeof(long));
    EQ((unsigned long)data[i], result);
  }

}

void blockWriteElement::multishort()
{
  vector<short>  data;
  unsigned short result;

  for (int i = 0; i < 0x200; i++) {
    data.push_back((i%1) ? 0xffff : 0x0);
  }
  CBlockWriteElement<short> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  for(int i = 0; i < 0x200; i++) {
    result = m_pPio->read16(0x39, 0x500000+i*sizeof(short));
    EQ((unsigned short)data[i], result);
  }
}

void blockWriteElement::multibyte()
{
  vector<char>   data;
  unsigned char  result;

  for (int i = 0; i < 0x100; i++) {
    data.push_back(i);
  }
  CBlockWriteElement<char> ele(0x39, 0x500000, data);
  ele(*m_pPio, *m_pList, &result);

  for (int i=0; i < 0x100; i++) {
    result = m_pPio->read8(0x39, 0x500000+i);
    EQ((unsigned char)i, result);
  }
}
