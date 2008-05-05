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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CAccessListCheck
// Provides access control list authentication.
// Maintains an stl map of text strings.  The interactor
// supplies a string which is hunted for in the
// map. Matches authenticate.  No-match fails.
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
#include "AccessListCheck.h"    				


using namespace std;


static char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved AccessListCheck.cpp \n";

// Functions for class CAccessListCheck

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(CInteractor& rInteractor)
//  Operation Type: 
//     
Bool_t CAccessListCheck::Authenticate(CInteractor& rInteractor)  
{
  // Fetches a key string from the authenticator.
  // If the keys string is in the m_Acl map, TRUE
  // is returned.
  
  std::string key = GetLine(rInteractor);
  StringMap::iterator i = m_AccessList.find(key);
  return (i != m_AccessList.end());

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     AddAclEntry(const string& rEntry)
//  Operation Type: 
//     
void CAccessListCheck::AddAclEntry(const std::string& rEntry)  
{

  // Note... existing entries are replaced silently since this is harmless.

  m_AccessList.insert(rEntry);
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     DeleteAclEntry(const string& rEntry)
//  Operation Type: 
//     
void CAccessListCheck::DeleteAclEntry(const std::string& rEntry)  
{
  // Removes an entry from the ACL.  If there is no
  // matching entry this is a no-op.

  StringMap::iterator i = m_AccessList.find(rEntry);
  if(i != m_AccessList.end())
    m_AccessList.erase(i);
}
