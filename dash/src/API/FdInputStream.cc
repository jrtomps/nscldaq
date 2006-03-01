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
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#include <dshapi/FdInputStream.h>
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
* @var fdinputstream_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fdinputstream_ioexception;

#define __MY_BUFFERSIZE__ 8192
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn FdInputStream::FdInputStream()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FdInputStream::FdInputStream() {
  ateof = false;
  my_fd = -1;
  my_bufsiz = __MY_BUFFERSIZE__;
  buffer.limit(my_bufsiz);
  my_work = new char[my_bufsiz];
  my_mark = (off_t)-1;
}

/*==============================================================*/
/** @fn FdInputStream::FdInputStream(int fd)
* @brief Constructor with a file descriptor.
*
* Constructor with a file descriptor.
*
* @param fd The file descriptor.
* @return this
*/                                                             
FdInputStream::FdInputStream(int fd) {
  ateof = false;
  my_bufsiz = __MY_BUFFERSIZE__;
  buffer.limit(my_bufsiz);
  my_work = new char[my_bufsiz];
  if (fd >= 0) my_fd = ::dup(fd);
  else fd = my_fd;
  my_mark = (off_t)-1;
}

/*==============================================================*/
/** @fn FdInputStream::FdInputStream(int fd,int bufsiz)
* @brief Constructor with a file descriptor and buffer size.
*
* Constructor with a file descriptor and buffer size.
*
* @param fd The file descriptor.
* @param bufsiz The buffer size.
* @return this
*/                                                             
FdInputStream::FdInputStream(int fd,int bufsiz) {
  ateof = false;
  my_bufsiz = __MY_BUFFERSIZE__;
  if (bufsiz >= 0) my_bufsiz = bufsiz;
  buffer.limit(my_bufsiz);
  if (bufsiz > 0) my_work = new char[my_bufsiz];
  if (fd >= 0) my_fd = ::dup(fd);
  else my_fd = fd;
  my_mark = (off_t)-1;
}

/*==============================================================*/
/** @fn FdInputStream::~FdInputStream()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FdInputStream::~FdInputStream() { 
  if (my_work != NULL) delete[] my_work;
  my_work = NULL; 
  if (my_fd >= 0) ::close(my_fd);
  my_fd = -1;
}

/*==============================================================*/
/** @fn bool FdInputStream::setBuffer(int bufsiz)
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
bool FdInputStream::setBuffer(int bufsiz) {
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
/** @fn bool FdInputStream::ready()
* @brief Check if this stream is ready to be read.
*
* Check if this stream is ready to be read.
*
* @param None
* @return If the stream is ready to be read from.
* @throw IOException if there is an IO error.
*/                                                             
bool FdInputStream::ready() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::ready() file not open"));

  if (buffer.capacity() > 0) return true;

  fd_set rfds;
  struct timeval tv;
  int rc = 0;

  FD_ZERO(&rfds);
  FD_SET(my_fd,&rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  rc = ::select(my_fd+1,&rfds,NULL,NULL,&tv);

  // Select returns ready if data can be read w/o blocking or eof
  if (rc > 0) {
    if (available() <= 0) ateof = true;
    return true;
  } else if (rc < 0) { // Error
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::ready() select error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }
 
  return false; 
}

/*==============================================================*/
/** @fn long FdInputStream::available()
* @brief Return the number of bytes available.
*
* Return the number of bytes available for reading without blocking.
*
* @param None
* @return The number of bytes available.
*/                                                             
long FdInputStream::available() {
  int cap = buffer.capacity();
  if (cap < my_bufsiz) fill_buffer();
  return buffer.capacity();
}

/*==============================================================*/
/** @fn void FdInputStream::close()
* @brief Close this file.
*
* Close the underlying file associated with this input stream.
*
* @param None
* @return None
* @throw IOException if close fails.
*/                                                             
void FdInputStream::close() {
  buffer.clear();
  if (my_fd >= 0) {
    ateof = true;
    if (::close(my_fd) < 0) {
      my_fd = -1;
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw fdinputstream_ioexception.format(CSTR("FdInputStream::close() failed to close file: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
    }
    my_fd = -1; 
  } 
}

/*==============================================================*/
/** @fn void FdInputStream::open(int newfd)
* @brief Open with a new descriptor.
*
* Open with a new descriptor.
*
* @param newfd The new file descriptor.
* @return None
* @throw IOException if the stream is already open.
*/                                                             
void FdInputStream::open(int newfd) {
  if (my_fd >= 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::open() stream is not closed"));
  if (newfd >= 0) my_fd = ::dup(newfd);
  else my_fd = newfd;
}

/*==============================================================*/
/** @fn int FdInputStream::read_open(const char *fname)
* @brief Open a file.
*
* Open a file.
*
* @param fname The file name.
* @return A file descriptor or -1.
* @throw IOException if the file cannot be opened.
*/                                                             
int FdInputStream::read_open(const char *fname) {
  my_fd = -1;
  if (fname == NULL) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_open() cannot open a NULL file name"));

  my_fd = ::open(fname,O_RDONLY);

  if (my_fd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_open() failed to open file \"%s\": msg=\"%s\" rc=%d"),fname,strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  return my_fd;
}

/*==============================================================*/
/** @fn int FdInputStream::getFD()
* @brief Get the underlying file descriptor.
*
* Get the underlying file descriptor.
*
* @param None
* @return The underlying file descriptor or -1.
*/                                                             
int FdInputStream::getFD() {
  return my_fd;
}

/*==============================================================*/
/** @fn int FdInputStream::read()
* @brief Read a byte.
*
* Read a byte from the input stream.
*
* @param None
* @return The byte or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdInputStream::read() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() file not open"));
  char b = (char)-1;  
  int rc = read_input((ubyte*)&b,0,1);
  if (rc < 0) return -1;
  else return (int)b; 
}

/*==============================================================*/
/** @fn int FdInputStream::read(ubyte *rArray,int rlen)
* @brief Read a bytes into a ubyte array.
*
* Reads up to rlen bytes into a byte array.
*
* @param rArray The Array to read into.
* @param rlen Request length
* @return The number of bytes read or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdInputStream::read(ubyte *rArray,int rlen) {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() file not open"));
  if (rlen <= 0) return 0; 
  if (rArray == NULL) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() parameter rArray cannot be NULL"));
  return read_input(rArray,0,rlen);
}

/*==============================================================*/
/** @fn int FdInputStream::read(ubyte *rArray,int oset,int len)
* @brief Read a bytes into a ubyte array.
*
* Reads up to len bytes into the ubyte array at an offset.
*
* @param rArray The Array to read into.
* @param oset The offset.
* @param len The maximum number of bytes to read.
* @return The number of bytes read or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdInputStream::read(ubyte *rArray,int oset,int len) {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() file not open"));

  if (len <= 0) return 0; 
  if (oset < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() offset cannot be <0"));
  if (rArray == NULL) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read() parameter rArray cannot be NULL"));

  return read_input(rArray,oset,len);
}

/*==============================================================*/
/** @fn long FdInputStream::skip(long n)
* @brief Skip and discard bytes from the input stream.
*
* Skip and discard bytes from the input stream.
*
* @param n The number of bytes to skip.
* @return The actual number of bytes skipped.
* @throw IOException if there's an IO error.
*/                                                             
long FdInputStream::skip(long n) {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::skip() file not open"));
  if (n <= 0) return 0;

  ubyte b[n];
  int rc = read_input((ubyte*)b,0,n);
  if (rc <= 0) return 0;
  else return rc;
}

/*==============================================================*/
/** @fn off_t FdInputStream::mark()
* @brief Mark the current stream position.
*
* Mark the current stream position so that a call to 
* reset() will reposition the stream to the marked position. 
*
* @param None
* @return The current mark
* @throw IOException if there's an IO error.
*/                                                             
off_t FdInputStream::mark() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::mark() file not open"));
  my_mark = ::lseek(my_fd,0,SEEK_CUR);
  if (my_mark == (off_t)-1) {
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::mark() IO error occured during lseek: msg=\"%s\" rc=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),errno);
  }
  return my_mark;
}

/*==============================================================*/
/** @fn void FdInputStream::reset()
* @brief Reposition the stream to a previous mark.
*
* Reposition this stream to the position established by a previous
* call to mark().  Any data the is currently buffered is cleared.
*
* @param None
* @return None
* @throw IOException if there's an IO error or mark has not been called.
*/                                                             
void FdInputStream::reset() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::mark() file not open"));

  if (my_mark == (off_t)-1) throw fdinputstream_ioexception.format(CSTR("FdInputStream::reset() no previous call to mark()"));

  if (::lseek(my_fd,my_mark,SEEK_SET) == (off_t)-1) {
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::reset() IO error occured during lseek: msg=\"%s\" rc=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  buffer.clear(); // Remove any remaining data
  ateof = false;
}

/*==============================================================*/
/** @fn void FdInputStream::cleareof()
* @brief Clear an eof on this stream.
*
* Clear an eof on this stream.  Useful when attempting to tail
* a stream.
*
* @param None
* @return None
* @throw IOException if there's an IO error.
*/                                                             
void FdInputStream::cleareof() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::cleareof() file not open"));
  ::lseek(my_fd,0,SEEK_CUR);
  if (::lseek(my_fd,0,SEEK_CUR) == (off_t)-1) {
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::cleareof() IO error occured during lseek: msg=\"%s\" rc=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),errno);
  }
  ateof = false;
}

/*==============================================================*/
/** @fn bool FdInputStream::eof()
* @brief Check for end-of-file.
*
* Check for end-of-file.
*
* @param None
* @return If at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
bool FdInputStream::eof() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::eof() file not open"));
  if (buffer.capacity() > 0) return false; // Still have some data buffered

  if (!ateof) { // Not already at eof, so test
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;  
    fd_set rfs;
    fd_set efs;
    FD_ZERO(&rfs);
    FD_ZERO(&efs);
    FD_SET(my_fd,&rfs);
    FD_SET(my_fd,&efs);
 
    // Select... 
    int rc = ::select(my_fd+1,&rfs,NULL,&efs,&tv);

    if (rc > 0) { 
      // Either we can read w/o blocking or the file is closed
      if (available() <= 0) { // See if any data is out there...
        // No data available and select said we were ready,
        // so we are at eof
        ateof = true;
      }
    } else if (rc < 0) { 
      // Select error
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      throw fdinputstream_ioexception.format(CSTR("FdInputStream::eof() select error: msg=\"%s\" rc=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),errno);
    }
  }

  return ateof;
}

/*==============================================================*/
/** @fn int FdInputStream::read_input(ubyte *buf,int oset,int len)
* @brief Read a bytes into a buffer.
*
* Reads up to len bytes into a buffer at an offset.
*
* @param buf The buffer.
* @param oset The offset.
* @param len The maximum number of bytes to read.
* @return The number of bytes read into buf or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdInputStream::read_input(ubyte *buf,int oset,int len) {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_input() file not open"));

  if (len <= 0) return 0; 
  if (oset < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_input() offset cannot be <0"));
  if (buf == NULL) throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_input() buffer cannot be NULL"));

  int cap = buffer.capacity();
  int c = 0;
  if (cap > 0) {
    int l = (cap < len) ? cap : len;
    if (l <= 0) if (ateof) return -1;
    ubyte *p = buf + oset;
    buffer.get(0,(ubyte*)p,l);
    buffer.consume(l); 
    c += l;
  } 

  if ((!ateof)&&(c < len)) { // Need more data
    int n = len - c; // What we need
    int myerrno = 0;
    int rc = 0;
    int err = 0;
    ubyte *p = buf + oset;
    while (n > 0) {
      err = rc = ::read(my_fd,p+c,n);
      myerrno = errno;
      if (err == 0) ateof = true;
      if (err <= 0) break;
      n -= rc;
      c += rc;
    }

    if ((err < 0)&&(myerrno != EAGAIN)) {
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      throw fdinputstream_ioexception.format(CSTR("FdInputStream::read_input() IO error on input stream: msg=\"%s\" rc=%d"),strerror_r(myerrno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
    }
  }

  if ((c == 0)&&ateof) return -1;
  else return c;
}

/*==============================================================*/
/** @fn int FdInputStream::fill_buffer()
* @brief Read a bytes into a the input buffer.
*
* Reads bytes into the input buffer without blocking.
*
* @param None
* @return The number of bytes read into buf or -1 at end-of-file.
* @throw IOException if there's an IO error.
*/                                                             
int FdInputStream::fill_buffer() {
  if (my_fd < 0) throw fdinputstream_ioexception.format(CSTR("FdInputStream::fill_buffer() file not open"));

  // We can only fill the buffer if buffer has space
  int cap = buffer.capacity();
  int len = my_bufsiz - cap;
  if (len <= 0) return 0;

  int err = 0;
  int myerrno = 0;
  int rc = 0;
  if (!ateof) { // Need more data
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(my_fd,&rfds);
    bool isb = is_blocking();
    if (isb) set_blocking(false);
    err = rc = ::read(my_fd,my_work,len);
    myerrno = errno;
    if (isb) set_blocking(true);
    if (rc > 0) buffer.put((ubyte*)(my_work),rc);
    if (err == 0) ateof = true;
  }

  if ((err < 0)&&(myerrno != EAGAIN)) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fdinputstream_ioexception.format(CSTR("FdInputStream::fill_buffer() IO error on input stream: msg=\"%s\" rc=%d"),strerror_r(myerrno,buf,DAQHWYAPI_STRERROR_SIZE),myerrno);
  }

  if ((rc == 0)&&ateof) return -1;
  else return rc;
}

/*==============================================================*/
/** @fn int FdInputStream::set_blocking(bool doblock)
* @brief Set for nonblocking/blocking reading.
*
* Set for nonblocking or blocking reading IO.
*
* @param doblock To block or not to block.
* @return 0 if OK or -1 on error.
*/                                                             
int FdInputStream::set_blocking(bool doblock) {
  if (my_fd < 0) return -1;

  // Get current settings.
  int val = fcntl(my_fd,F_GETFL,0);
  if (val < 0) return -1;

  // Set or unset blocking IO
  if (doblock) { // Set for blocking
    return(fcntl(my_fd,F_SETFL,val&(~O_NONBLOCK)));
  } else { // Set for nonblocking
    return(fcntl(my_fd,F_SETFL,val|O_NONBLOCK));
  }
}

/*==============================================================*/
/** @fn bool FdInputStream::is_blocking()
* @brief Check for blocking IO
*
* Check for blocking IO.
*
* @param None
* @return true if IO is blocking, false otherwise.
*/                                                             
bool FdInputStream::is_blocking() {
  if (my_fd < 0) return false;

  // Get current settings.
  int val = fcntl(my_fd,F_GETFL,0);
  if (val < 0) return false;

  return((val&O_NONBLOCK) == 0);
}
