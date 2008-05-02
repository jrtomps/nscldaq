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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
** Implementation of Tcl Server connection.
** 
*/
#include <config.h>
#include <tk.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "server.h"
#include "serverinstance.h"
#include "TclAuthorizer.h"
#include <string>
#ifdef WIN32
#include <winsock.h>
#endif

using namespace std;

CTclAuthorizer* pAuthenticator = 0;

static void
Server_Accept(ClientData cd, Tcl_Channel client, char* pHostname,
		   int nHostport)
{
  Tcl_Interp* pInterp     = (Tcl_Interp*)cd;

  // First be sure the client has any business connecting.

  if(!pAuthenticator) {		// This must be a breakin if no authenticator.
    Tcl_Close(pInterp, client);
    printf(">> No authenticator to check connection from %s on port %d\n", 
	   pHostname, nHostport);
    return;
  }
  if(!pAuthenticator->Authenticate(pHostname)) {
    printf(">> Rejected unauthorized connection from %s on port %d\n",
	   pHostname, nHostport);
    Tcl_Close(pInterp, client);
    return;
  }

  // Honor the connection

  ServerContext context;
  pServerContext pContext = &context;


  printf("Accepting connection from %s on port %d\n", pHostname, nHostport);
  pContext->pInterp = pInterp;
  Tcl_DStringInit(&(pContext->RemoteHost));
  Tcl_DStringAppend(&(pContext->RemoteHost), pHostname, -1);
  pContext->RemotePort = nHostport;
  pContext->DialogChannel = client;
  Tcl_DStringInit(&(pContext->command));

  new CServerInstance(context);
  
  Tcl_DStringFree(&(context.RemoteHost));
  Tcl_DStringFree(&(context.command));

}

void Server_Init(Tcl_Interp* pInterp, int SERVERPORT) 
{
  // Initialize the tcl/tcp server component:


  // If necessary create the authenticator and stock it.

  if(!pAuthenticator) {
    pAuthenticator = new CTclAuthorizer(pInterp);
    pAuthenticator->AddHost(std::string("localhost"));
  }

  // Open the server for business.

  Tcl_OpenTcpServer(pInterp, SERVERPORT, NULL, 
		    Server_Accept, (ClientData)pInterp);

}





