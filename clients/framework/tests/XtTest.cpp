//
// Tests the Xt event loop as a separate thread.
//
#include <spectrodaq.h>
#include <CXtEventLoop.h>
#include <XMPushbutton.h>
#include <iostream.h>
class CXMApplication : public CXtEventLoop
{
  XMPushButton* m_pButton;
protected:
  virtual void SetupWidgetTree(Widget top)
  {
    cerr << "Setting up widget tree\n";
    cerr.flush();
    m_pButton = new XMPushButton("Exit",
				 top,
				 CXMApplication::Exit,
				 (XtPointer)this);
    m_pButton->Manage();
    cerr << "Widget Tree set up.\n";
    cerr.flush();
  }
public:
  CXMApplication() :
    m_pButton(0)
  {
    cerr << "Motif app constructed\n";
    cerr.flush();
  }

private:
  static void  Exit(XMWidget* pMyWidget, XtPointer cd, XtPointer reason)
  {
    CXMApplication* pMe = (CXMApplication*)cd;
    delete pMe->m_pButton;
    pMe->exit();
  }

};

class Main : public DAQROCNode
{
  int operator()(int argc, char** argv)
  {
    CXMApplication app;
    DAQThreadId   tid = daq_dispatcher.Dispatch(app);
    DAQStatus     stat;

    Join(tid, &stat);
    cerr << "Motif gui thread exited: " << stat.GetStatusCode() << endl;
    return 0;
  }
};

static Main myApp;
