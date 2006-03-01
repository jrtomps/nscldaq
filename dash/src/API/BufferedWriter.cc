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

#ifndef DAQHWYAPI_BUFFEREDWRITER_H
#include <dshapi/BufferedWriter.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_INPUTSTREAM_H
#include <dshapi/OutputStream.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var bufferedwriter_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException bufferedwriter_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn BufferedWriter::BufferedWriter()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
BufferedWriter::BufferedWriter() {
  my_stream = NULL;
}

/*==============================================================*/
/** @fn BufferedWriter::BufferedWriter(OutputStream& rStream)
* @brief Constructor with an OutputStream.
*
* Constructor with an OutputStream.
*
* @param rStream An OutputStream.
* @return this
*/                                                             
BufferedWriter::BufferedWriter(OutputStream& rStream) {
  my_stream = &rStream;
}

/*==============================================================*/
/** @fn BufferedWriter::~BufferedWriter()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
BufferedWriter::~BufferedWriter() { 
  my_stream = NULL;
}

/*==============================================================*/
/** @fn void BufferedWriter::close()
* @brief Close this file.
*
* Close the underlying file associated with this input stream.
*
* @param None
* @return None
* @throw IOException if close fails.
*/                                                             
void BufferedWriter::close() {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::close() no stream to close"));
  my_stream->close();
}

/*==============================================================*/
/** @fn int BufferedWriter::write(int b)
* @brief Write a byte.
*
* Write a byte from to the output stream.
*
* @param None
* @return The number of characters written.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedWriter::write(int b) {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::write() no stream available"));
  return my_stream->write(b);
}

/*==============================================================*/
/** @fn int BufferedWriter::write(ubyte *pArray,int len)
* @brief Write bytes from an array.
*
* Write up to len bytes from an array.
*
* @param pArray The array to read into.
* @param len The number of bytes to read.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int BufferedWriter::write(ubyte *pArray,int len) {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::write() no stream available"));
  if (pArray == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::write() pArray cannot be NULL"));
  if (len <= 0) return 0;

  ubyte *b = pArray;
  return my_stream->write_output(b,0,len);
}

/*==============================================================*/
/** @fn int BufferedWriter::write(ubyte *pArray,int oset,int len)
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
int BufferedWriter::write(ubyte *pArray,int oset,int len) {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::read() no stream available"));
  if (pArray == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::read() pArray cannot be NULL"));
  if (len <= 0) return 0;

  ubyte *b = pArray;
  return my_stream->write_output(b,oset,len);
}

/*==============================================================*/
/** @fn void BufferedWriter::newLine()
* @brief Write a line separator.
*
* Write a line separator.
*
* @param None
* @return None
* @throw IOException if there's an IO error.
*/                                                             
void BufferedWriter::newLine() {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::newLine() no stream available"));
  my_stream->write('\n');
}

/*==============================================================*/
/** @fn void BufferedWriter::flush()
* @brief Flush the stream.
*
* Flush the stream.
*
* @param None
* @return None
* @throw IOException if there's an IO error.
*/                                                             
void BufferedWriter::flush() {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::flush() no stream available"));
  else (my_stream->flush());
}

/*==============================================================*/
/** @fn void BufferedWriter::writeLine(String& rStr)
* @brief Write a line.
*
* Write a string followed by a newline.
*
* @param rStr The string to write.
* @return None
* @throw IOException if there's an IO error.
*/                                                             
void BufferedWriter::writeLine(String& rStr) {
  if (my_stream == NULL) throw bufferedwriter_ioexception.format(CSTR("BufferedWriter::writeLine() no stream available"));
  ubyte *b = (ubyte*)(rStr.c_str());
  int len = rStr.size();
  my_stream->write_output(b,0,len);
  my_stream->write('\n');
}
