/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CSBSVMEEXCEPTION_H
#define __CSBSVMEEXCEPTION_H


#ifndef __EXCEPTION_H
#include <Exception.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


// Required for sbs bit3:


#ifndef BT1003
#define BT1003
#endif

#ifndef __SBS_BTAPI_H
extern "C" {
#include <btapi.h>
}
#ifndef __SBS_BTAPI_H
#define __SBS_BTAPI_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

/*!
    This is an exception that can be constructed and thrown
    to report unrecoverable errors that were detected in the return
    status of functions in the SBS/Bit3 API.
    The ReasonText will be constructed from the textual equivalent
    of the status value returned by the function that failed.
    The ReasonCode will be the bt_error_t returned by that routine.
*/

class CSBSVmeException : public CException
{
private:
  bt_error_t            m_errorCode;	// Error reason.
  mutable STD(string)   m_errorString;  // Needed to avoid scoping issues.
public:
  // Canonicals

  CSBSVmeException(bt_error_t error,
		   STD(string) wasDoing);
  CSBSVmeException(const CSBSVmeException& rhs);
  virtual ~CSBSVmeException();

  CSBSVmeException& operator=(const CSBSVmeException& rhs);
  int operator==(const CSBSVmeException& rhs);
  int operator!=(const CSBSVmeException& rhs);

  // 'the good stuff'.

  virtual const char* ReasonText() const;
  virtual int         ReasonCode() const;

};

#endif
