// Template for a test suite.
#include <config.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"
#include <iostream>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

extern long ModuleBase;

class MiscTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MiscTests);
  CPPUNIT_TEST(ChipIdTest);
  CPPUNIT_TEST(FWDTimeTest);
  CPPUNIT_TEST(ErrorsTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAENV1x90* m_pModule;

public:
  // Construct a module in powered up condition:

  void setUp() {
    m_pModule = new CCAENV1x90(1, 0, ModuleBase);
    m_pModule->Reset();
  }
  // Delete the module to prevent resource leaks.

  void tearDown() {
    delete m_pModule;
  }
protected:
  void ChipIdTest();
  void FWDTimeTest();
  void ErrorsTest();
 
};

CPPUNIT_TEST_SUITE_REGISTRATION(MiscTests);

// Check that the chip id is of the proper form.
void
MiscTests::ChipIdTest()
{
  unsigned short nChips = m_pModule->getChipCount();

  for(unsigned short i; i < nChips; i++) {
    unsigned int Id = m_pModule->GetChipId(i);
    EQ(static_cast<unsigned int>(0x8470da0), (0xfffffff0 & Id));
  }
}
// Firmware date time... just print it out, there's not
// much we can do to verify.  Assume the year of the firmware
// is in the 21'st century.

void 
MiscTests::FWDTimeTest()
{
  unsigned short rev, day, mon, year;

  m_pModule->GetuCFirmwareInfo(rev, day, mon, year);

  cerr << "\n Revision: " << hex << rev << dec;
  cerr << " Written "     << mon << '/' << day << '/' << year+2000 << endl; 
}

// Test ability to get the error mask.  After init, I think this
// should have a 0 value:

void
MiscTests::ErrorsTest()
{
  unsigned short nChips = m_pModule->getChipCount();
  for(unsigned short i = 0; i < nChips; i++) {
    unsigned mask = m_pModule->GetChipErrors(i);
    EQ(0U, mask);
  }
}
