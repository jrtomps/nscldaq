/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
  std::cout << "Starting " << argv[0] << std::endl;
  CppUnit::TextUi::TestRunner   
               runner; // Control tests.
  std::cout << "Test Runner created\n";
  CppUnit::TestFactoryRegistry& 
               registry(CppUnit::TestFactoryRegistry::getRegistry());

  std::cout << "Registry created\n";
  runner.addTest(registry.makeTest());
  std::cout << "Tests added\n";

  bool wasSucessful;
  try {
    std::cout << "Running tests\n";
    wasSucessful = runner.run("",false);
    std::cout << " Done running tests\n";
  } 
  catch(string& rFailure) {
    cerr << "Caught a string exception from test suites.: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  std::cerr << "Exiting\n";
  return !wasSucessful;
}
