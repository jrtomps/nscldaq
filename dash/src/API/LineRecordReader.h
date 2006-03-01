#ifndef DAQHWYAPI_LINERECORDREADER_H
#define DAQHWYAPI_LINERECORDREADER_H

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
* @class LineRecordReader
* @brief LineRecordReader for reading ASCII separator terminated data.
*
* LineRecordReader for reading ASCII separator data and
* converting each line into a record.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class LineRecordReader : public FdRecordReader {
  public: 
    LineRecordReader(FdInputStream&); // Constructor with an InputStream
    LineRecordReader(FdInputStream&,int); // With InputStream and max record
    virtual ~LineRecordReader(); // Destructor
    int readRecord(record_header_t&,ByteArray&); // Read a line
    bool eof(); // End of file
    bool ready(); // Have a record to read
    void close(); // Close this reader
    bool setSeparators(String&);
    bool setSeparators(const char*);
    bool setSeparators(std::string&);

  protected:
    LineRecordReader();
    int read_buffer();
    bool is_separator(char);

  private:
    ubyte *my_buffer;
    bool is_ready;
    int my_buflen;
    ubyte *my_wrkbuf;
    int cur_pos;
    int cur_avail;
    int my_maxbuf;
    String my_sepchars;
};

} // namespace daqhwyapi

#endif
