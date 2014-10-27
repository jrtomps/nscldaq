// Tests of the event orderer.


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <unistd.h>

// This dodge is used to ensure we can construct
// and not treat the fragment handler as a singleton.

#define private public
#include "CFragmentHandler.h"
#undef private


#include "fragment.h"



class ObserverTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ObserverTests);
  CPPUNIT_TEST(dlate);
  CPPUNIT_TEST(observedup);
  CPPUNIT_TEST_SUITE_END();


private:
    CFragmentHandler* m_pFragHandler;
    Tcl_Interp*       m_pInterp;           // Needed to make an event loop
public:                                    // for the timer handler.
  void setUp() {
    m_pInterp = Tcl_CreateInterp();
    m_pFragHandler = new CFragmentHandler();
  }
  void tearDown() {
    delete m_pFragHandler;
    Tcl_DeleteInterp(m_pInterp);
  }
protected:
  void dlate();
  void observedup();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ObserverTests);

// Our data late observer:

class MyLateHandler : public CFragmentHandler::DataLateObserver {
public:
    bool m_called;
    MyLateHandler() :m_called(false) {}
    void operator()(const ::EVB::Fragment& fragment,  uint64_t newest) {
        m_called = true;
    }
};

// Ensure the data late counters work.  
void ObserverTests::dlate() {
    // Set a data late observer, and set a ones second build window
    
    m_pFragHandler->setBuildWindow(0);
    
    MyLateHandler lateHandler;
    m_pFragHandler->addDataLateObserver(&lateHandler);
   
    // insert a fragment
    
    EVB::FlatFragment frag1;
    frag1.s_header.s_timestamp = 100;              // nonzero is the key.
    frag1.s_header.s_sourceId  = 1;
    frag1.s_header.s_size      = 0;
    frag1.s_header.s_barrier   = 0;
    
    m_pFragHandler->addFragments(sizeof(frag1), &frag1);
    m_pFragHandler->flushQueues();
    
    
    // insert an earlier fragment
    
    
    EVB::FlatFragment frag2;
    frag2.s_header.s_timestamp = 50;              // nonzero is the key.
    frag2.s_header.s_sourceId  = 2;
    frag2.s_header.s_size      = 0;
    frag2.s_header.s_barrier   = 0;
    m_pFragHandler->addFragments(sizeof(frag2), &frag2);
    m_pFragHandler->flushQueues();
    
    // Require that our observer was called.

    ASSERT(lateHandler.m_called);
}

class MyDuplicateTimestampObserver : public CFragmentHandler::DuplicateTimestampObserver
{
public:
    uint32_t m_sourceId;
    uint64_t m_timestamp;
    bool     m_called;
    
    MyDuplicateTimestampObserver() : m_called(false) {}
    
    void operator()(uint32_t sourceId, uint64_t timestamp) {
        m_sourceId  = sourceId,
        m_timestamp = timestamp;
        m_called    = true;
    }
};

void ObserverTests::observedup()
{
    // Register the observer.
    
    MyDuplicateTimestampObserver observer;
    m_pFragHandler->addDuplicateTimestampObserver(&observer);
    
    // Insert an fragment and force it to flush..
    
    EVB::FlatFragment frag;
    frag.s_header.s_timestamp = 100;
    frag.s_header.s_sourceId  = 1;
    frag.s_header.s_size      = 0;
    frag.s_header.s_barrier   = 0;
    m_pFragHandler->addFragments(sizeof(frag), &frag);
    m_pFragHandler->flushQueues();
    
    // Insert a fragment without a duplicate timestamp
    // flush it and ensure the observer was not called:
    
    frag.s_header.s_timestamp = 110;
    m_pFragHandler->addFragments(sizeof(frag), &frag);
    m_pFragHandler->flushQueues();
    
    ASSERT(!observer.m_called);
    
    // Insert a fragment with a duplicate timestamp ensure we observed it..correctly
    
    m_pFragHandler->addFragments(sizeof(frag), &frag);
    m_pFragHandler->flushQueues();
    
    ASSERT(observer.m_called);
    EQ(frag.s_header.s_timestamp, observer.m_timestamp);
    EQ(frag.s_header.s_sourceId,  observer.m_sourceId);
    
}

