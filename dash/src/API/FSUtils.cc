/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>

#ifndef DAQHWYAPI_FSUTILS_H
#include <dshapi/FSUtils.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_STRINGARRAY_H
#include <dshapi/StringArray.h>
#endif

namespace daqhwyapi {
/**
* @var fsutils_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException fsutils_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn FSUtils::FSUtils()
* @brief Default constructor.
*                                        
* Default concstructor.
*                                         
* @param None
* @return this
*/      
FSUtils::FSUtils() { }

/*===================================================================*/
/** @fn bool FSUtils::pathExists(String& path)
* @brief Check if a file exists.
*                                        
* Check if a file exists.  Returns true if the path exists.  The
* path can be a directory, file or link.
*                                         
* @param path The filesystem path to check.
* @return If the path exists and is accessible.
* @throw IOException On an IO error (such as access denied). 
*/      
bool FSUtils::pathExists(String& path) { 
 struct stat buf; 
 if (stat(path.c_str(),&buf) < 0) {
   int myerrno = errno;
   if ((myerrno == ENOENT)||(myerrno == ENOTDIR)) {
     return false;
   } else {
     char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
     throw fsutils_ioexception.format(CSTR("FSUtils::pathExists() stat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
   }
 }   
 return true;
}

/*===================================================================*/
/** @fn bool FSUtils::isaDirectory(String& path)
* @brief Check if a directory exists.
*                                        
* Check if a path exists and is a directory.  
*                                         
* @param path The filesystem directory to check.
* @return If the directory exists.
* @throw IOException On an IO error (such as access denied). 
*/      
bool FSUtils::isaDirectory(String& path) { 
 struct stat buf; 
 if (stat(path.c_str(),&buf) < 0) {
   int myerrno = errno;
   if ((myerrno == ENOENT)||(myerrno == ENOTDIR)) {
     return false;
   } else {
     char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
     throw fsutils_ioexception.format(CSTR("FSUtils::pathExists() stat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
   }
 }   

 if (S_ISDIR(buf.st_mode) > 0) return true;
 else return false; 
}

/*===================================================================*/
/** @fn off_t FSUtils::fileSize(String& path)
* @brief Return the current file size.
*                                        
* Return the current size of a file.
*                                         
* @param path The filesystem path to check.
* @return The current size of the specified file or -1 if the file doesn't exist.
* @throw IOException On an IO error.
*/      
off_t FSUtils::fileSize(String& path) { 
 struct stat buf; 
 if (stat(path.c_str(),&buf) < 0) {
   int myerrno = errno;
   if ((myerrno == ENOENT)||(myerrno == ENOTDIR)) {
     return -1;
   } else {
     char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
     throw fsutils_ioexception.format(CSTR("FSUtils::fileSize() stat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
   }
 }   

 return buf.st_size; 
}

/*===================================================================*/
/** @fn off_t FSUtils::fileSize(int fd)
* @brief Return the current file size.
*                                        
* Return the current size of a file.
*                                         
* @param fd File descriptor for an open file.
* @return The current size of the specified file.
* @throw IOException On an IO error.
*/      
off_t FSUtils::fileSize(int fd) { 
 struct stat buf; 
 if (fstat(fd,&buf) < 0) {
   int myerrno = errno;
   char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
   throw fsutils_ioexception.format(CSTR("FSUtils::fileSize() fstat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
 }   

 return buf.st_size; 
}

/*===================================================================*/
/** @fn time_t FSUtils::fileCTime(String& path)
* @brief Return the current file creation time.
*                                        
* Return the current file creation time in seconds since the epoch.
*                                         
* @param path The filesystem path to check.
* @return The creation time of the specified file or -1 if the file doesn't exist.
* @throw IOException On an IO error.
*/      
time_t FSUtils::fileCTime(String& path) { 
 struct stat buf; 
 if (stat(path.c_str(),&buf) < 0) {
   int myerrno = errno;
   if ((myerrno == ENOENT)||(myerrno == ENOTDIR)) {
     return -1;
   } else {
     char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
     throw fsutils_ioexception.format(CSTR("FSUtils::fileCTime() stat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
   }
 }   

 return buf.st_ctime; 
}

/*===================================================================*/
/** @fn time_t FSUtils::fileCTime(int fd)
* @brief Return the current file creation time.
*                                        
* Return the current file creation time in seconds since the epoch.
*                                         
* @param fd File descriptor for an open file.
* @return The creation time of the specified file in seconds since the epoch.
* @throw IOException On an IO error.
*/      
time_t FSUtils::fileCTime(int fd) { 
 struct stat buf; 
 if (fstat(fd,&buf) < 0) {
   int myerrno = errno;
   char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
   throw fsutils_ioexception.format(CSTR("FSUtils::fileCTime() fstat(2) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
 }   

 return buf.st_ctime; 
}

/*===================================================================*/
/** @fn void FSUtils::directoryList(StringArray& dirlist,String& dirpath)
* @brief Return a list of files in a directory.
*                                        
* Return a list of files in a directory.   The files will be sorted
* in alpha order. 
*                                         
* @param dirlist Output.  The list of files in a directory.
* @param dirpath The directory to scan for files.
* @return None.
* @throw IOException On an IO error.
*/      
void FSUtils::directoryList(StringArray& dirlist,String& dirpath) { 
  struct dirent **namelist = NULL;
  int n = ::scandir(dirpath.c_str(),&namelist,0,alphasort);
  if (n < 0) {
    int myerrno = errno;
    dirlist.clear();
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fsutils_ioexception.format(CSTR("FSUtils::directoryList() scandir(3) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
  } else {
    dirlist.renew(n); 
    for (int i = 0; i < n; i++) {
      dirlist.elements[i] = new String(namelist[i]->d_name);
      free(namelist[i]);
    }
    free(namelist);
  }
}

/*===================================================================*/
/** @fn void FSUtils::directoryList(StringArray& dirlist,String& dirpath,String& regx)
* @brief Return a list of files in a directory that match a pattern.
*                                        
* Return a list of files in a directory that match a pattern.   
* A directory is scanned for files that match a regular expression as
* defined by fnmatch(3).  The files that match the specified regular
* expression will be returned sorted in alpha order.
*                                         
* @param dirlist Output.  The list of files in a directory.
* @param dirpath The directory to scan for files.
* @param regx The fnmatch regular expression.
* @return None.
* @throw IOException On an IO error.
*/      
void FSUtils::directoryList(StringArray& dirlist,String& dirpath,String& regx) { 
  struct dirent **namelist = NULL;
  int n = ::scandir(dirpath.c_str(),&namelist,0,alphasort);
  if (n < 0) {
    int myerrno = errno;
    dirlist.clear();
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw fsutils_ioexception.format(CSTR("FSUtils::directoryList() scandir(3) error: msg=\"%s\" errno=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),myerrno);
  } else {
    int cnt = 0;
    StringArray warry(n);

    // First find the file names that match the regular expression
    for (int i = 0; i < n; i++) {
      if (fnmatch(regx.c_str(),namelist[i]->d_name,0) == 0) {
        warry.elements[cnt] = new String(namelist[i]->d_name);
        cnt++; 
      }
      free(namelist[i]);
    }
    free(namelist);

    // Now put matching entries in the dirlist
    dirlist.renew(cnt); 
    for (int i = 0; i < cnt; i++) {
      dirlist.elements[i] = warry.elements[i];
    }
  }
}

