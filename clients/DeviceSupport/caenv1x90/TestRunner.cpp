#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <stdio.h>
#include <iostream>

using namespace std;

// Global data used by tests:

unsigned long ModuleBase;	// Where the TDC lives in VME space.

//
static void Usage()
{
  cerr << "Usage: \n";
  cerr << "   tests base\n";
  cerr << " base - the base address of the module under test\n";

}

int main(int argc, char** argv)
{
  if(argc != 2) {
    Usage();
    exit(-1);
  }
  char* base = argv[1];
  if(sscanf(base, "%lx", &ModuleBase) != 1) {
    cerr << "Failed to scan module base: " << base << endl;
    Usage();
    exit(-1);
  }
  cerr << "Module base is 0x" << hex << ModuleBase << dec << endl;

  CppUnit::TextUi::TestRunner   
               runner; // Control tests.
  CppUnit::TestFactoryRegistry& 
               registry(CppUnit::TestFactoryRegistry::getRegistry());

  runner.addTest(registry.makeTest());

  bool wasSucessful;
  try {
    wasSucessful = runner.run("",false);
  } 
  catch(string& rFailure) {
    cerr << "Caught a string exception from test suites.: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  return !wasSucessful;
}
