#include <spectrodaq.h>
#include <SpectroFramework.h>

#include <iostream.h>

volatile long location = 0;	// Location monitored.

typedef CLocationEvent<long> LongMonitor;
typedef CPointerPredicate<long> LongPred;
typedef CChangedPredicate<long> LongChanged;

class MyLocation : public LongMonitor
{
public:
  MyLocation(const char* pName, LongPred& pred, long* pLoc); //  Constructor.
  virtual void OnLocationChanged(long newval);

};

MyLocation::MyLocation(const char* pName, LongPred& pred,  long* pLoc) :
  LongMonitor(pName, pLoc,  pred)
{
}
void
MyLocation::OnLocationChanged(long newval)
{
  cout << "Location changed. New value is: " << newval <<endl;
}


class MyApp : public DAQROCNode
{
protected:
  int operator()(int argc, char** argv);
};

MyApp theApp;

int
MyApp::operator()(int argc, char** argv)
{
  LongChanged predicate(location);
  MyLocation LocMonitor("TriggeronChanged", predicate, (long*)&location);

  LocMonitor.Enable();

  cout << " Location event: " << LocMonitor.DescribeSelf() << endl;
  cout << "Started\n";

  string input;
  while(1) {
    cin >> input;		//  Block until input...
    location++;			// Update location.
  }
}
