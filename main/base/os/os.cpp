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
#include "io.h"
#include <ErrnoException.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdexcept>
#include <netdb.h>
#include <sstream>
#include <limits.h>

#include <iostream>
#include <errno.h>

static const unsigned NSEC_PER_SEC(1000000000); // nanoseconds/second.


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
/**
 * Os::usleep
 *
 *    Wrapper for nanosleep since usleep is deprecated in POSIX
 *    but nanosleep is consider good.
 *
 * @param usec - Number of microseconds to sleep.
 * @return int - Status (0 on success, -1 on error)
 * 
 * @note No attempt is made to map errnos from usleep -> nanosleep...so you'll 
 *       get then nanosleep errnos directly.
 * @note We assume useconds_t is an unsigned int like type.
 */
int
Os::usleep(useconds_t usec)
{
  // usec must be converted to nanoseconds and then busted into
  // seconds and remaning nanoseconds
  // we're going to assume there's no overflow from this:
  
  useconds_t nsec = usec* 1000;			// 1000 ns in a microsecond.

  // Construct the nanosleep specification:

  struct timespec delay;
  delay.tv_sec  = nsec/NSEC_PER_SEC;
  delay.tv_nsec = nsec % NSEC_PER_SEC;


  struct timespec remaining;
  int stat;

  // Usleep is interrupted with no clue left about the remainnig time:

  return nanosleep(&delay, &remaining);

 
}
/**
 * Os::blockSignal
 *   Blocks the specified signal.
 *
 * @param sigNum - Number of the signal to block.
 *
 * @return value from sigaction
 */
int
Os::blockSignal(int sigNum)
{

  // Build the sigaction struct:

  struct sigaction action;
  action.sa_handler = 0;		// No signal handler.
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, sigNum);
  action.sa_flags = 0 ;

  struct sigaction oldAction;

  return sigaction(sigNum, &action, &oldAction);

  
}

int Os::checkStatus(int returnStatus, int checkedStatus, std::string msg) 
{
  if (returnStatus!=checkedStatus) {
    throw std::runtime_error(msg);
  }
  //otherwise, we should just return the status
  return returnStatus;
}

int Os::checkNegativeStatus(int returnStatus) 
{
  if (returnStatus<0) {
    throw CErrnoException(strerror(errno));
  }

  return returnStatus;
}
/**
 * getfqdn
 *    Return the fully qualified domain name of a host.
 *
 *  @param host - the host name to lookup.  Note that this can be either
 *                a partially or fully qualified domain name.
 *  @return std::string - Fully qualified domain name.
 *  @throw std::string  - If the gethostbyname_r call fails.  This
 *                        could happen for a few reasons including:
 *                        *  No DNS server available.
 *                        *  The input host does not exist.
 */
std::string
Os::getfqdn(const char* host)
{
  struct hostent result;
  struct hostent* pResult;
  char   buffer[8192];                 // Ought to be big enough.
  int    errorCode;
  
  gethostbyname_r(
    host, &result, buffer, sizeof(buffer), &pResult, &errorCode
  );
  if (!pResult) {
    throw std::string(hstrerror(errorCode));
  }
  return result.h_name;
}
/**
 * getProcesssCommand
 *    Get the command words that make up a process.  Specifically this
 *    reads /proc/pid/cmdline and breaks up each null terminated string into
 *    an element of the returned vector.
 *
 *   @param pid - pid to look up.
 *   @return std::vector<std::string> the words of the command line.
 *   @throw CErrnoException - if the cmdline proc 'file' can't be opened.
 */
std::vector<std::string>
Os::getProcessCommand(pid_t pid)
{
  
  // We're going to gulp in the entire file.  This requires we know the length
  // of the longest command line so we can allocate an appropriately sized
  // buffer.  Note that since spaces on the command line get replaced with
  // nulls, the value from sysconf(ARG_MAX). is sufficient:
  
  long cmdMax = sysconf(_SC_ARG_MAX);
  char* words;
  try {
    words = new char[cmdMax];   // throws on allocation failure.
  }
  catch (std::bad_alloc) {
    words = new char[20480]; // Probably still good.
    cmdMax = 20480;
  }
  memset(words, 0, cmdMax);

  std::vector<std::string> result;
  
  // The remainder of the code is in a try block so the words get deleted
  // in case of error.
  
  
  try {
    // Encode the name of the file we want to open:
    
    std::ostringstream os;
    os << "/proc/" << pid << "/cmdline";
    std::string path = os.str();
    
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
      throw CErrnoException("Opening proc special file element");
    }
    size_t n = io::readData(fd, words, cmdMax);
    // Now marshall the data into the std::vector<std::string>
    // words should be a pile of null terminated strings with an addtional
    // null at the end:
    
    const char* p = words;
    while (*p) {
      std::string aWord = p;
      result.push_back(aWord);
      p += strlen(p) + 1;                  // next string starts here.
    }
    
  }
  catch (...) {
    delete []words;
    throw;
  }
  delete []words;
  return result;

}
/**
 * hostname
 *   @return - Returns the fqdn for the host that's running this code.
 */
std::string
Os::hostname()
{
  char host[HOST_NAME_MAX+1];
  memset(host, 0, sizeof(host));
  
  checkNegativeStatus(gethostname(host, sizeof(host)-1));
  
  // Return the fqdn of host:
  
  return getfqdn(host);
}