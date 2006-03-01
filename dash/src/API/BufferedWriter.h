#ifndef DAQHWYAPI_BUFFEREDWRITER_H
#define DAQHWYAPI_BUFFEREDWRITER_H

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

#ifndef DAQHWYAPI_WRITER_H
#include <dshapi/Writer.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/OutputStream.h>
#endif

namespace daqhwyapi {

/**
* @class BufferedWriter
* @brief BufferedWriter for writing character streams.
*
* BufferedWriter for writing character streams.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedWriter : public Writer {
  public: 
    BufferedWriter(OutputStream&); // Constructor with an OutputStream
    virtual ~BufferedWriter(); // Destructor
    void close();     // Close this stream
    int write(int);       // Read a byte from this stream
    int write(ubyte*,int);   // Read an array of characters
    int write(ubyte*,int,int); // Read bytes from this stream
    void flush(); // Flush stream
    void newLine(); // Write a line separator
    void writeLine(String&); // Write a line with separator

  protected:
    BufferedWriter();
    int write_buffer();

  private:
    OutputStream *my_stream;  
};

} // namespace daqhwyapi

#endif
