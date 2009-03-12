// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>

#define private public		// dirty friendliness.
#include "CDocumentedVars.h"
#undef private

#include "RunState.h"
#include <vector>


using namespace std;

class DocVars : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocVars);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CDocumentedVars* m_pVars;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter();
    m_pVars   = new CDocumentedVars(*m_pInterp);
  }
  void tearDown() {
    delete m_pVars;
    delete m_pInterp;
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocVars);

void DocVars::aTest() {
}
