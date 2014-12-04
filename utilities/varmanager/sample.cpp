

#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <string>
#include <exception>
#include <iostream>
#include <utility>
#include <stdlib.h>

const char* topdir="/example";


static 
std::pair<CVariableDb*, CVarDirTree*>
MakeDirectory(const char* dbName, const char*  path)
{
  try {

    CVariableDb* db = new CVariableDb(dbName);
    CVarDirTree* pDir = new CVarDirTree(*db);
    pDir->mkdir(path);
    pDir->cd(path);


    return std::pair<CVariableDb*, CVarDirTree*>(db, pDir);
  }
  catch(std::exception& e) {
    std::cerr << "Exiting from MakeDirectory Because: \n";
    std::cerr << e.what() << std::endl;
    exit(-1);
  }
}

int main(int argc, char** argv)
{
  std::string makeThis=topdir;
  makeThis += "/";
  makeThis += "/mydir";
  MakeDirectory("myvariables.db", makeThis.c_str());
}
