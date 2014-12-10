

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>

#define private public
#define protected public
#include <C894.h>
#undef protected
#undef private

using namespace std;

class C894Tests : public CppUnit::TestFixture {
  private:
    unique_ptr<C894> m_cmd;

  public:
    CPPUNIT_TEST_SUITE(C894Tests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_cmd.reset(new C894("name"));
  }
  void tearDown() {
  }
protected:
  void create_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(C894Tests);

void C894Tests::create_0 () {
  
}
