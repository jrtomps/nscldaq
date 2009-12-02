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

#ifndef __CTCPCONNECTIONFAILED_H
#define __CTCPCONNECTIONFAILED_H

#ifndef __ERRNOEXCEPTION_H
#include <ErrnoException.h>
#endif


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

/*!
   Encapsulates a connection failure exception. Since connect(2) reports
   failure reasons through errno, this classs derives from CErrnoException.
   */
class CTCPConnectionFailed : public CErrnoException
{
  // Private member data:

  std::string m_Host;		//!< Attempted peername.
  std::string m_Service;		//!< Attempted connection point port. 
  mutable std::string m_ReasonText;	//!< Reason text is built up here.

  // Constructors and related functions.

public:
  CTCPConnectionFailed(const std::string& host,
		       const std::string& service,
		       const char*   pDoing);
  CTCPConnectionFailed(const CTCPConnectionFailed& rhs);
  ~CTCPConnectionFailed() {}	//!< Destructor.

  CTCPConnectionFailed& operator=(const CTCPConnectionFailed& rhs);
  int                   operator==(const CTCPConnectionFailed& rhs);

  // Selectors:

public:
  std::string getHost() const
  { return m_Host; }
  std::string getService() const
  { return m_Service; }
 
  // Mutators:

protected:
  void setHost(const std::string& rHost) 
  { m_Host = rHost; }
  void setService(const std::string& rService)
  { m_Service = rService; }

  // Class functions:

public:
  virtual const char* ReasonText() const;
  
};
#endif
