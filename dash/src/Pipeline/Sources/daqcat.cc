/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif

#ifndef DAQCAT_VERSION_H
#include <dshusage/daqcat_version.h>
#endif

#ifndef DAQCAT_HELP_H
#include <dshusage/daqcat_help.h>
#endif

using namespace daqhwyapi;

/**
* @var daqcat_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException daqcat_rtexception;

#define SEGMENTER_SEGFILE_FMAT "run%04d_%04d.evt"
#define SEGMENTER_SEGFILE_FSIZE 64

/*===================================================================*/
/**
* @class DAQCat
* @brief Dump records from segmented event files.
*
* Dump records from segmented event files.  That is, this program
* will follow existing event file segments and dump records to
* stdout.  This program terminates when either an ENDRUN or BADEND
* record is found or when the next segment does not exist.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DAQCat: public Main {
  private: unsigned long my_debug_delay;
  private: String dirpath;
  private: unsigned long cur_run_number;
  private: long cur_seq_number;
  private: FileInputStream *finput;
  private: BufferedRecordReader *reader;
  private: bool have_run_number;
    
  /*==============================================================*/
  /** @fn DAQCat()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: DAQCat() {
    my_debug_delay = 0;
    have_run_number = false;
    cur_run_number = (unsigned int)-1;
    cur_seq_number = -1;
    finput = NULL;
    reader = NULL;
  }

  /*==============================================================*/
  /** @fn ~DAQCat()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~DAQCat() {
    // In case the reader is still around.
    if (reader != NULL) {
      reader->close();
      delete reader;
      reader = NULL;
    }

    // In case the input file has been left open and should be
    // closed.
    if (finput != NULL) {
      finput->close();
      delete finput;
      finput = NULL;
    }
  }

  /*==============================================================*/
  /** @fn void usage()
  * @brief Display program usage.
  *
  * Display program usage.
  *
  * @param None
  * @return None
  */
  private: void usage() {
    cerr << help_string << endl;
  }

  /*==============================================================*/
  /** @fn void version()
  * @brief Display program version.
  *
  * Display program version information.
  *
  * @param None
  * @return None
  */
  private: void version() {
    cerr << version_string << endl;
  }

  /*==============================================================*/
  /** @fn bool parseCommandline(int argc,char *argv[])
  * @brief Parse the program command line.
  *
  * Parse the program command line and set parameters
  * as required.
  *
  * @param argc Number of commandline arguments including command name.
  * @param argv Array of commandline arguments.
  * @return If command line parsing was successful.
  */
  private: bool parseCommandline(int argc,char *argv[]) {
    // Need at least a direcory path 
    if (argc < 2) {
      return false;
    }

    // Parse the command line arguments
    int i = 1;  // Skip command name
    while (i < argc) { 
      String argstr;
      String parmstr;

      // Split arguments with options.
      if (argv[i] != NULL) {
        char *p = strchr(argv[i],'='); 
        if (p != NULL) {
          (*p) = '\0'; p++;
          argstr = argv[i];
          parmstr = p;
          (*(--p)) = '=';
        } else {
          argstr = argv[i];
        }
      }

      // Parse argment strings
      if (argstr[0] == '-') { // Option argument
        if (argstr.equals("--version")) { 
          version();
          exit(0);
        } else if (argstr.equals("--help")) { 
          usage();
          exit(0);
        } else if (argstr.equals("--debug")) { 
          if (parmstr.size() > 0) {
            my_debug_delay = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--debug\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--run")) { 
          if (parmstr.size() > 0) {
            cur_run_number = StringUtils::parseInteger(parmstr);
            have_run_number = true;
          } else {
            cerr << "Argument \"--run\" requires an integer parameter." << endl;
            return false;
          }
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } else { // Must be the directory name
        dirpath = argstr;
        if (dirpath.size() > 0) {
          // Add the trailing slash
          const char *dp = dirpath.c_str();
          if (dp[dirpath.size()-1] != '/') {
            dirpath += '/';
          }

          if (!FSUtils::isaDirectory(dirpath)) {
            cerr << "Specified path \"" << (dirpath.c_str()) << "\" is not an accessible directory." << endl;
            return false;
          }
        }
      }
      i++;
    }

    if (dirpath.size() <= 0) return false; // Must have a directory
    else if (!have_run_number) return false; // Need a run number
    else return true;
  }

  /*==============================================================*/
  /** @fn FileInputStream *next_seg_file()
  * @brief Select next new segment input file.
  *
  * Select the next segment input file using the current run number.
  * The current segment number gets incremented.
  *
  * The input stream returned needs to be deleted by the caller
  * when the caller is finished with the stream.
  *
  * @param None
  * @return A new file input stream or NULL if the segment does not exist.
  */
  private: FileInputStream *next_seg_file() {
    // Increment the sequence number
    cur_seq_number++;

    // Current segment file name work space
    String segfile(dirpath);

    // File name work space
    char filename[SEGMENTER_SEGFILE_FSIZE]; 

    // Add the run and sequence numbers
    snprintf(filename,SEGMENTER_SEGFILE_FSIZE,SEGMENTER_SEGFILE_FMAT,cur_run_number,cur_seq_number);

    // Append to the directory path
    segfile += filename;

    // If the file does not exits return NULL
    if (!FSUtils::pathExists(segfile)) return NULL;

    // Create the file input stream
    FileInputStream *fin = new FileInputStream(segfile);

    return fin;
  }

  /*==============================================================*/
  /** @fn void finish_input()
  * @brief Do finish processing on the input stream and reader.
  *
  * Do end of segment (finish) processing on the input stream
  * and buffered input reader.  Closes both the input stream and
  * the reader and then deletes them.  The input stream and
  * the reader are set to NULL.
  *
  * @param None
  * @return None
  */
  private: void finish_input() {
    // First process the reader if it is not NULL.
    if (reader != NULL) {
      reader->close();
      delete reader;
      reader = NULL; 
    }

    // Now close and delete the input stream.
    if (finput != NULL) {
      finput->close();
      delete finput;
      finput = NULL; 
    } 
  }

  /*==============================================================*/
  /** @fn void write_record(FdOutputStream& aStream,record_header_t& aHdr,ByteArray *data)
  * @brief Write a record to an output stream.
  *
  * Write a record to an output stream.  If the data pointer is NULL, only
  * the record header will be written.
  *
  * @param aStream The output stream to which to write.
  * @param aHdr The decoded record header to use. 
  * @param data A pointer to the record data.
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_record(FdOutputStream& aStream,record_header_t& aHdr,ByteArray *data) {
    // Declare a buffer for encoding records.
    ubyte hdrbuf[Record::encode_buffer_size];
    Record::encodeHeader(hdrbuf,Record::encode_buffer_size,aHdr);

    // Write the record header
    int rc = aStream.write(hdrbuf,Record::encode_buffer_size);
    if (rc < Record::encode_buffer_size) {
      throw daqcat_rtexception.format(CSTR("DAQCat::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw daqcat_rtexception.format(CSTR("DAQCat::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The daqcat main loop does the following.
  * 
  *   1. Starting with the first segment of a run, the method will
  *      dump complete records to stdout.
  *   2. If a ENDRUN or BADEND record is found, The record is
  *      dump to stdout and the method returns.
  *   3. If the next segment in a run does not exist, this method
  *      returns immediately.
  *
  * If the next segment cannot be found and an ENDRUN or BADEND
  * record has not been encountered, a RuntimeException will be
  * thrown.
  *
  * @param None
  * @return None
  * @throw RuntimeException If a run segment is missing.
  */
  private: void mainloop() {
    // Attach stdout.
    FdOutputStream foutput(1);

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // Start reading records.
    for (;;) { // Break this loop explicitly
      // If finput is NULL, get the next segment
      if (finput == NULL) {
        finput = next_seg_file();
        if (finput == NULL) {
          throw daqcat_rtexception.format(CSTR("DAQCat::mainloop() could not open segment=%d of run=%d in directory \"%s\""),cur_seq_number,cur_run_number,dirpath.c_str());
          return;
        }
        if (reader != NULL) delete reader;
        reader = new BufferedRecordReader(*finput);
      }

      // Are we at eof on this segment?
      if (reader->eof()) {
        finish_input();
        continue; // Skip the rest of the loop
      }

      // Read a record
      int recsiz = reader->readRecord(rechdr,recdata);
     
      if ((rechdr.record_type == Record::type_endrun)|| 
          (rechdr.record_type == Record::type_badend)) { // ENDRUN || BADEND
        write_record(foutput,rechdr,&recdata);
        foutput.flush();
        foutput.close();  // Close the output stream
        finish_input();   // Finish up with the input stream
        return; // Get out of here -- program should exit.
      } else if (rechdr.record_type == Record::type_continue) {
        // Just ignore CONTINUENEXTFILE records.  We should get
        // an EOF to trigger switching to the next segment.  Also,
        // daqcat does not attempt to follow new segment creation, so
        // if there is no next segment and we have not seen an ENDRUN
        // or BADEND record we will issue a message (exception) and
        // exit.
        continue; // Skip rest of the loop
      } 

      // All records are simply written to stdout.  We open existing 
      // segment files as input as needed along the way.
      if (recsiz > 0) { // Write header and data
        write_record(foutput,rechdr,&recdata);
      }
    }
  }

  /*==============================================================*/
  /** @fn void main(int argc,char *argv[]) 
  * @brief Mainline startup method.
  *
  * Mainline startup method.
  *
  * @param argc Number of commandline arguments including command name.
  * @param argv Array of commandline arguments.
  * @return None  
  */
  public: void main(int argc,char *argv[]) {
    if (!parseCommandline(argc,argv)) {
      usage();
      exit(-1);
    }
  
    // Do we want to attach a debugger?
    if (my_debug_delay > 0) {
      System.attachDebugger(argv[0],my_debug_delay);
    }
  
    // Everything appears ok.
    mainloop();
  }
};

DAQCat daqcat;
