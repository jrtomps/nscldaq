#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>
#include <string>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  std::string uri = argv[1];
  CVarMgrApi*  api = CVarMgrApiFactory::create(uri);
  api->mkdir("/mydir");
  api->cd("/mydir");
  api->declare("counter", "integer");

  int value(0);

  while(1) {
    char strCounter[100];
    sleep(1);
    value++;
    sprintf(strCounter, "%d", value);
    api->set("counter", strCounter);
    
  }
}
