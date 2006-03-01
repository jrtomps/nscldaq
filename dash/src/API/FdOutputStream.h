#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#define DAQHWYAPI_FDOUTPUTSTREAM_H

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

#ifndef DAQHWYAPI_OUTPUTSTREAM_H
#include <dshapi/OutputStream.h>
#endif

#ifndef DAQHWYAPI_BYTEBUFFER_H
#include <dshapi/ByteBuffer.h>
#endif

namespace daqhwyapi {

class UByteArray;

/**
* @class FdOutputStream
* @brief FdOutputStream for constructing streams from files.
*
* A class for constructing an input stream from a disk file.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdOutputStream : public OutputStream {
  public: 
    FdOutputStream(); // Default constructor 
    FdOutputStream(int); // Constructor with a file descriptor
    FdOutputStream(int,int); // Constructor with a file descriptor and buffer size
    virtual ~FdOutputStream();  // Destructor
    void close();     // Close this stream
    void open(int);   // Open with a new descriptor
    int getFD();      // Get the file descriptor 
    int write(int);   // Write a byte to this stream
    int write(ubyte*,int); // Write bytes to this stream 
    int write(ubyte*,int,int); // Write bytes to this stream 
    void flush(); // Flush this stream
    bool setBuffer(int); // Set the size of the IO buffer

  protected:
    int write_output(ubyte*,int,int); 

    int my_fd;
    bool ateof;
    int my_bufsiz;
    ByteBuffer buffer;
    char *my_work;
};

} // namespace daqhwyapi

#endif
