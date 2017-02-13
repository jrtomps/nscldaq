// Test the CExperiment class as best we can.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/types.h>
#include <unistd.h>
#include <CDataSink.h>
#include <CRingBuffer.h>
#include <CRingDataSource.h>
#include <CRingDataSink.h>
#include <CDataSourceFactory.h>
#include <RunState.h>
#include <StateException.h>
#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingStateChangeItem.h>
#include <RingIOV12.h>
#include <CTriggerLoop.h>
#include <string.h>
#include <string>
#include <CNullTrigger.h>
#include <tcl.h>
#include <os.h>


// class under test gets its member data exposed for me:
//

#define private public
#include <CExperiment.h>
#undef private

extern std::string uniqueName(std::string);

using namespace std;
using namespace DAQ::V12;

static const string ringName(uniqueName("experimentTest"));
static string testTitle("This is my title");

class experimentTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(experimentTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(buffersize);
  CPPUNIT_TEST(start);
  CPPUNIT_TEST(stop);
  CPPUNIT_TEST_SUITE_END();


private:
  CExperiment*  m_pExperiment;
public:
  void setUp() {
    if (CRingBuffer::isRing(ringName)) {
      CRingBuffer::remove(ringName); // In case it already exists, remove it
    }
    m_pExperiment = new CExperiment(ringName, 4096);
    m_pExperiment->EstablishTrigger(new CNullTrigger);
    m_pExperiment->setScalerTrigger(new CNullTrigger);
  }
  void tearDown() {
    delete m_pExperiment;
    m_pExperiment = nullptr;
    CRingBuffer::remove(ringName);
  }
protected:
  void construct();
  void buffersize();
  void start();
  void stop();

private:
  void renewTitle();
};

CPPUNIT_TEST_SUITE_REGISTRATION(experimentTests);


//////////////////////////////// Utilities ///////////////////////////

// Set the runs state title to our test title:

void
experimentTests::renewTitle()
{
  delete []m_pExperiment->m_pRunState->m_pTitle;
  m_pExperiment->m_pRunState->m_pTitle = new char[strlen(testTitle.c_str()) + 1];
  strcpy(m_pExperiment->m_pRunState->m_pTitle, testTitle.c_str());
  
}

//////////////////////////////// Tests ///////////////////////////////


//
// On construction, we should see that the requested ring buffer exists.
// Member data as follows:
//    m_pRing           - non null.
//    m_nDataBufferSize - Same as that provided the contructor.
//    m_pRunState       - non null and the state member should be inactive.
// ring bufferspace better be larger than the eventBufferSize else there's problems
// ahead.  We must be the producer.

//
void experimentTests::construct() {
  ASSERT(CRingBuffer::isRing(ringName));

  ASSERT(m_pExperiment->m_pRing); // Ring buffer must exist.
  ASSERT(m_pExperiment->m_pRunState);

  EQ(static_cast<size_t>(4096), 
     m_pExperiment->m_nDataBufferSize);

  auto pRingSource = dynamic_cast<CRingDataSink*>(m_pExperiment->m_pRing);
  ASSERT(pRingSource);
  CRingBuffer::Usage usage =  pRingSource->getRing().getUsage();
  ASSERT(usage.s_bufferSpace > 4096);
  EQ(getpid(), usage.s_producer);

  EQ(RunState::inactive, m_pExperiment->m_pRunState->m_state);
  
}

//  setBuffersize should null out the m_pDataBuffer.
//  getBuffersize should reflect the most recent buffersize selected.

void experimentTests::buffersize()
{
  EQ(static_cast<size_t>(4096), m_pExperiment->getBufferSize());
  
  m_pExperiment->setBufferSize(8192);

  EQ(static_cast<size_t>(8192), m_pExperiment->getBufferSize());
}

// start has two cases we look at here:
// - An initial start after constrution we test that start
//  o Should modify the runstate object:
//    - Transition state -> active.
//    - reset time offset -> 0
//  o Should submit a start run record to the ring buffer
//    - with a run number that has the correct run number (From state)
//    - with a title that is correct.
//    - with a time offset of zero
// - A start run with the run active throws a CStateException
//
void experimentTests::start()
{

    // order is important! The ring must be attached to prior to calling "start"
    // so that we do not attach too late and miss the state change item.
    CDataSource* pConsumer =
            CDataSourceFactory::makeSource(string("tcp://localhost/") + ringName, {}, {});

  m_pExperiment->m_pRunState->m_runNumber = 1234;
  m_pExperiment->m_pRunState->m_timeOffset = 5678;

  renewTitle();

  m_pExperiment->Start();	// start run.

  // Changes to run state:

  EQ(RunState::active, m_pExperiment->m_pRunState->m_state);


  CRawRingItem stateChangeItem, formatItem;
  *pConsumer >> formatItem >> stateChangeItem;
  CRingStateChangeItem* pState(0);

  delete pConsumer;

  try {
    pState = new CRingStateChangeItem(stateChangeItem);
  }
  catch(bad_cast) {
  }


  // Got a state change item:

  ASSERT(pState);

  // Contents of the state change item:

  
  EQ(static_cast<uint32_t>(1234), pState->getRunNumber());
  EQ(static_cast<uint32_t>(0), pState->getElapsedTime());
  EQ(testTitle, pState->getTitle());
  delete pState;

  // Starting the run now, when the run is active, should throw 
  // CStateException:

  bool thrown(false);

  try {
    m_pExperiment->Start();
  }
  catch (CStateException& e) {
    thrown = true;
  }
  ASSERT(thrown);

}
// - stopping an inactive run should throw a CStateException.
// - stopping an active run should:
//   o Transition state to inactive
//   o submit a stop run record to the ring buffer with:
//     - the title in the run state object
//     - the run number in the run state object
//     - The time offset in the run state object.

void
experimentTests::stop()
{
  bool thrown = false;
  m_pExperiment->m_pRunState->m_state = RunState::inactive;
  try {
    m_pExperiment->Stop();
  }
  catch(CStateException& e) {
    thrown = true;
  }
  ASSERT(thrown);

  // set up the run state object to be what we want it to be:

  m_pExperiment->m_pRunState->m_state     = RunState::active;
  m_pExperiment->m_pRunState->m_runNumber = 5678;
  m_pExperiment->m_pRunState->m_timeOffset = 666;
  renewTitle();


  // order is important. This must be created before we stop so that
  // we do not attach to the ring too late.
  CDataSource* pConsumer
          = CDataSourceFactory::makeSource(string("tcp://localhost/") + ringName,
                                            {}, {});

  m_pExperiment->syncEndRun(false);	// This should now work.

  // Need to run the event loop a bit:

  CRawRingItem countItem, stateItem;
  *pConsumer >> countItem >> stateItem;


  CRingStateChangeItem* pState(0);
  
  delete pConsumer;

  try {
    pState = new CRingStateChangeItem(stateItem);
  }
  catch(bad_cast) {
  }


  // Got a state change item:

  ASSERT(pState);


  //contents of the state change item:

  EQ(string(m_pExperiment->m_pRunState->m_pTitle), pState->getTitle());
  EQ(m_pExperiment->m_pRunState->m_runNumber,      pState->getRunNumber());


  // by now run is stopped(?) ...may have to run the event loop

  EQ(RunState::inactive, m_pExperiment->m_pRunState->m_state);


}

