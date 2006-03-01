#ifndef DAQHWYAPI_OBJECT_H
#define DAQHWYAPI_OBJECT_H

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

namespace daqhwyapi {

/*=====================================================================*/
/**
* @class Object
* @brief Basic object class.
*
* Basic object class to provide base object class from which
* other classes can inherit.  
*
* Does little currently except provide a general class from which
* to build collections or other constructs that can process
* more than one object type.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Object { 
};

} // namespace daqhwyapi

#endif
