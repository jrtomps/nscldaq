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

#ifndef DAQHWYAPI_LINERECORDREADER_H
#include <dshapi/LineRecordReader.h>
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
* @var linerecordreader_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException linerecordreader_ioexception;

#define LINEREADER_BUFFER_SIZE 2048
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn LineRecordReader::LineRecordReader()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
LineRecordReader::LineRecordReader() {
  my_maxbuf = Record::default_max_record_size; 
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = NULL;
  is_ready = false;
  my_wrkbuf = new ubyte[LINEREADER_BUFFER_SIZE];
  my_sepchars = "\r\n";
}

/*==============================================================*/
/** @fn LineRecordReader::LineRecordReader(FdInputStream& rStream)
* @brief Constructor with an InputStream.
*
* Constructor with an InputStream.
*
* @param rStream An InputStream.
* @return this
*/                                                             
LineRecordReader::LineRecordReader(FdInputStream& rStream) {
  my_maxbuf = Record::default_max_record_size; 
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = &rStream;
  is_ready = false;
  my_wrkbuf = new ubyte[LINEREADER_BUFFER_SIZE];
  my_sepchars = "\r\n";
}

/*==============================================================*/
/** @fn LineRecordReader::LineRecordReader(FdInputStream& rStream,int maxbuf)
* @brief Constructor with an InputStream and maximum size.
*
* Constructor with an InputStream and a maxium record data size.
*
* @param rStream An InputStream.
* @param maxbuf The maximum record data size (without header)
* @return this
*/                                                             
LineRecordReader::LineRecordReader(FdInputStream& rStream,int maxbuf) {
  my_maxbuf = Record::default_max_record_size; 
  if (maxbuf > 0) my_maxbuf = maxbuf;
  my_buffer = new ubyte[my_maxbuf];
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  my_stream = &rStream;
  is_ready = false;
  my_wrkbuf = new ubyte[LINEREADER_BUFFER_SIZE];
  my_sepchars = "\r\n";
}

/*==============================================================*/
/** @fn LineRecordReader::~LineRecordReader()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
LineRecordReader::~LineRecordReader() { 
  if (my_buffer != NULL) delete[] my_buffer;
  my_buffer = NULL;
  if (my_wrkbuf != NULL) delete[] my_wrkbuf;
  my_wrkbuf = NULL;
  is_ready = false;
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
}

/*==============================================================*/
/** @fn void LineRecordReader::close()
* @brief Close this reader.
*
* Close this reader.  Simply sets the input stream to NULL
* and clears the buffers; does not actually close the input stream.
*
* @param None
* @return None
*/                                                             
void LineRecordReader::close() { 
  my_stream = NULL; 
  my_buflen = 0;
  cur_pos = 0;
  cur_avail = 0;
  is_ready = false;
}

/*==============================================================*/
/** @fn void *LineRecordReader::is_separator(char c)
* @brief Check if a character is a line separator.
*
* Check if a character is a line separator.
*
* @param c The character to check.
* @return If c is a line separator.
*/                                                             
bool LineRecordReader::is_separator(char c) {
  if (my_sepchars.size() <= 0) return false; // No separators?

  int siz = my_sepchars.size();
  const char *p = my_sepchars.c_str(); 
  for (int i = 0; i < siz; i++) {
    if (p[i] == c) return true;
  }
 
  return false;
}

/*==============================================================*/
/** @fn bool LineRecordReader::setSeparators(String& nseps)
* @brief Set the line separator characters.
*
* Set the line separator characters.
*
* @param nseps The new line separators.
* @return True if the separators have been set.
*/                                                             
bool LineRecordReader::setSeparators(String& nseps) {
  my_sepchars = nseps; 
  return true;
}

/*==============================================================*/
/** @fn bool LineRecordReader::setSeparators(const char *nseps)
* @brief Set the line separator characters.
*
* Set the line separator characters.
*
* @param nseps The new line separators.
* @return True if the separators have been set.
*/                                                             
bool LineRecordReader::setSeparators(const char *nseps) {
  if (nseps == NULL) my_sepchars.clear();
  else my_sepchars = nseps; 
  return true;
}

/*==============================================================*/
/** @fn bool LineRecordReader::setSeparators(std::string& nseps)
* @brief Set the line separator characters.
*
* Set the line separator characters.
*
* @param nseps The new line separators.
* @return True if the separators have been set.
*/                                                             
bool LineRecordReader::setSeparators(std::string& nseps) {
  if (nseps.c_str() == NULL) my_sepchars.clear();
  else my_sepchars = nseps.c_str(); 
  return true;
}

/*==============================================================*/
/** @fn bool LineRecordReader::ready()
* @brief Check if this stream is ready to be read.
*
* Check if this stream is ready to be read.
*
* @param None
* @return If a record is available for reading.
* @trhow IOException if there is an IO error.
*/                                                             
bool LineRecordReader::ready() {
  if (is_ready) return true;
  if (my_stream == NULL) throw linerecordreader_ioexception.format(CSTR("LineRecordReader::ready() no stream available"));

  while (!is_ready) {
    while ((my_buflen < (my_maxbuf-1))&&(cur_avail > 0)) {
      char c = my_wrkbuf[cur_pos];
      cur_pos++; cur_avail--;
      if (is_separator(c)) {
        is_ready = true;
        break; 
      } else {
        my_buffer[my_buflen] = c;
        my_buflen++; 
      }
    }

    if (my_buflen >= (my_maxbuf-1)) { // Max
      is_ready = true;
    } else if (my_stream->ready()) { // More to read
      if (read_buffer() < 0) {
        is_ready = true; // EOF or error
        break;
      } 
    } else { // Nothing more to read, but can read more
      return false;
    }
  }

  if (is_ready) { // Null terminate
    my_buffer[my_buflen] = '\0';
    my_buflen++; 
  } 
  
  return is_ready;
}

/*==============================================================*/
/** @fn int LineRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray)
* @brief Read a line of characters as a String.
*
* Read a line of characters into a buffer.  Reading terminates
* when either maxbuf-1 bytes have been read or a newline is found in
* the stream.  The newline is consumed and the buffer null
* terminated. 
*
* @param rHdr Record header for this record.
* @param rArray The Array to read into.
* @return The number of bytes read.
* @throw IOException if there's an IO error.
*/                                                             
int LineRecordReader::readRecord(record_header_t& rHdr,ByteArray& rArray) {
  if (my_stream == NULL) throw linerecordreader_ioexception.format(CSTR("LineRecordReader::readRecord() no stream available"));

  Record::initHeader(rHdr); // Clean it up
  rHdr.status_code = Record::status_ok;
  rHdr.extended_header_size = 0;

  if (!is_ready) {
    for(;;) {
      while ((my_buflen < (my_maxbuf-1))&&(cur_avail > 0)) {
        char c = my_wrkbuf[cur_pos];
        cur_pos++; cur_avail--;
        if (is_separator(c)) { 
          // Short circuit escape if we get a newline 
          my_buffer[my_buflen] = '\0';
          int n = my_buflen-1;
          rArray.renew(n);
          memcpy(rArray.elements,my_buffer,n);
          my_buflen = 0;
          rHdr.record_size = n + Record::encode_buffer_size;
          rHdr.data_size = n;
          rHdr.entity_count = 1;
          is_ready = false;  // No longer ready 
          return n;
        } else {
          my_buffer[my_buflen] = c;
          my_buflen++; 
        }
      }

      if (my_buflen < (my_maxbuf-1)) {
        if (read_buffer() < 0) {
          rHdr.status_code = Record::status_err;
          break;
        }
      } else {
        rHdr.status_code = Record::status_trunc;
        break;
      }
    }
  }

  // If we get here, we didn't see a newline.  However, we've
  // either reached the maximum buffer size or hit EOF or error
  // on the stream.
  int n = 0;
  if (my_buflen > 0) {
    my_buffer[my_buflen] = '\0';
    n = my_buflen-1;
    rArray.renew(n);
    memcpy(rArray.elements,my_buffer,n);
    my_buflen = 0;
    rHdr.record_size = n + Record::encode_buffer_size;
    rHdr.data_size = n;
    rHdr.entity_count = 1;
  } else {
    rArray.renew(0);
    n = -1;
  }
  is_ready = false;  // No longer ready 
  return n;
}

/*==============================================================*/
/** @fn bool LineRecordReader::eof()
* @brief Check for end-of-file.
*
* Check for end-of-file.
*
* @param None
* @return True if at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
bool LineRecordReader::eof() {
  if (my_stream == NULL) throw linerecordreader_ioexception.format(CSTR("LineRecordReader::eof() no stream available"));
  int rc = true;
  if ((cur_avail > 0)||(my_buflen > 0)) rc = false;
  else rc = my_stream->eof();
  return(rc);
}

/*==============================================================*/
/** @fn int LineRecordReader::read_buffer()
* @brief Read in a buffer.
*
* Read in a buffer.
*
* @param None
* @return The number of characters available or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int LineRecordReader::read_buffer() {
  if (my_stream == NULL) throw linerecordreader_ioexception.format(CSTR("LineRecordReader::read_buffer() no stream available"));

  // Only read if there is space to read.
  if (cur_avail < LINEREADER_BUFFER_SIZE) {
    // Move data to beginning of buffer
    if (cur_pos != 0) {
      ::memmove(my_wrkbuf,&(my_wrkbuf[cur_pos]),cur_avail);
      cur_pos = 0;
    }

    int wn = 0;
    int rrc = 0;
    if (my_stream->ready()) { 
      int av = my_stream->available();
      int canget = LINEREADER_BUFFER_SIZE - cur_avail;
      wn = (av < canget) ? av : canget;
      if (wn > 0) {
        rrc = my_stream->read(my_wrkbuf,cur_avail,wn);
      } else { // Ready and nothing availbe --> eof
        if (cur_avail <= 0) rrc = -1;
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

