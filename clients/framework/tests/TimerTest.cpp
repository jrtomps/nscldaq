#include <spectrodaq.h>
#include <CTimerMonitor.h>
#include <iostream.h>
#include <CReactor.h>

class TimerReactor : public CReactor
{
public:
  TimerReactor(const char* pName) :
    CReactor(pName) {
      AppendClassInfo();
  }
  virtual void OnEvent(CEventMonitor& rMon);
  virtual void OnError(CEventMonitor& rMon);
  virtual void OnTimeout(CEventMonitor& rMon);
};

void
TimerReactor::OnEvent(CEventMonitor& rMon)
{
  cout << "Timer fired: I am " << DescribeSelf() << endl;
  cout << " I was invoked by " << rMon.DescribeSelf() << endl;
}


void
TimerReactor::OnError(CEventMonitor& rMon)
{
  cout << "Timer had error: I am " << DescribeSelf() << endl;
  cout << "I was invoked by " << rMon.DescribeSelf() << endl;
}

void 
TimerReactor::OnTimeout(CEventMonitor& rMon)
{
  cout << "Timer timed out: I am " << DescribeSelf() << endl;
  cout << "I was invoked by " << rMon.DescribeSelf() << endl;

}

class DAQClient : public DAQROCNode {
  int operator() (int argc, char** argv) {
    
    /*===========================================================
      The following code demonstrates the CTimerMonitor class
      =========================================================*/
    cout << "\n####The following code demonstrates the CTimerMonitor class####"
	 << endl;

    CTimerMonitor timermon;
    timermon.setTimeout(5000);

    // Describe the timer monitor    
    cout << "\nDescribing my TimerMonitor:" << endl;
    cout << timermon.DescribeSelf() << endl;
    
    // Test CTimerMonitor::operator()
    cout << "\nWaiting for timer to expire..." << endl;
    switch(timermon()) {
    case CEventMonitor::Occurred:
      cout << "Event Occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed out" << endl;
      break;
    default:
      cout << "Neither of the other cases occurred" << endl;
    }
    
    cout << "\nDescribing self again: " << endl;
    cout << timermon.DescribeSelf() << endl;
    
    cout << "\nAttempting to fire OneShot monitor a second time..." << endl;
    switch(timermon()) {
    case CEventMonitor::Occurred:
      cout << "Event Occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed out" << endl;
      break;
    default:
      cout << "Neither of the other cases occurred" << endl;
    }
    
    cout << "\nDemonstrating Repeat(false)" << endl;
    timermon.Repeat(false);
    cout << "Describing self again..." << endl;
    cout << timermon.DescribeSelf() << endl;

    //
    // Now we use the timer monitor in conjunction with the 
    // reactor base class (RF).
    //

    cout << "---------------------------------------------------" << endl;
    cout << "Using reactors and timers together" << endl;
    timermon.Repeat(true);	// Turn on as repeating.
    timermon.setTimeout(1000);  // I'm less patient than Jason.
    TimerReactor George("George");
    George(timermon, timermon());		// First time.
    cout << endl << "Again now: " << endl;
    George(timermon, timermon());
    cout << endl << "Now with non repeat mode " << endl;
    timermon.Repeat(false);
    George(timermon, timermon());
    cout << endl << "Next should timeout " << endl;
    George(timermon, timermon());
  }
};

// Instantiate the DAQClient
DAQClient mydaq;
