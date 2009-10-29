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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CHostListCheck
// Encapsulates host list authentication.
// {Add,Delete}AclEntry allow named hosts to be added.
//  gethostbyname is used to resolve these to IP adresses which,
//  represented in a.b.c.d form are entered in the map.
//  {Add,Delete}IpAddress are used to add an IP address.
//  The address is represented textually in a.b.c.d form and
//   entered in the map.
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
#include "HostListCheck.h" 
#ifdef WIN32
#include <winsock.h>
#else   				
#include <netdb.h>
#endif

#include <stdio.h>

using namespace std;


static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved HostListCheck.cpp \n";


/////////////////////////////////////////////////////////////////////////////
//
// Function:
//    Bool_t Authenticate(const string& rHostname)
// Operation type:
//  overload
//
Bool_t
CHostListCheck::Authenticate(const std::string& rHostname)
{
  in_addr Address;
  struct hostent* pEntry;
  struct hostent  entry;
  char            buffer[1024];
  int             error;
  
  int status = gethostbyname_r(rHostname.c_str(),
			       &entry, buffer, sizeof(buffer),
			       &pEntry, &error);
  if (status) return kfFALSE;
  Address.s_addr = ((struct in_addr*)(entry.h_addr))->s_addr;

  return Authenticate(Address);
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     AddIpAddress(in_addr Address)
//  Operation Type: 
//     
void CHostListCheck::AddIpAddress(in_addr Address)  
{
  // Translates the ip address into dotted form
  // and adds it to the map.

  CAccessListCheck::AddAclEntry(EncodeAddress(Address));
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     DeleteIpAddress(in_addr address)
//  Operation Type: 
//     
void CHostListCheck::DeleteIpAddress(in_addr address)  
{
  // Translates the ip address into a dotted string
  // and if present, removes it from the map.
  
  CAccessListCheck::DeleteAclEntry(EncodeAddress(address));
}
std::string CHostListCheck::EncodeAddress(in_addr Address)
{
  char iptext[100];
  sprintf(iptext,"%08x", Address.s_addr);
  return std::string(iptext);
}
