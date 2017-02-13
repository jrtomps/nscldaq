// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>

#define private public		// dirty friendliness.
#define protected public
#include "CDocumentedVars.h"
#undef private
#undef protected

#include "CVarList.h"

#include "RunState.h"
#include <vector>
#include <set>
#include <string>


using namespace std;

class DocVars : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocVars);
  CPPUNIT_TEST(construction);
  CPPUNIT_TEST(create1ofeach);
  CPPUNIT_TEST(createdups);
  CPPUNIT_TEST(deleteExisting);
  CPPUNIT_TEST(listAll);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CDocumentedVars* m_pVars;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter();
    m_pVars   = new CDocumentedVars(*m_pInterp);

    // things fail when the run is active, so let's make sure
    // no other prior tests did not clean up after themselves.
    RunState::getInstance()->m_state = RunState::inactive;
  }
  void tearDown() {
    delete m_pVars;
    delete m_pInterp;
  }
protected:
  void construction();
  void create1ofeach();
  void createdups();
  void deleteExisting();
  void listAll();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocVars);

/*
  When the object is constructed:
  - the state variable lists should be empty
  - The statevar token shoudl be non null.
  - The prior values map should be empty
*/

void DocVars::construction() {
  ASSERT(m_pVars->m_StateVariables.size() == 0);
  ASSERT(m_pVars->m_RunVariables.size() == 0);
  ASSERT(m_pVars->m_PriorValues.size() == 0);
  ASSERT(m_pVars->m_statevarToken);
}

/*
  Creating one of each should put an entry in each of the variable lists and one
  in the prior values map.  We're not going to try to build the variable 
  itself nor give it  an initial value
  prior value map entry should be a null pointer.
*/
void DocVars::create1ofeach()
{
  vector<CTCLObject> command;
  CTCLObject         word;
  
  // runvar george

  word = "runvar";
  command.push_back(word);
  word = "george";
  command.push_back(word);

  int status = (*m_pVars)(*m_pInterp, command);
  CPPUNIT_ASSERT_MESSAGE("add runvar OK", status == TCL_OK);

  // statevar harry

  command.clear();
  word = "statevar";
  command.push_back(word);
  word = "harry";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  CPPUNIT_ASSERT_MESSAGE("add state var OK", status == TCL_OK);

  // Now look into the state of the command object:

  CPPUNIT_ASSERT_MESSAGE("state vars", m_pVars->m_StateVariables.size() == 1);
  CPPUNIT_ASSERT_MESSAGE("run vars", m_pVars->m_RunVariables.size()   == 1);
  CPPUNIT_ASSERT_MESSAGE("prior values", m_pVars->m_PriorValues.size()    == 1);

  // The george entry of the prior values should be a null pointer.

  string* pPrior = m_pVars->m_PriorValues[string("george")];
  EQ(reinterpret_cast<string*>(0), pPrior);
}

/*
  Creating a dup will fail. For now the tests only look at the return code
  (should be TCL_ERROR), not the result.
*/
void DocVars::createdups()
{
  vector<CTCLObject> command;
  CTCLObject         word;

  word   = "runvar";
  command.push_back(word);
  word  = "george";
  command.push_back(word);

  int status = (*m_pVars)(*m_pInterp, command);
  status     = (*m_pVars)(*m_pInterp, command);

  EQMSG( " Duplicate runvar", TCL_ERROR, status);

  // runvar/statevar are in the same namespace so:

  word = "statevar";
  command[0] = word;
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG( "statevar same name as runvar.", TCL_ERROR, status);

  // Statevar should not tolerate dups either.

  word = "harry";
  command[1] = word;
  status = (*m_pVars)(*m_pInterp, command);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG( "statevar duplicate", TCL_ERROR, status);

  word = "runvar";
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG( "runvar same as statevar", TCL_ERROR, status);
}
/*
 * Ensure that:
 *  - delete existing works.
 *  - delete existing requires the right command to delete one.
 */
void DocVars::deleteExisting()
{
  CTCLObject word;
  vector<CTCLObject> command;
  // runvar george
  
  word   = "runvar";
  command.push_back(word);
  word  = "george";
  command.push_back(word);
  int status = (*m_pVars)(*m_pInterp, command);
  
  // runvar -delete george

  word = "-delete";
  command[1] = word;
  word = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("runvar -delete george", TCL_OK, status);

  // look at commando object internals:

  ASSERT(m_pVars->m_RunVariables.size() == 0);
  ASSERT(m_pVars->m_PriorValues.size() == 0);

  // Recreate george and attempt to delete it as a statevar:

  command.clear();
  word   = "runvar";
  command.push_back(word);
  word  = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("recreating runvar george", TCL_OK, status);
  
  word = "statevar";
  command[0] = word;
  word = "-delete";
  command[1] = word;
  word = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("deleting runvar as statevar", TCL_ERROR, status);
  
  word = "runvar";
  command[0] = word;
  (*m_pVars)(*m_pInterp, command); // actually kill the var.

  // Do all this with statevars too:

  command.clear();
  word   = "statevar";
  command.push_back(word);
  word  = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  
  // statevar -delete george

  word = "-delete";
  command[1] = word;
  word = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("runvar -delete george", TCL_OK, status);

  // look at commando object internals:

  ASSERT(m_pVars->m_StateVariables.size() == 0);
  ASSERT(m_pVars->m_PriorValues.size() == 0);

  // Recreate george and attempt to delete it as a runvar.

  command.clear();
  word   = "statevar";
  command.push_back(word);
  word  = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("recreating runvar george", TCL_OK, status);
  
  word = "runvar";
  command[0] = word;
  word = "-delete";
  command[1] = word;
  word = "george";
  command.push_back(word);
  status = (*m_pVars)(*m_pInterp, command);
  EQMSG("deleting statevar as runvar", TCL_ERROR, status);
  
}
/*!
   Make a set of runvars and list them all. We assume if one works both work.
*/
void DocVars::listAll()
{
  CTCLObject word;
  vector<CTCLObject> command;
  word = "runvar";
  command.push_back(word);
  word = "george";
  command.push_back(word);
  (*m_pVars)(*m_pInterp, command);

  word = "harry";
  command[1] = word;
  (*m_pVars)(*m_pInterp, command);

  word = "moe";
  command[1] = word;
  (*m_pVars)(*m_pInterp, command);

  // Construct/issue runvar -list

  int status;
  word = "-list";
  command[1] = word;
  status = (*m_pVars)(*m_pInterp, command);
  ASSERT(status == TCL_OK);

  // Get the interp result .. convert to a set of strings.

  const char* result = Tcl_GetStringResult(m_pInterp->getInterpreter());

  CTCLList list(m_pInterp);
  list.setList(result);
  StringArray lResult;
  list.Split(lResult);
  set<string> sResult;

  for (int i =0; i < lResult.size(); i++) {
    sResult.insert(lResult[i]);
  }

  // Ensure all elements are in the set.  This way it works regarless
  // of the list order;

  ASSERT(sResult.find("george") != sResult.end());
  ASSERT(sResult.find("harry") != sResult.end());
  ASSERT(sResult.find("moe") != sResult.end());
  
  // and only those items:

  ASSERT(sResult.size() == 3);
}
