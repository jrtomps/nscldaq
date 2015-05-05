// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// We want internal access to CVariableDb... by now it's tested..

#define private public
#include "CVariableDb.h"
#undef private

#include "CStateMachineType.h"
#include "CStateMachineTypeCreator.h"
#include "CStateMachineTypeFamilyHandler.h"
#include "CEnumeration.h"
#include "CVariable.h"
#include "CTypeFactory.h"
#include "CStateMachine.h"

#include <CSqlite.h>
#include <CSqliteStatement.h>

#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <unistd.h>


class StateMachineTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StateMachineTests);
  CPPUNIT_TEST(schema);
  
  // Tests for CStateMachineType:
  
  CPPUNIT_TEST(undefLegal);
  CPPUNIT_TEST(undefIllegal);
  CPPUNIT_TEST(legalTransition);
  CPPUNIT_TEST(illegalTransition);
  
  // Tests for CStateMachineTypeCreator
  
  CPPUNIT_TEST(constructOk);
  CPPUNIT_TEST(constructFail);
  CPPUNIT_TEST(createType);
  
  // Tests for the CStateMachineTypeFamilyHandler
  
  CPPUNIT_TEST(handlerMatch);
  CPPUNIT_TEST(handlerNoMatch);

  
  // Tests for CStateMachine::addTransition.
  
  CPPUNIT_TEST(addTransition);
  
  // Tests for CStateMachine::create
  
  CPPUNIT_TEST(addTypeOk);
  CPPUNIT_TEST(addTypeExists);
  CPPUNIT_TEST(badTypeToStates);
  
  // Tests for CStateMachine::isStateMachine
  
  CPPUNIT_TEST(isStateMachine);
  CPPUNIT_TEST(isNotStateMachine);
  CPPUNIT_TEST(isStateMachineNoSuch);
  
  // Tests for validNextStates:
  
  CPPUNIT_TEST(nextStatesOkType);
  CPPUNIT_TEST(nextStatesNoFrom);
  CPPUNIT_TEST(nextStatesNotStateMachine);
  
  CPPUNIT_TEST(nextStatesOkVar);   //  Built on the above so tests for errs not needed.
  
  // Tests for get transition map.
  
  CPPUNIT_TEST(getTransitionMapOk);
  
  CPPUNIT_TEST_SUITE_END();


private:
    char m_tempFile[100];
    int  m_fd;
    CVariableDb* m_db;
    
public:
  void setUp() {
    strcpy(m_tempFile, "testvardbXXXXXX");
    m_fd = mkstemp(m_tempFile);
    if(m_fd == -1) {
        throw std::runtime_error(strerror(errno));
    }
    CVariableDb::create(m_tempFile);
    m_db = new CVariableDb(m_tempFile);
  }
  void tearDown() {
    delete m_db;
    close(m_fd);
    unlink(m_tempFile);
  }
protected:
    void schema();
    
    void undefLegal();
    void undefIllegal();
    void legalTransition();
    void illegalTransition();
    
    void constructOk();
    void constructFail();
    void createType();
    
    void handlerMatch();
    void handlerNoMatch();
    
    void addTransition();
    
    void addTypeOk();
    void addTypeExists();
    void badTypeToStates();
    
    void isStateMachine();
    void isNotStateMachine();
    void isStateMachineNoSuch();
    
    void nextStatesOkType();
    void nextStatesNoFrom();
    void nextStatesNotStateMachine();
    
    void nextStatesOkVar();
    
    void getTransitionMapOk();
private:
    int makeStateMachine();
    void defineTransition(int typeId, const char* from, const char* to);
};


CPPUNIT_TEST_SUITE_REGISTRATION(StateMachineTests);

/*------------------------------------------------------------------------
 * Utility functions:
 */

/*
 * Use raw database operations to define a transition (e.g. make
 * an entry in the state_transitions table).
 * @param type  - id of the type.
 * @param from  - Initial state
 * @param to    - Allowed next state.
 */
void StateMachineTests::defineTransition(int typeId, const char* from, const char* to)
{
    int fromId = CEnumeration::getValueId(*m_db, typeId, from);
    int toId   = CEnumeration::getValueId(*m_db, typeId, to);
    
    CSqliteStatement add(
        *m_db,
        "INSERT INTO state_transitions (current_id, next_id)  \
            VALUES (?,?)"
    );
    add.bind(1, fromId);
    add.bind(2, toId);
    ++add;
}            
/*
 * Make a simple state machine by handwhich just has the state map:
 * States: {state1, state2, state3}
 * Transitios: state1 -> state2
 *              state2 -> state3
 *              state3 -> {state1, state2}
 * This is done by hand to allow testing the CStateMachineType before we
 * can actually generate state machines.
 *
 * @return - type of the enum.
 */
int StateMachineTests::makeStateMachine()
{
    // First just make an enum type with the required states:
    // state1 is the default/entry state.
    //
    std::vector<std::string> states;
    states.push_back("state1");
    states.push_back("state2");
    states.push_back("state3");
    int typeId = CEnumeration::create(*m_db, "statemachine", states);
   
    // manually enter the state map:
    
    defineTransition(typeId, "state1", "state2");
    defineTransition(typeId, "state2", "state3");
    defineTransition(typeId, "state3", "state1");
    defineTransition(typeId, "state3", "state2");
   
   return typeId;
}

/*------------------------------------------------------------------------
 * the tests:
 */

// The schema must have been created in the db file.

void StateMachineTests::schema() {
    ASSERT(m_db->tableExists("state_transitions"));
}

void StateMachineTests::undefLegal()
{
    int typeId = makeStateMachine();
    
    // only legal value is "state1"
    
    CStateMachineType t(typeId, "statemachine", *m_db);
    ASSERT(t.legal("state1", -1));               
}

void StateMachineTests::undefIllegal()
{
    int typeId = makeStateMachine();
    
    // something other than state1 is not legal:
    
    CStateMachineType t(typeId, "statemachine", *m_db);
    ASSERT(!t.legal("state2", -1));
}

// test that all legal transitions of a variable are legal.
// Note that for this test the state variable might be an enum or a
// state machine so when we modify its values we need to follow the
// legal transitions...which it is depends no how far along in dev.
// we are.
void StateMachineTests::legalTransition()
{
    int typeId = makeStateMachine();
    CVariable* pVar = CVariable::create(*m_db,  "/mymachine", "statemachine" );
    
    // Value is state
    
    CStateMachineType t(typeId, "statemachine", *m_db);
    int varid = pVar->getId();
    ASSERT(t.legal("state2", varid));
    
    pVar->set("state2");
    ASSERT(t.legal("state3", varid));
    
    pVar->set("state3");
    ASSERT(t.legal("state2", varid));
    ASSERT(t.legal("state1", varid));
    
    
    delete pVar;
}
/**
 *  Test that all illegal transitions are illegal:
 */
void StateMachineTests::illegalTransition()
{
    int typeId = makeStateMachine();
    CVariable* pVar = CVariable::create(*m_db,  "/mymachine", "statemachine" );
    
    // Value is state
    
    CStateMachineType t(typeId, "statemachine", *m_db);
    int varid = pVar->getId();
 
    // State 1.
 
    ASSERT(!(t.legal("state3", varid)));
    ASSERT(!(t.legal("state1", varid)));
    
    pVar->set("state2");
    ASSERT(!(t.legal("state1", varid)));
    ASSERT(!(t.legal("state2", varid)));
    
    pVar->set("state3");
    ASSERT(!t.legal("state3", varid));
 }
 // Create a state machine creator with a statemachine:
 
 void StateMachineTests::constructOk()
 {
    int typeId = makeStateMachine();
    
    CPPUNIT_ASSERT_NO_THROW(CStateMachineTypeCreator sm(*m_db, "statemachine"));
 }
 // Create a state machine with an unconstrained enum -- should throw std::runtime_error
 
 void StateMachineTests::constructFail()
 {
    std::vector<std::string> enumValues;
    enumValues.push_back(std::string("state1"));
    enumValues.push_back(std::string("state2"));
    enumValues.push_back(std::string("state3"));
    
    CEnumeration::create(*m_db, "not-a-statem-machine", enumValues);
    
    CPPUNIT_ASSERT_THROW(
        CStateMachineTypeCreator sm(*m_db, "not-a-statem-machine"),
        std::runtime_error
    );
 }
 
 // Ensure that the type that we get from this behaves like a
 // state machine checker.

 
 void StateMachineTests::createType()
 {
    int typeId = makeStateMachine();
    CStateMachineTypeCreator sm(*m_db, "statemachine");
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "statemachine"); // state1.
    int varid       = pVar->getId();
    
    CDataType* pType = sm();
    ASSERT(pType->legal("state2", varid));
    ASSERT(!(pType->legal("state3", varid)));
    
    delete pType;
    delete pVar;
    
 }
 // The statemachine type family handler should be able to create me a
 // type for a registered state machine.  Note we don't assume the
 // factory has a registration yet.
 
 void StateMachineTests::handlerMatch()
 {
    int                            typeId     = makeStateMachine();
    CTypeFactory                   f(*m_db);
    CStateMachineTypeFamilyHandler h(*m_db);
    
    CDataType* pDataType = h.create("statemachine", *m_db, f);
    ASSERT(pDataType);
    delete pDataType;
 }
 
 void StateMachineTests::handlerNoMatch()
 {
 CTypeFactory                   f(*m_db);
    CStateMachineTypeFamilyHandler h(*m_db);
    
    // Make an enum that is not a statemachine - the most stringent test.
    
    std::vector<std::string> states;
    states.push_back("first");
    states.push_back("second");
    states.push_back("third");
    CEnumeration::create(*m_db, "statemachine", states);
    
    CDataType* pDataType = h.create("statemachine", *m_db, f);
    ASSERT(!pDataType);   
 }
 
 // Should be able to add states to a transition map:
 // To test this we need to know a transition map is a map of sets.
 void StateMachineTests::addTransition()
 {
    CStateMachine::TransitionMap stateMap;
    CStateMachine::addTransition(stateMap, "from", "to");
    
    // From state must now exist:
    
    CStateMachine::TransitionMap::iterator p = stateMap.find("from");
    ASSERT(p != stateMap.end());
    
    // to state must be in it's transition set.
    
    std::set<std::string>::iterator pt = p->second.find("to");
    ASSERT(pt != p->second.end());
    
 }
 //  Add a state machine that works ok.
 
 void StateMachineTests::addTypeOk()
 {
    // Make a transition map for a statemachine like makeStateMachine is:
    
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);
    
    // If this all worked, We should be able to create a state machine variable:
    
    CVariable* pVar;
    
    CPPUNIT_ASSERT_NO_THROW(
        pVar = CVariable::create(*m_db, "/test", "statemachine")
    );
    // It's only a state machine if it enforces the state transition map:
    // *pVar is in state 'first' and should not be settable to 'third'.
    CPPUNIT_ASSERT_THROW(
        pVar->set("third"),
        std::runtime_error
    );
    
    delete pVar;
 }
 
 // If the type exists CStateMachine::create throws a CStateMachine::CException.
 
 void StateMachineTests::addTypeExists()
 {
     // Make a transition map for a statemachine like makeStateMachine is:
    
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    CPPUNIT_ASSERT_THROW(
        CStateMachine::create(*m_db, "integer", transitions),
        CStateMachine::CException
    );
 }
 // If a to state is not defined,
 // - An exception should be thrown.
 // - The new type should not be in the variable_types table due to a rollback.
 //
 void StateMachineTests::badTypeToStates()
 {
     // Make a transition map for a statemachine like makeStateMachine is:
    
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "second", "frist"); // bad  -mispelled first.
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    CPPUNIT_ASSERT_THROW(
        CStateMachine::create(*m_db, "statemachine", transitions),
        CStateMachine::CException
    );
    
    // In addition there should be no trace of the type in the type tables:
    // We look directly since the type could be in the table but the factory
    // unable to build a CDataType for it.
    CSqliteStatement s(
        *m_db,
        "SELECT COUNT(*) FROM variable_types WHERE type_name='statemachine'"
    );
    ++s;
    EQ(0, s.getInt(0));
       
 }
 //  state machine checker
 
 void StateMachineTests::isStateMachine()
 {
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);
    
    ASSERT(CStateMachine::isStateMachine(*m_db, typeId));
 }
 // Enumerations are not state machines:
 
 void StateMachineTests::isNotStateMachine()
 {
    std::vector<std::string> states;
    states.push_back("first");
    states.push_back("second");
    states.push_back("third");
    
    int typeId = CEnumeration::create(*m_db, "notStateMachine", states);
    ASSERT(!CStateMachine::isStateMachine(*m_db, typeId));
 }
 void StateMachineTests::isStateMachineNoSuch()
 {
    CPPUNIT_ASSERT_THROW(
        CStateMachine::isStateMachine(*m_db, 12345),
        CStateMachine::CException
    );
 }
 // I can get the set of state transitions for a type given a from state:
 
 void StateMachineTests::nextStatesOkType()
 {
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);
    
    std::vector<std::string> nextStates = CStateMachine::validNextStates(*m_db, typeId, "third");
    EQ(static_cast<size_t>(2), nextStates.size());
    
    // The states will come out alphabetically:
    
    EQ(std::string("first"), nextStates[0]);
    EQ(std::string("second"), nextStates[1]);
    
 }
 // next states with invalid from state should throw an exception.
 
 void StateMachineTests::nextStatesNoFrom()
 {
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);
    CPPUNIT_ASSERT_THROW(
        CStateMachine::validNextStates(*m_db, typeId, "nope"),
        CStateMachine::CException
    );
 }
 // Next states with a type that is not a state machine:
 
 void StateMachineTests::nextStatesNotStateMachine()
 {
    std::vector<std::string> states;
    states.push_back("first");
    states.push_back("second");
    states.push_back("third");
    
    int typeId = CEnumeration::create(*m_db, "notsm", states);
    CPPUNIT_ASSERT_THROW(
        CStateMachine::validNextStates(*m_db, typeId, "first"),
        CStateMachine::CException
    );
 }
 // Get the valid next states for a variable:
 
void StateMachineTests::nextStatesOkVar()
{
    // Make the statemachine type:
    //
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);

    // Create a variable and step it to the third state    
    
    CVariable* pVar = CVariable::create(*m_db, "/machine", "statemachine");
    pVar->set("second");
    pVar->set("third");
    
    // Tests are the same as for type but with the variable id passed:
    
    std::vector<std::string> nextStates = CStateMachine::validNextStates(
        *m_db, pVar->getId()
    );
    EQ(static_cast<size_t>(2), nextStates.size());
    
    // The states will come out alphabetically:
    
    EQ(std::string("first"), nextStates[0]);
    EQ(std::string("second"), nextStates[1]);
}
// Rebuild the transition map from the type definition.

void StateMachineTests::getTransitionMapOk()
{
    // Make the statemachine type:
    //
    CStateMachine::TransitionMap  transitions;
    CStateMachine::addTransition(transitions, "first", "second");
    CStateMachine::addTransition(transitions, "second", "third");
    CStateMachine::addTransition(transitions, "third", "first");
    CStateMachine::addTransition(transitions, "third", "second");
    
    int typeId = CStateMachine::create(*m_db, "statemachine", transitions);
    
    CStateMachine::TransitionMap rebuilt = CStateMachine::getTransitionMap(
        *m_db, typeId
    );
    ASSERT(transitions == rebuilt);
}