#ifndef DAQHWYAPI_MAIN_H
#define DAQHWYAPI_MAIN_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#ifndef __DAQHWYAPI_CONFIG_H
#include <dshapi/daqconfig.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_PROCESS_H
#include <dshapi/Process.h>
#endif

using namespace daqhwyapi;

namespace daqhwyapi {

/**
* @class Main
* @brief Basic Unix style process class.
*
* A class that reprsents the main execution process for a program 
* implemented using this API.  Essentially, it is only necessary to create 
* a class that inherits from this one, implement the main() method and 
* declare an instance of that class.   The API will automatically call 
* the newly implemented main().
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Main : public Process {
  public: 
    Main();                       // Constructor
    ~Main();                      // Destructor
    void boot(int,char **,char **);  // Bootstrap the system
    void run();  // Start main
    virtual void main(int argc,char *argv[]) = 0;  

  private:
    int moveEnviron(char **);        // Move the environment
};

} // namespace daqhwyapi

#endif
