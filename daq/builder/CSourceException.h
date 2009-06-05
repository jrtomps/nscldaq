#ifndef __CSOURCEEXCEPTION_H
#define __CSOURCEEXCEPTION_h
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

#ifndef __EXCEPTION_H
#include <Exception.h>
#endif


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif 



/*!
  Base class for all exceptions that are related to things that can go wrong with 
  the installation, configuration and operation of data sources.

*/
class CSourceException {
private:
  std::string   m_managerName;
  std::string   m_instanceName;


  /*!  Constructors and other canonicals   */

  CSourceException(const char* pDoing, std::string manager = string(""),
		   std::string instance = string(""));
  CSourceException(const CSourceException& rhs);
  CSourceException& operator=(const CSourceException& rhs);
  int operator==(const CSourceException& rhs);
  int operator!=(const CSourceException& rhs);

  // Selectors (getters to the java folks)

  std::string getManager() const;


};


#endif
