/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <string>

#ifndef DAQHWYAPI_MAIN_H
#include <dshapi/Main.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

using namespace daqhwyapi;

namespace daqhwyapi {
  Main *__daqhwyapi_mainsystem__ = NULL;
}

/*=======================================================================*/
int daqhwyapi_main(int argc,char **argv,char **envp)
{
  if (daqhwyapi::__daqhwyapi_mainsystem__ == NULL) {
    cerr << "daqhwyapi_main(): Cannot begin execution: __daqhwyapi_mainsystem__ is not initialized (Did you instantiate a Main object?)" << endl;
    return(-1);
  }

  try {
    (*daqhwyapi::__daqhwyapi_mainsystem__).boot(argc,argv,envp);
    // We call the nonforking start for the main process.
    (*daqhwyapi::__daqhwyapi_mainsystem__).start_main();
  } catch (std::runtime_error& re) {
    cerr << "*** Runtime error caught: \"" << (re.what()) << "\"" << endl;
    exit(-1);
  } catch (daqhwyapi::Exception& oe) {
    cerr << "*** Exception caught: \"" << (oe.what()) << "\"" << endl;
    exit(-2);
  } catch (std::exception& se) {
    cerr << "*** Exception caught: \"" << (se.what()) << "\"" << endl;
    exit(-3);
  } catch (std::string& stre) {
    cerr << "*** std::string exception caught: \"" << (stre.c_str()) << "\"" << endl;
    exit(-4);
  } catch (char *chre) {
    cerr << "*** char* exception caught: \"" << chre << "\"" << endl;
    exit(-5);
  } catch (...) {
    cerr << "*** Unknown exception caught" << endl;
    return(-errno);
  }

  return(0);
}
