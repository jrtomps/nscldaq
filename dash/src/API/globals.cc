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
#include <iostream>
#include <string>

#include <dshapi/cstr.h>

// Globals used by cstr, Process and Main
namespace daqhwyapi {
  int    daqhwyapi_argc = 0;
  char **daqhwyapi_argv = NULL;
  char **daqhwyapi_envp = NULL;
  int    daqhwyapi_envspace = 0;
  char daqhwyapi_work_area[MAX_SCRATCHPAD_AREA]; 
} // namespace daqhwyapi
