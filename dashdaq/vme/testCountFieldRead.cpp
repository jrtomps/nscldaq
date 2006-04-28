// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CSimulatedVMEList.h>
#include <CVMEPio.h>
#include <CSBSVMEInterface.h>
#include <CCountFieldRead.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



class testCountFieldRead : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testCountFieldRead);
  CPPUNIT_TEST(longs);
  CPPUNIT_TEST(shorts);
  CPPUNIT_TEST(bytes);
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
  void longs();
  void shorts();
  void bytes();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testCountFieldRead);

void testCountFieldRead::longs() {
  // Use the pio element to put a pattern in memory.
  // This will have the form:
  //      count data
  //      counting pattern
  //  Where count data will have count field in bits 8-15.
  //
  unsigned long base  = 0x500000;
  long          count = 100;
  m_pPio->write32(0x39, base, count << 8);
  for (long i =0; i < count; i++) {
    m_pPio->write32(0x39, base + (i+1)*sizeof(long), i);
  }
  // Set the list with the correct extraction parameters:
  // Create the element and execute it:

  m_pList->setCountExtractionParameters(8, 0xff);
  long   buffer[200];
  CCountFieldRead<long> ele(0x39, base);

  void* end = ele(*m_pPio, *m_pList, buffer);
  EQMSG("end pointer", (void*)(buffer + count+1), end);
  EQMSG("count data", (count << 8), buffer[0]);
  long* data = buffer;
  data++;
  
  for(int i=0; i < count; i++) {
    EQMSG("counting pattern", (long)i, *data);
    data++;
  }
  
}

void testCountFieldRead::shorts() {
  // Same as longs, but the pattern will be walking bits.

  unsigned long   base  = 0x500000;
  short           count = 100;
  m_pPio->write16(0x39, base, count << 8);
  for (long i = 0; i < count; i++) {
    m_pPio->write16(0x39, base + (i+1)*sizeof(short), 
		    (i%1) ? 0x5555 : 0xaaaa);
  }
  
  m_pList->setCountExtractionParameters(8, 0xff);
  short buffer[200];
  CCountFieldRead<short> ele(0x39, base);
  
  void* end = ele(*m_pPio, *m_pList, buffer);
  EQMSG("end pointer", (void*)(buffer + count+1), end);
  EQMSG("count data", (short)(count << 8), buffer[0]);
  short* data = buffer;
  data++;
  for (int i=0; i < count; i++) {
    EQMSG("data", (short)((i%1) ? 0x5555 : 0xaaaa), data[i]);
  }
}
void testCountFieldRead::bytes() {
  // Same as shorts but:
  //   count field is shifted by 1 bit only.
  //   pattern is a count down.

  unsigned long base = 0x500000;
  char          count= 100;
  m_pPio->write8(0x39, base, count <<1);
  for (long i =0; i < count; i++) {
    m_pPio->write8(0x39, base + i + 1,
		   (count - i));
  }

  m_pList->setCountExtractionParameters(1, 0x7f);
  char buffer[200];
  CCountFieldRead<char>  ele(0x39, base);
  void* end = ele(*m_pPio, *m_pList, buffer);
  EQMSG("end pointer", (void*)(buffer + count+1), end);
  EQMSG("count data",  (char)(count << 1), buffer[0]);
  char* data = buffer;
  data++;

  for(int i =0; i < count; i++) {
    EQMSG("data", (char)(count-i), data[i]);
  }
   
}
