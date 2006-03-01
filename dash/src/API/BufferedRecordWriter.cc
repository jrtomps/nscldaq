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


#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_BUFFEREDRECORDWRITER_H
#include <dshapi/BufferedRecordWriter.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_OUTPUTSTREAM_H
#include <dshapi/OutputStream.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var bufferedrecordwriter_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException bufferedrecordwriter_ioexception;

/**
* @var bufferedrecordwriter_rtexception
* @brief Exception to throw for runtime exceptions.
*
* Exception to throw for runtime exceptions.
*/
static RuntimeException bufferedrecordreader_rtexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn BufferedRecordWriter::BufferedRecordWriter()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
BufferedRecordWriter::BufferedRecordWriter() {
  my_stream = NULL;
  my_buffer.limit(Record::default_max_record_size);
  my_wrkbufsiz = Record::default_max_record_size;
  my_wrkbuf = new ubyte[my_wrkbufsiz];
}

/*==============================================================*/
/** @fn BufferedRecordWriter::BufferedRecordWriter(FdOutputStream& rStream)
* @brief Constructor with an OutputStream.
*
* Constructor with an OutputStream.
*
* @param rStream An OutputStream.
* @return this
*/                                                             
BufferedRecordWriter::BufferedRecordWriter(FdOutputStream& rStream) {
  my_stream = &rStream;
  my_buffer.limit(Record::default_max_record_size);
  my_wrkbufsiz = Record::default_max_record_size;
  my_wrkbuf = new ubyte[my_wrkbufsiz];
}

/*==============================================================*/
/** @fn BufferedRecordWriter::~BufferedRecordWriter()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
BufferedRecordWriter::~BufferedRecordWriter() { 
  my_stream = NULL;
  if (my_wrkbuf != NULL) delete[] my_wrkbuf;
  my_wrkbuf = NULL;
  my_wrkbufsiz = 0;
}

/*==============================================================*/
/** @fn void BufferedRecordWriter::close()
* @brief Close the underlying stream.
*
* Close the underlying stream.
*
* @param None
* @return None
* @throw IOException if there is an IO error.
*/                                                             
void BufferedRecordWriter::close() {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::close() no stream available"));
  my_buffer.clear();
  my_stream->close();
}

/*==============================================================*/
/** @fn void BufferedRecordWriter::flush()
* @brief Flush the underlying stream.
*
* Flush the underlying stream.
*
* @param None
* @return None
* @throw IOException if there is an IO error.
*/                                                             
void BufferedRecordWriter::flush() {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::flush() no stream available"));
  my_stream->flush();
}

/*==============================================================*/
/** @fn int BufferedRecordWriter::writeRecord(record_header_t& rHdr)
* @brief Write a record from the buffer.
*
* Write a record from the buffer.
*
* @param rHdr Record header for this record.
* @return The number of bytes written (the entire record length).
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordWriter::writeRecord(record_header_t& rHdr) {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::readRecord() no stream available"));
  return write_buffer(rHdr);
}

/*==============================================================*/
/** @fn int BufferedRecordWriter::write(int b)
* @brief Write a byte.
*
* Write a byte from to the output stream.
*
* @param None
* @return The number of characters written.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordWriter::write(int b) {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::write() no stream available"));
  int lim = my_buffer.limit();
  if (lim < (my_buffer.capacity() + 1)) my_buffer.limit(lim+256);
  my_buffer.put((ubyte)b);
  return 1;
}

/*==============================================================*/
/** @fn int BufferedRecordWriter::write(ubyte *pArray,int len)
* @brief Write bytes from an array.
*
* Write up to len bytes from an array.
*
* @param pArray The array to read into.
* @param len The number of bytes to read.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordWriter::write(ubyte *pArray,int len) {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::write() no stream available"));
  if (pArray == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::write() pArray cannot be NULL"));
  if (len <= 0) return 0;

  int lim = my_buffer.limit();
  if (lim < (my_buffer.capacity() + len)) my_buffer.limit(lim+len);
  my_buffer.put(pArray,len);
  return len;
}

/*==============================================================*/
/** @fn int BufferedRecordWriter::write(ubyte *pArray,int oset,int len)
* @brief Write bytes from an array.
*
* Write up to len bytes from an array at an offset.
*
* @param pArray The array to write.
* @param oset The offset.
* @param len The maximum number of bytes to write.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordWriter::write(ubyte *pArray,int oset,int len) {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::read() no stream available"));
  if (pArray == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::read() pArray cannot be NULL"));
  if (len <= 0) return 0;

  int lim = my_buffer.limit();
  if (lim < (my_buffer.capacity() + len)) my_buffer.limit(lim+len);
  my_buffer.put(pArray+oset,len);
  return len;
}

/*==============================================================*/
/** @fn int BufferedRecordWriter::write_buffer(record_header_t& rHdr)
* @brief Write out the buffer.
*
* Write out the buffer using the specified header.  The header data
* size will be modified to be the current capacity of this
* writer's buffer minus the size of the extended header indicated
* by rHdr. 
*
* @param rHdr The record header to use when writing.
* @return The number of characters actually written.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordWriter::write_buffer(record_header_t& rHdr) {
  if (my_stream == NULL) throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::write_buffer() no stream available"));
  ubyte hdrbuf[Record::encode_buffer_size];

  int n = my_buffer.capacity();
  int rn = n + Record::encode_buffer_size;
  int dn = n - rHdr.extended_header_size;
  rHdr.record_size = rn;
  rHdr.data_size = dn;  

  // First output the header
  Record::encodeHeader(hdrbuf,Record::encode_buffer_size,rHdr);
  int rlen = rn;
  int wlen = Record::encode_buffer_size;
  int oset = 0;
  while (wlen > 0) {
    int rc = my_stream->write(hdrbuf,oset,wlen);
    if (rc < 0) {
      throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::run() Failed to finish writing record header.  Wrote %d bytes of a record with byte length %d."),(rn - rlen),rn);
    }
    wlen -= rc;
    rlen -= rc;
    oset += rc;
  }

  // Do we have enough work space?
  if (n > my_wrkbufsiz) {
    if (my_wrkbuf != NULL) delete[] my_wrkbuf;
    my_wrkbufsiz = n;
    my_wrkbuf = new ubyte[my_wrkbufsiz];
  }
 
  // Get the data from the buffer 
  my_buffer.get(0,my_wrkbuf,n);
  my_buffer.consume(n);

  // Now output the data
  oset = 0;
  while (rlen > 0) {
    int rc = my_stream->write(my_wrkbuf,oset,rlen);
    if (rc < 0) {
      throw bufferedrecordwriter_ioexception.format(CSTR("BufferedRecordWriter::run() Failed to finish writing record data.  Wrote %d bytes of a record with byte length %d."),(rn - wlen),rn);
    }
    rlen -= rc;
    oset += rc;
  }

  return rn;
}

