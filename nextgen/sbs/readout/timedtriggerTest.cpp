// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <time.h>
#include <unistd.h>
#include <iostream>

#define private public
#include "CTimedTrigger.h"
#undef private
using namespace std;


static const long nsPerSec(1000*1000*1000);
static const struct timespec zero={0,0};

class timedtrig : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(timedtrig);
  CPPUNIT_TEST(normalize);
  CPPUNIT_TEST(sum);
  CPPUNIT_TEST(ge);
  CPPUNIT_TEST(eq);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(setup);
  CPPUNIT_TEST(canonicals);
  CPPUNIT_TEST(setperiod);
  CPPUNIT_TEST(trigger);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void normalize();
  void sum();
  void ge();
  void eq();
  void construct();
  void setup();
  void canonicals();
  void setperiod();
  void trigger();

};

CPPUNIT_TEST_SUITE_REGISTRATION(timedtrig);

//  Adding 0 to a time should normalize it
//  We'll do this with an already normalized time
// and one that isn't normalized.

void timedtrig::normalize() {
  struct timespec ts = {0, 100};	// this is defacto normalized.
  struct timespec result;

  // Already normalized.. no change.

  result = ts + zero;
  EQ(ts.tv_sec, result.tv_sec);
  EQ(ts.tv_nsec, result.tv_nsec);

  // Make ts un-normalized:

  ts.tv_nsec = nsPerSec;	// This is really now one second.
  result     = ts + zero;
  EQ(static_cast<time_t>(1), result.tv_sec);
  EQ(static_cast<long>(0),   result.tv_nsec);

}
// Test add without and with carries from ns -> sec.

void timedtrig::sum()
{
  struct timespec ts1 = {1, 100};
  struct timespec ts2 = {3, 400};

  // adding ts1, ts2 will not carry ns -> sec:

  struct timespec s = ts1 + ts2;
  EQ(static_cast<time_t>(4), s.tv_sec);
  EQ(static_cast<long>(500), s.tv_nsec);

  ts2.tv_nsec = nsPerSec - 1;	// forces a carry:

  s = ts1 + ts2;
  EQ(static_cast<time_t>(5), s.tv_sec);
  EQ(static_cast<long>(99), s.tv_nsec);
}
// test the >= operator for times.. normalized and otherwise.

void timedtrig::ge()
{
  struct timespec ts1 = {1,100};

  ASSERT(ts1 >= zero);
  ASSERT(ts1 >= ts1);
  ASSERT(!(zero >= ts1));

  // Now compare ts1 to a non normalized version of it.

  struct timespec ts1prime = {0, 100+nsPerSec};
  ASSERT(ts1prime >= ts1);
  ASSERT(ts1      >= ts1prime);

  ts1.tv_sec = 2;
  ASSERT(ts1 >= ts1prime);

  struct timespec ts2;

  ts1.tv_sec = 100; ts1.tv_nsec = 0;
  ts2.tv_sec = 90;  ts2.tv_nsec = 100;
  ASSERT(ts1 >= ts2);
  
  ASSERT(!(ts2 >= ts1));
 
}

// Test operator== for normalized and otherwise.

void timedtrig::eq()
{
  struct timespec ts1 = {1, 100};
  struct timespec ts2 = {1, 100};

  ASSERT(ts1 == ts2);
  ASSERT(ts2 == ts1);		// equality is reflexive.

  ts2.tv_nsec++;
  ASSERT(!(ts1 == ts2));

  ts2.tv_sec = 0;
  ts2.tv_nsec = 100+nsPerSec;
  ASSERT(ts1 == ts2);
  ASSERT(ts2 == ts1);
}
// Constrution should just set the periodicity.

void timedtrig::construct()
{
  struct timespec period = {2, 1}; // 2 second intervals...and a bit.
  CTimedTrigger   trig(period);
  ASSERT(period == trig.m_periodicity);

}
// setup should set last trigger to now and next trigger to
// now + period.
//
void timedtrig::setup()
{
  struct timespec period = {2,1};
  CTimedTrigger   trig(period);
  struct timespec now;
  struct timespec res;
  clock_getres(CLOCK_REALTIME, &res); //>> There could be one rt tick between.
  clock_gettime(CLOCK_REALTIME, &now); //>> Internals knowledge here
  trig.setup();
  ASSERT((now + res) >= trig.m_lastTrigger);
  ASSERT(trig.m_lastTrigger >= now);
  ASSERT((now + period + res) >= trig.m_nextTrigger);
  ASSERT(trig.m_nextTrigger >= now);


}

// Test the canonicals:
// - copy construction
// - assignment
// - equality compare.
// - inequality compare.

void timedtrig::canonicals()
{
  struct timespec period = {2,1};
  CTimedTrigger trig(period);
  trig.setup();			// just to get all members set

  // copy construction

  CTimedTrigger dup(trig); 	// should be the same as trig

  ASSERT(trig.m_lastTrigger == dup.m_lastTrigger);
  ASSERT(trig.m_nextTrigger == dup.m_nextTrigger);
  ASSERT(trig.m_periodicity == dup.m_periodicity);

  // Assignment:
  period.tv_sec = 1;		// Change up the period.
  CTimedTrigger lhs(period);
  lhs = trig;
  ASSERT(trig.m_lastTrigger == lhs.m_lastTrigger);
  ASSERT(trig.m_nextTrigger == lhs.m_nextTrigger);
  ASSERT(trig.m_periodicity == lhs.m_periodicity);

  // Equality comparison:

  ASSERT(trig == lhs);
  
  // Inequality comparison:

  CTimedTrigger different(period);
  ASSERT(different != trig);


}
// Setting the period should modify the period and be like a setup.
//
void timedtrig::setperiod()
{
  struct timespec period = {2,1};
  CTimedTrigger trig(period);

  period.tv_sec++;
  struct timespec now;
  struct timespec res;
  clock_getres(CLOCK_REALTIME, &res); //>> There could be one rt tick between.
  clock_gettime(CLOCK_REALTIME, &now); //>> Internals knowledge here
  trig.setPeriod(period);
  ASSERT((now + res) >= trig.m_lastTrigger);
  ASSERT(trig.m_lastTrigger >= now);
  ASSERT((now + period + res) >= trig.m_nextTrigger);
  ASSERT(trig.m_nextTrigger >= now);
  

}
// See if the module can actually fire a trigger; this test takes a few seconds
//

void timedtrig::trigger()
{
  struct timespec period={1,0};	// 1 second period.
  struct timespec sleeptime={1, 0};
  struct timespec remaining = sleeptime;

  CTimedTrigger trig(period);
  trig.setup();

  ASSERT(!trig());		// Shouldn't fire yet.
  sleep(1);
  ASSERT(trig());		// Should fire once.
  ASSERT(!trig());		// but not twice that quickly... however
  sleep(1);
  ASSERT(trig());

}
