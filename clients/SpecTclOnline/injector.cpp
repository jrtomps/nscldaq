// Inject a counting buffer into the local Spectrodaq server.
// The buffer will be tag 3

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


#include <spectrodaq.h>

static const int BufferSize(4096);

class DAQBuff : public DAQROCNode {
private:
  int operator()(int argc, char** argv);
};


int
DAQBuff::operator()(int argc, char** argv) 
{
  DAQWordBuffer words(BufferSize);
  for(int i =0; i < BufferSize; i++) {
    words[i] = i;
  }
  words.SetTag(3);
  words.Route();

  return 0;

}

DAQBuff TheInstance;
