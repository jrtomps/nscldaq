#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#define DAQHWYAPI_FDINPUTSTREAM_H

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

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/InputStream.h>
#endif

#ifndef DAQHWYAPI_BYTEBUFFER_H
#include <dshapi/ByteBuffer.h>
#endif

namespace daqhwyapi {

/**
* @class FdInputStream
* @brief FdInputStream for constructing streams from a file descriptor.
*
* A class for constructing an input stream from a file descriptor.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdInputStream : public InputStream {
  public: 
    FdInputStream(); // Default constructor 
    FdInputStream(int); // Constructor with a file descriptor
    FdInputStream(int,int); // Constructor with a file descriptor and buffer size
    virtual ~FdInputStream();  // Destructor
    long available();  // Number of bytes available for reading
    bool ready();  // Stream has characters to read
    void close();     // Close this stream
    void open(int);   // Open with a new descriptor
    int getFD();      // Get the file descriptor 
    int read();       // Read a byte from this stream
    int read(ubyte*,int); // Read bytes from this stream 
    int read(ubyte*,int,int); // Read bytes from this stream 
    long skip(long); // Skip characters.
    bool eof(); // At end-of-file
    bool setBuffer(int); // Set the size of the IO buffer
    off_t mark();  // Mark a stream position
    void reset(); // Reset stream position to that of last mark
    void cleareof(); // Clear the eof condition

  protected:
    int read_open(const char*); // Open a file for reading
    int read_input(ubyte*,int,int); // Read a buffer of characters (blocking)
    int set_blocking(bool); // Set non block IO
    bool is_blocking(); // Check for blocking IO
    int fill_buffer(); // Fill the buffer

    int my_fd;
    bool ateof;
    int my_bufsiz;
    ByteBuffer buffer;
    char *my_work;
    off_t my_mark;
};

} // namespace daqhwyapi

#endif
