// Template for a test suite.


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CConfigurableObject.h>
#include <algorithm>

using namespace std;



class configtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(configtest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(addstring);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(configseveral);
  CPPUNIT_TEST(intparam);
  CPPUNIT_TEST(boolparam);
  CPPUNIT_TEST(enumparam);
  CPPUNIT_TEST_SUITE_END();


private:
  CConfigurableObject* m_pObject;
public:
  void setUp() {
    m_pObject = new CConfigurableObject("testObject");
  }
  void tearDown() {
    delete m_pObject;
  }
protected:
  void construct();
  void addstring();
  void config();
  void configseveral();
  void intparam();
  void boolparam();
  void enumparam();
};

CPPUNIT_TEST_SUITE_REGISTRATION(configtest);

void configtest::construct() {
  EQ(string("testObject"), m_pObject->getName());
}

void configtest::addstring() {
  m_pObject->addParameter("item", NULL, NULL, "initialValue");
  EQ(string("initialValue"), m_pObject->cget("item"));
}

void configtest::config() {
  m_pObject->addParameter("item", NULL, NULL);
  m_pObject->configure("item", "testString");
  EQ(string("testString"), m_pObject->cget("testString"));

}



// really tsting the array returning version of cget.


void configtest::configseveral() {
  m_pObject->addParameter("one", NULL, NULL, "one");
  m_pObject->addParameter("two", NULL, NULL, "two");
  m_pObject->addParameter("three", NULL, NULL);
  m_pObject->configure("three", "three");

  CConfigurableObject::ConfigurationArray info = m_pObject->cget();

  // Each of these items should be findable, and have the right values.
  //
  pair<string,string> one("one", "one");
  pair<string,string> two("two", "two");
  pair<string,string> three("three", "three");

  EQMSG("one",true, find(info.begin(), info.end(), one) != info.end());
  EQMSG("two", true, find(info.begin(), info.end(), two) != info.end());
  EQMSG("three", true, find(info.begin(), info.end(), three) != info.end());

}
// Integer parameters are  parameters that have CConfigurableObject::isInteger
// as the validator. There are a couple of cases
// with and without range checking.
//
void configtest::intparam()
{
  // No range checking.
  m_pObject->addParameter("int", CConfigurableObject::isInteger, NULL, "0");
  CConfigurableObject::Limits range(CConfigurableObject::limit(0),
				    CConfigurableObject::limit(10));
  m_pObject->addParameter("zerototen", CConfigurableObject::isInteger, &range, "0");

  // Legal configuratiuon of int:

  bool thrown(false);
  try {
    m_pObject->configure("int", "1234");
  }
  catch(string msg) {
    thrown = true;
  }
  EQMSG("legal", false, thrown);

  // Illegal configuration of unconstrained int:

  try {
    m_pObject->configure("int", "justwrong");
  } 
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Illegal", true, thrown);
  thrown = false;

  // Legal sets of constrained int:

  try {
    m_pObject->configure("zerototen", "0");
    m_pObject->configure("zerototen", "10");
    m_pObject->configure("zerototen", "5");
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Legally constrained", false, thrown);

  // Illegal value for constrained int:

  try {
    m_pObject->configure("zerototen", "11"); // it  doesn't go to eleven ;-)
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Illegal range", true, thrown);

}

// bool parameters have the isBool validator.  This takes noparameters but
// accepts as true:
//    true, yes, 1 on, enabled
// and as false:
//    false, no, 0, off, disabled
//
void configtest::boolparam()
{

  // Good values;


  vector<string> good;
  good.push_back("true");  // True values:
  good.push_back("yes");
  good.push_back("1");
  good.push_back("on");
  good.push_back("enabled");

  good.push_back("false"); // False values.
  good.push_back("no");
  good.push_back("0");
  good.push_back("off");
  good.push_back("disabled");

  m_pObject->addParameter("bool", CConfigurableObject::isBool, NULL, "1");

  // legal values:

  string errormessage("Did not throw");
  bool thrown(false);
  try {
    for (int i=0; i < good.size(); i++) {
      m_pObject->configure("bool", good[i]);
    }
  }
  catch (string msg) {
    thrown = true;
    errormessage = msg;
  }
  EQMSG(errormessage, false, thrown);

  try {
    m_pObject->configure("bool", "junk");
  }
  catch (string msg) {
    thrown = true;
  }
  ASSERT(thrown);
}


void configtest::enumparam()
{
}
