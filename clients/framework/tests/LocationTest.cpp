#include <spectrodaq.h>
#include <CLocationMonitor.h>
#include <CChangedPredicate.h>
#include <CMaskedValuePredicate.h>
#include <iostream.h>
#include <pthread.h>

class DAQClient : public DAQROCNode {
  int operator() (int argc, char** argv) {

    /*===========================================================
      The following code demonstrates the CLocationMonitor class

      Here's what happens: 
      (1) Location monitor describes itself. Predicate is a 
          CChangedPredicate
      (2) Waits for location change, but times out because location
          doesn't change
      (3) Changes the value of the location being monitored in a
          separate thread after sleeping for 2 seconds. Causes
	  event to occurr.
      (4) Describes location monitor again
      (5) Changes the predicate to a CMaskedValuePredicate
      (6) Waits for significant bits of predicate value to change
          - in this case, the mask is COS_ALLBITS - event occurs
	  because predicate value was manually changed
      (7) Changes monitor location and predicate mask to 0xfffffffe
      (8) Changes the value of the location being monitored in a
          separate thread after monitoring for 2 seconds. Causes
	  event to occur.
      =========================================================*/
    
    cout << "\n####The following code demonstrates the CLocationMonitor" 
	 << " class####" << endl << endl;
    
    int Value = 30;
    CChangedPredicate<int> Pred(Value);
    CLocationMonitor<int> locmon(&Value, &Pred);
    locmon.setTimeout(3000);

    // Describe the monitor    
    cout << "Describing Location Monitor:" << endl;
    cout << locmon.DescribeSelf() << endl;

    // Now perform a wait which times out
    cout << "\nWaiting for location to change" << endl;
    switch(locmon()) {
    case CEventMonitor::Occurred:
      cout << "Event Occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Timed Out" << endl;
      break;
    case CEventMonitor::Error:
      cout << "Error - predicate threw an exception" << endl;
      break;
    default:
      cout << "None of the conditions occurred" << endl;
    }


    /*=======================================================
      Monitor a location which changes asynchronously to the
      thread which is monitoring it.
      =====================================================*/

    int MonitorResult = 0;   // Result of locmon.operator()
    int retcode = 0;         // return code of pthread_cond_timedwait()
    struct timeval now;      // time to start monitoring
    struct timespec timeout; // amount of time for thread to be locked
    pthread_mutex_t locMut = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    // Lock the mutex
    pthread_mutex_lock(&locMut);
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 2;
    timeout.tv_nsec = now.tv_usec * 1000;
    int newValue = 31;
    locmon.setTimeout(5000);
    locmon.ChangeLocation(&newValue);
    cout << "\nChanging location during monitor..." << endl;
    cout << "(The monitor will wait for up to 5 seconds, but the location\n";
    cout << "being monitored will change in 2 seconds...)" << endl;
    printf("\nGetting contents of location: %d stored at address 0x%x\n",
	   locmon.getContents(), locmon.getLocation());
    while(retcode != ETIMEDOUT) {
      retcode = pthread_cond_timedwait(&cond, &locMut, &timeout);
    }
    pthread_mutex_unlock(&locMut);

    // Now the mutex is unlocked and we display the results
    MonitorResult = locmon();
    switch(MonitorResult) {
    case CEventMonitor::Occurred:
      cout << "Event occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed Out" << endl;
      break;
    case CEventMonitor::Error:
      cout << "Event had an error" << endl;
      break;
    default:
      cout << "Default condition" << endl;
    }
    
    cout << "\nDescribing self again:" << endl;
    cout << locmon.DescribeSelf() << endl;


    /*======================================================
      Now change the predicate to a CMaskedValuePredicate
      ====================================================*/

    cout << "\n#### Changing predicate to CMaskedValuePredicate ####" << endl;
  
    CMaskedValuePredicate<int> newPred(newValue);
    locmon.ChangePredicate(&newPred);
    cout << "\nChanged predicate. Describing self" << endl;
    cout << locmon.DescribeSelf() << endl;
    
    MonitorResult = locmon();
    switch(MonitorResult) {
    case CEventMonitor::Occurred:
      cout << "Event occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed Out" << endl;
      break;
    case CEventMonitor::Error:
      cout << "Event had an error" << endl;
      break;
    default:
      cout << "Default condition" << endl;
    }

    // Lock the mutex
    pthread_mutex_lock(&locMut);
    retcode = 0;
    cout << "\nChanging monitor location and predicate mask..." << endl;
    int nextval = 115;
    int val = 114;
    locmon.ChangeLocation(&nextval);
    CMaskedValuePredicate<int> nextPred(val, 0xfffffffe);
    locmon.ChangePredicate(&nextPred);
    printf("\nContents of location is %d stored at address 0x%x\n",
	   locmon.getContents(), locmon.getLocation());
    cout << locmon.DescribeSelf() << endl;
    cout << "(Note that (Value & Mask) = (" << nextval << " & 0xfffffffe) = "
	 << (nextval & 0xfffffffe) << ")" << endl;
    cout << "\nPerforming wait (note that mutex is locked for 2 seconds)..."
	 << endl;
    
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 2;
    timeout.tv_nsec = now.tv_usec * 1000;    
    while(retcode != ETIMEDOUT) {
      retcode = pthread_cond_timedwait(&cond, &locMut, &timeout);
    }
    pthread_mutex_unlock(&locMut);
    
    // Now unlock the mutex and get the results
    MonitorResult = locmon();
    switch(MonitorResult) {
    case CEventMonitor::Occurred:
      cout << "Event occurred" << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed Out" << endl;
      break;
    case CEventMonitor::Error:
      cout << "Event had an error" << endl;
      break;
    default:
      cout << "Default condition" << endl;
    }
  }
};

DAQClient mydaq;
