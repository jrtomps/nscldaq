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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CUnixUserCheck
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
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "UnixUserCheck.h"    				
#include <pwd.h>
#include <crypt.h>
#ifdef Linux
#define _XOPEN_SOURCE
#include <unistd.h>
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

static char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved UnixUserCheck.cpp \n";

// Functions for class CUnixUserCheck

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(CInteractor& rInteractor)
//  Operation Type: 
//     
Bool_t 
CUnixUserCheck::Authenticate(CInteractor& rInteractor)  
{
  // Returns true if the username and password
  // gotten from the interactor match a pwdentry
  // for the current host.
  if(m_fPromptUser) {
    PutPrompt(rInteractor, m_sUserPrompt);
  }
  string sUser = GetLine(rInteractor);

  if(m_fPromptPassword) {
    PutPrompt(rInteractor, m_sPasswordPrompt);
  }
  string sPassword = GetLine(rInteractor);

  return Validate(sUser, sPassword);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SetPrompting(Bool_t fUserPrompt, Bool_t fPasswordPrompt)
//  Operation Type: 
//     
void 
CUnixUserCheck::SetPrompting(Bool_t fUserPrompt, Bool_t fPasswordPrompt)  
{
  // Enable/Disable prompting.

  m_fPromptUser     = fUserPrompt;
  m_fPromptPassword = fPasswordPrompt;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SetUserPrompt(const string& rNewPrompt)
//  Operation Type: 
//     
void 
CUnixUserCheck::SetUserPrompt(const string& rNewPrompt)  
{
  // Updates the username prompt.
  m_sUserPrompt = rNewPrompt;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SetPasswordPrompt(const string& rNewPrompt)
//  Operation Type: 
//     
void 
CUnixUserCheck::SetPasswordPrompt(const string& rNewPrompt)  
{
  // Set a new value for the password prompt.
  m_sPasswordPrompt = rNewPrompt;
}
/////////////////////////////////////////////////////////////////////////
//
// Function:
//    void   PutPrompt(CInteractor& rInteractor, const string& rPrompt)
// Operation Type:
//    Utility function.
//
void
CUnixUserCheck::PutPrompt(CInteractor& rInteractor, const string& rPrompt)

{
  rInteractor.Write(rPrompt.length(), (void*)rPrompt.c_str());
}
/////////////////////////////////////////////////////////////////////////
//
// Function:
//    Bool_t Validate(const string& sUser, const string& sPassword)
// Operation Type:
//    Utility:
//
Bool_t
CUnixUserCheck::Validate(const string& sUser, const string& sPassword)
{
  struct passwd* pEntry = getpwnam(sUser.c_str());
  if(!pEntry) return kfFALSE;	// No such user.
  string EncryptedPassword(pEntry->pw_passwd);
  string EncryptedEntry(crypt(sPassword.c_str(), EncryptedPassword.c_str()));

  return EncryptedPassword == EncryptedEntry;
}
