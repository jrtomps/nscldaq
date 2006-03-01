using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifndef DAQAPILIB_H
#include <dshapi/daqhwyapi.h>
#endif

using namespace daqhwyapi;

#include "calcstdavg.h"

#define FSIZE (1024*1024*50)
#define LOOPS 10

/*===================================================================*/
class Filter : public Main {
  void main(int argc,char *argv[]) {
    if (argc < 3) {
      fprintf(stderr,"Usage %s <id> <packet size>\n",argv[0]);
      exit(0);
    }

    int id = atoi(argv[1]);
    int pcktsiz = atoi(argv[2]);
    ubyte buf[pcktsiz];
    memset((void*)buf,'.',pcktsiz);
    buf[pcktsiz-1] = '\n';

    double vals[LOOPS];
    double vals2[LOOPS];

#ifdef MYBUFSIZ
    fprintf(stderr,"# bufsiz=%d pcktsiz=%d\n",MYBUFSIZ,pcktsiz);
    FdInputStream  fin(0,MYBUFSIZ);
    FdOutputStream fout(1,MYBUFSIZ);
#else
    fprintf(stderr,"# bufsiz=%d pcktsiz=%d\n",pcktsiz,pcktsiz);
    FdInputStream  fin(0,pcktsiz);
    FdOutputStream fout(1,pcktsiz);
#endif

    fin.read(buf,pcktsiz); // wait for first packet

    for (int i = 0; i < LOOPS; i++) {
      double stime = System.currentTimeMillis();
      int cnt = 0;
      while (cnt < FSIZE) {
        int rc = fin.read(buf,pcktsiz);
        if (rc < 0) break;
        rc = fout.write(buf,rc);
        if (rc < 0) break;
        cnt += rc;
      }
      double etime = System.currentTimeMillis();

      double millisecs = etime - stime;
      double secs = millisecs/1000.0;

      double bits = 8.0 * cnt;
      double bw = (bits/(1024.0*1024.0))/secs; 
      vals[i] = bw;
      vals2[i] = millisecs / (cnt / (1024.0 * 1024.0));
    }
 
    double s = 0, a = 0;
    double s2 = 0, a2 = 0;
    calcstdavg(vals,LOOPS,&a,&s);
    calcstdavg(vals2,LOOPS,&a2,&s2);
    fprintf(stderr,"%d %0.4lf %0.4lf %0.4lf %0.4lf %d\n",pcktsiz,a,s,a2,s2,id); 

    fin.close();
    fout.close();
  }
};

Filter filter; 
