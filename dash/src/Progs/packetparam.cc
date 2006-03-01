using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <netinet/in.h>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif


using namespace daqhwyapi;

/*===================================================================*/
class PacketParam : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stderr,"PacketParam is running\n");
    if (argc < 2) {
      fprintf(stderr,"Usage: packetparam <packet>\n"); 
      exit(-1);
    }

    String packet(argv[1]);
    uint32_t range[2];

    DSHUtils::parsePacketParam(packet,range);
    fprintf(stdout,"Range = %d,%d\n",range[0],range[1]);
  }
};

PacketParam pparam; 
