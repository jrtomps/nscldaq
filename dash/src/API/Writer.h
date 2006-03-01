#ifndef DAQHWYAPI_WRITER_H
#define DAQHWYAPI_WRITER_H

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
* @class Writer
* @brief Writer interface definition.
*
* The interface definition for the class that implements an Writer.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Writer : public Object {
  public: 
    virtual void close() = 0;     // Close this stream
    virtual int write(int) = 0;       // Write a byte to this stream
    virtual int write(ubyte*,int) = 0;   // Write an array of characters
    virtual int write(ubyte*,int,int) = 0; // Writer bytes to this stream
    virtual void flush() = 0; // Flush stream
};

} // namespace daqhwyapi

#endif
