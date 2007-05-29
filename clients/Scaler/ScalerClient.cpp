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
#include "ScalerClient.h"
#include "NSCLScalerBuffer.h"
#include "NSCLStateChangeBuffer.h"
#include <Consumer.h>
#include <stdio.h>
#include  <unistd.h>
#include <Iostream.h>   				
#include <CopyrightNotice.h>
#include <sys/types.h>
#include <pwd.h>

#include <stdlib.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

static char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved .cpp \n";

static const string kDefaultHost = string("localhost");
static const int    knDefaultPort= 2700;
static const string kDefaultURL  = string("TCP://localhost:2700/");

//
// The following are TCl script fragments we feed into the server:
//
static const string kZeroScript = 
           string(" eval { \n\
                     if {[info exists Scaler_Totals]} {              \n\
                     foreach element [array names Scaler_Totals] { \n\
                        set Scaler_Totals($element) 0              \n\
                     }                                             \n\
                     foreach element [array names Scaler_Increments] { \n\
                        set Scaler_Increments($element) 0          \n\
                     }                                             \n\
                   } } ");


// Static functions.

//
// Note that mygetopt only supports a single argument specification 
// in pSwitch, always starts from the beginning of argc/argv (always
// resets optindex ->1) and ignores switches which don't match, continuing
// to the next switch.
//
static char* optvalue;
static int mygetopt(int argc, char** argv, char* pSwitch) 
{
  bool hasValue =FALSE;
  optvalue        = "\0";
  if(strlen(pSwitch) == 2) hasValue = TRUE; // a Bit imprecise.
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-' && argv[i][1] == pSwitch[0]) { // Match.
      if (hasValue) {
	if(strlen(argv[i]) > 2) { // Value is right up against parameter.
	  optvalue = &(argv[i][2]);
	}
	else {			// Value, if it exists is next param.
	  if((argc - i) > 1) {
	    optvalue = argv[i+1];
	  }
	}
      }
      return argv[i][1];
    }
  }
  return -1;
}

/*!
   escape quotes in a const char* returning a string result.
   quotes are escaped to \" and \'
   \param pString (const char* [in]):
      The input string that will be stripped.
   \return string
      The stripped string.
*/
static string EscapeQuotes(const char* pString)
{
  string result;

  while(*pString != 0) {
    if(*pString == '\'') {
      result += "\\'";
    }
    else if (*pString == '"') {
      result += "\\\"";
    } else {
      result += *pString;
    }
    pString++;
  }
  return result;
}
// Functions for class CScalerClient


//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnScalerBuffer(NSCLScalerBuffer& rScalerBuffer)
//  Operation Type: 
//     
void CScalerClient::OnScalerBuffer(CNSCLScalerBuffer& rScalerBuffer)  
{

#ifdef DUMPSCALER
  rScalerBuffer.Dump(cout);
  cout << endl;
#endif

  char   cmd[100];
  // Called when a scaler buffer is received.
  //
  // Figure out the various times and update them:


  float fStartsAt = rScalerBuffer.getStartIntervalTime();
  float fEndsAt   = rScalerBuffer.getEndIntervalTime();
  float fDuration = fEndsAt - fStartsAt;

  sprintf(cmd, "set ElapsedRunTime %f", fEndsAt);
  string sCommand(cmd);
  m_Connection->SendCommand(sCommand);

  sprintf(cmd, "set ScalerDeltaTime %f", fDuration);
  sCommand = cmd;
  m_Connection->SendCommand(sCommand);
  UpdateRunNumber(rScalerBuffer.getRunNumber());
  UpdateRunState(RSActive);
  vector<ULong_t> scalers = rScalerBuffer.getAllScalers();
  if(scalers.size()) {
    UpdateScalers(scalers, rScalerBuffer.isSnapshot());
    sCommand=cmd;
    m_Connection->SendCommand(sCommand);
  }
  m_Connection->SendCommand(string("Update"));

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnBeginBuffer(CNSCLBeginBuffer& rBuffer)
//  Operation Type: 
//     
void CScalerClient::OnBeginBuffer(CNSCLStateChangeBuffer& rBuffer)  
{
  //  Called when a begin run buffer is received.

 
  ClearScalers();
  UpdateRunTitle(rBuffer.getTitle().c_str());
  UpdateRunNumber(rBuffer.getRunNumber());
  
  // Begins can only come our way from state halted... so even if this
  // is the first buffer, we know the state unambiguously.

  setRunState(RSHalted);
  UpdateRunState(RSActive);

  string cmd;

  cmd=string("set ElapsedRunTime 0.0");
  m_Connection->SendCommand(cmd);
  cmd=string("set ScalerDeltaTime 0.0");
  m_Connection->SendCommand(cmd);

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnEndBuffer(CNSCLEndRunBuffer& rBuffer)
//  Operation Type: 
//     
void CScalerClient::OnEndBuffer(CNSCLStateChangeBuffer& rBuffer)  
{
  // Called when an end run buffer is recieved from the DAQ.

  UpdateRunState(RSHalted);
  UpdateRunTitle(rBuffer.getTitle().c_str());
  UpdateRunNumber(rBuffer.getRunNumber());
  string cmd("Update  ");
  m_Connection->SendCommand(cmd);
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnPauseBuffer(CNSCLPauseBuffer& rBuffer)
//  Operation Type: 
//     
void CScalerClient::OnPauseBuffer(CNSCLStateChangeBuffer& rBuffer)  
{
  // Called when a pause run buffer is received by the daq.

  UpdateRunState(RSPaused);
  UpdateRunTitle(rBuffer.getTitle().c_str());
  UpdateRunNumber(rBuffer.getRunNumber());
  m_Connection->SendCommand(string("Update"));
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnResumeBuffer(CNSCLResumeBuffer& rBuffer)
//  Operation Type: 
//     
void CScalerClient::OnResumeBuffer(CNSCLStateChangeBuffer& rBuffer)  
{
  // Called when a resume run buffer is recieved from the daq.

  UpdateRunState(RSActive);
  UpdateRunTitle(rBuffer.getTitle().c_str());
  UpdateRunNumber(rBuffer.getRunNumber());
  m_Connection->SendCommand(string("Update"));
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnConnection(TclServerConnection& rConnection)
//  Operation Type: 
//     
void CScalerClient::OnConnection(TcpClientConnection& rConnection)  
{
  //  Called via ConnectionRelay when a connection has been 
  // established with the peer.
  
  m_eConnectionState = -1;	// Indicate connection.

  // As of 8.1 we indicate our username to the server...
  // the server will kill our connection if we don't match.
  //

  uid_t uid = geteuid();
  struct passwd* pwd = getpwuid(uid);
  string username = string(pwd->pw_name);
   username += "\n";
  
  // Authentication does not send a reply so we must do it this way.


  m_Connection->Send((void*)(username.c_str()), username.size());
  
  // It's possible we've been disconnected at this point:

  if(!m_eConnectionState) {
    cerr << "You are connecting to the wrong tclserver (wrong username)\n";
    exit(-1);

  }


  setRunState(RSUnknown);	// State of run not known.

  // Update the server variables we know about.


  m_Connection->SendCommand(string("set ScalerDeltaTime 0"));
  m_Connection->SendCommand(string("set ElapsedRunTime  0"));
  m_Connection->SendCommand(string("set RunNumber      >Unknown<"));
  UpdateRunState(RSUnknown);
  UpdateRunTitle(">>Unknnown<<");
  ClearScalers();

  // Start reading data and 


  EnableDataTaking();		// Connect to spectrodaq.
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     OnDisconnected(TclServerConnection& rConnection)
//  Operation Type: 
//     
void CScalerClient::OnDisconnected(TcpClientConnection& rConnection)  
{
  //  Called through DisconnectRelay whenever the 
  //  connection with the tcl server has been broken.
  

  cerr << "Disconnection triggered\n";

  DisableDataTaking();
  m_eConnectionState = 0;
  cerr << "Disconnected from Tcl/Tk server" << endl;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     ConnectionRelay(void* pObject, TCPClientConnection& rConnection)
//  Operation Type: 
//     
void 
CScalerClient::ConnectionRelay(TcpClientConnection& rConnection, 
			       void* pObject)  
{
  // Bridge to object context from Connection
  // callbacks in TCPClientConnection.
  //
  CScalerClient* pMe = (CScalerClient*)pObject;
  pMe->OnConnection(rConnection);

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     DisconnectRelay(void* pObject, TCPClientConnection& rConnection)
//  Operation Type: 
//     
void CScalerClient::DisconnectRelay(TcpClientConnection& rConnection,
				    void* pObject)  
{
  //  Relay into object context from disconnect callbacks
  //  in TCPServerConnection.
  
  CScalerClient* pMe = (CScalerClient*)pObject;
  pMe->OnDisconnected(rConnection);

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     operator()(int argc, char** pargv)
//  Operation Type: 
//     
int CScalerClient::operator()(int argc, char** pargv)  
{

  CConsumer::operator()(argc, pargv);

  //  Entry point of the system.

  CopyrightNotice::Notice(cerr, pargv[0], "1.0",
			  "2002");
  CopyrightNotice::AuthorCredit(cerr, pargv[0],
				"Ron Fox", (char*)NULL);

  string RemoteHost = GetRemoteHost(argc, pargv);
  int    RemotePort = GetRemotePort(argc, pargv);
  string DataSource = GetDataSourceURL(argc, pargv);

  if(m_fDefaultSource) {
    WarnDefaultSource();	// Warn if no -s switch.
  }

  m_Connection = new TclServerConnection(RemoteHost, RemotePort);
  m_Connection->SetConnectCallback(ConnectionRelay, (void*)this);
  m_Connection->SetDisconnectCallback(DisconnectRelay, (void*)this);


  AddDataSource(DataSource.c_str(), 3);   // Only need state change bufs.

  while(1) {
    if(!m_eConnectionState) {	// Attempt to form connection, OnConnection
      if(!m_Connection->Connect()) {	// Will alter state for us.
	sleep(1);		// Wait a sec. between retries.
      }
    }
    else {			// Connected, so get buffers:
      CheckForData((struct timeval*)0, 3); // OnConnection will connect to DAQ,
				// OnDisconnect will disconnect.
				// OnxxxBuffer will handle buffers.
    }
  }
}
///////////////////////////////////////////////////////////////////////////
//
//  Function:
//    string GetRemoteHost(int nArgs, char** pArgs)
//  Operation Type:
//    Private utility.
//
string
CScalerClient::GetRemoteHost(int nArgs, char** pArgs)
{
  // The host either is "localhost" if not supplied or is
  // the parameter to the -h switch.
  //
  string Host = kDefaultHost;
  GetSwitchParameter(Host, "h:", nArgs, pArgs);
  return Host;
}
//////////////////////////////////////////////////////////////////////
//
// Function:
//   int    GetRemotePort(int nArgs, char** pArgs)
// Operation type:
//   Static utility.
//
int
CScalerClient::GetRemotePort(int nArgs, char** pArgs)
{
  // Figure out the default port:

  int nPort = knDefaultPort;
  struct servent *pServiceInfo;
  pServiceInfo = getservbyname("daqstatus", "tcp");
  if(pServiceInfo) nPort= pServiceInfo->s_port;

  // Now see if this is overidden on the command line.

  string sPort;
  if(GetSwitchParameter(sPort, "p:", nArgs, pArgs)) {
    int np;
    if(sscanf(sPort.c_str(), "%d", &np) == 0) {
      Usage();
      throw "Invalid port number on -p switch";
    }
    else {
      nPort = np;
    }
  }
  return nPort;
}
////////////////////////////////////////////////////////////////////////
//
// Function:
//   Bool_t GetSwitchParameter(string& rValue, char* pSwitch, 
//                             int nArgs, char** pArgs)
//
// Operation Type:
//   Static utility.
//
Bool_t
CScalerClient::GetSwitchParameter(string& rValue, char* pSwitch, 
				  int nArgs, char** pArgs)
{
  // Look for a particular switch and return its value.
  //

  int stat = mygetopt(nArgs, pArgs, pSwitch);
  if(stat == pSwitch[0]) {
    rValue = optvalue;
    return kfTRUE;
  }
  return kfFALSE;
}
//////////////////////////////////////////////////////////////////////////
//
// Function:
//     void Usage()
// Function type:
//    Static utility.
//
void
CScalerClient::Usage()
{
  // Put the program usage information on stderr
  //
  cerr << "Usage:\n" << endl;
  cerr << "   sclclient [-h host] [-p port]\n" << endl;
  cerr << "      -s url   - Specifies the source of event data\n";
  cerr << "                 Defaults to TCP://localhost:2602/\n";
  cerr << "      -h host  - specifies remote host (defaults to localhost)\n";
  cerr << "      -p port  - specifies remote port (defaults to:" << endl;
  cerr << "                                        daqstatus if defined\n";
  cerr << "                                        2700 if no daqstatus)\n";

}

/////////////////////////////////////////////////////////////////////////
//
// Function:
//   void ClearScalers()
// Operation Type:
//   Utility.
//
void
CScalerClient::ClearScalers()
{
  // Empties the scaler array (the way we clear it).
  // Also zeroes the scaler array in the server.
  //
  m_vTotals.erase(m_vTotals.begin(), m_vTotals.end());
  m_vIncrements.erase(m_vIncrements.begin(), m_vIncrements.end());

  m_Connection->SendCommand(kZeroScript);
}
////////////////////////////////////////////////////////////////////////
//
// Function:
//   UpdateRunState(DAQRunState eNewState)
// Operation Type:
//   Utility:
//
void
CScalerClient::UpdateRunState(DAQRunState eNewRunState)
{
  switch(eNewRunState) {
  case RSActive:		// Active - matters what we are now though.
    m_Connection->SendCommand(string("set RunState Active"));
    switch(getRunState()) {
    case RSPaused:
      m_Connection->SendCommand(string("ResumeRun"));
      break;
    case RSUnknown:
      m_Connection->SendCommand(string("RunInProgress"));
      break;
    case RSActive:
      break;			// No actual run state change.
    case RSHalted:
    default:
      m_Connection->SendCommand(string("BeginRun"));
      break;
    }
    break;
  case RSPaused:		// Run state Unambiguously paused.
    m_Connection->SendCommand(string("set RunState Paused"));
    m_Connection->SendCommand(string("PauseRun"));
    break;
  case RSHalted:		// Run state unambiguously halted:
    m_Connection->SendCommand(string("set RunState Halted"));
    m_Connection->SendCommand(string("EndRun"));
    break;
  case RSUnknown:
  default:
    m_Connection->SendCommand(string("set RunState >Unknown<"));
    break;
  }
}
//////////////////////////////////////////////////////////////////////////
//
// Function:
//   void  UpdateRunTitle(const char* pNewTitle)
// Operation Type:
//   Utility.
//
void
CScalerClient::UpdateRunTitle(const char* pNewTitle)
{
  string cmd("set RunTitle \"");
  string title(EscapeQuotes(pNewTitle)); // else  embedded quotes are killers.
  cmd += title;
  cmd += "\"";



  m_Connection->SendCommand(cmd);

}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//    void  UpdateRunNumber(int        nNewRun)
//  Operation Type:
//    Utility
//
void
CScalerClient::UpdateRunNumber(int nNewRun)
{
  char cmd[100];
  sprintf(cmd,"set RunNumber %d  ",nNewRun);
  m_Connection->SendCommand(string(cmd));
}
////////////////////////////////////////////////////////////////////////////
//
// Function:
//   void  UpdateScaler(vector<ULong_t>& rScalers, Bool_t isSnapshot)
// Operation type:
//   Utility:
//
void
CScalerClient::UpdateScalers(vector<ULong_t>& rScalers, Bool_t isSnapshot)
{
  // Updates the current incremental scalers, and scaler totals.
  // these are then sent to the Tcl server.
  // There are 2 types of scaler buffers, snapshot buffers accumulate out
  // of band with the 'recorded' buffers.  The recorded buffers contain
  // the sum of earlier snapshots.  This implies differences in both the
  // way increments and totals are managed.

  if(m_vTotals.size() == 0) CreateArrays(rScalers.size());

  if(isSnapshot) {		// Snapshot buffer - sum increments add into
				// totals from buffer data directly.
    for(int i = 0; i < m_vTotals.size(); i++) {
      m_vTotals[i]     += rScalers[i];
      m_vIncrements[i] += rScalers[i];
    }
  }
  else {			// Not snapshot, Totals from diff between
				// scaler and increment, increment replaced.
    for(int i = 0; i < m_vTotals.size(); i++) {
      m_vTotals[i]    += (rScalers[i] - m_vIncrements[i]);
      m_vIncrements[i] = rScalers[i];
    }
  }
  // Send info to the server.

  for(int i = 0; i < m_vTotals.size(); i++) {
    char command[100];
    sprintf(command,"set Scaler_Increments(%d) %d  ", i, m_vIncrements[i]);
    m_Connection->SendCommand(string(command));

    sprintf(command,"set Scaler_Totals(%d) %d  ", i, m_vTotals[i]);
    m_Connection->SendCommand(string(command));
  }

  // If was snapshot, then post zero the increments:

  if(!isSnapshot) {
    for(int i = 0; i < m_vIncrements.size(); i++) {
      m_vIncrements[i] = 0;
    }
  }
    
}

///////////////////////////////////////////////////////////////////////////
//
// Function:
//   void  CreateArrays(int nScalers)
// Operation Type:
//   utility
void
CScalerClient::CreateArrays(int nScalers)
{
  m_vTotals.insert(m_vTotals.begin(), nScalers, 0);
  m_vIncrements.insert(m_vIncrements.begin(), nScalers, 0);
}
///////////////////////////////////////////////////////////////////////////
//
// Function:
//   string GetDataSourceURL(int nArgs, char** pArgs)
// Operation Type:
//   Utility
//
string
CScalerClient::GetDataSourceURL(int nArgs, char** pArgs)
{
  // We will look for the -s switch and eat up the next argument as the
  // url.  The default return value is TCP://localhost:2602/
  // which takes data from the local spectrodaq server.
  //
  string url(kDefaultURL);

  if(GetSwitchParameter(url, "s:", nArgs, pArgs)) {
    m_fDefaultSource = false;
  }
  return url;
}

/*!
  Warn the user that the default data source is being used (locahost).
  If the DISPLAY env. variable is defined, then this warnign is done
  via the scalerlocal.tk script to pop up an xwindows warning.
  Otherwise, the warning will go out stderr.

 */
void
CScalerClient::WarnDefaultSource()
{
  if(getenv("DISPLAY")) {	// X11 display defined...
    cerr << "Xwindows environment" << endl;
    string program(INSTDIR);
    program += "/bin/scalerlocal.tk";
    system(program.c_str());

  } else {			// Text console only.
    cerr << "sclclient - WARNING: the default data source is being\n";
    cerr << "                     used (localhost).  Please be sure\n";
    cerr << "                     this system is actually the one  \n";
    cerr << "                     connected to the hardware\n";

  }
}

//  Declare the application so that Spectrodaq client libraries can start it
//  up.
CScalerClient mydaq;
