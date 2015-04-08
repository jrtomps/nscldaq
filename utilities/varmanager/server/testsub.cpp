#include <iostream>
#include <CVarMgrSubscriptions.h>
#include <stdlib.h>

static std::string service="vardb-changes";

// Usage:
//   programname host path

int main(int argc, char** argv) 
{
  const char* host = argv[1];
  const char* path = argv[2];


  CVarMgrSubscriptions sub(host, service.c_str());
  sub.subscribe(path);

  while (1) {
    CVarMgrSubscriptions::Message msg = sub.read();

    std::cout << "Path: " << msg.s_path << std::endl;
    std::cout << "Op  : " << msg.s_operation << std::endl;
    std::cout << "Data: " << msg.s_data << std::endl << std::endl;
  }
   
}
