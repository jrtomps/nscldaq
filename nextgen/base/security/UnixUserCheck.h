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

// Class: CUnixUserCheck                     //ANSI C++
//
// Authenticates a unix username and password
// for the host system.  Uses the interactor to 
// obtain two records of information:
//   Username,
//   Password
// Username is used to do a getpwentry,
// Password is encrytped and compared to
// the password in the pwentry returned.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved UnixUserCheck.h
//

#ifndef __UNIXUSERCHECK_H  //Required for current class
#define __UNIXUSERCHECK_H

                               //Required for base classes
#ifndef __AUTHENTICATOR_H
#include "Authenticator.h"
#endif
                               

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CUnixUserCheck  : public CAuthenticator        
{                       
			
   std::string m_sUserPrompt;	//Username Prompt string
   std::string m_sPasswordPrompt;	//Password Prompt string.
   Bool_t m_fPromptUser;	//If true usename is prompted for.
   Bool_t m_fPromptPassword;	//If false, password is prompted for.        

protected:

public:

   // Constructors and other cannonical operations:

  CUnixUserCheck ()    : 
    CAuthenticator(),
    m_sUserPrompt(std::string("Username: ")),   
    m_sPasswordPrompt(std::string("Password: ")),   
    m_fPromptUser(kfTRUE),   
    m_fPromptPassword(kfTRUE) 
  { 
    
  } 
   ~ CUnixUserCheck ( )  // Destructor 
     { }  
  CUnixUserCheck (const std::string& am_sUserPrompt,  
		  const std::string& am_sPasswordPrompt,  
		  Bool_t am_fPromptUser,  Bool_t am_fPromptPassword  ) 
      : CAuthenticator()   ,
	m_sUserPrompt(am_sUserPrompt),
	m_sPasswordPrompt(am_sPasswordPrompt),
	m_fPromptUser(am_fPromptUser),
	m_fPromptPassword(am_fPromptPassword)
  { 
  }    

  
   //Copy constructor 

  CUnixUserCheck (const CUnixUserCheck& aCUnixUserCheck )   : 
    CAuthenticator (aCUnixUserCheck) 
  { 
   
    m_sUserPrompt = aCUnixUserCheck.m_sUserPrompt;
    m_sPasswordPrompt = aCUnixUserCheck.m_sPasswordPrompt;
    m_fPromptUser = aCUnixUserCheck.m_fPromptUser;
    m_fPromptPassword = aCUnixUserCheck.m_fPromptPassword;
     
  }                                     

   // Operator= Assignment Operator 

  CUnixUserCheck& operator= (const CUnixUserCheck& aCUnixUserCheck) {
    if(this != &aCUnixUserCheck) {
      CAuthenticator::operator=(aCUnixUserCheck);
      m_sUserPrompt = aCUnixUserCheck.m_sUserPrompt;
      m_sPasswordPrompt = aCUnixUserCheck.m_sPasswordPrompt;
      m_fPromptUser = aCUnixUserCheck.m_fPromptUser;
      m_fPromptPassword = aCUnixUserCheck.m_fPromptPassword;
    }
    return *this;
  }
 
   //Operator== Equality Operator 
private:
  int operator== (const CUnixUserCheck& aCUnixUserCheck) const;
public:

// Selectors:

public:

  std::string getUserPrompt() const
  { return m_sUserPrompt;
  }
  std::string getPasswordPrompt() const
  { return m_sPasswordPrompt;
  }
  Bool_t getPromptUser() const
  { return m_fPromptUser;
  }
  Bool_t getPromptPassword() const
  { return m_fPromptPassword;
  }
                       
// Mutators:

protected:

  void setUserPrompt (const std::string& am_sUserPrompt)
  { m_sUserPrompt = am_sUserPrompt;
  }
  void setPasswordPrompt (const std::string& am_sPasswordPrompt)
  { m_sPasswordPrompt = am_sPasswordPrompt;
  }
  void setPromptUser (Bool_t am_fPromptUser)
  { m_fPromptUser = am_fPromptUser;
  }
  void setPromptPassword (Bool_t am_fPromptPassword)
  { m_fPromptPassword = am_fPromptPassword;
  }
       
public:

  virtual   Bool_t Authenticate (CInteractor& rInteractor)    ;
  void SetPrompting (Bool_t fUserPrompt=kfTRUE, 
		     Bool_t fPasswordPrompt=kfTRUE)    ;
  void SetUserPrompt (const std::string& rNewPrompt=std::string("Username: "))    ;
  void SetPasswordPrompt (const std::string& rNewPrompt=std::string("Password: "))    ;
 
protected:
  void   PutPrompt(CInteractor& rInteractor, const std::string& rPrompt);
  Bool_t Validate(const std::string& sUser, const std::string& sPassword);

};

#endif
