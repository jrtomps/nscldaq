//
// Test program for the TcpHoist program.
// We are a client of spectrodaq.
// on entry we:
//   Make an all accepting sink for localhost.
//   fork off a server, that when connected will serve a single 8Kbyte buffer
//     with a counting pattern.
//   system() a TcpHoist program to accept this buffer and submit it.
//   Accept the buffer and check that it has the counting pattern.
//

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>

#include <Iostream.h>



class DAQBuff : public DAQROCNode {
private:
  int   operator()(int argc, char** argv);
  int  CheckBuffer(DAQWordBuffer& buffer){return 0;}

};

int CheckBuffer(DAQWordBuffer& buffer) {
  int size = buffer.GetLen();
  for(int i =0; i < size; i++) {
    if (buffer.GetPtr()[i] != i) {
      cerr << "Mismatch sb " << i << " was " << buffer.GetPtr()[i] << endl;
      return -1;
    }
  }
  return 0;
}

// Program entry point:

int
DAQBuff::operator()( int argc, char** argv)
{
  DAQWordBuffer  buffer(0);
  DAQURL url("tcp://localhost:2602/");
  long sink     = daq_link_mgr.AddSink(url, 3U, ALLBITS_MASK, COS_RELIABLE);
  if(sink == 0) {
    cerr << "Sink creation failed.  probably spectrodaq is not started\n";
    return -1;
  }
  buffer.SetTag(2);
  
  buffer.Accept();	// Accept the buffer.
  cerr << " top level: Got a buffer\n";

  if( (CheckBuffer(buffer)) == 0) {
    cerr << "PASSED TEST!!\n";
    return 0;
  }
  return -1;
  
  
}




// An instance of the app is required.

DAQBuff myapp;
