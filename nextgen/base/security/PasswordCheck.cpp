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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";
 
// Class: CPasswordCheck
// Performs password access control
// Note that the password is internally stored as
// clear text since it is assumed that external
// forces will be the ones attempting to break
// this loose security.
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
#include "PasswordCheck.h"

using namespace std;



static const  char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved PasswordCheck.cpp \n";

// Functions for class CPasswordCheck

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(CInteractor & rInteractor)
//  Operation Type: 
//     
Bool_t CPasswordCheck::Authenticate(CInteractor & rInteractor)  
{
  // Retrieves a password from the Authenticator
  // and compares it with the internal password.
  // Case is relevant.

  if(m_fWithPrompt) {
    rInteractor.Write(m_sPromptString.length(), 
		      (void*)m_sPromptString.c_str());
  }
  std::string Passkey = GetUserPassword(rInteractor);
  return Passkey == m_sPassword;
}

