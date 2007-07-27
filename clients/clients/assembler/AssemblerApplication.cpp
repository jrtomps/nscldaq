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

#include <config.h>
#include "AssemblerApplication.h"
#include "AssemblerCommand.h"
#include "AssemblerOutputStage.h"

#include <netdb.h>

#include <dshapi/daqhwyapi.h>
#include <dshnet/daqhwynet.h>

using namespace daqhwyapi;
using namespace daqhwynet;

#include <spdaq/spdaqlite.h>
using namespace spdaq;

#include <TCLInterpreter.h>

static const string serviceName("sdlite-buff");
static const int    defaultPort(2701);

/*
    Determine the source port.
    This is done by first attempting to translate the service name
    and, if that fails. falling back on the default port number.
*/
int
AssemblerApplication::portNumber()
{
  struct servent* pService = getservbyname(serviceName.c_str(),
					   NULL);
  if(pService) {
    return ntohs(pService->s_port);
  }
  else {
    return defaultPort;
  }
}

/*!
   This function is called during initialization.
   We need to create the extensions for our interpreter.
*/
int
AssemblerApplication::operator()()
{
  DAQDataStore& the_store = DAQDataStore::instance();
  the_store.setSourcePort(portNumber());

  CTCLInterpreter*      pInterp   = getInterpreter();
  AssemblerCommand*     pCommand  = new AssemblerCommand(*pInterp);
  AssemblerOutputStage* pOutput   = new AssemblerOutputStage(*pInterp);
  return TCL_OK;
}

AssemblerApplication myApp;
CTCLApplication*     gpTCLApplication(&myApp);

