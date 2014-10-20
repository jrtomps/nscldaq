// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CFragmentHandler.h"
#include "CDuplicateTimeStatCommand.h"
#undef private

#include "TCLInterpreter.h"
#include "TCLObject.h"
#include <tcl.h>
#include "fragment.h"



class DupStatCmdTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DupStatCmdTests);
  CPPUNIT_TEST(cmdregisters);
  CPPUNIT_TEST(needsubcommand);
  CPPUNIT_TEST(badsubcommand);
  CPPUNIT_TEST(getnoextras);
  CPPUNIT_TEST(getempty);
  CPPUNIT_TEST(getonedup);
  CPPUNIT_TEST(getmixeddups);
  CPPUNIT_TEST(clearnoextras);
  CPPUNIT_TEST(clear);
  CPPUNIT_TEST_SUITE_END();


private:
    CTCLInterpreter*  m_pInterp;
    CFragmentHandler* m_pOrderer;
    Tcl_Interp*       m_pNativeInterp;
public:
  void setUp() {
    m_pInterp = new CTCLInterpreter;
    m_pOrderer= new CFragmentHandler;
    m_pNativeInterp = m_pInterp->getInterpreter();
  }
  void tearDown() {
    delete m_pOrderer;
    delete m_pInterp;
  }
protected:
  void cmdregisters();
  void needsubcommand();
  void badsubcommand();
  void getnoextras();
  void getempty();
  void getonedup();
  void getmixeddups();
  void clearnoextras();
  void clear();
private:
    void registerCmd();
    void testBadCommand(const char* cmd);
    void checkDetails(int src, int count, int ts, CTCLObject element);
};

CPPUNIT_TEST_SUITE_REGISTRATION(DupStatCmdTests);

// Helper to register the command.
// Note that this leaks command processors but we don't care that much as this
// is test code, and those leaks will get recovered when we exit.

void
DupStatCmdTests::registerCmd()
{
    new CDuplicateTimeStatCommand(*m_pInterp, "dupstat");
}

//Helper to test a command that should return a failure.
//
void DupStatCmdTests::testBadCommand(const char* cmd)
{
    registerCmd();
    Tcl_Interp* pInterp = m_pNativeInterp;
    int stat = Tcl_Eval(pInterp, cmd);
    EQ(TCL_ERROR, stat);    
}

// Helper to check that a detailed information list is correct.
void DupStatCmdTests::checkDetails(int src, int count, int ts, CTCLObject element)
{
    element.Bind(m_pInterp);
    EQ(3, element.llength());
    EQ(src, (int)(element.lindex(0)));
    EQ(count, (int)(element.lindex(1)));
    EQ(ts, (int)(element.lindex(2)));
}

// The command should be visible:

void DupStatCmdTests::cmdregisters() {
    registerCmd();
    Tcl_CmdInfo info;
    int status = Tcl_GetCommandInfo(m_pInterp->getInterpreter(), "dupstat", &info);
    EQ(1, status);   // Status is not TCL_OK for this :-()
}

// The command should require a subcommand.

void DupStatCmdTests::needsubcommand()
{
    testBadCommand("dupstat");
}


// The command should not accept a bad subcommand

void DupStatCmdTests::badsubcommand()
{
    testBadCommand("dupstat bad");
}

// The get command should not allow extra parameters.

void DupStatCmdTests::getnoextras(
)
{
    testBadCommand("dupstat get extra");
}


// The get command should give some good statistics:

// Get with no dups:

void DupStatCmdTests::getempty()
{
    registerCmd();
    
    int stat = Tcl_Eval(m_pNativeInterp, "dupstat get");
    EQ(TCL_OK, stat);
    
    CTCLObject result(Tcl_GetObjResult(m_pNativeInterp));
    result.Bind(m_pInterp);
    
    EQ(2, result.llength());
    EQ(0, (int)(result.lindex(0)));
    EQ(std::string(""), std::string(result.lindex(1)));
}


// Get onedup in one source

void DupStatCmdTests::getonedup()
{
    registerCmd();
    
    // Send a set of fragments that result in one duplicat on source id 1:
    
    EVB::FlatFragment frag;
    frag.s_header.s_timestamp = 100;              // nonzero is the key.
    frag.s_header.s_sourceId  = 1;
    frag.s_header.s_size      = 0;
    frag.s_header.s_barrier   = 0;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    frag.s_header.s_timestamp = 110;              // not duplicate
    m_pOrderer->addFragments(sizeof(frag), &frag);
    m_pOrderer->addFragments(sizeof(frag), &frag); // duplicate
    
    frag.s_header.s_timestamp = 111;             // not duplicate
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    // Flush the queue as that's when duplicates are detected:
    
    m_pOrderer->flushQueues();
    
    EQ(TCL_OK, Tcl_Eval(m_pNativeInterp, "dupstat get"));
    
    
    CTCLObject result(Tcl_GetObjResult(m_pNativeInterp));
    result.Bind(m_pInterp);
    
    // Evaluate the result:
    
    EQ(2, result.llength());
    EQ(1, (int)(result.lindex(0)));            // There was one dup.
    
    // List of details:
    
    CTCLObject details = result.lindex(1);
    details.Bind(m_pInterp);
    EQ(1, details.llength());              // Only one source has a duplicate.
    
    checkDetails(1, 1, 110, details.lindex(0));
    
}


// Get dups in several sources...but some sources with no dups.
// We're going to fabricate data with event sources 1,2,3
// *  1 will have 3 dups.
// *  2 will have no dups.
// *  3 will have 1 dup.
//
// Whitebox assumption:  Since the data a maintained by the observer in a
// map indexed by sourceid the list should be in order of source id.
//

void DupStatCmdTests::getmixeddups()
{
    registerCmd();
    
    // Data for source 1 ts : 100, 110, 110, 110, 110 120
    
    EVB::FlatFragment frag;
    frag.s_header.s_timestamp = 100;              // nonzero is the key.
    frag.s_header.s_sourceId  = 1;
    frag.s_header.s_size      = 0;
    frag.s_header.s_barrier   = 0;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    frag.s_header.s_timestamp = 110;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 /// dup 1
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 // dup 2
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 // dup 3
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    frag.s_header.s_timestamp = 120;                 // not a dup.
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    // Data from source 2 is 100 110 120 130 140 150
    
    frag.s_header.s_sourceId = 2;
    frag.s_header.s_timestamp = 100;                   // yeah. could be a loop.
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 120;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 130;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 140;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 150;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    // Data from source 3 is 112, 112, 120, 125, 130, 132
    
    frag.s_header.s_sourceId = 3;
    frag.s_header.s_timestamp = 112;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 112;
    m_pOrderer->addFragments(sizeof(frag), &frag);    // dup 1 (and only)
    frag.s_header.s_timestamp = 120;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 125;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 130;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 132;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    // flush the data:
    
    m_pOrderer->flushQueues();
    
    // Run the command and analyze the results:
    
    EQ(TCL_OK, Tcl_Eval(m_pNativeInterp, "dupstat get"));
    CTCLObject result(Tcl_GetObjResult(m_pNativeInterp));
    result.Bind(m_pInterp);
    
    // Should have a 2 element list, element 0 is the integer 4
    // (sum of all dups over all sources.
    
    EQ(2, result.llength());
    EQ(4, int(result.lindex(0)));
    
    // The details should be a 2 element list:
    
    CTCLObject details(result.lindex(1));
    details.Bind(m_pInterp);
    EQ(2, details.llength());
    
    checkDetails(1, 3, 110, details.lindex(0));
    checkDetails(3, 1, 112, details.lindex(1));
    
}

// The clear command should not allow extra parameters.

void DupStatCmdTests::clearnoextras()
{
    testBadCommand("dupstat clear extra");
}

// The clear command should clear statistics.

void DupStatCmdTests::clear()
{
    registerCmd();
    
    
    // Make a bunch of dups, then clear/get the stats.
    
    
    
    // Data for source 1 ts : 100, 110, 110, 110, 110 120
    
    EVB::FlatFragment frag;
    frag.s_header.s_timestamp = 100;              // nonzero is the key.
    frag.s_header.s_sourceId  = 1;
    frag.s_header.s_size      = 0;
    frag.s_header.s_barrier   = 0;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    frag.s_header.s_timestamp = 110;
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 /// dup 1
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 // dup 2
    m_pOrderer->addFragments(sizeof(frag), &frag);
    frag.s_header.s_timestamp = 110;                 // dup 3
    m_pOrderer->addFragments(sizeof(frag), &frag);
    
    frag.s_header.s_timestamp = 120;                 // not a dup.
    m_pOrderer->addFragments(sizeof(frag), &frag);
    m_pOrderer->flushQueues();
    
    EQ(TCL_OK, Tcl_Eval(m_pNativeInterp, "dupstat clear"));
    EQ(TCL_OK, Tcl_Eval(m_pNativeInterp, "dupstat get"));
    CTCLObject result(Tcl_GetObjResult(m_pNativeInterp));
    result.Bind(m_pInterp);
    
    // totals are 0, and empty list
    
    EQ(0, (int)(result.lindex(0)));
    EQ(std::string(""), (std::string)(result.lindex(1)));
    
    
    
}



void* gpTCLApplication(0);                    // make the appframework happy.
