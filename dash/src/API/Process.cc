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
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#ifdef HAVE_SYS_PSTAT_H
# include <sys/pstat.h>
#endif

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_PROCESS_H
#include <dshapi/Process.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif


namespace daqhwyapi {
  /**
  * @var process_cannot_start
  * @brief Exception to throw when a process cannot start.
  *
  * Exception to throw when a process cannot start
  */
  static CannotStartException process_cannot_start;

  /**
  * @var process_general_exception.
  * @brief Exception to throw for general process errors.
  *
  * Exception to throw for general process errors.
  */
  static ProcessException process_general_exception;

  // Convenient typedef
  typedef void (*sighandler_t)(int);

  extern int    daqhwyapi_argc;
  extern char **daqhwyapi_argv;
  extern char **daqhwyapi_envp;
  extern int    daqhwyapi_envspace;
  extern char   bt_work_area[];
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn Process::Process()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
Process::Process() { }

/*===================================================================*/
/** @fn Process::~Process()
* @brief Destructor.
*                                        
* Destructor.
*                                         
* @param None
* @return None
*/      
Process::~Process() { }

/*===================================================================*/
/** @fn long Process::getId()
* @brief Get the process id.
*                                        
* Get the process id generated during the last call to start.
*                                         
* @param None
* @return The process id.
*/      
long Process::getId()
{
  return my_id;
}

/*===================================================================*/
/** @fn void Process::setTitle(const String& rTitle)
* @brief Set the process command line title.
*                                        
* Set the process command line title.
*                                         
* @param rTitle new process title.
* @return None
*/      
void Process::setTitle(const String& rTitle)
{
#ifndef NO_SET_PROCNAME 
#ifdef HAVE_SYS_PSTAT_H
  union pstun pst;
 
  pst.pst_command = rTitle.c_str();
  pstat(PSTAT_SETCMD, pst, 0, 0, 0);
  return; 
#else
  int i;
  char *ptr = NULL;

  if ((daqhwyapi::daqhwyapi_argc < 1)||(daqhwyapi::daqhwyapi_argv == NULL)) return;

  ptr = daqhwyapi::daqhwyapi_argv[0];
  for (i = 0; i < daqhwyapi::daqhwyapi_envspace; i++) {
    *ptr = '\0'; ptr++;
  }

  strncpy(daqhwyapi::daqhwyapi_argv[0], rTitle.c_str(), daqhwyapi::daqhwyapi_envspace);
  return;
#endif
#else
   return;
#endif
}

/*===================================================================*/
/** @fn const String& Process::getName() const
* @brief Get the process name.
*                                        
* Get the process name.
*                                         
* @param None
* @return The process name.
*/      
const String& Process::getName() const
{
  return(processname);
}

/*===================================================================*/
/** @fn void Process::setName(const String& rName)
* @brief Set the process name and process title.
*                                        
* Set the process name and process title.
*                                         
* @param rName A new name.
* @return None
*/      
void Process::setName(const String& rName)
{
  processname = rName;
  Process::setTitle(rName);
}

/*===================================================================*/
/** @fn void Process::start()
* @brief Start this process.
*                                        
* Start this process.
*                                         
* @param None
* @return None
*/      
void Process::start() { 
  try {
    my_id = fork();
    if (my_id < 0) {
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw process_cannot_start.format(CSTR("Process::start() fork failure msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
    }

    if (my_id == 0) { // child
      (*this).run();
      my_id = -1;
    } 
  } catch (std::runtime_error& re) {
    cerr << "*** Runtime error caught: \"" << (re.what()) << "\"" << endl;
    exit(-1);
  } catch (daqhwyapi::Exception& oe) {
    cerr << "*** Exception caught: \"" << (oe.what()) << "\"" << endl;
    exit(-2);
  } catch (std::exception& se) {
    cerr << "*** Exception caught: \"" << (se.what()) << "\"" << endl;
    exit(-3);
  } catch (std::string& stre) {
    cerr << "*** std::string exception caught: \"" << (stre.c_str()) << "\"" << endl;
    exit(-4);
  } catch (char *chre) {
    cerr << "*** char* exception caught: \"" << chre << "\"" << endl;
    exit(-5);
  } catch (...) {
    cerr << "*** Unknown exeception caught" << endl;
    exit(-errno);
  }
}

/*===================================================================*/
/** @fn void Process::start_main()
* @brief Start this process without forking (main()).
*                                        
* Start this process without forking (main()).
*                                         
* @param None
* @return None
*/      
void Process::start_main() { 
  try {
    my_id = getpid();  
    (*this).run();
    my_id = -1;  
  } catch (std::runtime_error& re) {
    cerr << "*** Runtime error caught: \"" << (re.what()) << "\"" << endl;
    exit(-1);
  } catch (daqhwyapi::Exception& oe) {
    cerr << "*** Exception caught: \"" << (oe.what()) << "\"" << endl;
    exit(-2);
  } catch (std::exception& se) {
    cerr << "*** Exception caught: \"" << (se.what()) << "\"" << endl;
    exit(-3);
  } catch (...) {
    cerr << "*** Unknown exeception caught" << endl;
    exit(-4);
  }
}

/*===================================================================*/
/** @fn void Process::daemonize(bool close_out,bool close_err,bool close_in)
* @brief Turn this process into a daemon.
*                                        
* Turn this process into a daemon.  That is, background this
* process, disconnect from stdin, stderr and stdout, ignore
* several signals (including: SIGTTOU, SIGTTIN, SIGTSTP and SIGHUP),
* and reset the umask and process group.
*                                         
* @param close_out True to close stdout (default).
* @param close_err True to close stderr (default).
* @param close_in True to close stdin(default).
* @return None
*/      
void Process::daemonize(bool close_out,bool close_err,bool close_in) {
  umask(0);

#ifdef SETPGRP_VOID 
  setsid();
  setpgrp();
#else
  setpgrp(0,setsid());
#endif

#ifdef SIGTTOU
  signal(SIGTTOU,SIG_IGN);
#endif
#ifdef SIGTTIN
  signal(SIGTTIN,SIG_IGN);
#endif
#ifdef SIGTSTP
  signal(SIGTSTP,SIG_IGN);
#endif
#ifdef SIGHUP
  signal(SIGHUP,SIG_IGN);
#endif

  if (close_in) close(0);
  if (close_out) close(1);
  if (close_err) close(2);
}

/*===================================================================*/
/** @fn long Process::waitForAnyChild(bool poll)
* @brief Wait for any child process to exit.
*                                        
* Wait for any child process to exit and reap that child.
*                                         
* @param poll Just poll.  If the process has not exited return immediately.
* @return The process id of the child that exited or -1.
* @throw ProcessException for wait errors.
*/      
long Process::waitForAnyChild(bool poll) {
  pid_t apid = -1;
  int status;
  int opts = 0; 

  if (poll) opts = WNOHANG;
  sighandler_t oldsig = signal(SIGCHLD,SIG_DFL); // Make sure we get the signal 

  if ((apid = wait3(&status,opts,(struct rusage *)0)) <= 0) {
    if ((poll)&&(apid == 0)) return 0; // Polling and no child available
    int myerrno = errno;
    signal(SIGCHLD,oldsig); // Reset the signal handler
    if (myerrno == ECHILD) {
      throw process_general_exception.format(CSTR("Process::waitForAnyChild() No children exist"));
    } else {
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw process_general_exception.format(CSTR("Process::waitForAnyChild() wait failure msg=\"%s\" rc=%d"),strerror_r(myerrno,buf,DAQHWYAPI_STRERROR_SIZE),myerrno);
    } 
  }

  signal(SIGCHLD,oldsig); // Reset the signal handler
  return apid;
}

/*===================================================================*/
/** @fn long Process::waitForChild(long aId,bool poll)
* @brief Wait for a specific child process to exit.
*                                        
* Wait for a specific child process to exit and reap that child.
* The aId parameter has the same effect as if passed to waitpid()
* and can be less-than, equal-to or greater-than zero.  Will return
* zero if polling is requested and no child is available.
*                                         
* @param aId The process id of the child to wait for.
* @param poll Just poll.  If the process has not exited return immediately.
* @return The process id of the child that exited, 0 or -1.
* @throw ProcessException for wait errors.
*/      
long Process::waitForChild(long aId,bool poll) {
  pid_t apid = -1;
  int opts = 0; 

  if (poll) opts = WNOHANG;
  sighandler_t oldsig = signal(SIGCHLD,SIG_DFL); // Make sure we get the signal 
  if ((apid = waitpid(aId,NULL,opts)) <= 0) {
    if ((poll)&&(apid == 0)) return 0; // Polling and no child available
    int myerrno = errno;
    signal(SIGCHLD,oldsig); // Reset the signal handler
    if (myerrno == ECHILD) {
      throw process_general_exception.format(CSTR("Process::waitForChild() Child %d does not exist exist"),aId);
    } else {
      char buf[DAQHWYAPI_STRERROR_SIZE+1];
      throw process_general_exception.format(CSTR("Process::waitForChild() wait failure msg=\"%s\" rc=%d"),strerror_r(myerrno,buf,DAQHWYAPI_STRERROR_SIZE),myerrno);
    } 
  }

  signal(SIGCHLD,oldsig); // Reset the signal handler
  return apid;
}
