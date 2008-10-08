#include <TCLApplication.h>
#include <TCLLiveEventLoop.h>

class evttclsh : public CTCLApplication
{
public:
  virtual int operator()();
};

int
evttclsh::operator()()
{
  CTCLLiveEventLoop* pLoop = CTCLLiveEventLoop::getInstance();
  pLoop->start();
  return TCL_ERROR;
}

CTCLApplication* gpTCLApplication = new evttclsh;
