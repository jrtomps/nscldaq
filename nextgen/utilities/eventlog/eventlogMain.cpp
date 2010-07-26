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
#include "eventlogMain.h"
#include "eventlogargs.h"



#include <CRingBuffer.h>

#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRemoteAccess.h>
#include <DataFormat.h>
#include <CAllButPredicate.h>

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>


using std::string;
using std::cerr;
using std::endl;


// constant defintitions.

static const uint64_t K(1024);
static const uint64_t M(K*K);
static const uint64_t G(K*M);

///////////////////////////////////////////////////////////////////////////////////
//
// Constructor and destructor  are basically no-ops:

EventLogMain::EventLogMain() :
  m_pRing(0),
  m_eventDirectory(string(".")),
  m_segmentSize((static_cast<uint64_t>(1.9*G))),
  m_exitOnEndRun(false)
{
}

EventLogMain::~EventLogMain()
{}
//////////////////////////////////////////////////////////////////////////////////
//
// Object member functions:
//


/*!
   Entry point is pretty simple, parse the arguments, 
   Record the data
*/
int
EventLogMain::operator()(int argc, char**argv)
{
  parseArguments(argc, argv);
  recordData();

}

///////////////////////////////////////////////////////////////////////////////////
//
// Utility functions...well really this is where all the action is.
//

/*
** Open an event segment.  Event segment filenames are of the form
**   run-runnumber-segment.evt
**
** runnumber - the run number. in %04d
** segment   - The run ssegment in %02d
**
** Note that all files are stored in the directory pointed to by
** m_eventDirectory.
**
** Parameters:
**     runNumber   - The run number.
**     segment     - The segment number.
**
** Returns:
**     The file descriptor or exits with an error if the file could not be opened.
**
*/
int
EventLogMain::openEventSegment(uint32_t runNumber, unsigned int segment)
{
  // Create the filename:

  string fullPath  = m_eventDirectory;

  char nameString[1000];
  sprintf(nameString, "/run-%04d-%02d.evt", runNumber, segment);
  fullPath += nameString;

  int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 
		S_IWUSR | S_IRUSR | S_IRGRP);
  if (fd == -1) {
    perror("Open failed for event file segment"); 
    exit(EXIT_FAILURE);
  }
  return fd;
  
} 


/*
**
** Record the data.  We're ready to roll having verified that
** we can open an event file and write data into it, the ring is also 
** open.
*/
void
EventLogMain::recordData()
{
  // if we are in one shot mode, indicate to whoever started us that we are ready to
  // roll.  That file goes in the event directory so that we don' thave to keep hunting
  // for it like we did in ye olde version of NSCLDAQ:

  if (m_exitOnEndRun) {
    string startedFile = m_eventDirectory;
    startedFile += "/.started";
    int fd = open(startedFile.c_str(), O_WRONLY | O_CREAT,
		  S_IRWXU );
    if (fd == -1) {
      perror("Could not open the .started file");
      exit(EXIT_FAILURE);
    }
    close(fd);
  }

  // Now we need to hunt for the BEGIN_RUN item:

  bool warned = false;
  CRingItem* pItem;
  CAllButPredicate all;

  // Loop over all runs.

  while(1) {

    // If necessary, hunt for the begin run.

    while (1) {
      pItem = CRingItem::getFromRing(*m_pRing, all);
      if (pItem->type() == BEGIN_RUN) break;
      
      if (!warned) {
	warned = true;
	cerr << "**Warning - first item received was not a begin run. Skipping until we get one\n";
      }
    }
    // Now we have the begin run item:
    
    CRingStateChangeItem item(*pItem);
    recordRun(item);
    delete pItem;    
    
    // Return/exit after making our .exited file if this is a one-shot.

    if (m_exitOnEndRun) {
      string exitedFile = m_eventDirectory;
      exitedFile       += "/.exited";
      int fd = open(exitedFile.c_str(), O_WRONLY | O_CREAT,
		S_IRWXU);
      if (fd == -1) {
	perror("Could not open .exited file");
	exit(EXIT_FAILURE);
	return;
      }
      return;
    }


  }


  
}

/*
** Record a run to disk.  This must
** - open the initial event file segment
** - Write items to the segment keeping track of the segment size,
**   opening new segments as needed until: 
** - The end run item is gotten at which point the run ends.
**
*/
void
EventLogMain::recordRun(const CRingStateChangeItem& item)
{
  unsigned int segment        = 0;
  uint32_t     runNumber      = item.getRunNumber();
  uint64_t     bytesInSegment = 0;
  int          fd             = openEventSegment(runNumber, segment);
  CAllButPredicate p;

  CRingItem*   pItem = new CRingStateChangeItem(item);
  uint16_t     itemType;

  while(1) {

    size_t size    = pItem->getBodySize() + sizeof(RingItemHeader);
    itemType       = pItem->type();

    // If necessary, close this segment and open a new one:

    if ( (bytesInSegment + size) > m_segmentSize) {
      close(fd);
      segment++;
      bytesInSegment = 0;

      fd = openEventSegment(runNumber, segment);
    }
    ssize_t n = write(fd, pItem->getItemPointer(), size);
    if (n != size) {
      int err =errno;
      cerr << "Unable to output a ringbuffer item : "  << strerror(err) << endl;
      exit(-1);
    }
    bytesInSegment  += size;

    delete pItem;

    if(itemType == END_RUN) break;

    pItem =  CRingItem::getFromRing(*m_pRing, p);

  } 

  close(fd);

    
}

/*
** Parse the command line arguments, stuff them where they need to be
** and check them for validity:
** - The ring must exist and be open-able.
** - The Event segment size, if supplied must decode properly.
** - The Event directory must be writable.
**
** Parameters:
**   argc  - Count of command words.
**   argv  - Array of pointers to the command words.
*/
void
EventLogMain::parseArguments(int argc, char** argv)
{
  gengetopt_args_info parsed;
  cmdline_parser(argc, argv, &parsed);

  // Figure out where we're getting data from:

  string ringUrl = defaultRingUrl();
  if(parsed.source_given) {
    ringUrl = parsed.source_arg;
  }
  // Figure out the event directory:

  if (parsed.path_given) {
    m_eventDirectory = string(parsed.path_arg);
  }

  if (parsed.oneshot_given) {
    m_exitOnEndRun = true;
  }

  // And the segment size:

  if (parsed.segmentsize_given) {
    m_segmentSize = segmentSize(parsed.segmentsize_arg);
  }

  // The directory must be writable:

  if (!dirOk(m_eventDirectory)) {
    cerr << m_eventDirectory 
	 << " must be an existing directory and writable so event files can be created"
	 << endl;
    exit(EXIT_FAILURE);
  }

  // And the ring must open:

  try {
    m_pRing = CRingAccess::daqConsumeFrom(ringUrl);
  }
  catch (...) {
    cerr << "Could not open the data source: " << ringUrl << endl;
    exit(EXIT_FAILURE);
  }

}

/*
** Return the default URL for the ring.
** this is tcp://localhost/username  where
** username is the name of the account running the program.
*/
string
EventLogMain::defaultRingUrl() const
{
  string url = "tcp://localhost/";

  // Figure out who we are:

  uid_t id = getuid();
  struct passwd* pwd = getpwuid(id);
  string user(pwd->pw_name);

  url += user;
  return url;
  
}

/*
** Given a segement size string either returns the size of the segment in bytes
** or exits with an error message.
**
** The form of the string can be one of:
**    number   - The number of bytes.
**    numberK  -  The number of kilobytes.
**    numberM  - The number of megabytes.
**    numberG  - The number o gigabytes.
*/
uint64_t
EventLogMain::segmentSize(const char* pValue) const
{
  char* end;
  uint64_t size = strtoull(pValue, &end, 0);

  // The remaning string must be 0 or 1 chars long:

  if (strlen(end) < 2) {
    
    // If there's a multiplier:

    if(strlen(end) == 1) {
      if    (*end == 'g') {
	size *= G;
      } 
      else if (*end == 'm') {
	size *= M;
      }
      else if (*end == 'k') {
	size *= K;
      }
      else {
	cerr << "Segment size multipliers must be one of g, m, or k" << endl;
	exit(EXIT_FAILURE);
      }

    }
    // Size must not be zero:

    if (size == (uint64_t)0) {
      cerr << "Segment size must not be zero!!" << endl;
    }
    return size;
  }
  // Some conversion problem:

  cerr << "Segment sizes must be an integer, or an integer followed by g, m, or k\n";
  exit(EXIT_FAILURE);
}

/*
** Determine if a path is suitable for use as an event directory.
** - The path must be to a directory.
** - The directory must be writable by us.
**
** Parameters:
**  dirname - name of the proposed directory.
** Returns:
**  true    - Path ok.
**  false   - Path is bad.
*/
bool
EventLogMain::dirOk(string dirname) const
{
  struct stat statinfo;
  int  s = stat(dirname.c_str(), &statinfo);
  if (s) return false;		// If we can't even stat it that's bad.

  mode_t mode = statinfo.st_mode;
  if (!S_ISDIR(mode)) return false; // Must be a directory.

  return !access(dirname.c_str(), W_OK | X_OK);
}

  
