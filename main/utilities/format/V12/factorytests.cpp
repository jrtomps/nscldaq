// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"

#include "V12/DataFormat.h"
#include "V12/CRingItemFactory.h"
#include "V12/CRingItem.h"
#include "V12/CRingStateChangeItem.h"
#include "V12/CRingTextItem.h"
#include "V12/CRingScalerItem.h"
#include "V12/CPhysicsEventItem.h"
#include "V12/CRawRingItem.h"
#include "V12/CRingPhysicsEventCountItem.h"
//#include "V12/CRingFragmentItem.h"
//#include "V12/CUnknownFragment.h"
#include "V12/CGlomParameters.h"


#include <ctime>
#include <vector>
#include <string>

// Test the ring item factory.  Our test strategy will be to build
// Ring items (with and without headers if appropriate) using the
// data format methods, run those through both createRingItem methods
// to check the result.
//
using namespace DAQ::V12;
using namespace DAQ;

class RingFactoryTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(RingFactoryTests);
    CPPUNIT_TEST(stateChangeTs_0);
    CPPUNIT_TEST(textTs_0);
    CPPUNIT_TEST(scalerTs_0);
    CPPUNIT_TEST(physicsTs_0);
//    CPPUNIT_TEST(triggersTs_0);
    CPPUNIT_TEST(glom_0);           // Never has a timetamp.
    CPPUNIT_TEST_SUITE_END();
    
    
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void stateChangeTs_0();
    void textTs_0();
    
    void scalerTs_0();
    
    void physicsTs_0();
    
//    void triggersTs_0();

    void glom_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingFactoryTests);

void
RingFactoryTests::stateChangeTs_0()
{
    CRingStateChangeItem item(BEGIN_RUN);
    CRawRingItem rawItem(item);

    std::unique_ptr<V12::CRingItem> pBaseItem = V12::CRingItemFactory::createRingItem(rawItem);
    ASSERT(pBaseItem);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("result is a valid derived type",
                                    dynamic_cast<V12::CRingStateChangeItem&>(*pBaseItem));
}


/**
 * textTs
 *
 * Test text item factory with a timestamp:
 */
void
RingFactoryTests::textTs_0()
{
    std::vector<std::string> strings = {              // not quote Dr. Suess.
        "one string",
        "two string",
        "three strings",
        "four",
        "five strings",
        "six strings",
        "seven strings",
        "more"
    };

    V12::CRawRingItem rawItem(CRingTextItem(V12::MONITORED_VARIABLES,
                                            0x1122334455667788ll, 1,
                                            strings, 1122, time(nullptr), 1));

    std::unique_ptr<CRingItem> pBaseItem = CRingItemFactory::createRingItem(rawItem);
    ASSERT(pBaseItem);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("result is a valid derived type",
                                    dynamic_cast<V12::CRingTextItem&>(*pBaseItem));
}

void
RingFactoryTests::scalerTs_0()
{
    CRawRingItem rawItem(CRingScalerItem(20));
    std::unique_ptr<CRingItem> pBaseItem = CRingItemFactory::createRingItem(rawItem);

    ASSERT(pBaseItem);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("result is a valid derived type",
                                    dynamic_cast<V12::CRingScalerItem&>(*pBaseItem));
}

void
RingFactoryTests::physicsTs_0()
{
    CRawRingItem rawItem(CPhysicsEventItem(123, 345, {1,2,3,4,5}));

    std::unique_ptr<CRingItem> pBaseItem  = CRingItemFactory::createRingItem(rawItem);

    ASSERT(pBaseItem);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("result is a valid derived type",
                                    dynamic_cast<V12::CPhysicsEventItem&>(*pBaseItem));
}

/**
 * glom
 *
 * Glom parameters item.. Never has a timestamp.
 */
void
RingFactoryTests::glom_0()
{
    CRawRingItem rawItem(CGlomParameters(1, false, CGlomParameters::first));
    std::unique_ptr<CRingItem> pBaseItem  = CRingItemFactory::createRingItem(rawItem);

    ASSERT(pBaseItem);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("result is a valid derived type",
                                    dynamic_cast<V12::CGlomParameters&>(*pBaseItem));
}
