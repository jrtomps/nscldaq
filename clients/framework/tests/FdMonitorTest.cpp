#include <iostream.h>
#include <fstream.h>
#include "CNamedObject.h"
#include "CObjectRegistry.h"
#include "CClassifiedObjectRegistry.h"
#include "CRegisteredObject.h"
#include "CFdMonitor.h"
#include <CFdReactor.h>
#include "CDuplicateNameException.h"
#include "CNoSuchObjectException.h"
#include <assert.h>
#include <stdio.h>


class CToStdOutput : public CFdReactor
{
  bool m_Done;
public:
  CToStdOutput(const char* pName) : 
    CFdReactor(pName),
    m_Done(false)
    {}
  virtual void OnReadable(CFdMonitor& rMonitor, int fd);
  virtual void OnTimeout(CEventMonitor& rMonitor);
  bool Continue();
};
void
CToStdOutput::OnReadable(CFdMonitor& rMonitor, int fd)
{
  string input;
  int c;
  FILE* pF=fdopen(fd, "r");
  while((c=fgetc(pF)) != '\n') {
    input += c;
  }
  cout << input << endl;
  cout.flush();
}
void
CToStdOutput::OnTimeout(CEventMonitor& rMonitor)
{
  m_Done = true;
}
bool
CToStdOutput::Continue() {
  return !m_Done;
}
int main()
{

  ios::sync_with_stdio();
  CFdMonitor fdMon(1);
  fdMon.setTimeout(15123);   // in m.s.
  fdMon.MonitorReadable();
  fdMon.MonitorWritable(0);
  fdMon.MonitorExceptions();

  cout << "\nDescribing self: \n";
  cout << fdMon.DescribeSelf() << endl;

  cout << "\nWaiting for file descriptor..." << endl;
  switch(fdMon()) {
  case CEventMonitor::Occurred:
    cout << "Event occurred" << endl;
    break;
  case CEventMonitor::TimedOut:
    cout << "Event timed out" << endl;
    break;
  case CEventMonitor::Error:
    cout << "Event had error" << endl;
    break;
  default:
    cout << "None of the other cases" << endl;
  }

  // Now use in conjunction with the monitor.  We accept input and
  // echo it to stdout until a timeout.. at which time we exit.
  //

  CToStdOutput Gigo("Harry");
  while(Gigo.Continue()) {
    Gigo(fdMon, fdMon());
  }
}
