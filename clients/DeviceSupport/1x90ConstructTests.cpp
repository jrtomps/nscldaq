
#include <config.h>
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"
#include <iostream>
#include <Exception.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DesignByContract;

extern unsigned long ModuleBase; // Entered by user.xG

class ConstructTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ConstructTests);
  CPPUNIT_TEST(ConstructFailures);
  CPPUNIT_TEST(GoodConstruct);
  CPPUNIT_TEST_SUITE_END();

#ifdef HAVE_STD_HEADER
  using namespace std;
#endif
private:

public:
  void setUp() {
  } 
  void tearDown() {
  }
protected:
  void ConstructFailures();
  void GoodConstruct();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConstructTests);

/*!
  Test various ways the constructor should fail:
*/

void ConstructTests::ConstructFailures()
{
  
  try {
    EXCEPTION((CCAENV1x90(21, 0, ModuleBase)),
	      Require);		// Bad slot
    EXCEPTION((CCAENV1x90(0, 0, ModuleBase)),
	      Require);		// Bad slot the other direction.
    EXCEPTION(CCAENV1x90(1, 8, ModuleBase) ,
	      CException);		// Bad crate.
    EXCEPTION((CCAENV1x90(1, 0, ModuleBase+4)),
	      string);		// Won't look like a module address wrong.
  }
  catch (Require& e) {
    cerr << "Caught require " << e << endl;
  }
  catch (Ensure& e) {
    cerr << "Caught ensure exceiption " << e << endl;
  }
  catch (DesignByContractException& e) {
    cerr << "Caught " << e << endl;
  }
  catch (string& r) {
    cerr << "String e " << r << endl;
  }
  catch (CException& rexcept) {
    cerr << "Caught nscl exception " << 
	  rexcept.ReasonText() << rexcept.WasDoing() << endl;
  }
  catch (...) {
    cerr << "Caught some exception dont know what??" << endl;
  }


}
/*!
   Test good construction.
*/
void
ConstructTests::GoodConstruct()
{
  CCAENV1x90 test(1, 0, ModuleBase);
  test.Reset();			// Ensure power up config
  //  sleep(1);
  
  ASSERT(!test.isTriggerMatching()); // Power up in trigger match mode.
}
