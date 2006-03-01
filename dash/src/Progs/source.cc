using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifndef DAQAPILIB_H
#include <dshapi/daqhwyapi.h>
#endif

using namespace daqhwyapi;

#define FSIZE (1024*1024*50)
#define LOOPS 10

/*===================================================================*/
class Source : public Main {
  void main(int argc,char *argv[]) {
    if (argc < 2) {
      fprintf(stderr,"Usage %s <packet size>\n",argv[0]);
      exit(0);
    }

    int pcktsiz = atoi(argv[1]);
    ubyte buf[pcktsiz];
    memset((void*)buf,'.',pcktsiz);
    buf[pcktsiz-1] = '\n';

#ifdef MYBUFSIZ
    FdOutputStream fout(1,MYBUFSIZ);
#else
    FdOutputStream fout(1,pcktsiz);
#endif

    int rc = fout.write(buf,pcktsiz);
    while (rc >= 0) rc = fout.write(buf,pcktsiz);
    fout.close();
  }
};

Source source; 
