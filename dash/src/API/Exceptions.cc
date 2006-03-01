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
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

using namespace daqhwyapi;

/*===================================================================*/
/*===================================================================*/
/** @fn Exception::Exception()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
Exception::Exception() {
  strncpy(msg,"Unspecified exception",DAQHWYAPI_EXCEPTION_MSGSIZE);
  strncpy(iam,"Exception",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/** @fn Exception::Exception(const Exception& e)
* @brief Copy constructor.
*                                        
* Copy constructor.
*                                         
* @param None
* @return this
*/      
Exception::Exception(const Exception& e) {
  strncpy(msg,e.msg,DAQHWYAPI_EXCEPTION_MSGSIZE);
  msg[DAQHWYAPI_EXCEPTION_MSGSIZE] = '\0'; 
}

/*===================================================================*/
/** @fn Exception& Exception::operator=(const Exception& e)
* @brief Assignment.
*                                        
* Assignment
*                                         
* @param e The other Exception.
* @return A reference to this.
*/      
Exception& Exception::operator=(const Exception& e) {
  strncpy(msg,e.msg,DAQHWYAPI_EXCEPTION_MSGSIZE);
  msg[DAQHWYAPI_EXCEPTION_MSGSIZE] = '\0'; 
  return(*this);
}

/*===================================================================*/
/** @fn const char* Exception::what()
* @brief Return a descriptive message.
*                                        
* Return a descriptive message.
*                                         
* @param None
* @return A descriptive message.
*/      
const char* Exception::what() const throw () {
  return(msg);
}

/*===================================================================*/
/** @fn Exception& Exception::format(const char *fmat,...)
* @brief Formated an exception message.
*
* Format an exception message.
*
* @param fmat The format string.
* @return A reference to this.
*/
Exception& Exception::format(const char *fmat,...)
{
  strcpy(msg,iam);
  strcat(msg,": ");
  int hsiz = strlen(msg);
  va_list alst;      
  va_start(alst,fmat);
  vsnprintf(msg+hsiz,DAQHWYAPI_EXCEPTION_MSGSIZE-hsiz,fmat,alst);
  va_end(alst);                                                        
  return(*this);
}

/*===================================================================*/
/*===================================================================*/
/** @fn BadMemoryAllocationException::BadMemoryAllocationException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
BadMemoryAllocationException::BadMemoryAllocationException() {
  strncpy(iam,"BadMemoryAllocation",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn IndexOutOfBoundsException::IndexOutOfBoundsException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
IndexOutOfBoundsException::IndexOutOfBoundsException() {
  strncpy(iam,"IndexOutOfBoundsException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn NoSuchElementException::NoSuchElementException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
NoSuchElementException::NoSuchElementException() {
  strncpy(iam,"NoSuchElementException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn CannotCreateNewInstanceException::CannotCreateNewInstanceException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
CannotCreateNewInstanceException::CannotCreateNewInstanceException() {
  strncpy(iam,"CannotCreateNewInstanceException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn NotFoundException::NotFoundException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
NotFoundException::NotFoundException() {
  strncpy(iam,"NotFoundException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn UninitializedException::UninitializedException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
UninitializedException::UninitializedException() {
  strncpy(iam,"UninitializedException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn CannotStartException::CannotStartException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
CannotStartException::CannotStartException() {
  strncpy(iam,"CannotStartException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn IOException::IOException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
IOException::IOException() {
  strncpy(iam,"IOException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn NumberFormatException::NumberFormatException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
NumberFormatException::NumberFormatException() {
  strncpy(iam,"NumberFormatException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn EmptyStackException::EmptyStackException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
EmptyStackException::EmptyStackException() {
  strncpy(iam,"EmptyStackException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn IllegalMonitorStateException::IllegalMonitorStateException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
IllegalMonitorStateException::IllegalMonitorStateException() {
  strncpy(iam,"IllegalMonitorStateException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn FullCollectionException::FullCollectionException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
FullCollectionException::FullCollectionException() {
  strncpy(iam,"FullCollectionException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn EmptyCollectionException::EmptyCollectionException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
EmptyCollectionException::EmptyCollectionException() {
  strncpy(iam,"EmptyCollectionException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn SocketException::SocketException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
SocketException::SocketException() {
  strncpy(iam,"SocketException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn UnknownHostException::UnkownHostException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
UnknownHostException::UnknownHostException() {
  strncpy(iam,"UnknownHostException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn BadCastException::BadCastException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
BadCastException::BadCastException() {
  strncpy(iam,"BadCastException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn StringFormatException::StringFormatException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
StringFormatException::StringFormatException() {
  strncpy(iam,"StringFormatException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn ProcessException::ProcessException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
ProcessException::ProcessException() {
  strncpy(iam,"ProcessException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

/*===================================================================*/
/*===================================================================*/
/** @fn RuntimeException::RuntimeException()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
RuntimeException::RuntimeException() {
  strncpy(iam,"RuntimeException",DAQHWYAPI_EXCEPTION_IAMSIZE);
}

