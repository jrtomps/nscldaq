// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/CGlomParameters.h"

// Tests for glom parameter ring item class:

using namespace DAQ::V12;

class GlomItemTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GlomItemTests);
    CPPUNIT_TEST(notBuilding);
    CPPUNIT_TEST(isBuilding);
//    CPPUNIT_TEST(structure);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void notBuilding();
    void isBuilding();
//    void structure();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GlomItemTests);

// Test parameters if not building:

void
GlomItemTests::notBuilding()
{
    CGlomParameters item(0x123456, 23, static_cast<uint64_t>(1234), false, CGlomParameters::last);

    EQMSG("evt tstamp", uint64_t(0x123456), item.getEventTimestamp());
    EQMSG("source id", uint32_t(23), item.getSourceId());
    EQMSG("coinc ticks", static_cast<uint64_t>(1234), item.coincidenceTicks());
    EQMSG("building", false, item.isBuilding());
    EQMSG("policy", CGlomParameters::last, item.timestampPolicy());
}
//
// Test parametrs if building:

void
GlomItemTests::isBuilding()
{
    CGlomParameters item(static_cast<uint64_t>(1234), true, CGlomParameters::first);
    EQ(true, item.isBuilding());
}

//// Make sure construction produces structurally correct stuff:

//void
//GlomItemTests::structure()
//{
//    CGlomParameters item(static_cast<uint64_t>(1234), false, CGlomParameters::average);
    
//    pGlomParameters pItem =
//        reinterpret_cast<pGlomParameters>(item.getItemPointer());
        
//    // Header stuff:
        
//    EQ(static_cast<uint32_t>(sizeof(GlomParameters)), pItem->s_header.s_size);
//    EQ(EVB_GLOM_INFO, pItem->s_header.s_type);
    
//    // Lack of body header:
    
//    EQ(static_cast<uint32_t>(0), pItem->s_mbz);
    
//    // Body contents:
    
//    EQ(static_cast<uint64_t>(1234), pItem->s_coincidenceTicks);
//    ASSERT(!pItem->s_isBuilding);
//    EQ(GLOM_TIMESTAMP_AVERAGE, pItem->s_timestampPolicy);
    
//}
