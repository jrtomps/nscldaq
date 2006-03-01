#ifndef DAQHWYAPI_FDRECORDWRITER_H
#define DAQHWYAPI_FDRECORDWRITER_H

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

#ifndef DAQHWYAPI_RECORDWRITER_H
#include <dshapi/RecordWriter.h>
#endif

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
#endif

namespace daqhwyapi {

/**
* @class FdRecordWriter
* @brief FdRecordWriter interface definition.
*
* The interface definition for the class that implements a
* an Fd record writer.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FdRecordWriter : public RecordWriter {
  public: 
    FdRecordWriter(FdOutputStream&); // Constructor with InputStream
    virtual ~FdRecordWriter(); // Destructor
    int getFD(); // Get the underlying file descriptor

  protected: 
    FdRecordWriter();  // Default constructor
    FdOutputStream *my_stream;
};

} // namespace daqhwyapi

#endif
