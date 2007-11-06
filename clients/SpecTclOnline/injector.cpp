// Inject a counting buffer into the local Spectrodaq server.
// The buffer will be tag 3

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <stdint.h>
#include <spectrodaq.h>

static const int BufferSize(4096);

class DAQBuff : public DAQROCNode {
private:
  int operator()(int argc, char** argv);
};


int
DAQBuff::operator()(int argc, char** argv) 
{
  uint16_t buffer[BufferSize];
  DAQWordBuffer words(BufferSize);
  for(int i =0; i < BufferSize; i++) {
    buffer[i] = i;
  }
  words.CopyIn(buffer, 0, sizeof(buffer) /sizeof(uint16_t));
  words.SetTag(3);
  words.Route();

  return 0;

}

DAQBuff TheInstance;

