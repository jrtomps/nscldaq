//  Test the CTimerEvent - timer event class.
//  We do the following:
//    - subclass CTimerEvent to create an object which
//      will signal a mutex the main thread is waiting on.
//      (via a condition) when it fires.  
//   - the main thread first starts the thread up in single shot mode.
//     and waits for the condition.  When the condition fires, 
//     the main thread prints that fact out.
//   - The main thread converts the timer into multishot mode and
//     restarts it... it then waits in a loop printing everytime the
//     condition is signalled.
//
//   
#include <spectrodaq.h>
#include <CTimerEvent.h>
#include <iostream.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

// Subclass the timer event:

class MyTimer : public CTimerEvent 
{
  DAQThreadCond&    m_TimerSignal;
public:
  MyTimer(unsigned long nms, DAQThreadCond& rCondition) : 
    CTimerEvent("MyTimer", nms, false),
    m_TimerSignal(rCondition)
  {}
  virtual void OnTimer() {
    m_TimerSignal.Signal();
  }
};


// Define the application class.

class Mytest : public DAQROCNode
{
private:
  DAQThreadMutex   condmutex;
  DAQThreadCond    TimerExpired;
protected:
  int operator()(int argc, char** argv);
};

int
Mytest::operator()(int argc, char** argv)
{
  // Create the timer:

  cerr << "Creating timer as oneshot...\n";

  MyTimer Timer(1000L, TimerExpired);	// One second timer.
  Timer.Enable();
  TimerExpired.Wait(condmutex);
  cerr << " Timer expired\n";

  // this next wait should time out...

  cerr << "Ensuring timer is oneshot...\n";
  struct timeval now;
  struct timespec timeout;
  if(gettimeofday(&now,NULL)) {
    cerr << "gettimeofday faild: " << strerror(errno) << endl;
  }
  timeout.tv_sec = now.tv_sec + 5;
  timeout.tv_nsec= now.tv_usec*1000;
  int stat = TimerExpired.TimedWait(condmutex, &timeout);
  if(stat) {
    if(errno ==ETIMEDOUT) {
      cerr << "Wait timed out as it should!\n";
    }
    else {
      cerr << "Error from Wait for timer: " << strerror(errno) << endl;
    }
  }
  else {
    cerr << " >>>ERROR<<< Timer should not have refired\n";
  }
  
  //  Now set the timer to be  a refiring timer.

  cerr << "Setting timer to repeat KILL to stop.\n";

  Timer.Enable(2000, true);

  while(1) {
    TimerExpired.Wait(condmutex);
    cerr << "Timer expired\n";
  }
}

Mytest testprogram;
