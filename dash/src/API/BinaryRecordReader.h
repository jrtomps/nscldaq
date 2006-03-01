#ifndef DAQHWYAPI_BINARYRECORDREADER_H
#define DAQHWYAPI_BINARYRECORDREADER_H

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
* @class BinaryRecordReader
* @brief BinaryRecordReader for reading binary data.
*
* BinaryRecordReader for reading binary data that is prepended
* with a data length.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BinaryRecordReader : public FdRecordReader {
  public: 
    BinaryRecordReader(FdInputStream&); // Constructor with an InputStream
    BinaryRecordReader(FdInputStream&,int); // With InputStream and max record
    virtual ~BinaryRecordReader(); // Destructor
    int readRecord(record_header_t&,ByteArray&); // Read a line
    void setTyped(bool);  // Set if binary records have been typed.
    bool eof(); // End of file
    bool ready(); // Have a record to read
    void close();  // Close this reader

  protected:
    BinaryRecordReader();
    int read_buffer();

  private:
    ubyte *my_buffer;
    bool is_ready;
    int my_buflen;
    ubyte *my_wrkbuf;
    int cur_pos;
    int cur_avail;
    int my_maxbuf;
    int my_reclen;
    uint32_t my_rectype;
    bool my_typed;
};

} // namespace daqhwyapi

#endif
