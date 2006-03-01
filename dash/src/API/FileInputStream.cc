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

#ifndef DAQHWYAPI_FILEINPUTSTREAM_H
#include <dshapi/FileInputStream.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var fileinputstream_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fileinputstream_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn FileInputStream::FileInputStream()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FileInputStream::FileInputStream() { }

/*==============================================================*/
/** @fn FileInputStream::FileInputStream(String& aFile)
* @brief Constructor with a file name.
*
* Constructor with a file name.
*
* @param aFile The file name
* @return this
*/                                                             
FileInputStream::FileInputStream(String& aFile) {
  my_fd = read_open(aFile.c_str());
}

/*==============================================================*/
/** @fn FileInputStream::FileInputStream(std::string& aFile)
* @brief Constructor with a file name.
*
* Constructor with a file name.
*
* @param aFile The file name
* @return this
*/                                                             
FileInputStream::FileInputStream(std::string& aFile) {
  my_fd = read_open(aFile.c_str());
}

/*==============================================================*/
/** @fn FileInputStream::FileInputStream(FILE *pFile)
* @brief Constructor with a FILE pointer.
*
* Constructor with a FILE pointer.
*
* @param pFile The file pointer. 
* @return this
*/                                                             
FileInputStream::FileInputStream(FILE *pFile) {
  my_fd = ::fileno(pFile);
  if (my_fd >= 0) my_fd = ::dup(my_fd);
}

/*==============================================================*/
/** @fn FileInputStream::FileInputStream(int fd)
* @brief Constructor with a file descriptor.
*
* Constructor with a file descriptor.
*
* @param fd The file descriptor.
* @return this
*/                                                             
FileInputStream::FileInputStream(int fd) {
  if (fd >= 0) my_fd = ::dup(fd);
  else my_fd = fd;
}

/*==============================================================*/
/** @fn FileInputStream::~FileInputStream()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FileInputStream::~FileInputStream() { 
  if (my_fd >= 0) ::close(my_fd);
  my_fd = -1;
}

/*==============================================================*/
/** @fn int FileInputStream::read_open(const char *fname)
* @brief Open a file.
*
* Open a file.
*
* @param fname The file name.
* @return A file descriptor or -1.
* @throw IOException if the file cannot be opened.
*/                                                             
int FileInputStream::read_open(const char *fname) {
  my_fd = -1;
  if (fname == NULL) throw fileinputstream_ioexception.format(CSTR("FileInputStream::read_open() cannot open a NULL file name"));

  my_fd = ::open(fname,O_RDONLY);

  if (my_fd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fileinputstream_ioexception.format(CSTR("FileInputStream::read_open() failed to open file \"%s\": msg=\"%s\" rc=%d"),fname,strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  return my_fd;
}
