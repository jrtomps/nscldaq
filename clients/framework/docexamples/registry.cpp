#include <spectrodaq.h>
#include <SpectroFramework.h>
#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

//
// Timer class intended to be 'remote controlled' by name.
//
class MyTimer : public CTimerEvent
{
public:
  MyTimer(const char* pName);
  virtual void OnTimer();
  virtual string DescribeSelf();
};
//  The constructor registers us, with the given name, however
//  since we won't be enabled, the initial timeout is 0 and we create
//  ourselves in norepeat mode.
//
MyTimer::MyTimer(const char* pName) :
  CTimerEvent(pName, 0, false)
{
  AppendClassInfo();		// Add our place in class hierarchy.
}
//  OnTimer is called when the timer does expire.. it just types out
//  "Timer fired" and it's description.
//
void
MyTimer::OnTimer()
{
  cout << "A MyTimer object fired:" << endl;
  cout << DescribeSelf() << endl;
  cout << "------------------------------------------" << endl;
}
string 
MyTimer::DescribeSelf()
{
  string result;
  result = "My timer object (one shot remote controlled timer).\n";
  result += CTimerEvent::DescribeSelf();

  return result;
}


// Fd class to monitor stdin and remote control a timer based on numeric
// input.  Note that the thread exits if non-numeric input is received.
// 
class Stdin : public CFileEvent
{
  string m_TimerName;
public:
  Stdin(const char* pName, const string& rTimerName);
  virtual void OnReadable(istream &  rInput  );
  virtual string DescribeSelf();
protected:
  void         StartTimer(int nSeconds);
  CTimerEvent* LocateTimer();
};

// Constructor creates us on STDIN_FILENO registered as per name:
//
Stdin::Stdin(const char *pName, const string& rTimerName) : 
  CFileEvent(STDIN_FILENO, string(pName)),
  m_TimerName(rTimerName)
{
  AppendClassInfo();		// Add our place in class hierarchy.
}
//
// OnReadable reads a number, locates and fires off the timer:
//
void
Stdin::OnReadable(istream& rInput)
{
  char number[100];
  int  nsec;

  rInput >> number;
  nsec = atoi(number);

  if(nsec != 0) {
    StartTimer(nsec);
  }
  else {
    setEnable(false);		// Disable ourselves.
  }
}

// Describe ourself:
//
string
Stdin::DescribeSelf()
{
  string result;
  result = " Stdin  object controlling the timer: ";
  result += m_TimerName;
  result += '\n';
  result += CFileEvent::DescribeSelf();

  return result;
}
//
// Start timer locates the timer, sets its timeout and enables it:
//
void
Stdin::StartTimer(int nseconds)
{
  nseconds = nseconds * 1000;	// Timer needs milliseconds, not seconds.
  CTimerEvent* pTimer = LocateTimer();
  if(pTimer) {
    pTimer->SetTimeout(nseconds);
    pTimer->Enable();
  }
  else {
    cerr << "Stdin::StartTimer(): The timer " << m_TimerName;
    cerr << " Does not exist in: " << endl;
    cerr << DescribeSelf();
  }
}
//
//  Locates the named timer controlled by this StdIn object.
//
CTimerEvent*
Stdin::LocateTimer()
{
  CClassifiedObjectRegistry* pRegistry = CApplicationRegistry::getInstance();
  try {
    ObjectIterator p = pRegistry->Find(string("Events"), m_TimerName);
    return (CTimerEvent*)(p->second);
  }
  catch (CNoSuchObjectException& rExcept) { // Timer not found.
    return (CTimerEvent*)NULL;
  }
  
}

class MyApp : public DAQROCNode
{
protected:
  int operator()(int argc, char** argv);
};

MyApp theApplication;

int
MyApp::operator()(int argc, char** argv)
{
  // Create and describe the timer, but don't enable it.. that's done
  // by StdIn's object.

  MyTimer timer("A timer");
  cout << "Created a timer: " << timer.DescribeSelf() << endl;

  // Create an Stdin which controls the timer:

  Stdin TimerController("TimerControl", string("A timer"));
  cout << "Starting Stdin object: " << TimerController.DescribeSelf() << endl;
  TimerController.Enable();

  DAQThreadId id = TimerController.getThreadId();
  Join(id);
  
}
