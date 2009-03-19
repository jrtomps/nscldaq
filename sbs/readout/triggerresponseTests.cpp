// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CEventSegment.h"
#include "CCompoundEventSegment.h"
#include "CScaler.h"
#include "CScalerBank.h"
#include <string>
#include <string.h>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <DataFormat.h>
#include <CRingScalerItem.h>
#include <CAllButPredicate.h>
#include <RunState.h>
#include <stdint.h>
#include <vector>
#include <TCLInterpreter.h>
#include <CVariableBuffers.h>

#define private public
#include "CExperiment.h"
#undef private




using namespace std;

static string ringName("testring");


// Exercises the various trigger operations on the CExperiment class.
// this includes the ability to establish event segments and
// scaler banks.
// The only thing not done by the time these tests all pass is to 
// establish actual triggers and run the trigger thread.
//

class triggerResponse : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(triggerResponse);
  CPPUNIT_TEST(eventsegs);
  CPPUNIT_TEST(readout);
  CPPUNIT_TEST(scalerbank);
  CPPUNIT_TEST(readscalers);
  CPPUNIT_TEST_SUITE_END();


private:
  CExperiment* m_pExperiment;
class TestSegment : public CEventSegment
{
public:
  size_t read(void* pBuffer, size_t maxwords)
  {
    uint16_t* p = reinterpret_cast<uint16_t*>(pBuffer);
    for (int i =0; i < 10; i++) {
      *p++ = i;
    }
    return 10;
  }
};


  class TestScaler : public CScaler
  {
  public:
    virtual vector<uint32_t> read() {
      vector<uint32_t> result;
      for (int i = 0; i < 32; i++) {
	result.push_back(i);
      }
      return result;
    }
  };


public:
  CTCLInterpreter* m_pInterp;
  CVariableBuffers* m_pBufs;
  void setUp() {    
    m_pInterp = new CTCLInterpreter;
    m_pBufs   = new CVariableBuffers(*m_pInterp);
    
    if (CRingBuffer::isRing(ringName)) {
      CRingBuffer::remove(ringName); // In case it already exists, remove it
    }
    m_pExperiment = new CExperiment(ringName, 4096);

  }
  void tearDown() {
    delete m_pBufs;
    delete m_pInterp;

    delete m_pExperiment;
    m_pExperiment = reinterpret_cast<CExperiment*>(0);
    CRingBuffer::remove(ringName);
  }
protected:
  void eventsegs();
  void readout();
  void scalerbank();
  void readscalers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(triggerResponse);


// Ensure that we can put an event segment in to an experiment object.
// The first time, that should create a root event segment that has what we added
// we can also remove that event segment leaving an emtpy root segment.

void triggerResponse::eventsegs() {
  TestSegment myseg;
  m_pExperiment->AddEventSegment(&myseg);

  ASSERT(m_pExperiment->m_pReadout);
  CCompoundEventSegment::EventSegmentIterator p =
    m_pExperiment->m_pReadout->begin();
  EQ(&myseg, dynamic_cast<TestSegment*>(*p));

  m_pExperiment->RemoveEventSegment(&myseg);
  ASSERT(m_pExperiment->m_pReadout); // root segment should still be there.

  EQ(static_cast<size_t>(0), m_pExperiment->m_pReadout->size()); // no entries though.
}
//
// If we add a test segment, and then hit the read member function:
// - The ring buffer should contain a physics item that has a body read by our event
//   segment.
void triggerResponse::readout()
{
  TestSegment myseg;
  CRingBuffer Ring(ringName);
  CAllButPredicate pred;

  m_pExperiment->AddEventSegment(&myseg);

  m_pExperiment->ReadEvent();

  CRingItem* item = CRingItem::getFromRing(Ring, pred);
  ASSERT(item);
  EQ(PHYSICS_EVENT, item->type());
  uint16_t* p = reinterpret_cast<uint16_t*>(item->getBodyPointer());
  for (uint16_t i=0; i < 10; i++) {
    EQ(i, *p++);
  }

  delete item;
}

// Should be able to add a scaler bank.. that will result in 
// - non-null m_pScalers
// - 1 entry in m_pScalers.
// - The entry is the one we inserted.
//
// Deleting shoulid leave us with an empty top level scaler bank:
void triggerResponse::scalerbank()
{
  TestScaler scaler;
  m_pExperiment->AddScalerModule(&scaler);

  CScalerBank* pBank = m_pExperiment->m_pScalers;

  ASSERT(pBank);
  EQ(static_cast<size_t>(1), pBank->size());
  CScalerBank::ScalerIterator p = pBank->begin();
  TestScaler* ps = dynamic_cast<TestScaler*>(*p);
  EQ(&scaler, ps); 

  m_pExperiment->RemoveScalerModule(&scaler);
  ASSERT(pBank->size() == 0);
}
// Reading the scalers should result in a ring buffer scaler item:

void triggerResponse::readscalers()
{  TestScaler scaler;
  m_pExperiment->AddScalerModule(&scaler);
  CRingBuffer Ring(ringName);
  CAllButPredicate pred;

  m_pExperiment->AddScalerModule(&scaler);


  RunState* pState = RunState::getInstance();
  m_pExperiment->m_nLastScalerTime = 0;
  pState->m_timeOffset = 10;
  pState->m_state      = RunState::active;
  pState->m_runNumber  = 1234;
  delete []pState->m_pTitle;
  pState->m_pTitle = new char[strlen("Test title") +1];
  strcpy(pState->m_pTitle, "Test title");
  m_pExperiment->TriggerScalerReadout();

  CRingItem* item = CRingItem::getFromRing(Ring, pred);
  ASSERT(item);
  EQ(INCREMENTAL_SCALERS, item->type());
  
  CRingScalerItem scalers(*item);
  delete item;

  EQ(static_cast<uint32_t>(0), scalers.getStartTime());
  EQ(static_cast<uint32_t>(10), scalers.getEndTime());
  for (uint32_t i = 0; i < 32; i++) {
    EQ(i, scalers.getScaler(i));
  }
  
}
