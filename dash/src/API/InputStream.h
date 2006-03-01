#ifndef DAQHWYAPI_INPUTSTREAM_H
#define DAQHWYAPI_INPUTSTREAM_H

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

class BufferedReader;

/**
* @class InputStream
* @brief InputStream for constructing streams.
*
* Interface for constructing an input stream.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class InputStream : public Object {
  public: 
    virtual long available() = 0; // Number of characters available for reading
    virtual bool ready() = 0;  // Characters readable without blocking
    virtual void close() = 0;     // Close this stream
    virtual int read() = 0;       // Read a byte from this stream
    virtual int read(ubyte*,int) = 0; // Read bytes from this stream 
    virtual int read(ubyte*,int,int) = 0; // Read bytes from this stream 
    virtual long skip(long) = 0; // Skip characters.
    virtual bool eof() = 0; // At end-of-file

  protected:
    friend class daqhwyapi::BufferedReader;
    virtual int read_input(ubyte*,int,int) = 0; 
};

} // namespace daqhwyapi

#endif
