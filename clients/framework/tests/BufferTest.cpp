#include <spectrodaq.h>
#include <CBufferMonitor.h>
#include <CBufferReactor.h>
#include <iostream.h>
#include <unistd.h>

// Size of the buffer to use
#define BUFLEN 256

class MyBufferReactor : public CWordBufferReactor
{
public:
  MyBufferReactor(const char* pName) : 
    CWordBufferReactor(pName)
  {
    AppendClassInfo();
  }
  virtual void OnTimeout(CEventMonitor& rMonitor);
  virtual void OnBuffer(CWordBufferMonitor& rMonitor,
			DAQWordBufferPtr pBuffer);
};

void
MyBufferReactor::OnTimeout(CEventMonitor& rMonitor)
{
  cout << " Timeout from: " << DescribeSelf() << endl;
  cout << "Triggered by: "  << rMonitor.DescribeSelf() << endl;
  cout.flush();
}

void 
MyBufferReactor::OnBuffer(CWordBufferMonitor& rMonitor,
			  DAQWordBufferPtr pBuffer)
{
  int nSize = rMonitor.getBuffer().GetLen();
  //
  // Dump; the buffer:
  //
  cout << DescribeSelf() << ": Dumping buffer received from " <<
    rMonitor.DescribeSelf() << endl;
  cout << hex;
  for(int i = 0; i < nSize; i++) {
    if((i % 8) == 0) cout << endl;
    cout << *pBuffer << ' ';
    ++pBuffer;
  }
  cout << dec << endl;
  cout.flush();
}

class DAQClient : public DAQROCNode {
  int operator() (int argc, char** argv) {

    /*===========================================================
      The following code demonstrates the CBufferMonitor class
      
      Here's what it does: it creates a buffer to send, then it
      sleeps for a couple of seconds (but not long enough for
      the event to timeout) then it receives the buffer into
      the buffer monitor.
      =========================================================*/  
    
    cout << "\n####The following code demonstrates the CBufferMonitor"
         << " class####" << endl << endl;
    DAQWordBuffer sbuf(BUFLEN); // the buffer to send

    CBufferMonitor<Word> buffermon;
    buffermon.setTimeout(5000);
    string sinkurl("TCP://localhost:2602/");
    long sinkid;
    
    if(argc > 1) {
      cerr << "Using URL: " << argv[1] << endl;
      sinkurl = argv[1];
    }

    sbuf.SetTag(2); // set the buffer's tag which we are sending

    buffermon.SetBufferTag(2);  // set the mask and tag of the buffer to
    buffermon.SetBufferMask(2); // receive the first buffer into.

    // Try adding a link to the monitor
    try {
      sinkid = buffermon.AddLink(sinkurl, 2, 2);
    }
    catch (CLinkFailedException& lfe) {
      cerr << "Failed to add link to link manager" << endl;
      cerr << lfe.ReasonText() << endl;
      exit(-1);
    }
    cout << "Added Sink Id: " << sinkid << endl;

    // Test timeout receive:

    switch(buffermon()) {
    case CEventMonitor::Occurred:
      cout << "Event occured, but should have been timeout\n";
      break;
    case CEventMonitor::TimedOut:
      cout << "Got a timeout as we should have\n";
      break;
    default:
      cout << "Got an error from receipt\n";
      break;
    }
  

    // Resize the buffer (since routing it will result in
    // the buffer length being zero.)
    sbuf.Resize(BUFLEN, true);
    if(sbuf.GetLen() > 0) {
      for(int i = 0; i < sbuf.GetLen(); i++) sbuf[i] = Byte(i&0x00ff);
      cerr << DAQCSTR("buffersend") << endl;
      sbuf.Dump(cerr, 30);
      cerr << "Sending buffer in 3 seconds...";
      sleep(3);
      cerr << "sent" << endl;
      sbuf.Route();
    }
    else {
      cerr << DAQCSTR("Failed to resize buffer.") << endl;
    }

    // Try to accept the buffer...
    switch(buffermon()) {
    case CEventMonitor::Occurred:
      cout << "Event Occurred" << endl;
      // Dump some buffer info
      cout << "Received one buffer" << endl;
      cout << "LEN(bufrecv): " << buffermon.getBuffer().GetLen() << endl;
      break;
    case CEventMonitor::TimedOut:
      cout << "Event Timed out" << endl;
      break;
    default:
      cout << "Neither of the other cases occurred" << endl;
    }

    // Describe the buffer monitor
    cout << "\nDescribing myBufferMonitor:" << endl;
    cout << buffermon.DescribeSelf() << endl;

    // Now use a buffer Reactor too:

    sbuf.Resize(BUFLEN, true);

    for(int i = 0; i < sbuf.GetLen(); i++) sbuf[i] = Byte(i&0x00ff);
    sbuf.Route();
    cout << "Using a buffer reactor: " << endl;
    MyBufferReactor George("George");
    George(buffermon, buffermon());

    // Next one should time out:

    buffermon.setTimeout(5000);
    
    George(buffermon, buffermon());
    
  }
};

DAQClient mydaq;
