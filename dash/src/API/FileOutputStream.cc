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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_FILEOUTPUTSTREAM_H
#include <dshapi/FileOutputStream.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var fileoutputstream_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fileoutputstream_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

int FileOutputStream::ModeTruncate = 1;
int FileOutputStream::ModeAppend = 2;
int FileOutputStream::DefaultPerms = S_IWUSR|S_IRUSR;

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
FileOutputStream::FileOutputStream() {
  my_fd = -1;
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(String& aFile)
* @brief Constructor with a file name.
*
* Constructor with a file name.  Opens the specified file
* for appending.
*
* @param aFile The file name
* @return this
*/                                                             
FileOutputStream::FileOutputStream(String& aFile) {
  my_fd = write_open(aFile.c_str(),FileOutputStream::ModeAppend,FileOutputStream::DefaultPerms);
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(std::string& aFile)
* @brief Constructor with a file name.
*
* Constructor with a file name.  Opens the specified file
* for appending.
*
* @param aFile The file name
* @return this
*/                                                             
FileOutputStream::FileOutputStream(std::string& aFile) {
  my_fd = write_open(aFile.c_str(),FileOutputStream::ModeAppend,FileOutputStream::DefaultPerms);
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(String& aFile,int aMode)
* @brief Constructor with a file name and mode.
*
* Constructor with a file name write mode 
* (ModeTruncate or ModeAppend).
*
* @param aFile The file name
* @param aMode File mode for opening the file.
* @return this
*/                                                             
FileOutputStream::FileOutputStream(String& aFile,int aMode) {
  my_fd = write_open(aFile.c_str(),aMode,FileOutputStream::DefaultPerms);
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(std::string& aFile,int aMode)
* @brief Constructor with a file name and mode.
*
* Constructor with a file name write mode 
* (ModeTruncate or ModeAppend).
*
* @param aFile The file name
* @param aMode File mode for opening the file.
* @return this
*/                                                             
FileOutputStream::FileOutputStream(std::string& aFile,int aMode) {
  my_fd = write_open(aFile.c_str(),aMode,FileOutputStream::DefaultPerms);
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(FILE *pFile)
* @brief Constructor with a FILE pointer.
*
* Constructor with a FILE pointer.
*
* @param pFile The file pointer. 
* @return this
*/                                                             
FileOutputStream::FileOutputStream(FILE *pFile) {
  my_fd = ::fileno(pFile);
  if (my_fd >= 0) my_fd = ::dup(my_fd);
}

/*==============================================================*/
/** @fn FileOutputStream::FileOutputStream(int fd)
* @brief Constructor with a file descriptor.
*
* Constructor with a file descriptor.
*
* @param fd The file descriptor.
* @return this
*/                                                             
FileOutputStream::FileOutputStream(int fd) {
  if (fd >= 0) my_fd = ::dup(fd);
  else my_fd = fd;
}

/*==============================================================*/
/** @fn FileOutputStream::~FileOutputStream()
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
FileOutputStream::~FileOutputStream() { 
  if (my_fd >= 0) ::close(my_fd);
  my_fd = -1;
}

/*==============================================================*/
/** @fn int FileOutputStream::write_open(const char *fname,int aFlags,mode_t aMode)
* @brief Open a file.
*
* Open a file.
*
* @param fname The file name.
* @param aFlags Where to open in append or truncate mode (ModeAppend|ModeTruncate).
* @param aMode Permissions for opening the file.
* @return A FILE pointer or NULL.
* @throw IOException if the file cannot be opened.
*/                                                             
int FileOutputStream::write_open(const char *fname,int aFlags,mode_t aMode) {
  int fd = -1;
  if (fname == NULL) throw fileoutputstream_ioexception.format(CSTR("FileOutputStream::write_open() cannot open a NULL file name"));

  if (aFlags == FileOutputStream::ModeTruncate) {
    fd = ::open(fname,O_WRONLY|O_CREAT|O_TRUNC,aMode);
  } else if (aFlags == FileOutputStream::ModeAppend) {
    fd = ::open(fname,O_WRONLY|O_CREAT|O_APPEND,aMode);
  } else {
    throw fileoutputstream_ioexception.format(CSTR("FileOutputStream::write_open() unknown mode = %d"),aMode);
  }

  if (fd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fileoutputstream_ioexception.format(CSTR("FileOutputStream::write_open() failed to open file \"%s\": msg=\"%s\" rc=%d"),fname,strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  return fd;
}

