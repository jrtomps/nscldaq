//
// 
// Same as CFileEventTest, however the 
// location monitor is now a location monitor event.
//

#include <spectrodaq.h>
#include <CEvent.h>
#include <CLocationEvent.h>
#include <CChangedPredicate.h>
#include <CFileEvent.h>
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>

volatile Word Bink = 0;

typedef CChangedPredicate<Word> WordChangedPredicate;
typedef CLocationEvent<Word>    WordEvent;

//
//  React to buffer available by printing bits of it.
//
class Binky : public WordEvent
{
public:
  Binky(const char* pName) :
    WordEvent(pName,
	      &Bink,
	      *(new WordChangedPredicate(Bink)))
  {}
  virtual void OnLocationChanged(Word newValue);
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
Binky::OnLocationChanged(Word newval)
{
  cout << "Bink value is: " << newval << endl;
  cout.flush();
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

  Binky BinkEvent("BinkMeister");
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
