using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>

#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#include "gettime.h"


#define BUFLEN 16384
#define BUFLEN2 8192

/*===================================================================*/
class DAQBuff : public DAQROCNode {
  int operator()(int argc,char **argv) {
    OstreamCompat oerr(cerr);
    DAQWordBuffer *pbbuf;
    DAQWordBufferPtr bpx;
    volatile Word *pmem = new Word[BUFLEN];

    pbbuf = new DAQWordBuffer(8192);
    bpx = &(*pbbuf);
    cerr << "Initial buffer length " << pbbuf->GetLen() << endl;

    // pbbuf->CopyOut((void*)pmem,0,BUFLEN2);
    // bpx.CopyOut((void*)pmem,0,BUFLEN2);

    for (int i = 0; i < (8192-10); i += 10) {
      // bpx.CopyOut((void*)pmem,i,10);
      cerr << "CopyIn(0x" << (void*)pmem << "," << i << ",10)" << endl;
      pbbuf->CopyIn((void*)pmem,i,10);
      cerr << "CopyOut(0x" << (void*)pmem << "," << i << ",10)" << endl;
      pbbuf->CopyOut((void*)pmem,i,10);
    }

    pbbuf->DumpNative(oerr,10);
    pbbuf->Release();
    delete pbbuf; pbbuf = NULL;

    return(0);
  } 
};

DAQBuff mydaq;
