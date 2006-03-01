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

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
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
* @var fdoutputstream_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fdoutputstream_ioexception;

#define __MY_BUFFERSIZE__ 8192
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn FdOutputStream::FdOutputStream()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FdOutputStream::FdOutputStream() {
  ateof = false;
  my_fd = -1;
  my_bufsiz = __MY_BUFFERSIZE__;
  buffer.limit(my_bufsiz);
  my_work = new char[my_bufsiz];
}

/*==============================================================*/
/** @fn FdOutputStream::FdOutputStream(int fd)
* @brief Constructor with a file descriptor.
*
* Constructor with a file descriptor.
*
* @param fd The file descriptor.
* @return this
*/                                                             
FdOutputStream::FdOutputStream(int fd) {
  ateof = false;
  if (fd >= 0) my_fd = ::dup(fd);
  else my_fd = fd;
  my_bufsiz = __MY_BUFFERSIZE__;
  buffer.limit(my_bufsiz);
  my_work = new char[my_bufsiz];
}

/*==============================================================*/
/** @fn FdOutputStream::FdOutputStream(int fd,int bufsiz)
* @brief Constructor with a file descriptor and a buffer size.
*
* Constructor with a file descriptor and an internal buffer size.
*
* @param fd The file descriptor.
* @param bufsiz Size of the IO buffer.
* @return this
*/                                                             
FdOutputStream::FdOutputStream(int fd,int bufsiz) {
  ateof = false;
  if (fd >= 0) my_fd = ::dup(fd);
  else my_fd = fd;
  my_bufsiz = bufsiz;
  my_bufsiz = __MY_BUFFERSIZE__;
  if (bufsiz >= 0) my_bufsiz = bufsiz;
  buffer.limit(my_bufsiz);
  my_work = new char[my_bufsiz];
}

/*==============================================================*/
/** @fn FdOutputStream::~FdOutputStream()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FdOutputStream::~FdOutputStream() { 
  if (my_work != NULL) delete[] my_work;
  my_work = NULL; 
  if (my_fd >= 0) ::close(my_fd);
  my_fd = -1;
}

/*==============================================================*/
/** @fn bool FdOutputStream::setBuffer(int bufsiz)
* @brief Set the IO buffer size.
*
* Change this stream's buffer size.  The current buffer capacity
* must not exceed the new buffer size or the buffer size will
* not be changed.
*
* @param bufsiz The new buffer size.
* @return If the buffer size was changed.
* @throw IOException if there is an IO error.
*/                                                             
bool FdOutputStream::setBuffer(int bufsiz) {
  if (bufsiz < buffer.capacity()) return false;
  if (bufsiz >= 0) my_bufsiz = bufsiz;
  buffer.limit(my_bufsiz);
  if (my_work != NULL) {
    delete[] my_work;
    my_work = NULL;
  } 
  if (bufsiz > 0) my_work = new char[my_bufsiz];
  return true;
}

/*==============================================================*/
/** @fn void FdOutputStream::close()
* @brief Close this file.
*
* Close the underlying file associated with this output stream.
*
* @param None
* @return None
* @throw IOException if close fails.
*/                                                             
void FdOutputStream::close() {
  if (my_fd >= 0) {
    flush();
    ateof = true;
    if (::close(my_fd) < 0) {
      my_fd = -1;
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::close() failed to close file: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
    }
    my_fd = -1; 
  } 
}

/*==============================================================*/
/** @fn void FdOutputStream::open(int newfd)
* @brief Open with a new descriptor.
*
* Open with a new descriptor.
*
* @param newfd The new file descriptor.
* @return None
* @throw IOException if the stream is already open.
*/                                                             
void FdOutputStream::open(int newfd) {
  if (my_fd >= 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::open() stream is not closed"));
  if (newfd >= 0) my_fd = ::dup(newfd);
  else my_fd = newfd;
}

/*==============================================================*/
/** @fn int FdOutputStream::getFD()
* @brief Get the underlying file descriptor.
*
* Get the underlying file descriptor.
*
* @param None
* @return The underlying file descriptor or -1.
*/                                                             
int FdOutputStream::getFD() {
  return my_fd;
}

/*==============================================================*/
/** @fn int FdOutputStream::write(int b)
* @brief Write a byte.
*
* Write a byte to the output stream.
*
* @param b The byte to write.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdOutputStream::write(int b) {
  if (my_fd < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write() file not open"));
  return write_output((ubyte*)&b,0,1);
}

/*==============================================================*/
/** @fn int FdOutputStream::write(ubyte *rArray,int rlen)
* @brief Write bytes from a ubyte array.
*
* Writes up to rlen bytes from the ubyte array.
*
* @param rArray The Array to write into.
* @param rlen The request number of bytes to read.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdOutputStream::write(ubyte *rArray,int rlen) {
  if (my_fd < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write() file not open"));
  if (rArray == NULL) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::read() parameter rArray cannot be NULL"));
  if (rlen <= 0) return 0; 
  return write_output(rArray,0,rlen);
}

/*==============================================================*/
/** @fn int FdOutputStream::write(ubyte *rArray,int oset,int len)
* @brief Write bytes into a ubyte array.
*
* Writes up to len bytes from the ubyte array at an offset.
*
* @param rArray The Array to write from.
* @param oset The offset.
* @param len The maximum number of bytes to write.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdOutputStream::write(ubyte *rArray,int oset,int len) {
  if (my_fd < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write() file not open"));

  if (len <= 0) return 0; 
  if (oset < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write() offset cannot be <0"));
  if (rArray == NULL) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::read() parameter rArray cannot be NULL"));

  return write_output(rArray,oset,len);
}

/*==============================================================*/
/** @fn void FdOutputStream::flush()
* @brief Flush this stream.
*
* Flush this stream.
*
* @param None
* @return None
* @throw IOException if there's an IO error.
*/                                                             
void FdOutputStream::flush() {
  if (my_fd < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::flush() file not open"));
  int cap = buffer.capacity();
  buffer.get(0,(ubyte*)my_work,cap);

  int c = 0;
  int n = cap;
  int myerrno = 0;
  char *p = my_work;
  int rc = 0;
  while (n > 0) {
    rc = ::write(my_fd,p+c,n);
    myerrno = errno;
    if (rc <= 0) break;
    n -= rc;
    c += rc;
  }

  if (rc < 0) {
    if (myerrno != EAGAIN) {
      ateof = true;
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write_output() IO error while writing: msg=\"%s\" rc=%d"),strerror_r(myerrno,buf,DAQHWYAPI_STRERROR_SIZE),myerrno);
    }
  }

  buffer.consume(c);
}

/*==============================================================*/
/** @fn int FdOutputStream::write_output(ubyte *buf,int oset,int len)
* @brief Write characters to the stream.
*
* Writes characters to a stream.
*
* @param buf The characters to write.
* @param oset The offset.
* @param len The maximum number of bytes to write.
* @return The number of bytes written or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdOutputStream::write_output(ubyte *buf,int oset,int len) {
  if (my_fd < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write_output() stream not open"));
  if (ateof) return -1;

  if (len <= 0) return 0; 
  if (oset < 0) throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write_output() offset cannot be <0"));

  int bn = len;
  int bc = 0;
  char *b = (char*)buf;
  b += oset;
  while (bn > 0) {
    if (my_bufsiz > 0) { // Buffered
      int rem = my_bufsiz - buffer.capacity();
      int badd = (rem > bn) ? bn : rem; 
      if (badd > 0) buffer.put((ubyte*)(b+bc),badd);
      bc += badd;
      bn -= badd;
    } else { // Unbuffered
      bn = 0;
    }

    if ((my_bufsiz <= 0)||(buffer.capacity() >= my_bufsiz)) { 
      int c = 0;
      int n = len;
      int myerrno = 0; 
      ubyte *p = NULL;
      if (my_bufsiz > 0) { // Buffered
        buffer.get(0,(ubyte*)my_work,my_bufsiz);
        n = my_bufsiz;
        p = (ubyte*)my_work;
      } else {  // Unbuffered
        n = len;
        p = buf + oset;
      }
      int rc = 0;
      while (n > 0) {
        rc = ::write(my_fd,p+c,n);
        myerrno = errno;
        if (rc <= 0) break;
        n -= rc;
        c += rc;
      }
      if ((c > 0)&&(my_bufsiz > 0)) buffer.consume(c);

      if (rc < 0) {
        if (myerrno != EAGAIN) {
          ateof = true;
          char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
          throw fdoutputstream_ioexception.format(CSTR("FdOutputStream::write_output() IO error while writing: msg=\"%s\" rc=%d"),strerror_r(myerrno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
        }
      }
    }
  }

  if ((bc == 0)&&(ateof)) return -1;
  else return bc;
}
