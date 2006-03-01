#ifndef DAQHWYAPI_BUFFEREDRECORDREADER_H
#define DAQHWYAPI_BUFFEREDRECORDREADER_H

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

#ifndef DAQHWYAPI_FDRECORDREADER_H
#include <dshapi/FdRecordReader.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/InputStream.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

namespace daqhwyapi {

/**
* @class BufferedRecordReader
* @brief BufferedRecordReader for reading encoded records from streams.
*
* BufferedRecordReader for reading encoded records from streams.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BufferedRecordReader : public FdRecordReader {
  public: 
    BufferedRecordReader(FdInputStream&); // Constructor with an InputStream
    virtual ~BufferedRecordReader(); // Destructor
    int readRecord(record_header_t&,ByteArray&); // Read a line
    bool eof(); // End of file
    bool ready(); // Have a record to read
    void close(); // Close this reader
    bool tailReady(); // Have a record to read ignoring eof

  protected:
    BufferedRecordReader();
    int read_buffer(int);  
    int check_record(record_header_t&,bool&);
    int read_header(record_header_t&,bool&);

  private:
    ByteBuffer my_buffer;
    record_header_t my_hdr;
    bool is_ready;
    ubyte *my_wrkbuf;
    int cur_pos;
    int cur_avail;
    bool have_hdr;
};

} // namespace daqhwyapi

#endif
