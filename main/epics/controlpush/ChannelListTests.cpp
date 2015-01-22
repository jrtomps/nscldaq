// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CChannel.h>
#include <CChannelList.h>
#include "CChannelVisitor.h"
#include <string>
#include <iostream>
#include <time.h>


using namespace std;


class ChannelListTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ChannelListTests);
  CPPUNIT_TEST(StockTest);	// Test ability to build list.
  CPPUNIT_TEST(IterationTest);	// Test veracity of the iterators.
  CPPUNIT_TEST(ForeachTest);	// Test visitation rights.
  CPPUNIT_TEST_SUITE_END();


private:
  void Stock(CChannelList& rList);	// Fill the channel list (common).
  void CheckChannels(CChannelList& rList,
		     time_t UpdateTime); // Check channels look ok.
public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void StockTest();
  void IterationTest();
  void ForeachTest();
  void Update1Test();
  void UpdateGroupTest();
  void UpdateTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChannelListTests);



const string GoodName("Z002DH"); // Good name but no units.
const string BadName("NOSUCHNAME");  // Bad name (no such channel.
const string RealName("Z001DV");     // Good name with units 

const char* Names[] = {
  "Z002DH",
  "Z001DV",
  "STRCHAN50",
  "STRCHAN40",
  "FLTCHAN59",
  "FLTCHAN67",
  "FLTCHAN70"
};
const int nNames=(sizeof(Names)/sizeof(char*));
//
// Common code to stock a channel list with the channels named in names.
//
void
ChannelListTests::Stock(CChannelList& l)
{
  for(int i =0; i < nNames; i++) {
    // The lookup below ensures that regardless of what we do to 
    // the channel it will be destroyable.
    //
    CChannel* p = new CChannel(string(Names[i]));
    p->Connect();
    l.push_back(*p);
  }
  CChannel::doEvents(0.5);
}


//  Stock a list channel list:
//  Expected results:
//    size == nNames.
//    begin != end.
//    Can iterate nNames times.
//
void
ChannelListTests::StockTest()
{
  CChannelList clist;
  Stock(clist);

  EQ(nNames, clist.size());
  ASSERT(clist.begin() != clist.end());

  CChannelList::ChannelIterator p = clist.begin();
  int i = 0;
  while (p != clist.end()) {
    p++;
    i++;
  }
  EQ(nNames, i);
}
//  Stock a channel list..
//  Iterate through requiring that:
//  names of the channels are the same as those put in (order preserved!).
//
void
ChannelListTests::IterationTest()
{
  CChannelList clist;
  Stock(clist);

  CChannelList::ChannelIterator  p = clist.begin();
  int i = 0;
  while(p != clist.end()) {
    EQ(string(Names[i]), (*p)->getName());

    p++;
    i++;
  }
}
//
/// The class below is a simple channel list iterator... it ensures
// all channels visited match  the ones put in in the same order.
//
class CNameCheckVisitor : public CChannelVisitor
{
 private:
  int i;
 public:
  CNameCheckVisitor() : i(0) {}
  virtual void operator()(CChannel* p) {
    EQ(string(Names[i]), p->getName());
    i++;
  }
};

// Check that channel list foreach visits channels in the order in 
// which they are put in and visits all of them:

void
ChannelListTests::ForeachTest()
{
  CChannelList clist;
  Stock(clist);
  CNameCheckVisitor v;
  clist.foreach(v);

}


