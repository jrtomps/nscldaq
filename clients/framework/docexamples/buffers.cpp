#include <spectrodaq.h>
#include <SpectroFramework.h>
#include <iostream.h>

typedef CBufferEvent<Word> WordBufferEvent;
typedef Pointer<DAQBuffer<Word>,Word> WordPointer;


static const int WordsToDump = 64;
static const int WordsPerLine= 16;
class MyBuffers : public WordBufferEvent
{
  ostream& m_rOutfile;
public:
  MyBuffers(const char* pName, ostream& routfile);
  virtual void OnBuffer(WordPointer& pBuffer);
};

MyBuffers::MyBuffers(const char* pname, ostream& routfile) :
  WordBufferEvent(pname),
  m_rOutfile(routfile)
{}

void
MyBuffers::OnBuffer(WordPointer& pBuffer)
{
  m_rOutfile << "---------------------- buffer -----------------";
  m_rOutfile << hex;
  for(int i =0; i < WordsToDump; i++) {
    if( (i% WordsPerLine) == 0) {
      cout << endl;
    }
    cout << *pBuffer << " ";
    ++pBuffer;
  }
  m_rOutfile << endl;

  m_rOutfile << dec;
}

class MyApp : public DAQROCNode
{
protected:
  int operator() (int argc, char** argv);

};
MyApp theApplication;

int
MyApp::operator()(int argc, char** argv)
{
  MyBuffers Receiver("BuferReceiver", cout);
  Receiver.setBufferTag(0);
  Receiver.setBufferMask(0);
  
  Receiver.AddLink("TCP://localhost:2602/", 0, 0);   // Relieable, all events
  Receiver.Enable();
  
  DAQThreadId id = Receiver.getThreadId();
  
  Join(id);
}

