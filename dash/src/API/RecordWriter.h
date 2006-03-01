#ifndef DAQHWYAPI_RECORDWRITER_H
#define DAQHWYAPI_RECORDWRITER_H

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

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

namespace daqhwyapi {

/**
* @class RecordWriter
* @brief RecordWriter interface definition.
*
* The interface definition for the class that implements an RecordWriter.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RecordWriter : public Object {
  public: 
    virtual void close() = 0;     // Close this writer
    virtual void flush() = 0;     // Flush this writer
    virtual int write(int) = 0;       // Write a byte to this writer
    virtual int write(ubyte*,int) = 0;   // Write an array of characters
    virtual int write(ubyte*,int,int) = 0; // Writer bytes to this stream
    virtual int writeRecord(record_header_t&) = 0; // Write a record to this stream
};

} // namespace daqhwyapi

#endif
