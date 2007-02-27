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
#include <CChannel.h>

#include <TCLInterpreter.h>
#include <cadef.h>
#include <map>
#include <string>
#include <tcl.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static char* version= "1.1";

static const int caPollInterval(16); // ms per epics poll.


// Ensures that ca_pend_event is called often enough
// for epics to stay happy.
//

static void pollEpics(ClientData ignored) {

  CChannel::doEvents(0.01);
  Tcl_CreateTimerHandler(caPollInterval, pollEpics, (ClientData)NULL);

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

