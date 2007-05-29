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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";
//
// This program creates a SpectroDaq data taking run by reading an event file
// and sending it to the local SpectroDaq server.  Mostly intended for
// test purposes as the data analysis software can be directly pointed at
// event files.
//
// Assumptions:
//   Buffers tagged type 2 are event buffers.
//   Buffers tagged type 3 are control buffers.
//
// Ron Fox
// NSCL
// Michigan State University
// mailto:  fox@nscl.msu.edu
//

#include <config.h>

#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <Iostream.h>
#include <Iomanip.h>
#include <buftypes.h>
#include <buffer.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <CopyrightNotice.h>
#include <libgen.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <spectrodaq.h>



/*===================================================================*/
class DAQBuff : public DAQROCNode {
  char* m_pFilename;		//!< Name of file analyzing.
  int   m_nRunNumber;		//!< Number of run being analyzed.
  int   m_nBufferWords;		//!< Size of buffer in words.
  long  m_nBuffers;		//!< Number of buffers submitted.
  int   m_fd;			//!< Fd open on the file. 
  DAQDataStore*  m_pDataStore;

public:
  DAQBuff();
  int operator()(int argc,char **argv);
  static void Usage(ostream& rstream);
  void   UpdateTitle();
  int   SubmitBuffer();
};

/*!
   Construct the entry thread:
*/
DAQBuff::DAQBuff() :
  m_pFilename(0),
  m_nRunNumber(0),
  m_nBufferWords(0),
  m_nBuffers(0),
  m_fd(-1), 
  m_pDataStore(&(DAQDataStore::instance()))
{


  int port;
  struct servent* serviceInfo = getservbyname("sdlite-buff",
                                              "tcp");
  if (serviceInfo) {
    port = ntohs(serviceInfo->s_port);
  } 
  else {
    port = 2701;
  }

                                              
  

  m_pDataStore->setSourcePort(port);


}


/*!
   Print usage information.

   \param stream  - Stream on which the usage information is put.
   */
void DAQBuff::Usage(ostream& rstream)
{
  rstream << "Usage:\n";
  rstream << "   DiskFile file\n";
  rstream << "   file: the name of the disk file to read.  The name must\n";
  rstream << "   have the format: run[n]-[size].evt\n";
  rstream << "     [n]     - The run number.\n";
  rstream << "     [size]  - The number of words in a buffer.\n";
}

/*!
   Updates the process title.  The title will be:
   Replaying run [n] : [m]
   where:
   - n: The number of the run.
   - m: The number of buffers analylzed.

   Implicit input:
   - m_nRunNumber - Run number decoded from file.
   - m_nBuffers   - Number of analyzed buffers.
   */

void DAQBuff::UpdateTitle()
{
  char title[100];
  sprintf(title, "Replaying run %d: %d", m_nRunNumber, m_nBuffers);
  SetProcessTitle(title);
}

/*! 
  Entry point for the program.  
  Usage:
     DiskFile  filename

  The filename must be of the form:  run[n]-[size].evt
  where [n] is a run number, [size] is the number of words in the 
  event file.

  \param argc  Number of command line arguments.
  \param argv  The parameters themselves.

  */
int
DAQBuff::operator()(int argc, char** argv)
{
  CopyrightNotice::Notice(cerr, argv[0], "1.0", "2002");
  CopyrightNotice::AuthorCredit(cerr, argv[0], "Ron Fox", NULL);
  if(argc != 2) {
    Usage(cerr);
    exit(-1);
  }

  // Get the filename and decode it into it's components.

  m_pFilename = new char[strlen(argv[1])+1];
  strcpy(m_pFilename, argv[1]);
  char* pBase = basename(argv[1]);

  if(sscanf(pBase, 
	    "run%d-%d.evt", &m_nRunNumber, &m_nBufferWords) != 2) {
    Usage(cerr);
    exit(-1);
  }

  // Try to open the file:

  m_fd = open(m_pFilename, O_RDONLY);
  if(m_fd < 0) {
    cerr << "Could not open " << m_pFilename << ": " << strerror(errno)
	 << endl;
    exit(errno);
  }
  m_nBuffers = 0;
  UpdateTitle();
  
  while(SubmitBuffer()) {
    m_nBuffers++;
    UpdateTitle();
  }
  close(m_fd);

  // Flush the data source:

  BufferedRecordWriter* pWriter = m_pDataStore->getSource();
  pWriter->flush();

  sleep(5);
  return 0;
}

/*!
    Read a buffer from the run file and submit it to the DAQ system.
    Implicit Inputs:
    - m_fd           File descriptor from which to issue the read.
    - m_nBufferWords Number of buffers in the word.
    Returns:
    -  0 - end of file or error.
    - -1 - buffer submitted.
 */
int
DAQBuff::SubmitBuffer()
{
  DAQWordBuffer buf(m_nBufferWords);
  if(buf.Read(m_fd, 0, m_nBufferWords) <= 0) return 0;

  // The buffer is tagged according to it's type.

  unsigned short type = buf.GetPtr()[1];
  buf.SetTag( (type == DATABF) ? 2 : 3);
  buf.Route();
  return -1;
}

DAQBuff mydaq;




