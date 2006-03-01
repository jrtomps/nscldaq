/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#include <dshapi/FdInputStream.h>
#endif

#ifndef DAQHWYAPI_FDRECORDREADER_H
#include <dshapi/FdRecordReader.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

namespace daqhwyapi {
/**
* @var fdreader_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fdreader_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn FdRecordReader::FdRecordReader()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FdRecordReader::FdRecordReader() {
  my_stream = NULL;
}

/*==============================================================*/
/** @fn FdRecordReader::FdRecordReader(FdInputStream& rStream)
* @brief Constructor with a FdInputStream.
*
* Constructor with a FdInputStream.
*
* @param aStream The FdInputStream.
* @return this
*/                                                             
FdRecordReader::FdRecordReader(FdInputStream& rStream) {
  my_stream = &rStream;
}

/*==============================================================*/
/** @fn FdRecordReader::~FdRecordReader()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FdRecordReader::~FdRecordReader() { 
  my_stream = NULL; 
}

/*==============================================================*/
/** @fn int FdRecordReader::getFD()
* @brief Get the underlying file descriptor.
*
* Get the underlying file descriptor.
*
* @param None
* @return The underlying file descriptor or -1.
*/                                                             
int FdRecordReader::getFD() {
  if (my_stream == NULL) return -1;
  return my_stream->getFD();
}

/*==============================================================*/
/** @fn int FdRecordReader::skip()
* @brief Skip and discard the current record.
*
* Skip and discard the current record.  There must be
* a complete record available for this to have any effect.
*
* @param None
* @return The actual number of bytes skipped.
* @throw IOException if there's an IO error.
*/                                                             
int FdRecordReader::skip() {
  if (my_stream == NULL) throw fdreader_ioexception.format(CSTR("FdRecordReader::skip() no stream available"));
  ByteArray barry;
  record_header_t hdr;
  Record::initHeader(hdr); 
  int n = readRecord(hdr,barry);
  return(n);
}

