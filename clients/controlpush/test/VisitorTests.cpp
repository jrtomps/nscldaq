// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CChannel.h>
#include <CNoUnitsException.h>
#include <CAllOk.h>
#include <CSGBuilder.h>
#include <CSingleUpdater.h>
#include <CLookupVisitor.h>
#include <CBuildChannelData.h>
#include <string>
#include <iostream>
#include <time.h>

class VisitorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VisitorTests);
  CPPUNIT_TEST(AllOkTest);	// Should reset update failure.
  CPPUNIT_TEST(SGBuilderTest);	// Test the channel group builder.
  CPPUNIT_TEST(UpdaterTest);	// Test the single updater.
  CPPUNIT_TEST(LookupTest);	// Test the lookup visitor.
  CPPUNIT_TEST(GetDataTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void AllOkTest();
  void SGBuilderTest();
  void UpdaterTest();
  void LookupTest();
  void GetDataTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VisitorTests);

// Make a good channel...the lookup should succed after
// which the channel's state will be:
//    LookedUp
//

const string GoodName("Z002DH"); // Good name but no units.
const string BadName("NOSUCHNAME");  // Bad name (no such channel.
const string RealName("Z001DV");     // Good name with units 
//
// 1. Create a channel:  Realname
// 2. Fail it.
// 3. Ensure that state is FailedUpdate
// 4. Pass to an CAllOk visitor..
// Expected Results:
//  channel state == Updated.
//
void
VisitorTests::AllOkTest()
{
  CChannel c(RealName);
  c.Lookup();

  c.FailUpdate();
  EQ(c.GetState(), CChannel::FailedUpdate);

  CAllOk v;
  v(&c);

  EQ(c.GetState(), CChannel::Updated);
}
//
// 1. Create a pair of good channels.
// 2. Create a CSGBuilder and have it visit both channels.
// 3. ca_sg_block()
// Expected:  status from ca_sg_block is normal.
// 4. Visit with All ok visitor.
// Expected:
//        Both channels are updated.
//        Both channels have values and units fields != "-not-updated-"
//
void
VisitorTests::SGBuilderTest()
{
  CChannel c1(RealName);
  CChannel c2(GoodName);
  c1.Lookup();
  c2.Lookup();


  CA_SYNC_GID gid;
  int status = ca_sg_create(&gid);
  EQ(status, (int)ECA_NORMAL);

  CSGBuilder builder(gid);

  builder(&c1);
  builder(&c2);

  status = ca_sg_block(gid, 2.0); // Wait for completion..
  EQ(status, (int)ECA_NORMAL);

  CAllOk ok;
  ok(&c1);
  ok(&c2);

  EQ(c1.GetState(), CChannel::Updated);
  EQ(c2.GetState(), CChannel::Updated);

  ASSERT(c1.GetValue() != string("-not-updated-"));
  ASSERT(c1.GetUnits() != string("-not-updated-"));
  ASSERT(c2.GetValue() != string("-not-updated-"));
  ASSERT(c2.GetUnits() != string("-not-updated-"));
  
  status = ca_sg_delete(gid);
  EQ(status, (int) ECA_NORMAL);
}
//
// 1. Create a pair of good channels.
// 2. Visit them with a CSingleUpdater.
// Expected Results:
//   -  No exceptions thrown.
//   -  Both channels are in the Updated state.
//   -  Both channels have units and value strings != "-not-updated-".
//
void 
VisitorTests::UpdaterTest()
{
  CChannel c1(RealName);
  CChannel c2(GoodName);
  c1.Lookup();
  c2.Lookup();


  CSingleUpdater v(2.0);	// Single updaters need a timeout.

  bool good = true;
  try {
    v(&c1);
    v(&c2);
  }
  catch (...) {
    good = false;
  }
  ASSERT(good);			// Should not have thrown.

  // Both channels should be updated:

  EQ(c1.GetState(), CChannel::Updated);
  EQ(c2.GetState(), CChannel::Updated);

  // Both channels have values and units that have been updated..

  ASSERT(c1.GetValue() != string("-not-updated-"));
  ASSERT(c2.GetValue() != string("-not-updated-"));
  ASSERT(c1.GetUnits() != string("-not-updated-"));
  ASSERT(c2.GetUnits() != string("-not-updated-"));
    

}
// 1. Create a pair of good channels.
// 2. Kill the first off via FailUpdate calls.
// 3. Visit both with CLookupVisitor (should revive the dead one).
//    Expected results:
//      Both channels states are Connected.
// 4. Visit both channels with a Single updater.
//    Expected Results:
//      Both channel states are Updated.
//      Both Channels have values and units that are not "-not-updated-"
void
VisitorTests::LookupTest()
{
  CChannel c1(RealName);
  CChannel c2(GoodName);
  c1.Lookup();
  c2.Lookup();


  while(c2.GetState() != CChannel::Dead) {
    c2.FailUpdate();		// This way we don't need to know the retry count.
  }

  CLookupVisitor l(2.0,5.0);	// The parameters here and on the next line are
  CSingleUpdater u(2.0);	// timeouts.

  // See about resurrecting the dead one and relooking up the live one:

  l(&c1);
  l(&c2);

  EQ(c1.GetState(), CChannel::LookedUp);
  EQ(c2.GetState(),  CChannel::LookedUp);

  // Update them...
  
  u(&c1);
  u(&c2);

  EQ(c1.GetState(), CChannel::Updated);
  EQ(c1.GetState(), CChannel::Updated);
 
  ASSERT(c1.GetValue()  != string("-not-updated-"));
  ASSERT(c2.GetValue()  != string("-not-updated-"));
  ASSERT(c1.GetUnits()  != string("-not-updated-"));
  ASSERT(c2.GetUnits()  != string("-not-updated-"));

}
// 1. Create a pair of channels.
// 2. Create a Single updater visitor.
// 3. Create a BuildChannel data visitor.
// 4. Visit the channels withthe single udpater.
// 5. Visit the channesl with the build channel data visitor.
// Expected results:
//    - Both channels are updated.
//    - the size parameter from the CBuildChannelData visitor will be 2.

//    - Iterating through the data from the channels shows:
//      o Both channels give names that match what we put in.
//      o Both channels have units strings that are non empty.
//      o Both channels have units and data strings that are
//        != "-not-updated-"
//      o The update time on both units is good within a second of when
//        we visited them with the Single updater.
//
void
VisitorTests::GetDataTest()
{
  CChannel c1(RealName);
  CChannel c2(GoodName);
  c1.Lookup();
  c2.Lookup();

  string   names[2] = {RealName, GoodName};

  CSingleUpdater    u(2.0);
  CBuildChannelData b;

  // Update the channels:
  
  u(&c1);
  u(&c2);

  // Get their data:

  time_t UpdateTime = time(NULL);

  b(&c1);
  b(&c2);

  // Check that we got what's expected:

  // Both are updated:

  EQ(c1.GetState(), CChannel::Updated);
  EQ(c2.GetState(), CChannel::Updated);

  //   I built 2 things:

  EQ(2, b.size());

  // I can iterate to get the data out:

  CBuildChannelData::ChannelItemIterator p = b.begin();

  int i = 0; 
  while(p != b.end()) {
    string Name = p->first;
    string Value= p->second.m_sValue;
    string Units= p->second.m_sUnits;
    time_t t    = p->second.m_Updated;

    EQ(Name, names[i]);
    ASSERT(Units != string(""));
    
    ASSERT(Value != string("-not-updated-"));
    ASSERT(Units != string("-not-updated-"));
    ASSERT((t - UpdateTime) <= 1);
    
    
    i++;
    p++;
  }
  

}
