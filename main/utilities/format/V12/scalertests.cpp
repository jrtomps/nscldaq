// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingScalerItem.h>
#include <ByteBuffer.h>
#include <Deserializer.h>

#include "DebugUtils.h"

#include <time.h>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace DAQ;
using namespace DAQ::V12;

class scltests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(scltests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons_0);
  CPPUNIT_TEST(castcons_1);
//  CPPUNIT_TEST(accessors);
//  CPPUNIT_TEST(copycons);
//  CPPUNIT_TEST(tstampCons);
//  CPPUNIT_TEST(tstampCopyCons);
//  CPPUNIT_TEST(fractionalRunTime);
//  CPPUNIT_TEST(incremental);
//  CPPUNIT_TEST(setScalers_0);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void simplecons();
  void fullcons();
  void castcons_0();
  void castcons_1();
//  void accessors();
//  void copycons();
//  void tstampCons();
//  void tstampCopyCons();
//  void fractionalRunTime();
//  void incremental();
//  void setScalers_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(scltests);


// Test the simple constructor.

void scltests::simplecons() {
  CRingScalerItem s(32);

  
  EQMSG("type", PERIODIC_SCALERS, s.type());
  uint32_t zero(0);
  EQMSG("start offset", zero, s.getStartTime());
  EQMSG("end offset", zero, s.getEndTime());
  EQMSG("unix timestamp", time(NULL), s.getTimestamp());
  EQMSG("scaler count", (uint32_t)32, s.getScalerCount());
  EQMSG("incremental", true, s.isIncremental());
  EQMSG("interval divisor", (uint32_t)1, s.getTimeDivisor());
  EQMSG("scaler width", (uint32_t)32, s.getScalerWidth());
  EQMSG("scalers", std::vector<uint32_t>(32,0), s.getScalers());
  EQMSG("size", uint32_t(176), s.size());
}
// Exercise the fully specifying constructor.

void scltests::fullcons()
{
  
  vector<uint32_t> scalerValues;
  for(uint32_t i = 0; i < 32; i++) {
    scalerValues.push_back(i);
  }

  time_t now = time(NULL);
  CRingScalerItem s(1, 2, now, scalerValues, false, 2, 43);

  EQMSG("type",PERIODIC_SCALERS, s.type());
  EQMSG("start", (uint32_t)1, s.getStartTime());
  EQMSG("stop", (uint32_t)2, s.getEndTime());
  EQMSG("unix time", now, s.getTimestamp());
  EQMSG("count", (uint32_t)32, s.getScalerCount());
  EQMSG("scalers", scalerValues, s.getScalers());
  EQMSG("width", uint32_t(43), s.getScalerWidth());

}
// Exercise the 'downcast'  construction from an existing ring buffer.
//  This also exercises the getter accessors.
void scltests::castcons_0()
{

    vector<uint32_t> scalerValues;
    for(uint32_t i = 0; i < 32; i++) {
        scalerValues.push_back(i);
    }

    time_t now = time(NULL);
    CRingScalerItem s(1, 2, now, scalerValues, false, 2, 43);

    CRawRingItem item(VOID);
    s.toRawRingItem(item);

    EQMSG("type",PERIODIC_SCALERS, s.type());
    EQMSG("size", s.size(), item.size());
    EQMSG("evt tstamp", s.getEventTimestamp(), item.getEventTimestamp());
    EQMSG("source id", s.getSourceId(), item.getSourceId());

    Buffer::Deserializer<Buffer::ByteBuffer> stream(item.getBody());

    uint32_t offsetStart, offsetStop, time, divisor, count, incremental, width;

    stream >> offsetStart >> offsetStop >> time >> divisor;
    stream >> count >> incremental >> width;

    EQMSG("start", (uint32_t)1, offsetStart);
    EQMSG("stop", (uint32_t)2, offsetStop);
    EQMSG("unix time", (uint32_t)now, time);
    EQMSG("divisor", uint32_t(2), divisor);
    EQMSG("count", (uint32_t)32, count);
    EQMSG("incremental", false, (incremental == 1));
    EQMSG("width", uint32_t(43), width);

    uint32_t value;
    for (uint32_t i=0; i<count; ++i) {
        stream >> value;
        if (!stream.eof()) {
            EQMSG("value", i, value);
        } else {
            CPPUNIT_FAIL("Insufficient data in buffer");
        }
    }
}


// Exercise the 'downcast'  construction from an existing ring buffer.
//  This also exercises the getter accessors.
void scltests::castcons_1()
{

    uint32_t size = 60;
    uint32_t type = PERIODIC_SCALERS;
    uint32_t sourceId = 2345;
    uint64_t timestamp = 0x123456789;
    uint32_t offsetStart = 1;
    uint32_t offsetStop = 3;
    uint32_t time = 0xa0a0a0a0;
    uint32_t divisor = 44;
    uint32_t count = 3;
    uint32_t incremental = 1;
    uint32_t width = 20;

    Buffer::ByteBuffer buffer;
    buffer << size << type << timestamp << sourceId;
    buffer << offsetStart << offsetStop;
    buffer << time;
    buffer << divisor;
    buffer << count;
    buffer << incremental;
    buffer << width;
    buffer << uint32_t(23) << uint32_t(45) << uint32_t(67);

    V12::CRawRingItem raw(buffer);
    V12::CRingScalerItem sclr(raw);

    EQMSG("type",PERIODIC_SCALERS, sclr.type());
    EQMSG("size", uint32_t(60), sclr.size());
    EQMSG("evt tstamp", uint64_t(0x123456789), sclr.getEventTimestamp());
    EQMSG("source id", uint32_t(2345), sclr.getSourceId());
    EQMSG("start", (uint32_t)1, sclr.getStartTime());
    EQMSG("stop", (uint32_t)3, sclr.getEndTime());
    EQMSG("unix time", (time_t)0xa0a0a0a0, sclr.getTimestamp());
    EQMSG("divisor", uint32_t(44), sclr.getTimeDivisor());
    EQMSG("count", (uint32_t)3, sclr.getScalerCount());
    EQMSG("incremental", true, sclr.isIncremental());
    EQMSG("width", uint32_t(20), sclr.getScalerWidth());
    EQMSG("values", std::vector<uint32_t>({23,45,67}), sclr.getScalers());
}

// Test the setting accessors.

//void scltests::accessors()
//{
//  CRingScalerItem s(32);
//  s.setStartTime(100);
//  s.setEndTime(105);
//  for (int i = 0; i < 32; i++) {
//    s.setScaler(i, i);
//  }

//  EQ((uint32_t)100, s.getStartTime());
//  EQ((uint32_t)105, s.getEndTime());
//  for (uint32_t i = 0; i < 32; i++) {
//    EQ(i, s.getScaler(i));
//  }
//}
//// Test copy construction.

//void scltests::copycons()
//{
//  vector<uint32_t> simulatedScaler;
//  for (uint32_t i=0; i < 64; i++) {
//    simulatedScaler.push_back(i);
//  }

//  CRingScalerItem original(0, 10, 12345, simulatedScaler);
//  CRingScalerItem copy(original);

//  EQ(original.getBodySize(), copy.getBodySize());
//  _RingItem* porig = original.getItemPointer();
//  _RingItem* pcopy = copy.getItemPointer();

//  // headers must match

//  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
//  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

//  // Contents must match:


//  EQ(original.getStartTime(),   copy.getStartTime());
//  EQ(original.getEndTime(),    copy.getEndTime());
//  EQ(original.getTimestamp(),  copy .getTimestamp());
//  EQ(original.getScalerCount(), copy.getScalerCount());
//  for (uint32_t i =0; i < 64; i++) {
//    EQ(i, copy.getScaler(i));
//  }
  
//}
///*
//  Test construction of timestamped scaler items.
//*/
//void
//scltests::tstampCons()
//{
//    std::vector<uint32_t> scalers;
//    for(int i=0; i < 16; i++) {
//        scalers.push_back(i);
//    }
//    CRingScalerItem item(
//        0x1234567887654321ll, 1, 0,
//        10, 20, (time_t)1111, scalers
//    );
    
//    // Ensure we can get the body header stuff:
    
//    EQ(true, item.hasBodyHeader());
//    EQ(static_cast<uint64_t>(0x1234567887654321ll), item.getEventTimestamp());
//    EQ(static_cast<uint32_t>(1), item.getSourceId());
//    EQ(static_cast<uint32_t>(0), item.getBarrierType());
    
//    // And that we can still get the stuff from the event body:
    
//    EQ(static_cast<uint32_t>(10), item.getStartTime());
//    EQ(static_cast<uint32_t>(20), item.getEndTime());
//    EQ(static_cast<time_t>(1111), item.getTimestamp());
//    EQ(static_cast<uint32_t>(16), item.getScalerCount());
    
//    for(int i = 0; i < 16; i++) {
//        EQ(static_cast<uint32_t>(i), item.getScaler(i));
//    }
    
//    // Make sure that scaler value alterations work too:
    
//    for(int i = 0; i < 16; i++) {
//        item.setScaler(i, 100-i);
//    }
//    for (int i = 0; i < 16; i++) {
//        EQ(static_cast<uint32_t>(100-i), item.getScaler(i));
//    }
//}
///*
//  test copy construction with body headers:
//*/
//void
//scltests::tstampCopyCons()
//{
//    std::vector<uint32_t> scalers;
//    for(int i=0; i < 16; i++) {
//        scalers.push_back(i);
//    }
//    CRingScalerItem orig(
//        0x1234567887654321ll, 1, 0,
//        10, 20, (time_t)1111, scalers
//    );
//    CRingScalerItem item(orig);
    
//    // Ensure we can get the body header stuff:
    
//    EQ(true, item.hasBodyHeader());
//    EQ(static_cast<uint64_t>(0x1234567887654321ll), item.getEventTimestamp());
//    EQ(static_cast<uint32_t>(1), item.getSourceId());
//    EQ(static_cast<uint32_t>(0), item.getBarrierType());
    
//    // And that we can still get the stuff from the event body:
    
//    EQ(static_cast<uint32_t>(10), item.getStartTime());
//    EQ(static_cast<uint32_t>(20), item.getEndTime());
//    EQ(static_cast<time_t>(1111), item.getTimestamp());
//    EQ(static_cast<uint32_t>(16), item.getScalerCount());
    
//    for(int i = 0; i < 16; i++) {
//        EQ(static_cast<uint32_t>(i), item.getScaler(i));
//    }
    
//    // Make sure that scaler value alterations work too:
    
//    for(int i = 0; i < 16; i++) {
//        item.setScaler(i, 100-i);
//    }
//    for (int i = 0; i < 16; i++) {
//        EQ(static_cast<uint32_t>(100-i), item.getScaler(i));
//    }
//}
///*
//* tests for the compute*Time methods
//*/
//void
//scltests::fractionalRunTime()
//{
//    std::vector<uint32_t> scalers;
//    for(int i=0; i < 16; i++) {
//        scalers.push_back(i);
//    }
//    CRingScalerItem orig(
//        0x1234567887654321ll, 1, 0,
//        10, 20, (time_t)1111, scalers, 2
//    );
//    EQ(static_cast<float>(10.0/2.0), orig.computeStartTime());
//    EQ(static_cast<float>(20.0/2.0), orig.computeEndTime());
//}
///**
// * test isIncremental.
// */
//void
//scltests::incremental()
//{
//    std::vector<uint32_t> scalers;
//    for(int i=0; i < 16; i++) {
//        scalers.push_back(i);
//    }
//    CRingScalerItem inc(
//        0x1234567887654321ll, 1, 0,
//        10, 20, (time_t)1111, scalers, 2
//    );
//    CRingScalerItem notinc(
//        0x1234567887654321ll, 1, 0,
//        10, 20, (time_t)1111, scalers, 2, false
//    );
//    ASSERT(inc.isIncremental());
//    ASSERT(!notinc.isIncremental());
    
//}


//void scltests::setScalers_0()
//{
//    CRingScalerItem sclr(32);

//    // we only support the setting of scalers with a vector whose
//    // size is the same as what currently exist
//    vector<uint32_t> values(sclr.getScalerCount());
//    iota(begin(values), end(values), 0);

//    sclr.setScalers(values);

//    ASSERT( values == sclr.getScalers() );
//    EQ( values.size(), static_cast<size_t>(sclr.getScalerCount()) );
//}
