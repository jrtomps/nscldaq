
#include <iostream>
#include <CFatalException.h>
#include <CFilterMain.h>

#include "CTemplateFilter.cpp"

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

    // Create the main
    CFilterMain theApp(argc,argv);


    // Construct filter(s) here.
    CTemplateFilter user_filter;

    // Register the filter(s) here. Note that if more than
    // one filter will be registered, the order of registration
    // will define the order of execution. If multiple filters are
    // registered, the output of the first filter will become the
    // input of the second filter and so on. 
    theApp.registerFilter(&user_filter);

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

