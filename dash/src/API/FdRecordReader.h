#ifndef DAQHWYAPI_FDRECORDREADER_H
#define DAQHWYAPI_FDRECORDREADER_H

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

#ifndef DAQHWYAPI_RECORDREADER_H
#include <dshapi/RecordReader.h>
#endif

#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#include <dshapi/FdInputStream.h>
#endif

namespace daqhwyapi {

/**
* @class FdRecordReader
* @brief FdRecordReader interface definition.
*
* The interface definition for the class that implements a
* an Fd record reader.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdRecordReader : public RecordReader {
  public: 
    FdRecordReader(FdInputStream&); // Constructor with InputStream
    virtual ~FdRecordReader(); // Destructor
    int skip(); // Skip current record
    int getFD(); // Get the underlying file descriptor

  protected: 
    FdRecordReader();  // Default constructor
    FdInputStream *my_stream;
};

} // namespace daqhwyapi

#endif
