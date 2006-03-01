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
#include <math.h>
#include <errno.h>
#include <limits.h>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_PIPECONNECTOR_H
#include <dshapi/PipeConnector.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

namespace daqhwyapi {
/**
* @var pipeconnector_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException pipeconnector_ioexception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn PipeConnector::PipeConnector()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
PipeConnector::PipeConnector() {
  pipefd[0] = -1;
  pipefd[1] = -1;
}

/*==============================================================*/
/** @fn PipeConnector::~PipeConnector()
* @brief Destructor.
*
* Destroy this PipeConnector.
*
* @param None
* @return None
*/                                                             
PipeConnector::~PipeConnector() {
  close();
}

/*==============================================================*/
/** @fn bool PipeConnector::connect(FdInputStream& aStream)
* @brief Connect an input stream to this connector.
*
* Connect a Fd input stream to this connector.
*
* @param aStream The input stream to connect.
* @return If the connection was successful.
* @throw IOException If the pipe is not open or a dup(2) failure.
*/                                                             
bool PipeConnector::connect(FdInputStream& aStream) {
  if (pipefd[0] < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connect() Pipe is not open"));
  int fd = aStream.getFD();
  int newfd = -1;
  if (fd < 0) newfd = ::dup(pipefd[0]);
  else newfd = ::dup2(pipefd[0],fd);

  if (newfd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw pipeconnector_ioexception.format(CSTR("PipeConnector::connect() dup(2) error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  } else {
    aStream.open(newfd);   
    return true;
  }

  return false;
}

/*==============================================================*/
/** @fn bool PipeConnector::connectInput(int fd)
* @brief Connect a file descriptor to this connector.
*
* Connect a file descriptor to the input side of this connector.
* This method uses dup2(2) to close and duplicate the read-side
* pipe(2) on to the provided file descriptor.
*
* @param fd The file descriptor to connect.
* @return If the connection was successful.
* @throw IOException If the pipe is not open, dup(2) failure or fd < 0.
*/                                                             
bool PipeConnector::connectInput(int fd) {
  if (pipefd[0] < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectInput() Pipe is not open"));
  if (fd < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectInput() Bad file descriptor"));

  int newfd = ::dup2(pipefd[0],fd);
  if (newfd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectInput() dup2(2) error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  } 

  return true;
}

/*==============================================================*/
/** @fn bool PipeConnector::connect(FdOutputStream& aStream)
* @brief Connect an output stream to this connector.
*
* Connect a Fd output stream to this connector.
*
* @param aStream The output stream to connect.
* @return If the connection was successful.
* @throw IOException If the pipe is not open or a dup(2) failure.
*/                                                             
bool PipeConnector::connect(FdOutputStream& aStream) {
  if (pipefd[1] < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connect() Pipe is not open"));
  int fd = aStream.getFD();
  int newfd = -1;
  if (fd < 0) newfd = ::dup(pipefd[1]);
  else newfd = ::dup2(pipefd[1],fd);

  if (newfd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw pipeconnector_ioexception.format(CSTR("PipeConnector::connect() dup(2) error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  } else {
    aStream.open(newfd);   
    return true;
  }
  
  return false;
}

/*==============================================================*/
/** @fn bool PipeConnector::connectOutput(int fd)
* @brief Connect a file descriptor to this connector.
*
* Connect a file descriptor to the output side of this connector.
* This method uses dup2(2) to close and duplicate the write-side
* pipe(2) on to the provided file descriptor.
*
* @param fd The file descriptor to connect.
* @return If the connection was successful.
* @throw IOException If the pipe is not open, dup(2) failure or fd < 0.
*/                                                             
bool PipeConnector::connectOutput(int fd) {
  if (pipefd[1] < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectOutput() Pipe is not open"));
  if (fd < 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectOutput() Bad file descriptor"));

  int newfd = ::dup2(pipefd[1],fd);
  if (newfd < 0) {
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw pipeconnector_ioexception.format(CSTR("PipeConnector::connectOutput() dup2(2) error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  } 

  return true;
}

/*==============================================================*/
/** @fn void PipeConnector::open()
* @brief Ready this connector.
*
* Ready this connector to receive connections.
*
* @param None
* @return None
* @throw IOException on pipe creation errors or if the pipe is already open.
*/                                                             
void PipeConnector::open() {
  if (pipefd[0] >= 0) throw pipeconnector_ioexception.format(CSTR("PipeConnector::open() Pipe already open"));
  else make_pipe(pipefd);
}

/*==============================================================*/
/** @fn void PipeConnector::close()
* @brief Close this connector.
*
* Close this connector.
*
* @param None
* @return None
*/                                                             
void PipeConnector::close() {
  if (pipefd[0] >= 0) ::close(pipefd[0]);
  pipefd[0] = -1;
  if (pipefd[1] >= 0) ::close(pipefd[1]);
  pipefd[1] = -1;
}

/*==============================================================*/
/** @fn void PipeConnector::make_pipe(int fd[])
* @brief Make a pipe.
*
* Make a Unix pipe for this pipe connector.
*
* @param fd The pipe fd pair.  Must be an int[2].
* @return None
* @throw IOException if the pipe could not be created.
*/                                                             
void PipeConnector::make_pipe(int fd[]) {
  int rc = ::pipe(fd);
  if (rc < 0) {
    fd[0] = -1; fd[1] = -1;
    char buf[DAQHWYAPI_STRERROR_SIZE+1];
    throw pipeconnector_ioexception.format(CSTR("PipeConnector::make_pipe() pipe creation error: msg=\"%s\" rc=%d"),strerror_r(errno,buf,DAQHWYAPI_STRERROR_SIZE),errno);
  }
}

/*==============================================================*/
/** @fn int PipeConnector::getInputFD()
* @brief Get the fd used for reading.
*
* Get the file descriptor that is used for reading.
* The return value may be -1 if the file descriptor is not
* open.
*
* @param None
* @return The read-side file descriptor.
*/                                                             
int PipeConnector::getInputFD() {
  return pipefd[0];
}

/*==============================================================*/
/** @fn int PipeConnector::getOutputFD()
* @brief Get the fd used for writing.
*
* Get the file descriptor that is used for writing.
* The return value may be -1 if the file descriptor is not
* open.
*
* @param None
* @return The write-side file descriptor.
*/                                                             
int PipeConnector::getOutputFD() {
  return pipefd[1];
}

