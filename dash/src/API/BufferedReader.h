#ifndef DAQHWYAPI_BUFFEREDREADER_H
#define DAQHWYAPI_BUFFEREDREADER_H

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

#ifndef DAQHWYAPI_READER_H
#include <dshapi/Reader.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_h
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/InputStream.h>
#endif

namespace daqhwyapi {

/**
* @class BufferedReader
* @brief BufferedReader for reading character streams.
*
* BufferedReader for reading character streams.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedReader : public Reader {
  public: 
    BufferedReader(InputStream&); // Constructor with an InputStream
    virtual ~BufferedReader(); // Destructor
    bool ready();  // Stream has characters to read
    void close();     // Close this stream
    int read();       // Read a byte from this stream
    int read(ubyte*,int);   // Read an array of characters
    int read(ubyte*,int,int); // Read bytes from this stream
    long skip(long); // Skip characters.
    String readLine(); // Read a line
    bool eof(); // At end-of-file

  protected:
    BufferedReader();
    int read_buffer();

  private:
    InputStream *my_stream;  
    ByteArray *my_buffer;
    int cur_pos;
    int cur_avail;
};

} // namespace daqhwyapi

#endif
