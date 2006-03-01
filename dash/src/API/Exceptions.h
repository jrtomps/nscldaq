#ifndef DAQHWYAPI_EXCEPTIONS_H
#define DAQHWYAPI_EXCEPTIONS_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

namespace daqhwyapi {

#define DAQHWYAPI_EXCEPTION_MSGSIZE 1024
#define DAQHWYAPI_EXCEPTION_IAMSIZE 40
#define DAQHWYAPI_STRERROR_SIZE 256

/**
* @class Exception
* @brief Basic Exception.
*
* Basic Exception.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Exception : virtual public std::exception {
  public:
    Exception();
    Exception(const Exception&);  // Copy constructor
    Exception& operator=(const Exception&);  // Assignment
    virtual const char* what() const throw ();
    virtual Exception& format(const char *fmat,...);

  protected:
    char msg[DAQHWYAPI_EXCEPTION_MSGSIZE+1];
    char iam[DAQHWYAPI_EXCEPTION_IAMSIZE+1];
};

/**
* @class BadMemoryAllocationException
* @brief Exception for bad memory allocations.
*
* Exception for bad memory allocations.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BadMemoryAllocationException : public Exception {
  public:
    BadMemoryAllocationException();
};

/**
* @class IndexOutOfBoundsException
* @brief Exception for indexing problems.
*
* Exception for indexing problems.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class IndexOutOfBoundsException: public Exception {
  public:
    IndexOutOfBoundsException();
};

/**
* @class NoSuchElementException
* @brief Exception for bad memory allocations.
*
* Exception for bad memory allocations.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class NoSuchElementException : public Exception {
  public:
    NoSuchElementException();
};

/**
* @class CannotCreateNewInstanceException
* @brief Exception for new instance failures.
*
* Exception for when a new class instance cannot be completed.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class CannotCreateNewInstanceException : public Exception {
  public:
    CannotCreateNewInstanceException();
};

/**
* @class NotFoundException
* @brief Exception for when an object cannot be located.
*
* Exception for when an object cannot be located. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class NotFoundException : public Exception {
  public:
    NotFoundException();
};

/**
* @class UninitializedException
* @brief Exception for something is not initialized.
*
* Exception for when something is not inititalized.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class UninitializedException : public Exception {
  public:
    UninitializedException();
};

/**
* @class CannotStartException
* @brief Exception for runnables that cannot start.
*
* Exception for runnables that cannot start.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class CannotStartException : public Exception {
  public:
    CannotStartException();
};

/**
* @class IOException
* @brief Exception for IO errors.
*
* Exception for IO errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class IOException : public Exception {
  public:
    IOException();
};

/**
* @class NumberFormatException
* @brief Exception for number conversion errors.
*
* Exception for number conversion errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class NumberFormatException : public Exception {
  public:
    NumberFormatException();
};

/**
* @class EmptyStackException
* @brief Exception for empty stack errors.
*
* Exception for empty stack errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class EmptyStackException : public Exception {
  public:
    EmptyStackException();
};

/**
* @class IllegalMonitorStateException
* @brief Exception for notify()/notifyAll() errors.
*
* Exception for notify()/notifyAll() errors and other
* thread monitor problems.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class IllegalMonitorStateException : public Exception {
  public:
    IllegalMonitorStateException();
};

/**
* @class FullCollectionException
* @brief Exception for full collection errors.
*
* Exception for full collection errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FullCollectionException : public Exception {
  public:
    FullCollectionException();
};

/**
* @class EmptyCollectionException
* @brief Exception for empty collection errors.
*
* Exception for empty collection errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class EmptyCollectionException : public Exception {
  public:
    EmptyCollectionException();
};

/**
* @class SocketException
* @brief Exception for socket related errors.
*
* Exception for socket related errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class SocketException : public Exception {
  public:
    SocketException();
};

/**
* @class UnknownHostException
* @brief Exception for host or address resolution problems.
*
* Exception for host or address resolution problems.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class UnknownHostException : public Exception {
  public:
    UnknownHostException();
};

/**
* @class BadCastException
* @brief Exception for bad dynamic casts.
*
* Exception for bad dynamic casts.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class BadCastException : public Exception {
  public:
    BadCastException();
};

/**
* @class StringFormatException
* @brief Exception for String parsing errors.
*
* Exception for general String parsing errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class StringFormatException : public Exception {
  public:
    StringFormatException();
};

/**
* @class ProcessException
* @brief Exception for errors related process handling.
*
* Exception for errors related to Unix process handling.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class ProcessException : public Exception {
  public:
    ProcessException();
};

/**
* @class RuntimeException
* @brief Exception for general runtime errors.
*
* Exception for general runtime errors.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RuntimeException : public Exception {
  public:
    RuntimeException();
};

} // namespace daqhwyapi

#endif
