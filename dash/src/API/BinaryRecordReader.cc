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

#ifndef DAQHWYAPI_BINARYRECORDREADER_H
#include <dshapi/BinaryRecordReader.h>
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

#ifndef DAQHWYAPI_DSHUTILS_H
#include <dshapi/DSHUtils.h>
#endif

namespace daqhwyapi {
/**
* @var binaryrecordreader_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException binaryrecordreader_ioexception;

#define BINARYREADER_BUFFER_SIZE 2048
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn BinaryRecordReader::BinaryRecordReader()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
BinaryRecordReader::BinaryRecordReader() {
  my_maxbuf = Record::default_max_record_size; 
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = NULL;
  is_ready = false;
  my_wrkbuf = new ubyte[BINARYREADER_BUFFER_SIZE];
  my_reclen = -1;
  my_typed = false;
}

/*==============================================================*/
/** @fn BinaryRecordReader::BinaryRecordReader(FdInputStream& rStream)
* @brief Constructor with an InputStream.
*
* Constructor with an InputStream.
*
* @param rStream An InputStream.
* @return this
*/                                                             
BinaryRecordReader::BinaryRecordReader(FdInputStream& rStream) {
  my_maxbuf = Record::default_max_record_size; 
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = &rStream;
  is_ready = false;
  my_wrkbuf = new ubyte[BINARYREADER_BUFFER_SIZE];
  my_reclen = -1;
  my_typed = false;
}

/*==============================================================*/
/** @fn BinaryRecordReader::BinaryRecordReader(FdInputStream& rStream,int maxbuf)
* @brief Constructor with an InputStream and maximum size.
*
* Constructor with an InputStream and a maxium record data size.
*
* @param rStream An InputStream.
* @param maxbuf The maximum record data size (without header)
* @return this
*/                                                             
BinaryRecordReader::BinaryRecordReader(FdInputStream& rStream,int maxbuf) {
  my_maxbuf = Record::default_max_record_size; 
  if (maxbuf > 0) my_maxbuf = maxbuf;
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = &rStream;
  is_ready = false;
  my_wrkbuf = new ubyte[BINARYREADER_BUFFER_SIZE];
  my_reclen = -1;
  my_typed = false;
}

/*==============================================================*/
/** @fn BinaryRecordReader::~BinaryRecordReader()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
BinaryRecordReader::~BinaryRecordReader() { 
  if (my_buffer != NULL) delete[] my_buffer;
  my_buffer = NULL;
  if (my_wrkbuf != NULL) delete[] my_wrkbuf;
  my_wrkbuf = NULL;
  is_ready = false;
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_reclen = -1;
  my_typed = false;
}

/*==============================================================*/
/** @fn void BinaryRecordReader::close()
* @brief Close this reader.
*
* Close this reader.  Simply sets the input stream to NULL
* and clears the buffers; does not actually close the input stream.
*
* @param None
* @return None
*/                                                             
void BinaryRecordReader::close() { 
  my_stream = NULL; 
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  is_ready = false;
  my_reclen = -1;
}

/*==============================================================*/
/** @fn bool BinaryRecordReader::ready()
* @brief Check if this stream is ready to be read.
*
* Check if this stream is ready to be read.
*
* @param None
* @return If a record is available for reading.
* @trhow IOException if there is an IO error.
*/                                                             
bool BinaryRecordReader::ready() {
  if (is_ready) return true;
  if (my_stream == NULL) throw binaryrecordreader_ioexception.format(CSTR("BinaryRecordReader::ready() no stream available"));

  int hdrlen = sizeof(uint32_t);
  if (my_typed) hdrlen = 2 * sizeof(uint32_t);

  while (!is_ready) {
    while ((my_buflen < my_maxbuf)&&(cur_avail > 0)) {
      if (my_reclen <= 0) {
        if (cur_avail >= hdrlen) {
          // There's always a length
          char ibytes[sizeof(uint32_t)];
          ::memcpy(ibytes,&(my_wrkbuf[cur_pos]),sizeof(uint32_t));
          cur_pos += sizeof(uint32_t); 
          cur_avail -= sizeof(uint32_t); 
          my_reclen = (*((uint32_t*)ibytes));

          // Sometimes there's a record type
          if (my_typed) {
            ::memcpy(ibytes,&(my_wrkbuf[cur_pos]),sizeof(uint32_t));
            cur_pos += sizeof(uint32_t); 
            cur_avail -= sizeof(uint32_t); 
            my_rectype = (*((uint32_t*)ibytes));
          }
        } else {  // Need to read more
          break;
        }
      } else if (my_buflen >= my_reclen) {
        is_ready = true;
        break;
      } else {
        int need = (my_reclen - my_buflen); 
        int n = need < cur_avail ? need : cur_avail;
        n = (my_buflen + n) <= my_maxbuf ? n : (my_maxbuf - my_buflen);
        ::memcpy(&(my_buffer[my_buflen]),&(my_wrkbuf[cur_pos]),n);
        my_buflen += n; 
        cur_pos += n;
        cur_avail -= n;
      }
    }

    if (!is_ready) {
      if (my_buflen >= my_maxbuf) { // Max
        is_ready = true;
      } else if ((my_reclen > 0)&&(my_buflen >= my_reclen)) {
        is_ready = true;
        break;
      } else if (my_stream->ready()) { // More to read
        if (read_buffer() < 0) {
          is_ready = true; // EOF or error
          break;
        }
      } else { // Nothing more to read, but can read more
        return false;
      }
    }
  }

  return is_ready;
}

/*==============================================================*/
/** @fn int BinaryRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray)
* @brief Read bytes into an array. 
*
* Read a bytes into an array.
*
* @param rHdr Record header for this record.
* @param rArray The Array to read into.
* @return The number of bytes read.
* @throw IOException if there's an IO error.
*/                                                             
int BinaryRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray) {
  if (my_stream == NULL) throw binaryrecordreader_ioexception.format(CSTR("BinaryRecordReader::readRecord() no stream available"));

  Record::initHeader(rHdr); // Clean it up
  rHdr.status_code = Record::status_ok;
  rHdr.extended_header_size = 0;

  int hdrlen = sizeof(uint32_t);
  if (my_typed) hdrlen = 2 * sizeof(uint32_t);

  while (!is_ready) {
    while ((my_buflen < my_maxbuf)&&(cur_avail > 0)) {
      if (my_reclen <= 0) {
        if (cur_avail >= hdrlen) {
          // There's always a length
          char ibytes[sizeof(uint32_t)];
          ::memcpy(ibytes,&(my_wrkbuf[cur_pos]),sizeof(uint32_t));
          cur_pos += sizeof(uint32_t); 
          cur_avail -= sizeof(uint32_t); 
          my_reclen = (*((uint32_t*)ibytes));

          // Sometimes there's a record type
          if (my_typed) {
            ::memcpy(ibytes,&(my_wrkbuf[cur_pos]),sizeof(uint32_t));
            cur_pos += sizeof(uint32_t); 
            cur_avail -= sizeof(uint32_t); 
            my_rectype = (*((uint32_t*)ibytes));
          }
        } else {  // We need to read more
          break;
        }
      } else if (my_buflen >= my_reclen) {
        is_ready = true;
        break;
      } else {
        int need = (my_reclen - my_buflen); 
        int n = need < cur_avail ? need : cur_avail;
        n = (my_buflen + n) <= my_maxbuf ? n : (my_maxbuf - my_buflen);
        ::memcpy(&(my_buffer[my_buflen]),&(my_wrkbuf[cur_pos]),n);
        my_buflen += n; 
        cur_pos += n;
        cur_avail -= n;
      }
    }

    if (!is_ready) {
      if ((my_reclen > 0)&&(my_buflen >= my_reclen)) {
        is_ready = true;
        break;
      } else if (my_buflen <= my_maxbuf) {
        if (read_buffer() < 0) {
          rHdr.status_code = Record::status_err;
          is_ready = true;
          break;
        }
      } else {
        rHdr.status_code = Record::status_trunc;
        is_ready = true;
        break;
      }
    }
  }

  // Now we can send up the record if we have one.
  int n = 0;
  if (my_reclen > 0) {
    n = my_reclen <= my_buflen ? my_reclen : my_buflen;
    rArray.renew(n);
    memcpy(rArray.elements,my_buffer,n);
    my_buflen -= n;
    rHdr.record_size = n + Record::encode_buffer_size;
    rHdr.data_size = n;
    rHdr.entity_count = 1;
    if (my_typed) rHdr.record_type = my_rectype;

    // Check if the max record size was exceeded and we
    // truncated the record.  If so, process the record 
    // piecemeal.
    if (rHdr.status_code == Record::status_trunc) {
      my_reclen -= n;
    } else {
      my_reclen = -1;
    }
  } else {
    rArray.renew(0);
    n = -1;
  }
  is_ready = false;  // No longer ready 
  return n;
}

/*==============================================================*/
/** @fn bool BinaryRecordReader::eof()
* @brief Check for end-of-file.
*
* Check for end-of-file.
*
* @param None
* @return True if at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
bool BinaryRecordReader::eof() {
  if (my_stream == NULL) throw binaryrecordreader_ioexception.format(CSTR("BinaryRecordReader::eof() no stream available"));
  int rc = true;
  if ((cur_avail > 0)||(my_buflen > 0)) rc = false;
  else rc = my_stream->eof();
  return(rc);
}

/*==============================================================*/
/** @fn int BinaryRecordReader::read_buffer()
* @brief Read in a buffer.
*
* Read in a buffer.
*
* @param None
* @return The number of characters available or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BinaryRecordReader::read_buffer() {
  if (my_stream == NULL) throw binaryrecordreader_ioexception.format(CSTR("BinaryRecordReader::read_buffer() no stream available"));

  // Only read if there is space to read.
  if (cur_avail < BINARYREADER_BUFFER_SIZE) {
    // Move data to beginning of buffer
    if ((cur_avail > 0)&&(cur_pos > 0)) {
      ::memmove(my_wrkbuf,&(my_wrkbuf[cur_pos]),cur_avail);
    }
    cur_pos = 0;

    int wn = 0;
    int rrc = 0;
    if (my_stream->ready()) { 
      int av = my_stream->available();
      int canget = BINARYREADER_BUFFER_SIZE - cur_avail;
      wn = (av < canget) ? av : canget;
      if (wn > 0) {
        rrc = my_stream->read(my_wrkbuf,cur_avail,wn);
      } else {
        rrc = -1; 
      }
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
/** @fn void BinaryRecordReader::setTyped(bool isTyped)
* @brief Indicate if binary records are typed.
*
* Indicate if the binary records that will be read have been typed.
* That is, typed binary records have a host-byte-order integer, 
* following the length, that indicates the record type of the
* record.  This type will be added to the record header type
* field when the record is read.   
*
* @param isTyped Indicate if typed records will be read.
* @return None
*/                                                             
void BinaryRecordReader::setTyped(bool isTyped) { 
  my_typed = isTyped;
}

