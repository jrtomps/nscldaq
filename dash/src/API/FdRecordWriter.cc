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

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
#endif

#ifndef DAQHWYAPI_FDRECORDWRITER_H
#include <dshapi/FdRecordWriter.h>
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

using namespace daqhwyapi;

/*==============================================================*/
/** @fn FdRecordWriter::FdRecordWriter()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FdRecordWriter::FdRecordWriter() {
  my_stream = NULL;
}

/*==============================================================*/
/** @fn FdRecordWriter::FdRecordWriter(FdOutputStream& rStream)
* @brief Constructor with a FdOutputStream.
*
* Constructor with a FdOutputStream.
*
* @param rStream The FdOutputStream.
* @return this
*/                                                             
FdRecordWriter::FdRecordWriter(FdOutputStream& rStream) {
  my_stream = &rStream;
}

/*==============================================================*/
/** @fn FdRecordWriter::~FdRecordWriter()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FdRecordWriter::~FdRecordWriter() { 
  my_stream = NULL; 
}

/*==============================================================*/
/** @fn int FdRecordWriter::getFD()
* @brief Get the underlying file descriptor.
*
* Get the underlying file descriptor.
*
* @param None
* @return The underlying file descriptor or -1.
*/                                                             
int FdRecordWriter::getFD() {
  if (my_stream == NULL) return -1;
  return my_stream->getFD();
}
