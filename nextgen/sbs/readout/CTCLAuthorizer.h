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


#ifndef __CTCLAUTHORIZER_H  //Required for current class
#define __CTCLAUTHORIZER_H

#ifndef __DAQTYPES_H
#include <daqdatatypes.h>
#endif

#ifndef __STL_STRING
#include <string>        //Required for include files
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __TCLINTERPRETER_H
#include <TCLInterpreter.h>        //Required for include files
#define __TCLINTERPRETER_H
#endif

#ifndef __TCLPROCESSOR_H
#include <TCLProcessor.h>
#endif


#ifndef __TCLVARIABLE_H
#include <TCLVariable.h>        //Required for include files
#endif

#ifndef __TCLLIST_H
#include <TCLList.h>        //Required for include files
#endif

#ifndef __TCLRESULT_H
#include <TCLResult.h>        //Required for include files
#endif
                               
class CTCLAuthorizer : public CTCLProcessor     
{                       
  CTCLInterpreter* m_pInterpreter;
  CTCLVariable* m_pHostNames; //List of allowed hostnames.
  CTCLVariable* m_pHostIps; //List of allowed host IPs.        

protected:

public:

   // Constructors and other cannonical operations:

  CTCLAuthorizer (CTCLInterpreter* pInterp);
  ~ CTCLAuthorizer ( )  // Destructor 
  {
    delete m_pHostNames;
    delete m_pHostIps;
  }  

  
   //Copy constructor 
private:
  CTCLAuthorizer (const CTCLAuthorizer& aCTCLAuthorizer ) ;
  CTCLAuthorizer& operator= (const CTCLAuthorizer& aCTCLAuthorizer);
  int operator== (const CTCLAuthorizer& aCTCLAuthorizer) const;
public:
	
// Selectors:

public:

  const CTCLVariable* getHostNames() const
  { 
    return m_pHostNames;
  }
  const CTCLVariable* getHostIps() const
  { 
    return m_pHostIps;
  }
                       
// Mutators:

protected:

  void setHostNames (CTCLVariable* am_pHostNames)
  { 
    m_pHostNames = am_pHostNames;
  }
  void setHostIps (CTCLVariable* am_pHostIps)
  { 
    m_pHostIps = am_pHostIps;
  }
       
public:

  virtual   int operator() (CTCLInterpreter& rInterp, CTCLResult& rResult, 
			    int nArgs, char* pArgs[])    ;
  Bool_t AddHost (const std::string& HostOrIp)    ;
  Bool_t RemoveHost (const std::string& NameOrIP)    ;
  std::string ListHosts ()    ;
  Bool_t Authenticate (const std::string& rNameOrIp)    ;

protected:
  int   Process(CTCLInterpreter& rInterp, 
		  int nArgs, char* pArgs[])    ;
  Bool_t  HostToIp(std::string& rName);
  Int_t   GetIndex (const std::string& rHostOrIp)   ;
  Bool_t ConvertHost(const std::string& rInName, 
	                 std::string& rOutname, std::string& rCanonicalIP)   ;

  Int_t   Usage(CTCLInterpreter& rInterp);


};

#endif
