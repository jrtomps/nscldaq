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

static const char* Copyright = "(C) Copyright Michigan State University 1944, All rights reserved";
// Author:
//   Jason Venema
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:venemaja@msu.edu
//
// Copyright
//   NSCL All rights reserved.
//
// See CAlarmServer.h for a description of this class.
//

#include <config.h>

#include <Iostream.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <CAlarmServer.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

extern gdbm_error gdbm_errno;           // errno for gdbm
// extern int errno;                       // in case the server dies

/*
  "int SortHistory"

  This function is used as a function object by the STL sort algorithm. 
  The purpose of it is to sort history entries from the alarm database
  chronologically, starting with the earliest recorded alarm. The parameters
  are strings representing an entire alarm entry (including facility, status,
  message, etc...)

  \param string First - The first history entry to compare
  \param string Second - The second history entry to compare

  \return 1 if the first entry is younger than the second, 0 otherwise.
*/
int SortHistory(string First, string Second)
{
  // First get the position of the creation date...
  int pos1 = First.find("CREATED:");
  int pos2 = Second.find("CREATED:");

  // Now break the date up into year, month, day, etc... and 
  // turn them into integers for comparison...
  int year1 = atoi((First.substr(pos1+11, 4)).c_str());
  int year2 = atoi((Second.substr(pos2+11, 4)).c_str());
  int month1 = atoi((First.substr(pos1+16, 2)).c_str());
  int month2 = atoi((Second.substr(pos2+16, 2)).c_str());
  int day1 = atoi((First.substr(pos1+19, 2)).c_str());
  int day2 = atoi((Second.substr(pos2+19, 2)).c_str());
  int hour1 = atoi((First.substr(pos1+22, 2)).c_str());
  int hour2 = atoi((Second.substr(pos2+22, 2)).c_str());
  int minute1 = atoi((First.substr(pos1+25, 2)).c_str());
  int minute2 = atoi((Second.substr(pos2+25, 2)).c_str());
  int second1 = atoi((First.substr(pos1+28, 2)).c_str());
  int second2 = atoi((Second.substr(pos2+28, 2)).c_str());

  // Now perform the comparision
  int result;                        // the result of the comparison
  if(year1 < year2) result = 1;
  else if(year1 > year2) result = 0;
  else {
    if(month1 < month2) result = 1;
    else if(month1 > month2) result = 0;
    else {
      if(day1 < day2) result = 1;
      else if(day1 > day2) result = 0;
      else {
	if(hour1 < hour2) result = 1;
	else if(hour1 > hour2) result = 0;
	else {
	  if(minute1 < minute2) result = 1;
	  else if(minute1 > minute2) result = 0;
	  else {
	    if(second1 < second2) result = 1;
	    else result = 0;
	  }
	}
      }
    }
  }
  return result;
}

/*!
  "Default Constructor"  This is the default constructor. It constructs a
  CAlarmServer, and creates the associated gdbm database files necessary
  to maintain the alarm information:
  
     .alarmdb - Contains the actual alarm information including alarm id,
                logging facility, status, message and date/time.
     .alarmcount - Contains the number of alarms logged for each experiment.
                   This is useful so the displayer knows what alarm id to
		   assign to the next incoming alarm, when starting over.
  
  If the port daqalarm is configured in /etc/services, then we will use that
  one, instead of hardcoding the port. If it is not configured, then we
  default to 2702.
*/
CAlarmServer::CAlarmServer()
{
  // See if "daqalarm" is configured
  struct servent* pServ = getservbyname("daqalarm", "tcp");
  if(pServ) {
    Int_t nPort = pServ->s_port;
    char port[4];
    sprintf(port, "%d", nPort);
    m_sPort = string(port);
  }
  else {
    // default to 2703
    m_sPort = "2703";
  }

  GDBM_FILE DBFWriter;

  // We open these files so that they will be created. That way, we don't
  // have to worry about doing this later, during the actual fulfilling of
  // client requests.
  if(!(DBFWriter = gdbm_open(".alarmcount", 512, GDBM_WRCREAT, 
			     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0))) {
    string reason = "Attempting to open .alarmcount while ";
    reason += "constructing CAlarmServer";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
  gdbm_close(DBFWriter);
  if(!(DBFWriter = gdbm_open(".alarmdb", 512, GDBM_WRCREAT,
			     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0))) {
    string reason = "Attempting to open .alarmcount while ";
    reason += "constructing CAlarmServer";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }

  // It is VERY important to close the gdbm file every time we are done writing
  // to it or reading from it, so that some cleanup operations may be performed
  gdbm_close(DBFWriter);
}

/*!
  "Copy Constructor"  This is the copy constructor. Given a reference to
  another CAlarmServer, it constructs a copy of it.

  \param aCAlarmServer - the reference object whose attributes will be copied
*/
CAlarmServer::CAlarmServer(const CAlarmServer& aCAlarmServer)
{
  m_sExpId = aCAlarmServer.m_sExpId;
}

/*!
  "CAlarmServer::operator()"

  Operation Type:
     Server listener

  This is the heart of the server. First, a child process is spawned, so that
  if the server dies, the parent can restart it by simply spawing a new child
  process to be the new server. Second, the child creates a new socket and
  performs a bind, listen and accept routine. Any clients attempting to make
  connection requests can then be heard. Finally, messages are received through
  the socket connection, and the server performs the requested action.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.
  \exception CErrnoException       - Errno exception occurred
  \exception CTCPBadSocketState    - CSocket::m_State was not disconnected
  \exception CTCPNoSuchHost        - Host not in DNS or nonexistent
  \exception CTCPNoSuchService     - Named service does not translate.
  \exception CTCPConnectionFailed  - Connection refused by remote host
  \exception CTCPConnectionLost    - Connection terminated by remote host
*/
bool
CAlarmServer::operator()()
{
  CSocket* newSock;   // the new socket
  string client_ip;   // the client ip address
  pid_t child_pid;    // the pid of the child process
  if((child_pid = fork()) < 0) {
    // If we can't even fork a new process, we had better bail out!
    cerr << "Error while attempting to fork a new process." << endl;
    return 0;
  }

  // The parent will wait for the child to die, and restart it if need be...
  else if(child_pid > 0) {
    int x;
    wait(&x);

    //If the exit status is SIGHUP, then someone asked us to shutdown
    if(x == SIGHUP) {
      cout << "Received hangup!" << endl;
      return 0;
    }

    // Otherwise, something happened that shouldn't have and we try
    // to restart the server.
    else {
      cerr << "Server dying!" << endl;
      perror(strerror(x));
      perror(strerror(errno));
      return 1;
    }
  }

  // The child process will execute this part...
  else {
    try {
      CSocket sock;

      // The following code is so that the server can be restarted immediately
      // upon exiting, without having to wait for the OS to free up the address
      // space it was using.
      int fd = sock.getSocketFd();
      int state = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &state, sizeof(int));

      // Now we bind, listen and accept any requests
      sock.Bind(m_sPort);

      // BUGBUG Listen has a parameter nBacklog -- should we change it?
      // The value is defaulted to 5, meaning that the limit on the queue size
      // for incoming connections is 5. Additional connections are refused.
      sock.Listen();
      cout << "Listening for connection requests on port " << m_sPort 
	   << "...\n";
      while(1) {
	newSock = sock.Accept(client_ip);

	// Get the id and reason of the experiment making the request
	void* pBuf = (void*)(new char[512]);

	// Block until some command comes through the socket
	int nB = newSock->Read(pBuf, 512);

	// Break that command down so we can process it
	char command[512];
	sprintf(command, "%s", (char*)pBuf);
	char* pCommand = command;

	// Check the length to make sure it's not so long that malicious
	// users could include long programs. The packets are limited to
	// 512 characters to accomodate relatively long messages.
	if(strlen(pCommand) >= 512) {
	  CAlarmServerException ase
	    ("Attempting to parse command which is too long!",
	     pCommand);
	  throw ase;	  
	}
	char* temp;
	int reason;
	string fac, mess, date, from;
	temp = strsep(&pCommand, " ");
	m_sExpId = string(temp);  // set the experiment ID for this request
	temp = strsep(&pCommand, " ");
	reason = atoi(temp);      // this is why we're being called

	// Now we figure out what the request is all about
	switch(reason) {
	case LOG: {
	  // Break the command up into the alarm information so it can
	  // be logged to the logfile
	  temp = strsep(&pCommand, "~");
	  fac = string(temp);
	  temp = strsep(&pCommand, "~");
	  mess = string(temp);
	  temp = strsep(&pCommand, "~");
	  date = string(temp);
	  temp = strsep(&pCommand, "\n");
	  from = string(temp);
	  
	  // Make sure the information is valid
	  if(!(fac.size()) || !(mess.size()) || 
	     !(date.size()) || !(from.size())) {
	    CAlarmServerException ase
	      ("Attempting to parse log request in CAlarmServer::operator()",
	       pCommand);
	    throw ase;
	  }
	  
	  // Let the Log() function take care of the rest
	  Log(fac, mess, date, from);
	  break;
	}
	case ACKNOWLEDGE:
	case DISMISS:
	case REMOVE: {
	  // Break the command down further and get the alarm id to edit
	  temp = strsep(&pCommand, " \n");
	  int id = atoi(temp);
	  char szBuf[20];
	  sprintf(szBuf, "%d", id);
	  m_sAlarmId = string(szBuf);

	  // Let the EditAlarm() function do the rest
	  EditAlarm(reason);
	  break;
	}
	case UPDATE: {
	  // We perform the update stuff here, because we need to use the
	  // socket that is already opened to send update information to the
	  // client.
	  GDBM_FILE DBFReader;
	  if(!(DBFReader = gdbm_open(".alarmdb", 512 ,GDBM_READER, 0, 0))) {
	    string reason = "Attempting to open .alarmdb in ";
	    reason += "CAlarmServer::Update()";
	    CGDBMException dbme(reason, gdbm_errno);
	    throw dbme;
	  }
	  string packet = "";  // this will be the final packet we send
	  string sPiece;       // this is just a piece of the packet
	  datum NextKey;
	  datum ContentVal;

	  // Start by retrieving the first key from the database
	  datum KeyVal = gdbm_firstkey(DBFReader);
	  while(KeyVal.dptr) {

	    // Now we examine every key, and "build up" our packet with
	    // each entry for this experiment
	    NextKey = gdbm_nextkey(DBFReader, KeyVal);
	    ContentVal = gdbm_fetch(DBFReader, KeyVal);
	    sPiece = string(ContentVal.dptr);
	    if(atoi(&(KeyVal.dptr)[0]) == atoi(m_sExpId.c_str())) {
	      sPiece.resize(ContentVal.dsize);
	      packet += string(sPiece);
	      packet += "\n";
	    }
	    KeyVal = NextKey;
	  }

	  // This eof tells the caller that the packet is done. Now we
	  // send it out!
	  packet += "eof";
	  newSock->Write((void*)packet.c_str(), packet.size());
	  gdbm_close(DBFReader);
	  break;
	}
	case INIT: {
	  string sAlarmId = Init();

	  // Now we will write the alarm count to the socket and send it
	  // off to the alarm displayer to find out if the user really wants
	  // us to create this new experiment
	  int nB = newSock->Write((void*)(sAlarmId.c_str()), sAlarmId.size());
	  break;
	}
	case CREATE: {
	  CreateExperiment();
	  break;
	}
	case HISTORY: {
	  string sHistory = GetExperimentHistory();
	  int nB = newSock->Write((void*)sHistory.c_str(), sHistory.size());
	}
	}
	
	// Perform some memory management here
	newSock->Shutdown();
	delete newSock;
	delete [] (char*)pBuf;
      }
    }
    catch (CException& e) {
      cerr << "Caught exception while attempting to listen on port " 
	   << m_sPort << endl;
      cerr << "Reason was: " << e.ReasonText() << endl;
      cerr << e.WasDoing() << endl;
      exit(2);
    }
  }
}

/*!
  "CAlarmServer::Log"

  This performs the actual storing of the information into the database. The
  keys are of the form 'experimentId.alarmId', and are stored accordingly. 
  Following the entry of this information into the database file .alarmdb,
  the file .alarmcount is updated to reflect an additional alarm in the
  database for this experiment.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.

  \param string& srFacility - the facility performing the logging
  \param string& srMessage  - the message describing the nature of the alarm
  \param string& srDate     - the date and time of the alarm occurrence
*/
void
CAlarmServer::Log(string& srFacility, string& srMessage, 
		  string& srDate, string& srFrom)
{
  // First we create key and content values to store in the database.
  // The content consists of the alarm information, the key is of the
  // form "m_sExpId.m_sAlarmId"
  MapIterator It;
  for(It = m_CurrAlarm.begin(); It != m_CurrAlarm.end(); It++) {
    if(It->first == m_sExpId) {
      m_sAlarmId = It->second;
      int nAlarmId = atoi(m_sAlarmId.c_str());
      int nChars   = (int)log10((double)(nAlarmId+1)) + 1;
      char alm[nChars];
      sprintf(alm, "%d", (++nAlarmId));
      string sAlm = string(alm);
      sAlm.resize(nChars);
      m_CurrAlarm.erase(It);
      m_CurrAlarm.insert(make_pair(m_sExpId, sAlm));
    }
  }

  string key = m_sExpId + "." + m_sAlarmId;
  string content = "ALARMID: ";
  content += m_sAlarmId;
  content += " FACILITY: { ";
  content += srFacility;
  content += "} STATUS: n ";
  content += "MESSAGE: {";
  content += srMessage;
  content += "} CREATED: {";
  content += srDate;
  content += " } ACKNOWLEDGED: {unchanged}";
  content += " DISMISSED: {unchanged} REMOVED: {unchanged}";
  content += " FROM: {";
  content += srFrom;
  content += " }";

  // Items of type "datum" are stored in a gdbm database. A datum
  // consists of two fields: char* dptr and int dsize.
  datum key_value,
    content_value;
  key_value.dptr = (char*)(key.c_str());
  key_value.dsize = strlen(key.c_str());
  content_value.dptr = (char*)(content.c_str());
  content_value.dsize = strlen(content.c_str());

  // We can only allow one writer to open the file with write access
  // at a time.  We need to write the information to log it.
  GDBM_FILE DBFWriter;
  
  // There may be problems opening the database file
  if(!(DBFWriter = gdbm_open(".alarmdb", 0, GDBM_WRCREAT, 
			     S_IRUSR | S_IWUSR | S_IRGRP | 
			     S_IROTH, 0))) {
    string reason = "Attempting to open .alarmdb in CAlarmServer::Log()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }

  // Need to make sure that the information was successfully stored
  if(gdbm_store(DBFWriter, key_value, content_value, GDBM_INSERT)) {
    string reason = "Attemping to store in .alarmdb in CAlarmServer::Log()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
    gdbm_close(DBFWriter);
  }

  // A gdbm_open **needs** to be accompanied by a gdbm_close, in
  // order for gdbm to "tidy up" the database file entries.
  gdbm_close(DBFWriter);
  
  // Now we need to increment the number of alarms this experiment
  // has in the .alarmcount file
  GDBM_FILE DBFReader;
  if(!(DBFReader = gdbm_open
       (".alarmcount", 512, GDBM_READER, 644, 0))) {
    string reason = "Attempting to open .alarmcount in CAlarmServer::Log()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }

  // So we create a key and see if it already exists in the database
  datum Key;
  Key.dptr = const_cast<char*>(m_sExpId.c_str());
  Key.dsize = strlen(m_sExpId.c_str());
  
  // We check to see if the experiment has any alarms logged yet.
  int KeyExists = gdbm_exists(DBFReader, Key);
  
  // There are some alarms logged for this experiment, so we need
  // to update the current entry...
  datum NewContent;
  if(KeyExists) {
    datum ContentValue = gdbm_fetch(DBFReader, Key);
    int nOldCount = atoi(ContentValue.dptr);
    char NewCount[5];
    sprintf(NewCount, "%d", ++nOldCount);
    NewContent.dptr = NewCount;
    NewContent.dsize = strlen(NewCount);
  }
  
  // or, there haven't been any alarms logged here yet, so we simply
  // create a new entry
  else {
    char NewCount[1];
    sprintf(NewCount, "%d", 1);
    NewContent.dptr = NewCount;
    NewContent.dsize = strlen(NewCount);
  }
  gdbm_close(DBFReader);
  
  // Now we need to open a new writer to write the new entry
  if(!(DBFWriter = gdbm_open(".alarmcount", 0, GDBM_WRCREAT,
			     S_IRUSR | S_IWUSR | S_IRGRP |
			     S_IROTH, 0))) {
    string reason = "Attempting to open .alarmcount in CAlarmServer::Log()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
  
  // We tell gdbm to replace the old entry with the new one
  // with the flag GDBM_REPLACE
  int ret = gdbm_store(DBFWriter, Key, NewContent, GDBM_REPLACE);
  gdbm_close(DBFWriter);
  if(ret) {
    string reason = "Attempting to store new alarm count in ";
    reason += ".alarmcount while in CAlarmServer::Log()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
}

/*!
  "CAlarmServer::EditAlarm"

  Performs the editing of information on alarms already stored in the database.
  For instance, an alarm whose status changes needs to have that reflected in
  the database file. Thus, "Acknowledge", "Dismiss" and "Remove" requests will
  invoke this function.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.

  \param int reason - Indicates whether the caller is requesting an,
  Acknowledge, Dismiss, or Remove for this alarm.
*/
void
CAlarmServer::EditAlarm(Int_t nReason)
{
  GDBM_FILE DBFReader;
  GDBM_FILE DBFWriter;
  
  // First open the database for reading and find the key for this
  // alarm and experiment
  if(!(DBFReader = gdbm_open(".alarmdb", 512, GDBM_READER, 644, 0))) {
    string why = "Attempting to open .alarmdb in CAlarmServer::EditAlarm()";
    CGDBMException dbme(why, gdbm_errno);
    throw dbme;
  }
  datum Key;
  string sKey = m_sExpId + "." + m_sAlarmId;
  Key.dptr = const_cast<char*>(sKey.c_str());
  Key.dsize = strlen(sKey.c_str());
  datum ContentValue = gdbm_fetch(DBFReader, Key);
  
  // Pull out the alarm data
  if(!ContentValue.dptr) {
    string why = "Attempting to fetch data from .alarmdb in ";
    why += "CAlarmServer::EditAlarm()";
    CGDBMException dbme(why, gdbm_errno);
    throw dbme;
  }
  gdbm_close(DBFReader);
  
  // Get ready to change the datum by opening up for writing
  if(!(DBFWriter = gdbm_open(".alarmdb", 0, GDBM_WRCREAT,
			     S_IRUSR | S_IWUSR | S_IRGRP |
			     S_IROTH, 0))) {
    string why = "Attempting to open .alarmdb in CAlarmServer::EditAlarm()";
    CGDBMException dbme(why, gdbm_errno);
    throw dbme;
  }
  
  // We will rebuild the content string based on the old content
  // and the new status
  string sNewContent = "";
  char* token;
  
  // Now we rebuild the content string with the new STATUS field
  // Note that there are two cases, one for Acknowledge and one for Dismiss
  while(token = strsep(&ContentValue.dptr, " ")) {
    string tok = string(token);
    if(tok == "STATUS:") {
      sNewContent += tok;
      sNewContent += " ";
      token = strsep(&ContentValue.dptr, " ");
      tok = string(token);
      switch(nReason) {
      case ACKNOWLEDGE: {
	if(tok == "n") sNewContent += "a";
	else sNewContent += tok;
	break;
      }
      case DISMISS: {
	if(tok == "a") sNewContent += "d";
	else sNewContent += tok;
	break;
      }
      case REMOVE: {
	if(tok == "d") sNewContent += "r";
	else sNewContent += tok;
	break;
      }
      }
    }
    else if((tok == "ACKNOWLEDGED:" && nReason == ACKNOWLEDGE) ||
	    (tok == "DISMISSED:"    && nReason == DISMISS)     ||
	    (tok == "REMOVED:"      && nReason == REMOVE)) {
      // First we get the new "changed" time to add to the database. Note
      // the curly braces so the can be used as a tcl list by the displayer
      time_t tim = time(NULL);
      char time_string[28];
      if(tim != (time_t)-1) {
	struct tm* t = localtime(&tim);
	sprintf(time_string, "%04d-%02d-%02d %02d:%02d:%02d",
		(t->tm_year+1900), (t->tm_mon+1), (t->tm_mday),
		(t->tm_hour), (t->tm_min), (t->tm_sec));
	if(t->tm_isdst)
	  strcat(time_string, " DST");
	else
	  strcat(time_string, " EST");
      }
      else {
	sprintf(time_string, "unavailable");
      }

      // Now we make the new time entry and insert it into the approp. field
      sNewContent += tok;
      sNewContent += " ";
      token = strsep(&ContentValue.dptr, " ");
      tok = string(token);
      sNewContent += "{" + string(time_string) + "}";
    }
    else {
      sNewContent += tok;
    }
    sNewContent += " ";
  }

  // Create the new content datum
  sNewContent.resize(ContentValue.dsize+14);
  datum NewContent;
  NewContent.dptr = const_cast<char*>(sNewContent.c_str());
  NewContent.dsize = sNewContent.length();

  // Instruct gdbm_store to replace the entry with the same key,
  // since it is the old entry
  int ret = gdbm_store(DBFWriter, Key, NewContent, GDBM_REPLACE);
  if(ret) {
    gdbm_close(DBFWriter);
    string why = "Attempting to store acknowledgement in .alarmdb ";
    why += "in CAlarmServer::EditAlarm()";
    CGDBMException dbme(why, gdbm_errno);
    throw dbme;
  }

  // Don't forget how important it is to close this file!
  gdbm_close(DBFWriter);
}

/*!
  "CAlarmServer::Init"

  This function performs initialization tasks once an experiment ID has been
  entered into the displayer. The .alarmcount database file is accessed, and
  the alarm count for this experiment is returned. This will be used by the
  caller to determine the id of the next alarm that will be logged.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.

  \return - The number of alarms already logged for this experiment
*/
string
CAlarmServer::Init()
{
  // We open a reader to get the initial alarm count 
  // for this experiment
  GDBM_FILE DBFReader;
  if(!(DBFReader = gdbm_open(".alarmcount", 512, GDBM_READER, 0, 0))) {
    string reason = "Attempting to open .alarmcount in CAlarmServer::Init()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
  datum Key;
  Key.dptr = const_cast<char*>(m_sExpId.c_str());
  Key.dsize = strlen(m_sExpId.c_str());

  // Attempt to fetch the alarm count from the database
  datum ContentValue = gdbm_fetch(DBFReader, Key);
  gdbm_close(DBFReader);

  // If there is no count, then this is a new experiment, and its alarm
  // count is set to zero.
  if(!ContentValue.dptr) {
    datum NewContent;
    char content[2];
    sprintf(content, "0\0");
    NewContent.dptr = content;
    NewContent.dsize = strlen(content);
    ContentValue = NewContent;
  }
  string alarm = string(ContentValue.dptr);
  alarm.resize(ContentValue.dsize);
  m_CurrAlarm.insert(make_pair(m_sExpId, alarm));
  return alarm;
}

/*!
  "CAlarmServer::CreateExperiment"

  Creates a new experiment ID in the database. This is done so that new alarms
  can be logged to this experiment.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.
*/
void
CAlarmServer::CreateExperiment() 
{
  GDBM_FILE DBFWriter;
  if(!(DBFWriter = gdbm_open(".alarmcount", 0, GDBM_WRCREAT,
			     S_IRUSR | S_IWUSR | S_IRGRP |
			     S_IROTH, 0))) {
    string reason = "Trying to open .alarmcount in CAlarmServer::Init()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
  datum Key;
  Key.dptr  = const_cast<char*>(m_sExpId.c_str());
  Key.dsize = strlen(m_sExpId.c_str());
  datum NewContent;
  char content[4];
  sprintf(content, "%d", 0);
  NewContent.dptr = content;
  NewContent.dsize = strlen(content);

  gdbm_store(DBFWriter, Key, NewContent, GDBM_REPLACE);
  gdbm_close(DBFWriter);
}

/*!
  "CAlarmServer::GetExperimentHistory"

  Returns a string containing the entire history for a given experiment to
  the caller. The history string is a single line per entry, with each
  field delimited by spaces.

  \exception CGDBMException - An exception having to do with the gdbm database
  manager occurred.
*/
string
CAlarmServer::GetExperimentHistory()
{
  // Open the alarm database, so we can read the alarm information
  GDBM_FILE DBFReader;
  if(!(DBFReader = gdbm_open(".alarmdb", 512, GDBM_READER, 0, 0))) {
    string reason = "Trying to open .alarmcount in CAlarmServer::Init()";
    CGDBMException dbme(reason, gdbm_errno);
    throw dbme;
  }
  string packet = "";  // this will be the final packet we send
  string sPiece;       // this is just a piece of the packet
  datum NextKey;       // This is the value of the next key
  datum ContentVal;    // The contents of the keyed entry
  
  // Start by retrieving the first key from the database
  datum KeyVal = gdbm_firstkey(DBFReader);
  vector<string> Entries;
  while(KeyVal.dptr) {
    
    // Now we examine every key, and "build up" our packet with
    // each entry for this experiment. We place the pieces of history
    // into a vector, which can then be sorted chronologically.
    NextKey = gdbm_nextkey(DBFReader, KeyVal);
    ContentVal = gdbm_fetch(DBFReader, KeyVal);
    sPiece = string(ContentVal.dptr);
    if(atoi(&(KeyVal.dptr)[0]) == atoi(m_sExpId.c_str())) {
      sPiece.resize(ContentVal.dsize);
      Entries.push_back(sPiece);
    }
    KeyVal = NextKey;
  }

  // Now we sort the entries by creation date, oldest first, using the STL
  // algorithm "sort", and function object SortHistory.
  sort(Entries.begin(), Entries.end(), SortHistory);
  vector<string>::iterator It;
  for(It = Entries.begin(); It != Entries.end(); It++) {
    packet += *It;
    packet += "\n";
  }

  // The displayer expects an "eof" at the end of the history text
  packet += "eof";
  gdbm_close(DBFReader);
  return packet;
}
