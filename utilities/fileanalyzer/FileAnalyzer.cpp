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

#include <CFatalException.h>
#include <CFilterMain.h>

#include "CSourceCounterFilter.h"

#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

void printSpecialUsage() {
  std::cout << "  -O" << std::endl;
  std::cout << "  --output-file   The name of the file to write statistics to" << std::endl;
}

std::vector<std::string> argV;
std::string outputFile;

std::vector<std::string> 
cArgsToCppArgs(int argc, char* argv[]) 
{
  std::vector<std::string> args(argc);
  for (int i=0; i<argc; ++i) {
    args[i] = std::string(argv[i]);
  }
  return args;
}


std::vector<std::string> 
processAndRemoveSpecialArgs(const std::vector<std::string>& argv)
{
  std::vector<std::string> filteredArgv;
  for (size_t i=0; i<argv.size(); ++i) {
    std::string option(argv[i]);
//    std::cout << option << std::endl;
    if (option.find("--output-file") == 0) {
      if (option.size() == 13) {
        outputFile = argv[i+1]; 
        ++i;
      } else {
        outputFile = option.substr(14); 
      }
    } else if (option.find("-O")== 0 ) {
      if (option.size() == 2) {
        outputFile = argv[i+1]; 
        ++i;
      } else {
        outputFile = option.substr(3); 
      }
    } else if (option == "--help" || option == "-h") {
      atexit( printSpecialUsage );
    } else {
      filteredArgv.push_back(std::string(argv.at(i)));
    } 
  }
  return filteredArgv;
}

char** createNewCArgV(const std::vector<std::string>& argV)
{
  char** pArgV = new char*[argV.size()];
  for (int i=0; i<argV.size(); ++i) {
    pArgV[i] = new char[argV[i].size()];
    strcpy(pArgV[i], argV[i].data());
  }

  return pArgV;
}

/// The main function
/**! main function
  Creates a CFilterMain object and 
  executes its operator()() method. 

  \return 0 for normal exit, 
          1 for known fatal error, 
          2 for unknown fatal error
*/
int main(int argc, char* argv[])
{
  int status = 0;

  try {

    auto newArgV = cArgsToCppArgs(argc, argv);
    newArgV = processAndRemoveSpecialArgs(newArgV);
    if (newArgV == argV) {
      std::cout << "User did not provide an output file. Specify --output-file or -O option" << std::endl;
      return 1;
    }

    argc = newArgV.size();
    argv = createNewCArgV(newArgV);

    // Create the main
    CFilterMain theApp(argc,argv);

    // Construct filter(s) here.
    CSourceCounterFilter srcCounter(std::numeric_limits<uint32_t>::max(), outputFile);

    theApp.registerFilter(&srcCounter);

    // Run the main loop
    theApp();

  } catch (CFatalException exc) {
    status = 1;
  } catch (...) {
    std::cout << "Caught unknown fatal error...!" << std::endl;
    status = 2;
  }

  return status;
}

