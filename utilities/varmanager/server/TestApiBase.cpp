// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CVarMgrApi.h>

// This is a dummy concrete class we can instantiate to test the base methods


class DummyApi : public CVarMgrApi
{
public:
    virtual void cd(const char* path = "/") {}
    virtual std::string getwd() {return "";}
    virtual void mkdir(const char* path)    {}
    virtual void rmdir(const char* path)    {}
    virtual void declare(const char* path, const char* type, const char* initial=0) {}
    virtual void set(const char* path, const char* value) {}
    virtual std::string get(const char* path) {return "";}
    virtual void defineEnum(const char* typeName, EnumValues values) {}
    virtual void defineStateMachine(const char* typeName, StateMap transitions) {}
    virtual std::vector<std::string> ls(const char* path = 0) {std::vector<std::string> result;
                                                                return result;}
    virtual std::vector<VarInfo> lsvar(const char* path = 0) {std::vector<VarInfo> result; return result;}
    virtual void rmvar(const char* path) {}

    std::pair<std::string, std::string> ivt(CVarMgrApi::StateMap map) {
        return findInvalidTransition(map);
    }
    std::string fus(CVarMgrApi::StateMap map) {
        return findUnreachableState(map);
    }
};

class APIBaseTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(APIBaseTests);
  CPPUNIT_TEST(addTransition);
  
  CPPUNIT_TEST(validMap);
  CPPUNIT_TEST(undefStatesMap);
  CPPUNIT_TEST(unreachableStateMap);
  
  
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void addTransition();
  void validMap();
  void undefStatesMap();
  void unreachableStateMap();
};

CPPUNIT_TEST_SUITE_REGISTRATION(APIBaseTests);

void APIBaseTests::addTransition() {
    CVarMgrApi::StateMap map;
    DummyApi   api;
    api.addTransition(map, "state1", "state2");
    EQ(static_cast<size_t>(1), map.size());
    EQ(static_cast<size_t>(1), map["state1"].size());
    EQ(size_t(1), map["state1"].count("state2"));
    
}

void APIBaseTests::validMap()
{
    CVarMgrApi::StateMap map;
    DummyApi api;
    api.addTransition(map, "state1", "state2");
    api.addTransition(map, "state2", "state1");
    ASSERT(api.validTransitionMap(map));
    
}

void APIBaseTests::undefStatesMap()
{
    CVarMgrApi::StateMap map;
    DummyApi api;
    api.addTransition(map, "state1", "state2");   // State 2 undefined.
    ASSERT(!api.validTransitionMap(map));
    std::pair<std::string, std::string> sb("state1", "state2");
    std::pair<std::string, std::string> is = api.ivt(map);
    EQ(sb.first, is.first);
    EQ(sb.second, is.second);
}

void APIBaseTests::unreachableStateMap()
{
    CVarMgrApi::StateMap map;
    DummyApi api;
    api.addTransition(map, "0state1", "state2");           // initial unreachable ok.
    api.addTransition(map, "state2", "0state1");
    api.addTransition(map, "state3", "0state1");          // state 3 is unreachable.
    
    ASSERT(!api.validTransitionMap(map));
    EQ(std::string("state3"), api.fus(map));
    
}