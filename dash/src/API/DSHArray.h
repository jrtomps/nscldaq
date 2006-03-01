#ifndef DAQHWYAPI_DSHARRAY_H
#define DAQHWYAPI_DSHARRAY_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

namespace daqhwyapi {

/**
* @class DSHArray
* @brief DSHArray interface class.
*
* The DSHArray interface class that defines the interface to
* DSH API Arrays. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DSHArray : public Object {
  public: 
    virtual void clear() = 0;  // Empty this Array.
    virtual void renew(unsigned long) = 0;  // Renew this array
};

} // namespace daqhwyapi

#endif
