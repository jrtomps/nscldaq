// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CCESCBD8210.h>

#include <CBiRA1302CES8210.h>
#include <CCES8210Creator.h>
using namespace std;


static Warning msg("cescreator needs crate 1 in branch 0, vme 0 powered up");

static const int crate= 1;
static const int branch= 0;
static const int vme= 0;
static const int slot=16;


class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();

private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;

public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vme);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);


    // The creator does the rest.
  }
  void tearDown() {
    delete m_pSubsystem;
  
  }

protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
  CCES8210Creator c;
  CCAMACInterface* pInterface = c("-vme 0 -branch 0 -crate 1");
  CCESCBD8210*     pBranch    = dynamic_cast<CCESCBD8210*>(pInterface);
  ASSERT(pBranch);

  // The branch should have a crate... number 1:

  ASSERT(pBranch->haveCrate(1));



  delete pInterface;
}
