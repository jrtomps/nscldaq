#ifndef DAQHWYAPI_READER_H
#define DAQHWYAPI_READER_H

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
* @class Reader
* @brief Reader interface definition.
*
* The interface definition for the class that implements an Reader.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Reader : public Object {
  public: 
    virtual bool ready() = 0;  // Stream has characters to read
    virtual void close() = 0;     // Close this stream
    virtual int read() = 0;       // Read a byte from this stream
    virtual int read(ubyte*,int) = 0;   // Read an array of characters
    virtual int read(ubyte*,int,int) = 0; // Read bytes from this stream
    virtual long skip(long) = 0; // Skip characters.
    virtual bool eof() = 0; // At end-of-file
};

} // namespace daqhwyapi

#endif
