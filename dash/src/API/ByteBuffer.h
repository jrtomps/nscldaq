#ifndef DAQHWYAPI_BYTEBUFFER_H
#define DAQHWYAPI_BYTEBUFFER_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_BUFFER_H
#include <dshapi/Buffer.h>
#endif

namespace daqhwyapi {

/**
* @class ByteBuffer
* @brief ByteBuffer definition.
*
* The definition for the ByteBuffer class implementing a dynamic
* string.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class ByteBuffer : public Buffer {
  public: 
    ByteBuffer();          // Default constructor 
    virtual ~ByteBuffer();          // Destructor
    ByteBuffer(const ByteBuffer&);   // Copy constructor 
    ByteBuffer(const ubyte*,int); // Constructor with ubyte* and length

    ubyte *array(ubyte*,int&); // Get the buffer as an array of ubytes.  
    bool consume(int);     // Consume characters at start of buffer.
    bool truncate(int);    // Keep only the first n characters.
    bool put(ubyte*,int);   // Relative put a ubyte* and length.
    bool put(ubyte);   // Relative put a ubyte.
    ubyte *get(ubyte*,int);  // Relative get a ubyte* with length.
    ubyte get();   // Relative get a ubyte.
    bool put(int,ubyte*,int); // Absolute put a ubyte* and length.
    bool put(int,ubyte);   // Absolute put a ubyte.
    ubyte *get(int,ubyte*,int);  // Absolute get a ubyte* with length.
    ubyte get(int);   // Absolute get a ubyte.
};


} // namespace daqhwyapi

#endif
