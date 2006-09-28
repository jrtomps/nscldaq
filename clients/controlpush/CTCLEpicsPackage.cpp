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
#include "CTCLEpicsCommand.h"
#include "CTCLChannelCommand.h"
#include <TCLInterpreter.h>
#include <cadef.h>
#include <map>
#include <string>
#include <tcl.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

typedef map<string,CTCLChannelCommand*> CommandMap;

static char* version= "1.0";

static const int caPollInterval(500); // ms per epics poll.

static CommandMap  linkages;


static void pollEpics(ClientData ignored) {

  ca_pend_event(0.00000001);
  Tcl_CreateTimerHandler(caPollInterval, pollEpics, (ClientData)NULL);

  CommandMap::iterator i = linkages.begin();
  while (i != linkages.end()) {
    CTCLChannelCommand* pCommand = i->second;
    pCommand->UpdateLinkedVariable();
    i++;
  }

}

void addLinkage(CTCLChannelCommand* pCommand)
{
  linkages[pCommand->getName()] = pCommand;
}
void removeLinkage(CTCLChannelCommand* pCommand)
{
  CommandMap::iterator i = linkages.find(pCommand->getName());
  if (i != linkages.end()) {
    linkages.erase(i);
  }
}

extern "C" {
  int Epics_Init(Tcl_Interp* pInterp)
  {
    Tcl_PkgProvide(pInterp, "epics", version);

    CTCLInterpreter* pI = new CTCLInterpreter(pInterp);	
    new CTCLEpicsCommand(*pI);
    pollEpics((ClientData)NULL);


    return TCL_OK;
  }
  void* gpTCLApplication(0);

}

