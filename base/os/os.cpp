/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "os.h"
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * Get name of current user.
 * @return std::string
 */
std::string
Os::whoami()
{
  struct passwd  Entry;
  struct passwd* pEntry;
  char   dataStorage[1024];	// Storage used by getpwuid_r(3).
  uid_t  uid = getuid();

  if (getpwuid_r(uid, &Entry, dataStorage, sizeof(dataStorage), &pEntry)) {
    int errorCode = errno;
    std::string errorMessage = 
      "Unable to determine the current username in CTheApplication::destinationRing: ";
    errorMessage += strerror(errorCode);
    throw errorMessage;
    
  }
  return std::string(Entry.pw_name);
}
/**
 * Os::authenticateUser:  Authenticate a user given a username and password:
 *
 * @param sUser - the username.
 * @param sPassword - the cleartext passwordl
 *
 * @return - true if the username/password authenticates in the underlying os:
 *
 */
bool
Os::authenticateUser(std::string sUser, std::string sPassword)
{
  struct passwd Entry;
  struct passwd* pEntry;
  char   dataStorage[1024];

  if (getpwnam_r(sUser.c_str(), &Entry, dataStorage, sizeof(dataStorage), &pEntry)) {
    int errorCode = errno;
    std::string errorMsg = "Call to getpwnam_r failed at os level: ";
    errorMsg += strerror(errorCode);
    throw errorMsg;
  }
  if(!pEntry) return false;	// No such user.
  std::string EncryptedPassword(pEntry->pw_passwd);
  std::string EncryptedEntry(crypt(sPassword.c_str(), EncryptedPassword.c_str()));
  
  return EncryptedPassword == EncryptedEntry;
}
