#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <spectrodaq.h>
#include <CReadoutMain.h>
#include <CExperiment.h>
#include <CInterpreterStartup.h>
#include <CInterpreterCore.h>
#include <CRunVariableCommand.h>
#include <CRunVariable.h>
#include <CStateVariableCommand.h>
#include <CStateVariable.h>
#include <TCLInterpreter.h>
#include <CDAQTCLProcessor.h>
#include <CVMEScalerLRS1151.h>
#include <CTraditionalEventSegment.h>
#include <CTraditionalScalerReadout.h>
#include <iostream>

class DAQmain : public CReadoutMain {
  
  int main(int argc, char** argv);
protected:
  virtual int operator() (int argc, char** argv) {
    main(argc, argv);
    exit(0);
  }
public:
  virtual ~DAQmain() {}
};

DAQmain MyApp;

int 
DAQmain::main(int argc, char** argv)
{
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
    cerr << "Caught a string exception from tests: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  return !wasSucessful;
}
