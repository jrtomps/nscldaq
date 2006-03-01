#ifndef DAQHWYAPI_OUTPUTSTREAM_H
#define DAQHWYAPI_OUTPUTSTREAM_H

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

class BufferedWriter;

/**
* @class OutputStream
* @brief OutputStream for constructing streams.
*
* Interface for constructing an input stream.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class OutputStream : public Object {
  public: 
    virtual void close() = 0;     // Close this stream
    virtual int write(int) = 0;       // Write a byte to this stream
    virtual int write(ubyte*,int) = 0; // Write bytes to this stream 
    virtual int write(ubyte*,int,int) = 0; // Write bytes to this stream 
    virtual void flush() = 0; // Flush the stream

  protected:
    friend class daqhwyapi::BufferedWriter;
    virtual int write_output(ubyte*,int,int) = 0; 
};

} // namespace daqhwyapi

#endif
