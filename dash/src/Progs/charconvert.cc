using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif


using namespace daqhwyapi;

/*===================================================================*/
class CharConvert : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stderr,"CharConvert is running\n");
    if (argc < 2) {
      fprintf(stderr,"Usage: charconvert <string>\n"); 
      exit(-1);
    }

    String teststr(argv[1]);
    fprintf(stdout,"Original String: ");
    fwrite(teststr.c_str(),1,teststr.size(),stdout); 
    fprintf(stdout,"\n");

    DSHUtils::convertEscapeCharacters(teststr);

    fprintf(stdout,"Converted String: ");
    fwrite(teststr.c_str(),1,teststr.size(),stdout); 
    fprintf(stdout,"\n");
  }
};

CharConvert charconvert; 
