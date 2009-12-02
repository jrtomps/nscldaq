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



// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
//   NSCL All rights reserved.
//
#ifndef __CTCPNOSUCHSERVICE_H
#define __CTCPNOSUCHSERVICE_H

// Headers

#ifndef __ERRNOEXCEPTION_H
#include "ErrnoException.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

/*! 
  Encapsulates an excpetion which indicates a failure to translate a service
  specification string into a service port number.  Service strings can be
  either a service name which is looked up in e.g. /etc/services or a 
  stringified service port number.

 */
class CTCPNoSuchService : public CErrnoException
{
  // Member data [private]
private:
  std::string         m_Service;	//!< cached value of service.
  mutable std::string m_Reason;	//!< Where the reason text is built up.

  // Constructors and related functions:
public:
  CTCPNoSuchService(const std::string& service, const std::string& Doing); 
  CTCPNoSuchService(const CTCPNoSuchService& rhs); //!< Copy Constructor.
  virtual ~CTCPNoSuchService() {}                  //!< Destructor.

  CTCPNoSuchService& operator=(const CTCPNoSuchService& rhs);
  int                operator==(const CTCPNoSuchService& rhs);

  // Selectors:

public:
  std::string getService() const	//!< Return service name.
  { return m_Service; }

  // Mutators:

protected:
  void setService(const std::string& newVal) 
  { m_Service = newVal; }

  // Operations on the class:
public:
  virtual const char* ReasonText() const;
};

#endif
