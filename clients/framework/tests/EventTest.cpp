//  Tests the event class with two event processorsing threads:
//
// 1. Buffer dumping Event: When a buffer comes in, it is partially dumped.
// 2. stdin monitoring event: When input arrives, the buffer dumper is exited,
//    and the program as a whole exits.  This event is sensitized to
//    monitor timeouts and just types "URK" on timeouts.
// The main program will do a join on the second event in order to 
// know how to exit.
//

#include <spectrodaq.h>
#include <CEvent.h>
#include <CLocationMonitor.h>
#include <CLocationReactor.h>
#include <CFdMonitor.h>
#include <CFdReactor.h>
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>

Word Bink = 0;

typedef CLocationReactor<Word> WordReactor;
typedef CLocationMonitor<Word> WordLocMon;
//
//  React to buffer available by printing bits of it.
//
class Binky : public WordReactor
{
public:
  Binky() :
    WordReactor("Binkmeister")
  {}
  virtual void OnLocationChanged(WordLocMon& rMon, Word newval);
};


// React to stdin events:

class StdinProcessor : public CFdReactor
{
public:
  StdinProcessor()  {}
  virtual void OnReadable(CFdMonitor& rMon, int fd);
  virtual void OnTimeout(CEventMonitor& rMon);
  
};

// Main program.

class EventTest : public DAQROCNode
{
  DAQThreadCond  Synch;
  DAQThreadMutex mtx;
public:
  void AllDone() { Synch.Signal(); }
protected:
  virtual int operator()(int argc, char** argv);

};

//// Implementation of Binky class:

void
Binky::OnLocationChanged(WordLocMon& rMon, Word newval)
{
  cout << "Bink value is: " << newval << endl;
}

//// Implementation of StdinProcessor

void
StdinProcessor::OnTimeout(CEventMonitor& rMon) {
  cout << "Urk\n";
  cout.flush();
}

void
StdinProcessor::OnReadable(CFdMonitor& rMon, int fd)
{

  // Respond visibly to the event.

  ifstream input(fd);
  string   line;
  cin >> line;
  cout << "Received: " << line << " Binking:  \n";

  Bink++;
}


//// Implementation of main.

int
EventTest::operator()(int argc, char** argv)
{
  // Configure the Location monitor.

  Binky Binker;
  CChangedPredicate<Word> pred("Predicate", 0);
  WordLocMon  LocMon(&Bink, &pred);

  CEvent BinkEvent(LocMon, Binker);
  BinkEvent.Enable();

  // Configure the stdin handler:
  
  mtx.Lock();
  CFdMonitor stdinmon(0);
  stdinmon.MonitorReadable();
  StdinProcessor stdinproc;
  CEvent Input(stdinmon, stdinproc);

  Input.ReactToTimeouts(true);
  Input.setReactivity(2000);
  Input.Enable();
  Synch.Wait(mtx);

  exit(0);
  
}

EventTest anObject;
