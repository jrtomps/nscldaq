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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";//
//   Event logger: Responsible for logging event files in the
//                 local directory.
//   Author:
//      Ron Fox
//      NSCL
//      Michigan State University
//      East Lansing, MI 48824-1321
//      (c) Copyright NSCL 2000 All rights reserved.
//
//
// Usage:
// eventlog [-help]|[-source url][-ftp ftpurl][-user username] [-pass password]
//          [-one]
//   -help        -- Prints a help message (essentially usage) and then exits.
//   -source url  -- Sets the url of the data source defaults to 
//                   localhost's spectrodaq server.
//   -one         -- Indicates a single run should be recorded and then the
//                   program exits (used in the stager environment.
//
// Modifications:
//    November 2000:
//       Keep file open state and run number state to detect the following:
//       - Run started when we are activated
//       - Readout producer exited and restarted.
//    March, April 2001:
//       Allow saving data across ftp link.
//
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <Iostream.h>
#include <Fstream.h>
#include <Iomanip.h>
#include <buftypes.h>
#include <buffer.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ftplib.h>
#include <string.h>		// C run time library string(3) functions.
#include <string>		// STL C++ string class.

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <CopyrightNotice.h>
#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Define the legal switches:
//
enum Switch {
  Help,
  SourceUrl,
  FtpUrl,
  UserName,
  Password,
  OneRun,
  Unrecognized
};
struct SwitchEntry {
  const char* pSwitchName;
  Switch      nSwitch;
};

SwitchEntry SwitchTable[] = {
  {"-help",   Help},
  {"-source", SourceUrl},
  {"-ftp",    FtpUrl},
  {"-user",   UserName},
  {"-pass",   Password},
  {"-one",    OneRun}
};
static const int nswitches = sizeof(SwitchTable)/sizeof(SwitchEntry);

// How big a buffer do we want?

#define BUFLEN 0

static char *Months[] = {
"January ",
"February ",
"March ",
"April ",
"May ",
"June ",
"July ",
"August ",
"September ",
"October ",
"November ",
"December "
};

static const int MegaByte = 1024*1024;

#ifdef DEBUG
static 
const int knSegmentSize = 3;       // Mbytes per segment in debug.
#else
static 
const int knSegmentSize = 2000;    // Megabytes per segment (less than 2 gig).
#endif
///////////////////////////////////////////////////////////////////////
//
// MatchSwitch:
//   Matches parameter text with a switch in the switchtable.
//
Switch MatchSwitch(char* pSwitch) 
{
  SwitchEntry* pSwitches = SwitchTable;
  for(int i = 0; i < nswitches; i ++) {
    if(strcmp(pSwitch, pSwitches->pSwitchName) == 0) {
      return pSwitches->nSwitch;
    }
    pSwitches++;
  }
  return Unrecognized;
}
///////////////////////////////////////////////////////////////////////
//
// isControl:
//   Determine if buffer is a control type buffer (has a timestamp e.g.).
//
inline int
isControl(unsigned short type)
{
  type = type > 0 ? type : -type; // abs type.
  return ( (type == BEGRUNBF)  ||
	   (type == ENDRUNBF)  ||
	   (type == PAUSEBF)   ||
	   (type == RESUMEBF));
}

///////////////////////////////////////////////////////////////////////
//
// TimeStamp:
//   Put out a timestamp on a stream from a bftime struct:
//
void TimeStamp(ostream& str, bftime& tod)
{
  // Note Months in bftime start from 1 not zero. 

	str << Months[tod.month-1] << tod.day << ", ";
	str << tod.year << " ";


	int OldFill = str.fill('0');
	int OldWidth= str.width(2);
	str << tod.hours << ":" << tod.min << ":" << tod.sec << endl;
	str.fill(OldFill);
	str.width(OldWidth);

}
/*==================================================================*/
// 
// CopyOut - Copy a bunch of data out of a word buffer.
//
void CopyOut(void* pDest, DAQWordBufferPtr psrc, unsigned int nWds)
{
  DAQWordBufferPtr pSrc(psrc);
  short* pWDest = (short*)pDest;
  for(int i = 0; i < nWds; i++) {
    *pWDest++ = *pSrc;
    ++pSrc;			// With objects ++pre is faster than post++
  }
}

/*===================================================================*/
class DAQBuff : public DAQROCNode {
private:
  netbuf*    pControl;		// Ftp Control connection
  netbuf*    pData;		// Ftp data connection.
  DAQURL sinkurl;
  DAQURL ftpurl;
  bool   ftpValid;
  string Username;
  string Password;
  int    eventFd;		// File descriptor of event file.
  bool   OneRun;
  int	 m_nBuffersPerSegment;
  int	 m_nRemainingInSegment;
  long   m_lUnsampledSink;

public:
  DAQBuff() : DAQROCNode(),	// Constructor.
    pControl(0),
    pData(0),
    sinkurl( "TCP://localhost:2700/"),
    ftpValid(0),
    Username("anonymous"),
    Password("user@nscl.msu.edu"),
    eventFd(-1),
    OneRun(0)
  {
    FtpInit();			// Initialize ftp package.
  }
private:
  void Usage();
  void ParseFlags(int argc, char** argv);
  void OpenEventFile(int nRun, int nBufferSize,int nSegment=0);
  void CloseEventFile();
  void ComputeSegmentSize(int nBufSize) {
	float Filesize = (float)MegaByte * (float)knSegmentSize;
	float BufsPerSeg = Filesize / (float)nBufSize;
	m_nBuffersPerSegment = (int)BufsPerSeg;
  }

  int operator()(int argc,char **argv) {

    int nSegment(0);
    // Output copyright notice and author credit:

    CopyrightNotice::Notice(cerr, argv[0], 
			    "2.0", "2002");
    CopyrightNotice::AuthorCredit(cerr, argv[0],
				  "Ron Fox",
				  "Eric Kasten",
				  (char*)NULL);


    //
    DAQWordBuffer bbuf(BUFLEN);

    
    ParseFlags(argc, argv);	// Parse program options.
    
    // Tag this buffer so we know what type of buffer to receive.

    SetProcessTitle("NSCLEventLog");   // Don't show uname/pwd.
    bbuf.SetTag(2);




    // Add a sink for this tag

    m_lUnsampledSink = daq_link_mgr.AddSink(sinkurl,3,2, COS_RELIABLE);

    // If the sinkid == 0, then the AddSink failed.
    cerr << "Added Sink Id " << m_lUnsampledSink << endl;
    if (m_lUnsampledSink <= 0) {
      cerr << "Failed to add a sink." << endl;
      exit(-1);
    }

    // Receive buffers ad infinitum.

    bool     FileOpen = FALSE;
    bool     LastRunNumberValid = FALSE;
    INT16      LastRunNumber = -1;

    //
    // Let experiment manager know when we are ready to roll.
    //
    if(OneRun) {
      ofstream *ready = new ofstream(".ready");
      *ready << endl;
      delete ready;
    }

    while(1) {



      // Accept a buffer (with wait).
      do {
	bbuf.Accept();
      } while(bbuf.GetLen() == 0);
      //
      // Should at least be able to pull out the buffer header:
      //
      assert(bbuf.GetLen() > sizeof(bheader)/sizeof(short));
      DAQWordBufferPtr pBuf = &bbuf;
      
      int nLength = bbuf.GetLen();


      bheader  Header;
      bheader* pHeader(&Header);

      CopyOut(&Header,pBuf,sizeof(Header)/sizeof(short));

      struct {
	bheader Header;
	ctlbody ControlBody;
      } ControlBuf;

      if(isControl(Header.type)) {
	CopyOut(&ControlBuf, pBuf, sizeof(ControlBuf)/sizeof(short));
      }
      ctlbody* pBody(&(ControlBuf.ControlBody));


      char    Filename[100];

      // Buffer type specific processing:

      if(pHeader->type == BEGRUNBF) {
	TimeStamp(cerr, pBody->tod);
	cerr << "Begin run received for run "
	     << pHeader->run << endl;
	cerr << pBody->title << endl;
	cerr.flush();

	//
	//  The following must be handled:
	//  1. Actual new run on closed fd.
	//  2. Begin run on open fd - run number the same - continue
	//                                                  appending.
	//  3. Begin run on open fd - run number different. - close
	//                                                   and reopen.
	if(!FileOpen) {		// Actual new run on closed fd.
	  nSegment = 0;
	  OpenEventFile(pHeader->run, nLength);
	  FileOpen = TRUE;
	  LastRunNumber  = pHeader->run;
	  LastRunNumberValid = TRUE;


	}
	else if(LastRunNumberValid && (LastRunNumber == pHeader->run)) {
	  // Begin run on open fd -- run number same continue appending.
	  
	  cerr << ">>Additional begin run for currently open run<<\n";
	  cerr << "  Data will be appended to current runfile\n";
	}    
	else {			// Begin run no match however:
	  cerr << ">>Begin run received for new run while event file open\n";
	  cerr << "  Closing current event file and opening new file\n";
	  CloseEventFile();
	  nSegment = 0;
	  OpenEventFile(pHeader->run, nLength);
	  FileOpen = TRUE;
	  LastRunNumber  = pHeader->run;
	  LastRunNumberValid = TRUE;
	 }
   }
    else {			// If not open, then do an emergency open.
	if(!FileOpen) {
	  time_t epochtime = time(&epochtime);
	  char   timebuffer[64];
	  cerr << ctime_r(&epochtime, timebuffer);

	  cerr << ">>Received a non-begin run buffer while file closed\n";
	  cerr << "  Opening a new event file in spite of this.\n";
	  cerr << "  The culprit is run: " << pHeader->run << endl;
	  sprintf(Filename, "run%u-%d.evt", pHeader->run, nLength);
	  nSegment = 0;               // Figure we joined in progress.
	  OpenEventFile(pHeader->run, nLength);
	  FileOpen = TRUE;
	  LastRunNumber  = pHeader->run;
	  LastRunNumberValid = TRUE;
	}
   }

    if(bbuf.Write(eventFd, 0, nLength) !=  nLength) {
	cerr.flush();
	perror("Write to event data file failed!!");
	daq_link_mgr.DeleteSink(m_lUnsampledSink);
	exit(errno);
      }	   
      if(pHeader->type == ENDRUNBF) {
	assert(eventFd != -1);	// File should be open.
	TimeStamp(cerr, pBody->tod);
	cerr << "End run received for run "
	     << pHeader->run << endl;
	cerr << pBody->title << endl;
	cerr.flush();
	CloseEventFile();
	FileOpen = FALSE;
	LastRunNumberValid = FALSE;
	if(OneRun) {		// Program is operating in single mode
	  ofstream *done = new ofstream(".done");
	  *done << endl;
	  delete done;
	  daq_link_mgr.DeleteSink(m_lUnsampledSink);
	  exit(0);
	}
      }
      m_nRemainingInSegment--;
      if(m_nRemainingInSegment <= 0) {
	nSegment++;
#ifdef DEBUG
      	cerr << "Need to make a new segment"<< endl;
#endif
	CloseEventFile();
#ifdef DEBUG
	cerr << "Old event file closed\n";
#endif
	OpenEventFile(pHeader->run, nLength, nSegment);
#ifdef DEBUG
	cerr << "New event file opened\n";
#endif
	m_nRemainingInSegment = m_nBuffersPerSegment;
      }
      
      bbuf.Release();		// Release implicitly resizes the buffer to 0
				// so we won't deadlock.

    }

  //sleep(10);
  // Delete the sink.
    daq_link_mgr.DeleteSink(m_lUnsampledSink);
  } 
};
////////////////////////////////////////////////////////////////////////
//
//  Usage:
//     Send program usage information to stderr.
//
void 
DAQBuff::Usage() 
{
  cerr << "Usage:\n";
  cerr << "  eventlog [-help]|[-source url] [-ftp url] [-user name]" <<
    " [-pass pwd]\n";
  cerr << "Where:\n";
  cerr << "  -help     : Causes this message to be printed to sdterr.\n";
  cerr << "  -source   : Causes the next parameter to be used as the data\n";
  cerr << "              source URL.  Defaults to tcp://localhost:2700/ \n";
  cerr << "  -ftp      : Causes the next parameter to be taken as the \n";
  cerr << "              url specifying the directory into which event files\n";
  cerr << "              will be ftp'd.  If omitted, files are stored in the\n";
  cerr << "              current working directory.  Example:\n";
  cerr << "                 tcp://daq1.nscl.msu.edu:ftp/usr/DAQ/event1\n";
  cerr << " -user      : The next parameter is taken as the ftp login name\n";
  cerr << "              defaults to anonymous which usually won't work.\n";
  cerr << " -pass      : The next parameter is taken as the password for\n";
  cerr << "              the ftp login.\n";
  cerr << " -one       : Requests that only the next run be recorded\n";
  cerr << "              Once that file is recorded, eventlog will create\n";
  cerr << "              a file named .done and exit.  This is normally\n";
  cerr << "              only by the stager to determine if the run file \n";
  cerr << "              has been closed.\n";
}
/////////////////////////////////////////////////////////////////////
//
//  ParseFlags:
//    Parse the input flags.
//
void 
DAQBuff::ParseFlags(int argc, char** argv)
{
  argc--; argv++;		// Skip program name.
  while(argc) {
    Switch sw = MatchSwitch(*argv);
    argc--; argv++;		// Skip to next param.
    switch(sw) {
    case Help:
      Usage();			// Help means print usage info.
      exit(0);			// and exit... nothing more.
    case SourceUrl:
      if(argc) {
	sinkurl = *argv;	// Parse a new URL.
	argc--; argv++;
      }
      else {
	cerr << " -source requires a URL.parameter.\n";
	Usage();
	exit(-1);		// Command slicing error.
      }
      break;
    case FtpUrl:
      if(argc) {
	ftpurl =*argv;
	ftpValid = TRUE;
	argc--; argv++;
      }
      else {
	cerr << "-ftp requires an ftp url parameter\n";
	Usage();
	exit(-1);
      }
      break;
    case ::UserName:
      if(argc) {
	Username = *argv;
	argv++; argc--;
      }
      else {
	cerr << "-user requires a username parameter.\n";
	Usage();
	exit(-1);
      }
      break;
    case ::Password:
      if(argc) {
	Password = *argv;
	argv++; argc--;
      }
      else {
	cerr << "-pass requires a password parameter\n";
	exit(-1);
      }
      break;
    case ::OneRun:
      OneRun = TRUE;
      break;
    case ::Unrecognized:
    default:
      Usage();
      exit(-1);
    }
  }
}
///////////////////////////////////////////////////////////////////////////
//
// DAQBuff::OpenEventFile:
//    Opens an event file associated with a run and buffersize.
//    Two cases:
//      If !ftpvalid, a regular file is opened in the cwd.
//      If ftpvalid,  an ftp session is formed with the associated server
//                    the directory cd'd to and an data connection openened
//                    to PUT the data into a file in that dir.
//    In either case the file is named:  run$runnum-$bufsize.evt
//
//
void
DAQBuff::OpenEventFile(int nRun, int nBufferSize, int nSegment)
{
#ifdef DEBUG
  cerr << "Opening event file..\n";
#endif
  ComputeSegmentSize(nBufferSize * sizeof(short));
  m_nRemainingInSegment = m_nBuffersPerSegment;
#ifdef DEBUG
  cerr << m_nBuffersPerSegment << " buffers per file segment" << endl;
#endif
  char FileName[256];
  string FullFilename;
  if(nSegment == 0) {
	sprintf(FileName, "run%d-%d.evt", nRun, nBufferSize);
  } 
  else {
	sprintf(FileName, "run%d_%d-%d.evt", nRun, nSegment, nBufferSize);
  }
  cerr << "Run File is: " << FileName << endl;

  if(ftpValid) {		// Open file as ftp client.
    string Host(ftpurl.GetHostName().c_str());
    string Directory(ftpurl.GetPath().c_str());
    
    if(!FtpConnect(Host.c_str(), &pControl)) { // Connect to ftp server
      cerr << "Failed to connect to ftp host - check your ftp url.";
      daq_link_mgr.DeleteSink(m_lUnsampledSink);
      exit(errno);
    }
    if(!FtpLogin(Username.c_str(), Password.c_str(), pControl)) { // login
      cerr << "Failed to login to ftp host. - check your username/password \n";
      daq_link_mgr.DeleteSink(m_lUnsampledSink);
      exit(errno);
    }
    cerr << "Setting the directory for transfer to: " << Directory << endl;

    if(!FtpChdir(Directory.c_str(), pControl)) { // CD to login directory.
      cerr << "Failed to set directory - check your ftp url\n";
      daq_link_mgr.DeleteSink(m_lUnsampledSink);
      exit(errno);
    }
    if(!FtpAccess(FileName, FTPLIB_FILE_WRITE, FTPLIB_IMAGE, pControl,
		  &pData)) {
      cerr << "Failed to start the ftp put -- check for sufficient disk space\n";
      daq_link_mgr.DeleteSink(m_lUnsampledSink);
      exit(errno);
    }
    // Everything's working now so:

    eventFd = FtpGetFd(pData);	// Handle is the fd for the socket.
    FullFilename = ftpurl.toString().c_str();
    FullFilename += "/";
    FullFilename += FileName;
  }
  else {			// Open file in cwd.
    int fd = creat(FileName, 0x1ff);
    if(fd == -1) {
      cerr.flush();
      perror("Open for local event file failed");
      daq_link_mgr.DeleteSink(m_lUnsampledSink);
      exit(errno);
    }
    eventFd = fd;
    FullFilename = FileName;
  }
  cerr << FullFilename << " Opened for event recording\n";
}
/////////////////////////////////////////////////////////////////////////
//
//  DAQBuff::CloseEventFile:
//     Closes an open event file.  There are two possibilities:
//     1. !ftpValid  - Just close the fd, it's a local file.
//     2. ftpValid   - FtpClose current data connection to terminate xfer.
//                     FtpQuit  to quit current session and disconnect.
//
void
DAQBuff::CloseEventFile()
{
  if(ftpValid) {		// CLose ftp connection.
    cerr << "Closing ftp transfer\n";
    FtpClose(pData);
    pData = NULL;
    FtpQuit(pControl);
    pControl = NULL;
  }
  else {			// Close regular file.
    cerr << "Closing via fd\n";
    close(eventFd);
  }

  eventFd = -1;		// Mark closed.
}

DAQBuff mydaq;

