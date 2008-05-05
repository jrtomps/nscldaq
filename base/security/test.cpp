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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";//
// test program for the security members.
//
#include <config.h>

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <FdInteractor.h>
#include <IOInteractor.h>
#include <StringInteractor.h>
#include <PasswordCheck.h>
#include <UnixUserCheck.h>
#include <AccessListCheck.h>
#include <HostListCheck.h>


using namespace std;


struct in_addr NameToIn(const string& rName)
{
  struct in_addr retval;
  struct hostent* pEntry = gethostbyname(rName.c_str());
  if(!pEntry) {
    retval.s_addr = 0;
    cerr << "NO SUCH HOST " << rName << endl;
    return retval;
  }
  retval.s_addr = ((struct in_addr*)(pEntry->h_addr))->s_addr;
  return retval;
}

int
main(int argc, char** argv)
{
  cerr << "Constructing FdInteractor on stdin: "; 
  CFdInteractor Stdin(STDIN_FILENO);
  cerr << endl;

  cerr << "Constructing FdInteractor on stdout: ";
  CFdInteractor Stdout(STDOUT_FILENO);
  cerr << endl;

  cerr << "Binding Stdin, Stdout into an IOInteractor: ";
  CIOInteractor InOut(Stdin, Stdout);
  cerr << endl;

  cerr << "Building string interactor containing: Junk";
  CStringInteractor Junk("Junk");

  cerr << "Building password checker for string: Password\n";
  CPasswordCheck Password(string("Password"), string("Password: "),
			  kfTRUE);
  cerr << "Checking against Junk string... should fail\n";
  if(Password.Authenticate(Junk)) {
    cerr << " >> Authenticated when shouldn't\n";
  }
  else {
    cerr << " Failed - as it should\n";
  }
  cerr << "Checking against user input: \n";
  while(!Password.Authenticate(InOut)) {
    cerr << "Failed!\n";
  }
  cerr << "Passed!\n";
 
  cerr << "Checking for valid unix username: \n";
  CUnixUserCheck UnixUser(string("Username: "), string("Password: "),
			  kfTRUE, kfTRUE);
  while(!UnixUser.Authenticate(InOut)) {
    cerr << "Failed\n";
  }
  cerr << "Passed\n";

  cerr << "Constructing an access control list: \n";
  CAccessListCheck Acl;
  cerr << "Entries for the list (finish with a .)\n";
  string entry;
  do {
    cin >> entry;
    if(entry != string("."))
      Acl.AddAclEntry(entry);
  } while (entry != string("."));
  cerr << "Also stocking with 'A test entry'\n";
  Acl.AddAclEntry(string("A test entry"));
  cerr << "Now try entries exits when passed\n";
  while(!Acl.Authenticate(Stdin)) {
    cerr << "Failed\n";
  }
  cerr << "Passed\n";

  // Create a host list authenticator and stock it with hosts:

  CHostListCheck hosts;
  cerr << "Enter host names end with . by itself\n";
  do {
    cin >> entry;
    if(entry != string(".")) {
      struct in_addr host = NameToIn(entry);
      hosts.AddIpAddress(host);
    }
  } while (entry != string("."));
  cerr << "Enter host names... loop exits when one authenticates\n";
  do {
    cin >> entry;
    if(!hosts.Authenticate(entry)) cerr<< "Failed " << endl;
  } while (!hosts.Authenticate(entry));
  cerr << "Passed";

}

void* gpTCLApplication;
