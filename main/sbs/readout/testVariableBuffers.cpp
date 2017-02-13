// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CVariableBuffers.h"
#undef private


#include <TCLInterpreter.h>
#include <TCLVariable.h>

#include <CRingBuffer.h>
#include <CRingDataSource.h>
#include <CRingDataSink.h>
#include <CDataSourceFactory.h>
#include <CDataSinkFactory.h>

#include <V12/CRingItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CRawRingItem.h>
#include <RingIOV12.h>
#include <Thread.h>

#include <string>
#include <string.h>
#include <tcl.h>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace DAQ::V12;

extern std::string uniqueName(std::string);

static const string RINGNAME(uniqueName("testring"));

class VarBufs : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarBufs);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(strunvar);
  CPPUNIT_TEST(ststatevar);
  CPPUNIT_TEST(dtrunvar);
//  CPPUNIT_TEST(dtstatevar);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterp;
  CRingDataSink*     m_pProducer;
  CRingDataSource*     m_pConsumer;

public:
  void setUp() {
    if (CRingBuffer::isRing(RINGNAME)) {
      CRingBuffer::remove(RINGNAME);
    }
    m_pInterp = new CTCLInterpreter();
    new CVariableBuffers(*m_pInterp); // singleton..so saved.

    CRingBuffer::create(RINGNAME);
    m_pProducer =
            dynamic_cast<CRingDataSink*>(CDataSinkFactory().makeSink(string("tcp://localhost/") + RINGNAME));
    m_pConsumer =
            dynamic_cast<CRingDataSource*>(CDataSourceFactory::makeSource(string("tcp://localhost/") + RINGNAME,
    {}, {}));
  }
  void tearDown() {
    delete CVariableBuffers::getInstance();
    delete m_pInterp;
    delete m_pProducer;
    delete m_pConsumer;
    CRingBuffer::remove(RINGNAME);
  }
protected:
  void construct();
  void strunvar();
  void ststatevar();
  void dtrunvar();
  void dtstatevar();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarBufs);

//
// Construction should allow us to get the instance variable as nonnull.
//
void VarBufs::construct() {
  CVariableBuffers* p = CVariableBuffers::getInstance();
  ASSERT(p);
  ASSERT(p->m_pVars);
  ASSERT(p->m_pTriggerGuard);
  ASSERT(p->m_pTriggerSynchronize);
  ASSERT(p->m_interpreterThread); // Assumes there's no thread id 0.

    
}
//
// Make some run variables and trigger a dump of them in thedtrun same thread.
//
void VarBufs::strunvar() {
  m_pInterp->Eval("runvar george 5");
  m_pInterp->Eval("runvar tom 6");
  m_pInterp->Eval("runvar dick 0");


  CVariableBuffers* p = CVariableBuffers::getInstance();
  p->triggerRunVariableBuffer(m_pProducer, 0);

//  while (!Tcl_DoOneEvent(0)) {}

  // There should be an event in the ring buffer:
  
  CRawRingItem rawItem;
  *m_pConsumer >> rawItem;
  
  ASSERT(! m_pConsumer->eof());
  EQ(MONITORED_VARIABLES, rawItem.type());

  // The item should be a bunch of strings that when run in a captive
  // interpreter should reproduce the values of george, tom and dick:

  CRingTextItem item(rawItem);
  vector<string> commands = item.getStrings();
  CTCLInterpreter interp;
  for (int i=0; i < commands.size(); i++) {
    interp.Eval(commands[i]);
  }
  CTCLVariable george(&interp, "george", false);
  EQ(strcmp("5", george.Get()), 0);
  
  CTCLVariable tom(&interp, "tom", false);
  EQ((strcmp("6", tom.Get())), 0);
  
  CTCLVariable dick(&interp, "dick", 0);
  EQ(strcmp("0", dick.Get()), 0); 
  
}
//
// Make some state vars and trigger a dump of them in the same thread.
//
void VarBufs::ststatevar() {
  m_pInterp->Eval("statevar george 5");
  m_pInterp->Eval("statevar tom 6");
  m_pInterp->Eval("statevar dick 0");


  CVariableBuffers* p = CVariableBuffers::getInstance();
  p->triggerStateVariableBuffer(m_pProducer, 0);

  // There should be an event in the ring buffer:
  
  CRawRingItem rawItem;
  *m_pConsumer >> rawItem;
  
  ASSERT(! m_pConsumer->eof());
  EQ(MONITORED_VARIABLES, rawItem.type());

  // The item should be a bunch of strings that when run in a captive
  // interpreter should reproduce the values of george, tom and dick:

  CRingTextItem item(rawItem);
  vector<string> commands = item.getStrings();
  CTCLInterpreter interp;
  for (int i=0; i < commands.size(); i++) {
    interp.Eval(commands[i]);
  }
  CTCLVariable george(&interp, "george", false);
  EQ(strcmp("5", george.Get()), 0);
  
  CTCLVariable tom(&interp, "tom", false);
  EQ((strcmp("6", tom.Get())), 0);
  
  CTCLVariable dick(&interp, "dick", 0);
  EQ(strcmp("0", dick.Get()), 0); 
  
}

// The following are thread classes that trigger run and state variable emissions:
//
class CTriggerRunVar : public Thread
{
  CRingDataSink* m_pRing;
public:
  CTriggerRunVar(CRingDataSink* pR) :
    Thread("trigger"),
    m_pRing(pR) {}


  virtual void run() {
    CVariableBuffers* p  = CVariableBuffers::getInstance();
    p->triggerRunVariableBuffer(m_pRing, 0);

  }
};

class CTriggerStateVar : public Thread
{
  CRingDataSink* m_pRing;
public:
  CTriggerStateVar(CRingDataSink* pR) :
    Thread("trigger"),
    m_pRing(pR) {}
  virtual void run() {
    CVariableBuffers* p = CVariableBuffers::getInstance();
    p->triggerStateVariableBuffer(m_pRing, 0);
  }
};


//
//  Make some run vars, and start a separate thread that triggers the runvars
//
void VarBufs::dtrunvar() {
  m_pInterp->Eval("runvar george 5");
  m_pInterp->Eval("runvar tom 6");
  m_pInterp->Eval("runvar dick 0");


  // Start up a thread to trigger the event and then enter the event loop
  // so the event can be processed. when that's done
  CTriggerRunVar trigger(m_pProducer);
  trigger.start();
  while (!Tcl_DoOneEvent(0)) {}

  // There should be an event in the ring buffer:
  
  CRawRingItem rawItem;
  *m_pConsumer >> rawItem;
  
  ASSERT(!m_pConsumer->eof());
  EQ(MONITORED_VARIABLES, rawItem.type());

  // The item should be a bunch of strings that when run in a captive
  // interpreter should reproduce the values of george, tom and dick:

  CRingTextItem item(rawItem);
  vector<string> commands = item.getStrings();
  CTCLInterpreter interp;
  for (int i=0; i < commands.size(); i++) {
    interp.Eval(commands[i]);
  }
  CTCLVariable george(&interp, "george", false);
  EQ(strcmp("5", george.Get()), 0);
  
  CTCLVariable tom(&interp, "tom", false);
  EQ((strcmp("6", tom.Get())), 0);
  
  CTCLVariable dick(&interp, "dick", 0);
  EQ(strcmp("0", dick.Get()), 0); 

}
//
// Make some statevars, and start a separate thread that triggers the statevars.
void VarBufs::dtstatevar() {
  m_pInterp->Eval("statevar george 5");
  m_pInterp->Eval("statevar tom 6");
  m_pInterp->Eval("statevar dick 0");


  // Start up a thread to trigger the event and then enter the event loop
  // so the event can be processed. when that's done

  CTriggerStateVar trigger(m_pProducer);
  trigger.start();
  while (!Tcl_DoOneEvent(0)) {}

  // There should be an event in the ring buffer:
 
  CRawRingItem rawItem;
  *m_pConsumer >> rawItem;
  
  ASSERT(!m_pConsumer->eof());
  EQ(MONITORED_VARIABLES, rawItem.type());

  // The item should be a bunch of strings that when run in a captive
  // interpreter should reproduce the values of george, tom and dick:

  CRingTextItem item(rawItem);
  vector<string> commands = item.getStrings();
  CTCLInterpreter interp;
  for (int i=0; i < commands.size(); i++) {
    interp.Eval(commands[i]);
  }
  CTCLVariable george(&interp, "george", false);
  EQ(strcmp("5", george.Get()), 0);
  
  CTCLVariable tom(&interp, "tom", false);
  EQ((strcmp("6", tom.Get())), 0);
  
  CTCLVariable dick(&interp, "dick", 0);
  EQ(strcmp("0", dick.Get()), 0); 

}
