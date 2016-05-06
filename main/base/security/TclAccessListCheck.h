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

// Class: CTclAccessListCheck                     //ANSI C++
//
// Like CAccessListCheck, but the strings are in Tcl lists.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved TclAccessListCheck.h
//

#ifndef __TCLACCESSLISTCHECK_H  //Required for current class
#define __TCLACCESSLISTCHECK_H

                               //Required for base classes
#ifndef __AUTHENTICATOR_H
#include "Authenticator.h"
#endif
                               
                               //Required for 1:1 association classes
#ifndef __TCLVARIABLE_H
#include <TCLVariable.h>
#endif

                               //Required for 1:1 association classes
#ifndef __TCLINTERPRETER_H
#include <TCLInterpreter.h>
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

class CAccessListCheck;
                                                               
class CTclAccessListCheck  : public CAuthenticator        
{                       
			
  CTCLVariable*    m_pAccessList; //1:1 association object data member
  CTCLInterpreter* m_pInterp;	//1:1 association object data member      

			
protected:

public:

   // Constructors and other cannonical operations:

	CTclAccessListCheck (Tcl_Interp* pInterp, 
		                 const std::string& rName);  

   ~ CTclAccessListCheck ( )  // Destructor 
     {
       delete m_pAccessList;
       delete m_pInterp;
     }  
private:
  CTclAccessListCheck (const CTclAccessListCheck& aCTclAccessListCheck);
  CTclAccessListCheck& operator= (const CTclAccessListCheck& aCTclAccessListCheck);
  int operator== (const CTclAccessListCheck& aCTclAccessListCheck) const;
public:	
// Selectors:

public:
                       
  CTCLVariable* getVariable() const
  { 
    return m_pAccessList;
  }
  CTCLInterpreter* getInterpreter() const
  { 
    return m_pInterp;
  }
                       
// Mutators:

protected:
       
  void setVariable (CTCLVariable* am_AccessList)
  { 
    m_pAccessList = am_AccessList;
  }
  void setInterpreter (CTCLInterpreter* am_pInterp)
  { 
    m_pInterp = am_pInterp;
  }

public:

  virtual   bool Authenticate (CInteractor& rInteractor);

protected:
  CAccessListCheck* GenerateList();
};

#endif
