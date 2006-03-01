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

#ifndef DAQHWYAPI_BUFFEREDRECORDREADER_H
#include <dshapi/BufferedRecordReader.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/InputStream.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var bufferedrecordreader_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException bufferedrecordreader_ioexception;

/**
* @var bufferedrecordreader_rtexception
* @brief Exception to throw for runtime exceptions.
*
* Exception to throw for runtime exceptions.
*/
static RuntimeException bufferedrecordreader_rtexception;

#define BUFFEREDREADER_BUFFER_SIZE 1024
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn BufferedRecordReader::BufferedRecordReader()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
BufferedRecordReader::BufferedRecordReader() {
  my_stream = NULL;
  is_ready = false;
  cur_pos = 0;
  cur_avail = 0;
  have_hdr = false;
  my_buffer.limit(Record::default_max_record_size);
  my_wrkbuf = new ubyte[BUFFEREDREADER_BUFFER_SIZE];
  Record::initHeader(my_hdr);
}

/*==============================================================*/
/** @fn BufferedRecordReader::BufferedRecordReader(FdInputStream& rStream)
* @brief Constructor with an InputStream.
*
* Constructor with an InputStream.
*
* @param rStream An InputStream.
* @return this
*/                                                             
BufferedRecordReader::BufferedRecordReader(FdInputStream& rStream) {
  my_stream = &rStream;
  is_ready = false;
  cur_pos = 0;
  cur_avail = 0;
  have_hdr = false;
  my_buffer.limit(Record::default_max_record_size);
  my_wrkbuf = new ubyte[BUFFEREDREADER_BUFFER_SIZE];
  Record::initHeader(my_hdr);
}

/*==============================================================*/
/** @fn BufferedRecordReader::~BufferedRecordReader()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
BufferedRecordReader::~BufferedRecordReader() { 
  if (my_wrkbuf != NULL) delete[] my_wrkbuf;
  my_wrkbuf = NULL;
  is_ready = false;
  cur_pos = 0;
  cur_avail = 0;
  have_hdr = false;
}

/*==============================================================*/
/** @fn void BufferedRecordReader::close()
* @brief Close this reader.
*
* Close this reader.  Will discard any remaining data and
* close the underlying stream.
*
* @param None
* @return None
*/                                                             
void BufferedRecordReader::close() {
  my_buffer.clear();
  if (my_stream != NULL) my_stream->close();
}

/*==============================================================*/
/** @fn bool BufferedRecordReader::ready()
* @brief Check if this reader has a record ready to be read.
*
* Check if this reader has a record ready to be read.
*
* @param None
* @return If a record is available for reading.
* @throw IOException if there is an IO error.
*/                                                             
bool BufferedRecordReader::ready() {
  if (is_ready) return true;
  if (my_stream == NULL) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::ready() no stream available"));

  // No header see if something is out there.
  int rlen = 0;
  if (!have_hdr) {
    rlen = check_record(my_hdr,have_hdr);
    if (!have_hdr) return false; // No record available
    if (rlen > my_buffer.limit()) my_buffer.limit(rlen);
  }

  int curcap = my_buffer.capacity();
  while (!is_ready) {
    while ((curcap < rlen)&&(cur_avail > 0)) {
      int rem = (cur_avail < (rlen - curcap)) ? cur_avail : (rlen - curcap);
      if (rem <= 0) break; // We have all the data we need
      my_buffer.put(my_wrkbuf+cur_pos,rem);
      curcap += rem;
      cur_pos += rem; cur_avail -= rem;
    }

    if (curcap >= rlen) {
      is_ready = true;
    } else if (my_stream->ready()) { // More to read
      if (read_buffer(rlen - curcap) < 0) is_ready = true; // EOF or error
    } else { // Nothing more to read, but can read more
      return false;
    }
  }

  return is_ready;
}

/*==============================================================*/
/** @fn bool BufferedRecordReader::tailReady()
* @brief Check if this stream is ready to be read.
*
* Check if this reader has a record ready.  This method ignores
* EOF in favor of tailing a file.  That is, the underlying stream
* is reset and rechecked at each call.
*
* @param None
* @return If a record is available for reading.
* @throw IOException if there is an IO error.
*/                                                             
bool BufferedRecordReader::tailReady() {
  if (is_ready) return true;
  if (my_stream == NULL) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::tailReady() no stream available"));

  // Reset the stream
  my_stream->cleareof();

  // No header see if something is out there.
  int rlen = 0;
  if (!have_hdr) {
    rlen = check_record(my_hdr,have_hdr);
    if (!have_hdr) return false; // No record available
    if (rlen > my_buffer.limit()) my_buffer.limit(rlen);
  }

  int curcap = my_buffer.capacity();
  while (!is_ready) {
    while ((curcap < rlen)&&(cur_avail > 0)) {
      int rem = (cur_avail < (rlen - curcap)) ? cur_avail : (rlen - curcap);
      if (rem <= 0) break; // We have all the data we need
      my_buffer.put(my_wrkbuf+cur_pos,rem);
      curcap += rem;
      cur_pos += rem; cur_avail -= rem;
    }

    if (curcap >= rlen) {
      is_ready = true;
    } else if (my_stream->ready()) { // More to read
      my_stream->cleareof();
      if (read_buffer(rlen - curcap) < 0) {
        return false; // EOF or error
      }
    } else { // Nothing more to read, but can read more
      return false;
    }
  }

  return is_ready;
}

/*==============================================================*/
/** @fn int BufferedRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray)
* @brief Read a record from the stream.
*
* Read a record into a buffer.  
*
* @param rHdr Record header for this record.
* @param rArray The Array to read into.
* @return The number of bytes read or -1 on EOF.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray) {
  if (my_stream == NULL) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::readRecord() no stream available"));

  // No header see if something is out there.
  int rlen = 0;
  if (!have_hdr) {
    rlen = read_header(my_hdr,have_hdr);
    if (rlen == -1) return -1; // EOF
    if (rlen < 0) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::readRecord() could not read record header."));
    if (rlen > my_buffer.limit()) my_buffer.limit(rlen);
  }

  int curcap = my_buffer.capacity();
  while (!is_ready) {
    while ((curcap < rlen)&&(cur_avail > 0)) {
      int rem = (cur_avail < (rlen - curcap)) ? cur_avail : (rlen - curcap);
      if (rem <= 0) break; // We have all the data we need
      my_buffer.put(my_wrkbuf+cur_pos,rem);
      curcap += rem;
      cur_pos += rem; cur_avail -= rem;
    }

    if (curcap >= rlen) {
      is_ready = true;
    } else { // Read more
      if (read_buffer(rlen - curcap) < 0) {
        is_ready = true; // EOF or error
        rHdr.status_code = Record::status_err;
      }
    } 
  }

  // Copy the read header to set all defaults
  Record::copyHeader(rHdr,my_hdr);

  // There may have been an error and the record may be short
  // as a result.  Repackage accordingly and set the status
  // code indicating a problem record. 
  int rn = my_hdr.record_size; 
  int en = my_hdr.extended_header_size; 
  int dn = my_hdr.data_size; 
  int cap = my_buffer.capacity();

  if (cap < (dn+en)) { // Short record?
    // If the read status code says everything before was
    // ok, then the error probably originated here, so mark
    // the record accordingly.
    if (rHdr.status_code == Record::status_ok) {
      rHdr.status_code = Record::status_err;  // Error code
    }
    if (cap < en) {
      en = cap;
      dn = 0;
    } else {
      dn = cap - en;
    }
    rn = dn + en + Record::encode_buffer_size; 
  }

  // Now return what we got and reset the header accordingly.
  rArray.renew(dn+en);
  my_buffer.get(0,rArray.elements,dn+en);
  my_buffer.consume(dn+en);
  rHdr.record_size = rn;
  rHdr.extended_header_size = en;
  rHdr.data_size = dn;

  // We are no longer ready
  is_ready = false;
  have_hdr = false;

  // Return what we actually read
  return(rn);
}

/*==============================================================*/
/** @fn bool BufferedRecordReader::eof()
* @brief Check for end-of-file.
*
* Check for end-of-file.
*
* @param None
* @return True if at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
bool BufferedRecordReader::eof() {
  if (my_stream == NULL) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::eof() no stream available"));
  if ((cur_avail > 0)||(my_buffer.capacity() > 0)) return false;
  else return (my_stream->eof());
}

/*==============================================================*/
/** @fn int BufferedRecordReader::read_buffer(int maxcnt)
* @brief Read in a buffer.
*
* Read in a buffer.
*
* @param maxcnt Maximum number of bytes to read.
* @return The number of characters available or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedRecordReader::read_buffer(int maxcnt) {
  if (my_stream == NULL) throw bufferedrecordreader_ioexception.format(CSTR("BufferedRecordReader::read_buffer() no stream available"));

  if (maxcnt <= 0) return 0;

  int limcnt = (maxcnt < BUFFEREDREADER_BUFFER_SIZE) ? maxcnt : BUFFEREDREADER_BUFFER_SIZE;

  // Only read if there is space to read.
  if (cur_avail < limcnt) {
    // Move data to beginning of buffer
    if (cur_pos != 0) {
      ::memmove(my_wrkbuf,&(my_wrkbuf[cur_pos]),cur_avail);
      cur_pos = 0;
    }

    int wn = 0;
    int rrc = 0;
    if (my_stream->ready()) { 
      int av = my_stream->available();
      int canget = limcnt - cur_avail;
      wn = (av < canget) ? av : canget;
      rrc = my_stream->read(my_wrkbuf,cur_avail,wn);
    } else {
      rrc = my_stream->read(my_wrkbuf,cur_avail,1);
    }

    if (rrc < 0) {
      return -1;
    } else {
      cur_avail += rrc;
    }
  }

  return cur_avail; 
}

/*==============================================================*/
/** @fn int BufferedRecordReader::check_record(record_header_t& hdr,bool& gotrec)
* @brief Check the input stream for a new record header.
*
* Check the input stream for a new record header.  This method
* does not block.
*
* @param hdr A record header to fill in.
* @param gotrec Output.  Set to true if a record header was read.
* @return The remaining length of the new record to read or <0 on error.
*/                                                             
int BufferedRecordReader::check_record(record_header_t& hdr,bool& gotrec) {
  int rlen = 0;
  gotrec = false;

  if (my_stream->ready()) { // Have some bytes on input
    // Maybe we have enough for a new header 
    if (my_stream->available() >= Record::encode_buffer_size) {
      rlen = read_header(hdr,gotrec);
    }
  }

  return rlen;
}

/*==============================================================*/
/** @fn int BufferedRecordReader::read_header(record_header_t& hdr,bool& gotrec)
* @brief Read a record header from the input stream.
*
* Read a record header from the input stream.  This method
* blocks until it has a record header.
*
* @param hdr A record header to fill in.
* @param gotrec Output.  Set to true if a record header was read.
* @return The remaining length of the new record to read, -1 on EOF or -2 on short read.
*/                                                             
int BufferedRecordReader::read_header(record_header_t& hdr,bool& gotrec) {
  ubyte hdrbuf[Record::encode_buffer_size]; // For reading headers
  int rlen = 0;
  gotrec = false;
  Record::initHeader(hdr); // Clean it up

  int rc = my_stream->read(hdrbuf,Record::encode_buffer_size); 
  if (rc < Record::encode_buffer_size) { // EOF or other error
    if (rc == -1) rlen = -1;  // EOF
    else rlen = -2; // Short read
  } else {
    // If we can't decode the header, we have a problem
    if (Record::decodeHeader(hdrbuf,Record::encode_buffer_size,hdr) < 0) {
      throw bufferedrecordreader_rtexception.format(CSTR("BufferedRecordReader::read_header() Failed to decode header"));
    }
    gotrec = true;
    // Calculate the remaining record size 
    rlen = hdr.record_size + hdr.extended_header_size - Record::encode_buffer_size; 
  }

  return rlen;
}
