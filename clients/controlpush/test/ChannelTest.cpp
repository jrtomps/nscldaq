// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CChannel.h>
#include <CNoUnitsException.h>
#include <string>
#include <iostream>
#include <time.h>

class ChannelTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ChannelTest);
  CPPUNIT_TEST(Failure);	// Should be able to force into failure..
  CPPUNIT_TEST(MakeGood);	// Make a good channel...
  CPPUNIT_TEST(MakeBad);	// Make a bad channel...
  CPPUNIT_TEST(HaveUnits);	// Check that I can get units from a channel.
  CPPUNIT_TEST(SingleUpdate);	// Try to do a single update..
  CPPUNIT_TEST(NoUnits);	// Single update a channel with no units.
  CPPUNIT_TEST(LastUpdated);	// Test last update time
  CPPUNIT_TEST(SGUpdate);	// Update a synchronous group...
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void MakeGood();
  void MakeBad();
  void HaveUnits();
  void SingleUpdate();
  void NoUnits();
  void LastUpdated();
  void SGUpdate();
  void Failure();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChannelTest);

// Make a good channel...the lookup should succed after
// which the channel's state will be:
//    LookedUp
//

const string GoodName("STRCHAN100"); // Good name but no units.
const string BadName("NOSUCHNAME");  // Bad name (no such channel.
const string RealName("Z001DV");     // Good name with units 
void 
ChannelTest::MakeGood()
{
  CChannel good(GoodName);
  good.Lookup();

  EQ(good.GetState(), CChannel::LookedUp);
}

void 
ChannelTest::MakeBad()
{
  CChannel bad(BadName); 
  bad.Lookup();
  EQ(bad.GetState(), CChannel::Dead); // Bad name...

}
//
// Check that have units and getunits works..
//
void
ChannelTest::HaveUnits()
{
  CChannel units(RealName);
  units.Lookup();

  ASSERT(units.HaveUnits());

  bool good = true;
  try {
    cerr << units.GetUnits() << " Was the units Ok! " << endl;
  }
  catch(CNoUnitsException except) {
    good = false;		// Should not throw if units..
  }
  // With goodname we should get the throw:
  CChannel noUnits(GoodName);
  noUnits.Lookup();
  ASSERT(!noUnits.HaveUnits());
  good = false;
  try {
    noUnits.GetUnits();
  } 
  catch (CNoUnitsException except) {
    good = true;
    cerr << " OK! exception caught said: " << except.ReasonText() << endl;
  }
  ASSERT(good);
  
}
//
// Do a single shot update for a channel:
//
void 
ChannelTest::SingleUpdate()
{
  CChannel units(RealName);
  units.Lookup();

  units.Get();
  int status = ca_pend_io(2.0);	// This should be normal...
  EQ(status, (int)ECA_NORMAL);

  ASSERT(units.GetValue() != string("-not-updated-"));
  ASSERT(units.GetUnits() != string("-not-updated-"));

  cerr << "OK! " << RealName << " = " << units.GetValue() << units.GetUnits()
       << endl;
}
// Try a channel with no units:
//
void
ChannelTest::NoUnits()
{
  CChannel nounits(GoodName);	// Channel without associated units...
  nounits.Lookup();

  nounits.Get();		// Queue the get...
  int status = ca_pend_io(2.0);	// This should be normal:

  EQ(status, (int)ECA_NORMAL);
  ASSERT(nounits.GetValue() != string("-not-updated-"));

  // Should be no units...
  
  ASSERT(!nounits.HaveUnits());	// Should be false...
  
  // Output the stuff.

  cerr << "Ok!  Nounits got: " << nounits.GetValue() << endl;
	 
}
// 1. Make a channel (RealName).
// 2. Do a get.
// 3. ca_pend_io to wait for channel data to come in.
// 4. time to get the time of day.
// 5. Update() to update channel state.
// Expected results: 
//  -  Last update should be within 1 second of the time we got.
//  -  State should be Updated
//
void
ChannelTest::LastUpdated()
{
  CChannel c(RealName);
  c.Lookup();

  c.Get();

  int status = ca_pend_io(2.0);
  EQ(status, (int)ECA_NORMAL);

  time_t t = time(NULL);
  c.Update();
  ASSERT((c.GetUpdateTime() - t) <= 1);
  ASSERT(c.GetState() == CChannel::Updated);
}
// 1. Create a synch group.
// 2. Create a pair of channels RealName, GoodName.
// 3. Call both channels AddGetToSg().
// 4. Wait for sg done
// Expected results:
//   - Return from wait is normal.
//   - Update on both channels leaves them in updated state.
//   - Return values can be gotten that are not "-not-updated-".
//
void
ChannelTest::SGUpdate()
{
  CChannel c1(RealName);
  CChannel c2(GoodName);
  c1.Lookup();
  c2.Lookup();


  CA_SYNC_GID gid;
  int status = ca_sg_create(&gid);
  EQ(status, (int)ECA_NORMAL);

  c1.AddGetToSg(gid);
  c2.AddGetToSg(gid);

  status = ca_sg_block(gid, 2.0); // Wait for completion.
  EQ(status, (int)ECA_NORMAL);

  c1.Update();
  c2.Update();
  ASSERT(c1.GetState() == CChannel::Updated);
  ASSERT(c1.GetValue() != string("-not-updated-"));
  ASSERT(c2.GetState() == CChannel::Updated);
  ASSERT(c2.GetValue() != string("-not-updated-"));

  cerr << "OK!! " << RealName << " = " << c1.GetValue()
       << c1.GetUnits() << endl;
  cerr << "and " << GoodName << " = " << c2.GetValue() << endl;
}
// 1. Create a channel.
// 2. Fail 3 times.
// Expected results:
//   Channel state == dead.
// 3. Create a channel.
// 4. Fail once.
// 5. Update once.
// 6. Fail twice
// Expected results:
//    Channel state == FailedUpdate
// 7. Fail 3'd time.
// Expected results:
//   Chanel state == dead.
//
void
ChannelTest::Failure()
{
  // Test straightforward failure.
  {
    CChannel c1(RealName);	// Create a channel...
    c1.Lookup();

    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::FailedUpdate);
    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::FailedUpdate);
    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::Dead);
  }
  //  Test retry count reset on update.
  
  {
    CChannel c1(RealName);
    c1.Lookup();

    c1.FailUpdate();
    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::FailedUpdate);
    c1.Update();
    ASSERT(c1.GetState() == CChannel::Updated);
    
    // Now need 3 failures again to mark dead.

    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::FailedUpdate);
    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::FailedUpdate);
    c1.FailUpdate();
    ASSERT(c1.GetState() == CChannel::Dead);
  }
    
}
