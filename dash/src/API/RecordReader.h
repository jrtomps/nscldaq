#ifndef DAQHWYAPI_RECORDREADER_H
#define DAQHWYAPI_RECORDREADER_H

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
* @class RecordReader
* @brief RecordReader interface definition.
*
* The interface definition for the class that implements a
* record reader.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RecordReader : public Object {
  public: 
    virtual bool ready() = 0;  // Stream has a record to read
    virtual void close() = 0;     // Close this reader
    virtual int readRecord(record_header_t&,ByteArray&) = 0;  // Read a record from this stream
    virtual int skip() = 0; // Skip current record
    virtual bool eof() = 0; // At end-of-file
};

} // namespace daqhwyapi

#endif
