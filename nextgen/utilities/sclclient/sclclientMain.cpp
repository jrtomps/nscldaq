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
#include "sclclientMain.h"
#include "sclclientargs.h"
#include "TclServerConnection.h"
#include "TcpClient.h"

#include <CRingBuffer.h>
#include <CRingScalerItem.h>
#include <CRingStateChangeItem.h>
#include <CRemoteAccess.h>
#include <CDesiredTypesPredicate.h>
#include <DataFormat.h>
#include <CPortManager.h>
#include <ErrnoException.h>
#include <CInvalidArgumentException.h>

#include <iostream>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals we bothered to implement:
//

/*!
  Construct the object.  This just initializes stuff to reasonable defaults.
  The real preparation is done in operator() since we need the command
  arguments to know what to do.
*/
SclClientMain::SclClientMain() :
  m_pRing(0),
  m_Port(-1),
  m_pServer(0)
{}

/*!
   Destruction just kills off the two dynamic elements:

*/
SclClientMain::~SclClientMain()
{
  delete m_pServer;
  delete m_pRing;
}
  
//////////////////////////////////////////////////////////////////////////////
//
// Public interface
//

int
SclClientMain::operator()(int argc, char** argv)
{
  // process the arguments:

  gengetopt_args_info parse;
  cmdline_parser(argc, argv, &parse);
  
  // First lets get the ring buffer URL:

  string url = defaultRing();
  if (parse.source_given) {
    url = parse.source_arg;
  }
  m_pRing = CRingAccess::daqConsumeFrom (url);

  // remote host initializes to "localhost"
  // port initializes to "managed"

  m_Host = "localhost";
  if (parse.host_given) {
    m_Host = parse.host_arg;
  }

  // The port defaults to managed but could be given. If given it's either
  // "managed", or an integer or an error.
  //

  string port  = "managed";	// sclclient defaults to managed ports.
  if (parse.port_given) {
    port  = parse.port_arg;
  }

  m_Port = getDisplayPort(port);

  // Now we have the host name and port and can connect:

  connectTclServer();

  // Process items from the ring.

  processItems();

  return EXIT_SUCCESS;		// Probably we get thrown right past this.
    
}
/////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//


/*
** Get the port over which we should connect to the Tcl Server.
**
** Parameters:
**   portArg   - The port argument.  This should be one of the following:
**                * The string "managed" which makes us look up
**                  the scaler display program via the Port Manager.
**                * An integer, which is integerized and returned without
**                  further attempts to do anything useful to it.
**                * Anything else.. which results in a throw of an
**                  invalid argument exception.
** Implicit:
**   m_Host is the host name to which we'll want to connect.
** Returns:
**    The port on which the program should connect to the scaler display.
**
*/
int
SclClientMain::getDisplayPort(string portArg)
{
  if (portArg == string("managed") ) {
    // Use port manager.

    string                          me(whoAmI());
    CPortManager                    manager(m_Host);
    vector<CPortManager::portInfo>  ports = manager.getPortUsage();

    for (int i = 0; i < ports.size(); i++) {
      if ((ports[i].s_Application == string("ScalerDisplay")   &&
	   ports[i].s_User        == me)) {
	return ports[i].s_Port;
      }
    }
    // Landing here means ther's no scaler display in the server.

    errno = ECONNREFUSED;
    throw CErrnoException("No Scaler Server found in the host");

  }
  else {
    // portArg should be an integer.

    char* end;
    unsigned long int port = strtoul(portArg.c_str(), &end, 0);
    if (*end != '\0') {
      throw CInvalidArgumentException(portArg, 
				      "Must be 'managed' or an integer port number",
				      "Processing the --port option");
    }
    return port;
  }

}

/*
** Process the items from m_pRing.. this should not exit.  The user
** will most likely kill this or the system will reboot or the user
** will logout etc. etc.
*/
void
SclClientMain::processItems()
{
  initializeStateChange();	// set up the initial values of the server vars.

  // Set up the ring predicate so that we're only going to be seeing
  // state change and scaler items.
  // TODO: 
  //   Arrange a way for scaler display to also show trigger rates and
  //  trigger counts.
  // 

  CDesiredTypesPredicate predicate;
  predicate.addDesiredType(BEGIN_RUN);
  predicate.addDesiredType(END_RUN);
  predicate.addDesiredType(PAUSE_RUN);
  predicate.addDesiredType(RESUME_RUN);
  predicate.addDesiredType(INCREMENTAL_SCALERS);

  bool beginSeen = false;

  while(1) {
    CRingItem*  pItem = CRingItem::getFromRing(*m_pRing, predicate);

    // Dispatch to the correct handler:

    try {
      switch (pItem->type()) {
      case BEGIN_RUN:
	beginSeen = true;
      case END_RUN:
      case PAUSE_RUN:
      case RESUME_RUN:
	{
	  CRingStateChangeItem item(*pItem);
	  processStateChange(item);
	}
	break;
      case INCREMENTAL_SCALERS:
	{
	  // If the begin run not seen.. call RunInProgres in the server

	  
	  if (!beginSeen) {
	    m_pServer->SendCommand("RunInProgress");
	    beginSeen = true;	// only do this once though.
	  }
	  CRingScalerItem item(*pItem);
	  processScalers(item);
	}
	break;
	
      }
      delete pItem;
      pItem = 0;		// See the catch block below.
    }
    catch (...) {		// Just in case exceptions are not fatal.
      delete pItem;
      throw;
    }
  }
}
/*
** Process scaler items.
** We need to update 
** - Scaler_Totals     - the new scaler totals.
** - Scaler_Increments - The scaler increments for this interval.
** - ScalerDeltaTime   - The length of the scaler interval
** - ElapsedRunTime    - How long the run has been active.
** Having done all this we need to call
** Update              - Which is supposed to make any display changes that require
**                       computation.
**
*/
void
SclClientMain::processScalers(const CRingScalerItem& item)
{
  uint32_t startTime           = item.getStartTime();
  uint32_t endTime             = item.getEndTime();
  vector<uint32_t> increments  = item.getScalers();

  // If the totals don't exist, zero them:

  if (m_Totals.size() == 0) {
    for (int i=0; i < increments.size(); i++) {
      m_Totals.push_back(0.0);
    }
  }
  
  // Update the totals from the increments:

  for (int i=0; i < increments.size(); i++) {
    m_Totals[i] += static_cast<double>(increments[i]);
  }

  // Compute the elapsed and delta times:

  uint32_t deltaTime   = endTime - startTime;
  uint32_t elapsedTime = endTime;

  // Interact with the server:

  setInteger("ScalerDeltaTime", deltaTime);
  setInteger("ElapsedRunTime",  elapsedTime);
  for (int i = 0; i < increments.size(); i++) {
    setInteger("Scaler_Increments", increments[i], i);
    setDouble("Scaler_Totals",  m_Totals[i], i);
  }

  m_pServer->SendCommand("Update");



}
/*
** Process state change items.
** We must update:
**  - RunState
**  - Run Title
**  - RunNumber
**  - ElapsedRunTime
** Call:
**  The appropriate state change proc
**  Update.
*/
void
SclClientMain::processStateChange(const CRingStateChangeItem& item)
{
  uint16_t type    = item.type();
  uint32_t run     = item.getRunNumber();
  uint32_t elapsed = item.getElapsedTime();
  

  setInteger("RunNumber", run);
  setInteger("ElapsedRunTime", elapsed);

  string command = "set RunTitle {";
  command += item.getTitle();
  command += "}";
  m_pServer->SendCommand(command);

  string stateproc;

  command = "set RunState ";
  switch (type) {
  case BEGIN_RUN:
    command += "Active";
    stateproc = "BeginRun";
    break;
  case RESUME_RUN:
    command += "Active";
    stateproc = "ResumeRun";
    break;
  case PAUSE_RUN:
    command += "Paused";
    stateproc = "PauseRun";
    break;
  case END_RUN:
    command += "Inactive";
    stateproc= "EndRun";
    break;
  }
  m_pServer->SendCommand(command);
  m_pServer->SendCommand(stateproc);

  //  m_pServer->SendCommand("Update");

  

}
/*
** Provide initial values for the various state change variables:
*/
void
SclClientMain::initializeStateChange()
{
  m_pServer->SendCommand("set RunState *Unknown*");
  m_pServer->SendCommand("set RunTitle *Unknown*");
  m_pServer->SendCommand("set RunNumber *Unknown*");
  m_pServer->SendCommand("set ElapsedRunTime *Unknown");
}
/*
** Connect to the Tcl server and establish connection lost callbacks.
**
*/
void
SclClientMain::connectTclServer()
{
  m_pServer = new TclServerConnection(m_Host, m_Port);

  int retries=10;
  while (!m_pServer->Connect()) {
    cerr << "sclclient - failed to connect with Tcl server " << retries << " retries remaining\n";
    retries--;
    sleep(2);
    if (retries < 0) {
      cerr << "sclclient - failed to connect with Tcl server .. no more retries.\n";
      exit(EXIT_FAILURE);
    }
  }
  // Register the connection lost relay in case we lose the connection:

  string name = whoAmI();
  name += "\n";
  m_pServer->SetDisconnectCallback(ConnectionLostRelay, this);
  m_pServer->Send(const_cast<char*>(name.c_str()), name.size()); // 'authentication'.
}
/*
** Process connection losses.. report via cerr, and let connectTclServer attempt to
** retry.. if successful, we assume the state is lost
*/
void
SclClientMain::ConnectionLost()
{
  cerr << "Connection to the Tcl server was lost. Attempting to reconnect\n";

  delete m_pServer;
  connectTclServer();
  initializeStateChange();
}

/*
** Called by the m_pServer object when the connectino was lost.
** Establish object context and chain to ConnectionLost via it.
*/
void
SclClientMain::ConnectionLostRelay(TcpClientConnection& connection, void* theObject)
{

  SclClientMain* pObject = reinterpret_cast<SclClientMain*>(theObject);
  pObject->ConnectionLost();
}

/*
** Return the name url of the default ring
**/
string
SclClientMain::defaultRing()
{
  string url = "tcp://localhost/";


  url += whoAmI();
  return url;  
}

/*
** Figure out my username:
*/
string
SclClientMain::whoAmI()
{
  // Figure out who we are:

  uid_t id = getuid();
  struct passwd* pwd = getpwuid(id);
  string user(pwd->pw_name);

  return user;
}

/*
** Set an integer Tcl variable in the server:
**
** Parameters:
**   name    - Base name of the array.
**   value   - Value to set.
**   index   - if non-negative an integer array index.
*/
void
SclClientMain::setInteger(string name, int value, int index)
{
  string command;
  char   buffer[128];
  if (index < 0) {
    sprintf(buffer, "set %s %d", name.c_str(), value);
  }
  else {
    sprintf(buffer, "set %s(%d) %d", name.c_str(), index, value);
  }
  command = buffer;
  m_pServer->SendCommand(command);
}

/*
** Sets a floating point tcl variable in the server.
** Parameters are similar to setInteger above.
*/
void
SclClientMain::setDouble(string name, double value, int index)
{
  string command;
  char   buffer[256];

  if (index < 0) {
    sprintf(buffer, "set %s %f", name.c_str(), value);
  }
  else {
    sprintf(buffer, "set %s(%d) %.0f", name.c_str(),  index, value);
  }
  command = buffer;

  m_pServer->SendCommand(command);
}
