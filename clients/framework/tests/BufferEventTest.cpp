// This program tests the 
// CBufferEvent class.
//   We derive a subclass from CBufferEvent
//   which dumps the first n words of the buffer.
// A derivation of a CFileEvent listens for input on
// stdin.  If the input was numeric, the number of buffer
// words to be dumped is modified to be that number.
// If not, or the input is 0, the program exits.
//
#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <CFileEvent.h>
#include <CBufferEvent.h>


// Class to dump data:

typedef CBufferEvent<Word> CWordBufferEvent;

class Dumper : public CWordBufferEvent
{
  unsigned int m_nWords;	// Number of words to dump.
public:
  Dumper() :
    CWordBufferEvent("BufferDumper"),
    m_nWords(32)
  {}
  virtual void OnBuffer(Pointer<DAQBuffer<Word>,Word>& pBuffer);
  void setWordCount(unsigned int i) {
    m_nWords = i;
  }
};

// Class to process stdin:

class BufferTest;		// Forward references.
class StdinProcessor : public CFileEvent
{
  Dumper&     m_rDumper;
  BufferTest& m_rMain;
  
public:
  StdinProcessor(BufferTest& rMain, Dumper& rDumper) :
    CFileEvent(0, "WCAdjust"),
    m_rDumper(rDumper),
    m_rMain(rMain)
    {}
  virtual void OnReadable(istream& rInput);
};


// Main program class

class BufferTest : public DAQROCNode
{
  DAQThreadCond Synch;
  DAQThreadMutex mtx;
public:
  void AllDone() {Synch.Signal(); }
protected:
  virtual int operator()(int argc, char** argv);
};

//
// Implementation of main program:

int
BufferTest::operator()(int argc, char** argv)
{
  Dumper Dump;
  Dump.AddLink("tcp://localhost:2602", 2);
  Dump.setBufferTag(2);
  Dump.setBufferMask(2);
  Dump.Enable();

  StdinProcessor console(*this, Dump);
  console.Enable();

  Synch.Wait(mtx);
  exit(0);
}

// instantiation of main buffer. class.

BufferTest app;


// Implementation of the buffer dumper class:
void
Dumper::OnBuffer(Pointer<DAQBuffer<Word>,Word>& pBuffer)
{
  cout << "-----------------------------------";
  cout << hex;
  for(int i = 0; i < m_nWords; i++) {
    if((i % 16) == 0) cout << '\n';
    cout << *pBuffer << ' ';
    ++pBuffer;
  }
  cout << '\n';
  cout << dec;
}

// Implementation of StdinProcessor
//
void
StdinProcessor::OnReadable(istream& rInput)
{
  string line;
  int    nwds;
  rInput >> line;
  if((nwds = atoi(line.c_str())) == 0) { // Either convert failed or 0 entered.
    m_rMain.AllDone();		// Signal main to exit.
  }
  m_rDumper.setWordCount((unsigned int) nwds);
}
