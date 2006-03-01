#ifndef DAQHWYAPI_BUFFEREDRECORDWRITER_H
#define DAQHWYAPI_BUFFEREDRECORDWRITER_H

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

#ifndef DAQHWYAPI_FDRECORDWRITER_H
#include <dshapi/FdRecordWriter.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_OUTPUTSTREAM_H
#include <dshapi/OutputStream.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

namespace daqhwyapi {

/**
* @class BufferedRecordWriter
* @brief BufferedRecordWriter for writing encoded records to streams.
*
* BufferedRecordWriter for writing encoded records to streams.
*
* The write() methods write to an internal buffer until the
* writeRecord() method is called.  When writeRecord() is called 
* the internal buffer is written to the underlying OutputStream.
*
* The data_size and record_size header contents are modified as
* a result of calling writeRecord().  It is intended that the
* write() methods are used to write both the extended header followed
* by the record data.  As such, the data_size field is calculated
* as the current buffer capacity minus the value of the 
* extended_header_size field provided in the header passed to
* writeRecord().  The record_size is calculated as the buffer 
* capacity plus the size of the encoded header.
*
* All other header fields are left as passed to writeRecord().
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedRecordWriter : public FdRecordWriter {
  public: 
    BufferedRecordWriter(FdOutputStream&); // Constructor with an InputStream
    virtual ~BufferedRecordWriter(); // Destructor
    int writeRecord(record_header_t&); // Write a buffer
    int write(int); // Write a byte
    int write(ubyte*,int); // Write an array
    int write(ubyte*,int,int); // Write an array with offset
    void close(); // Close the stream
    void flush(); // Flush the stream

  protected:
    BufferedRecordWriter();
    int write_buffer(record_header_t&);  

  private:
    ByteBuffer my_buffer;
    ubyte *my_wrkbuf;
    int my_wrkbufsiz;
};

} // namespace daqhwyapi

#endif
