#ifndef DAQHWYAPI_SELECTOR_H
#define DAQHWYAPI_SELECTOR_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

namespace daqhwyapi {

/*=====================================================================*/
/**
* @class Selector
* @brief Basic selector interface.
*
* Basic selector interface for implementing selection operations
* on sets of file descriptors or other objects or primitives.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Selector : public Object {
  public:
    virtual int select() = 0;          // Wait forever
    virtual int select(long long) = 0; // With time in microseconds
    virtual int poll() = 0; // Poll for events
};

} // namespace daqhwyapi

#endif
