//
// FileEvent test:
//   This works in the same way as the EventTest, but the input
//   event thread is run from a CFileEvent derived object.
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
#include <CFileEvent.h>
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

class StdinProcessor : public CFileEvent
{
public:
  StdinProcessor() :
    CFileEvent(0,"Bonker") {}
  virtual void OnReadable(istream& rInput);
  virtual void OnTimeout(iostream& rInput);
  
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
StdinProcessor::OnTimeout(iostream& rStream) {
  cout << "Urk\n";
  cout.flush();
}

void
StdinProcessor::OnReadable(istream& rStream)
{

  // Respond visibly to the event.


  string   line;
  rStream >> line;
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
  
  StdinProcessor Input;

  Input.ReactToTimeouts(true);
  Input.setReactivity(2000);
  Input.Enable();
  Synch.Wait(mtx);

  exit(0);
  
}

EventTest anObject;
