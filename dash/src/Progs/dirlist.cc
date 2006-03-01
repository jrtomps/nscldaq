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
class Dirlist : public Main {
  void main(int argc,char *argv[]) {
    fprintf(stderr,"Dirlist is running\n");
    if (argc < 3) {
      fprintf(stderr,"Usage: dirlist <directory> <regex>\n"); 
      exit(-1);
    }

    String path(argv[1]);
    String rx(argv[2]);
    StringArray dlist;
    IntArray runlist;

    if (!FSUtils::isaDirectory(path)) {
      fprintf(stderr,"The path \"%s\" is not a directory\n",path.c_str());
      exit(-2);
    }
   
    // Get a full directory list 
    FSUtils::directoryList(dlist,path);

    fprintf(stdout,"There are %d files in directory \"%s\"\n",dlist.length,path.c_str());

    for (int i = 0; i < dlist.length; i++) {
      fprintf(stdout,"%s\n",dlist.elements[i]->c_str());   
    }

    dlist.clearAndDelete();

    // Now for filtered directory list
    FSUtils::directoryList(dlist,path,rx);

    fprintf(stdout,"There are %d files in directory \"%s\" that match regex=\"%s\"\n",dlist.length,path.c_str(),rx.c_str());

    for (int i = 0; i < dlist.length; i++) {
      fprintf(stdout,"%s\n",dlist.elements[i]->c_str());   
    }

    // Now let's sort them be run number
    DSHUtils::sortFilesByRunNumber(dlist);

    fprintf(stdout,"Sorted..........................\n");
    for (int i = 0; i < dlist.length; i++) {
      fprintf(stdout,"%s\n",dlist.elements[i]->c_str());   
    }

    dlist.clearAndDelete();

    // How about just extracting the run numbers...
    // Now let's sort them be run number
    DSHUtils::getDirectoryRunNumbers(runlist,path);

    fprintf(stdout,"Run numbers..........................\n");
    for (int i = 0; i < runlist.length; i++) {
      fprintf(stdout,"%d\n",runlist.elements[i]);   
    }
  }
};

Dirlist dirlist; 
