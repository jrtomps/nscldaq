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

#ifndef DAQHWYAPI_BUFFEREDREADER_H
#include <dshapi/BufferedReader.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/InputStream.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var bufferedreader_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException bufferedreader_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

#define BUFFEREDREADER_BUFFER_SIZE 1024

/*==============================================================*/
/** @fn BufferedReader::BufferedReader()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
BufferedReader::BufferedReader() {
  my_buffer = new ByteArray(BUFFEREDREADER_BUFFER_SIZE);
  cur_pos = 0;
  cur_avail = 0;
  my_stream = NULL;
}

/*==============================================================*/
/** @fn BufferedReader::BufferedReader(InputStream& rStream)
* @brief Constructor with an InputStream.
*
* Constructor with an InputStream.
*
* @param rStream An InputStream.
* @return this
*/                                                             
BufferedReader::BufferedReader(InputStream& rStream) {
  my_buffer = new ByteArray(BUFFEREDREADER_BUFFER_SIZE);
  cur_pos = 0;
  cur_avail = 0;
  my_stream = &rStream;
}

/*==============================================================*/
/** @fn BufferedReader::~BufferedReader()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
BufferedReader::~BufferedReader() { 
  my_stream = NULL;
  if (my_buffer != NULL) delete my_buffer;
  my_buffer = NULL;
  cur_pos = 0;
  cur_avail = 0;
}

/*==============================================================*/
/** @fn bool BufferedReader::ready()
* @brief Check if this stream is ready to be read.
*
* Check if this stream is ready to be read.
*
* @param None
* @return The number of characters available (always zero).
* @trhow IOException if there is an IO error.
*/                                                             
bool BufferedReader::ready() {
  if (my_stream == NULL) return false;
  if (cur_avail > 0) return true;
  else return my_stream->ready();
}

/*==============================================================*/
/** @fn void BufferedReader::close()
* @brief Close this file.
*
* Close the underlying file associated with this input stream.
*
* @param None
* @return None
* @throw IOException if close fails.
*/                                                             
void BufferedReader::close() {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::close() no stream to close"));
  my_stream->close();
}

/*==============================================================*/
/** @fn int BufferedReader::read()
* @brief Read a byte.
*
* Read a byte from the input stream.
*
* @param None
* @return The byte or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedReader::read() {
  if (cur_avail <= 0) {
    if (read_buffer() <= 0) return -1;
  }

  char *elems = (char*)(my_buffer->elements);
  int cint = elems[cur_pos];
  cur_pos++; cur_avail--;
  return cint;
}

/*==============================================================*/
/** @fn int BufferedReader::read_buffer()
* @brief Read in a buffer.
*
* Read in a buffer.
*
* @param None
* @return The number of characters available or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedReader::read_buffer() {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read_buffer() no stream available"));

  // Only read if none available
  if (cur_avail <= 0) {
    ubyte *dst = my_buffer->elements;
    cur_avail = my_stream->read_input(dst,0,BUFFEREDREADER_BUFFER_SIZE);
    cur_pos = 0; 
    if (cur_avail < 0) {
      cur_avail = 0; 
      return -1;
    }
  }

  return cur_avail; 
}

/*==============================================================*/
/** @fn int BufferedReader::read(ubyte *pArray,int len)
* @brief Read bytes.
*
* Reads up to len bytes.
*
* @param pArray The array to read into.
* @param len The number of bytes to read.
* @return The number of bytes read or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedReader::read(ubyte *pArray,int len) {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read() no stream available"));
  if (pArray == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read() pArray cannot be NULL"));
  if (len <= 0) return 0;

  int inlen = len;
  ubyte *inelems = pArray;
  int inpos = 0;
  int tcnt = 0;

  while (inlen > 0) {
    if (cur_avail <= 0) {
      if (read_buffer() <= 0) {
        if (tcnt == 0) return -1;
        else return tcnt;
      }
    }

    ubyte *elems = my_buffer->elements;
    int cnt = inlen < cur_avail ? inlen : cur_avail;
    for (int i = 0; i < cnt; i++) {
      inelems[inpos] = elems[cur_pos]; 
      cur_pos++; cur_avail--; 
      inpos++; inlen--;
      tcnt++;
    }
  }

  return tcnt;
}

/*==============================================================*/
/** @fn int BufferedReader::read(ubyte *pArray,int oset,int len)
* @brief Read a bytes into an array. 
*
* Reads up to len bytes into the array at an offset.
*
* @param pArray The array to read into.
* @param oset The offset.
* @param len The maximum number of bytes to read.
* @return The number of bytes read or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedReader::read(ubyte *pArray,int oset,int len) {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read() no stream available"));
  if (pArray == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read() pArray cannot be NULL"));

  if (len <= 0) return 0; 
  if (oset < 0) throw bufferedreader_ioexception.format(CSTR("BufferedReader::read() offset cannot be <0"));

  ubyte *inelems = pArray;
  int inlen = len;
  int inpos = oset;
  int tcnt = 0;

  while (inlen > 0) {
    if (cur_avail <= 0) {
      if (read_buffer() <= 0) {
        if (tcnt == 0) return -1;
        else return tcnt;
      }
    }

    ubyte *elems = my_buffer->elements;
    int cnt = inlen < cur_avail ? inlen : cur_avail;
    for (int i = 0; i < cnt; i++) {
      inelems[inpos] = elems[cur_pos]; 
      cur_pos++; cur_avail--; 
      inpos++; inlen--;
      tcnt++;
    }
  }

  return tcnt;
}

/*==============================================================*/
/** @fn long BufferedReader::skip(long n)
* @brief Skip and discard bytes from the input stream.
*
* Skip and discard bytes from the input stream.
*
* @param n The number of bytes to skip.
* @return The actual number of bytes skipped.
* @throw IOException if there's an IO error.
*/                                                             
long BufferedReader::skip(long n) {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::skip() no stream available"));

  int rmain = n;
  int csum = cur_avail <= n ? cur_avail : n;
  
  if (csum > 0) {
    cur_avail -= csum;
    cur_pos += csum;
    rmain -= csum;
  }

  int rc = 0;
  if (rmain > 0) rc = my_stream->skip(rmain);
  return(rc+csum);
}

/*==============================================================*/
/** @fn String BufferedReader::readLine()
* @brief Read a line of characters as a String.
*
* Read a line of characters as a String.
*
* @param None
* @return A String of characters.
* @throw IOException if there's an IO error.
*/                                                             
String BufferedReader::readLine() {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::readLine() no stream available"));
  String str;

  for(;;) {
    ubyte *elems = my_buffer->elements;
    while (cur_avail > 0) {
      char c = elems[cur_pos];
      cur_pos++; cur_avail--;
      if ((c == '\r')||(c == '\n')) {
        return str;
      } else {
        str.append(c);
      }
    }
    if (read_buffer() < 0) break;
  }

  return str;
}

/*==============================================================*/
/** @fn bool BufferedReader::eof()
* @brief Check for end-of-file.
*
* Check for end-of-file.
*
* @param None
* @return True if at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
bool BufferedReader::eof() {
  if (my_stream == NULL) throw bufferedreader_ioexception.format(CSTR("BufferedReader::eof() no stream available"));
  if (cur_avail > 0) return false;
  else return (my_stream->eof());
}
