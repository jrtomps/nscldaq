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
#include <dshusage/daqtail_version.h>
#endif

#ifndef DAQCAT_HELP_H
#include <dshusage/daqtail_help.h>
#endif

using namespace daqhwyapi;

/**
* @var daqtail_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException daqtail_rtexception;

#define SEGMENTER_SEGFILE_FMAT "run%04d_%04d.evt"
#define SEGMENTER_SEGFILE_FSIZE 64

#define COLLATE_CTIME 1 
#define COLLATE_RUNNUMBER 2

#define SLEEP_DELAY 500

/*===================================================================*/
/**
* @class DAQTail
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
class DAQTail: public Main {
  private: unsigned long my_debug_delay;
  private: String dirpath;
  private: unsigned long cur_run_number;
  private: long cur_seq_number;
  private: FileInputStream *finput;
  private: BufferedRecordReader *reader;
  private: int collate_type;
  private: bool catchup;
  private: bool filemode;
    
  /*==============================================================*/
  /** @fn DAQTail()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: DAQTail() {
    my_debug_delay = 0;
    cur_run_number = (unsigned int)-1;
    cur_seq_number = -1;
    finput = NULL;
    reader = NULL;
    collate_type = COLLATE_CTIME;
    catchup = false;
    filemode = false;
  }

  /*==============================================================*/
  /** @fn ~DAQTail()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~DAQTail() {
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
        } else if (argstr.equals("--catchup")) { 
          catchup = true; 
        } else if (argstr.equals("--file")) { 
          filemode = true; 
        } else if (argstr.equals("--collate")) { 
          if (parmstr.equalsIgnoreCase("ctime")) {
            collate_type = COLLATE_CTIME;
          } else if (parmstr.equalsIgnoreCase("runnumber")) {
            collate_type = COLLATE_RUNNUMBER;
          } else {
            cerr << "Argument \"--collate\" requires a string parameter of either 'ctime' or 'runnumber'." << endl;
            return false;
          }
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } else { // Must be the directory name
        dirpath = argstr;
      }
      i++;
    }

    if (dirpath.size() <= 0) return false; // Must have a directory

    if (!filemode) {
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

    return true;
  }

  /*==============================================================*/
  /** @fn unsigned long get_current_run(String& dpath)
  * @brief Find the most recent run in the specified directory.
  *
  * Find the most recent run in the specified directory and return
  * the run number.
  *
  * @param dpath The directory to search.
  * @return The most recent run number or (unsigned long)-1 on error.
  */
  private: unsigned long get_current_run(String& dpath) {
    IntArray runs;

    // Get sorted list of run numbers.
    if (collate_type == COLLATE_RUNNUMBER) {  // Sort by run number
      if (!DSHUtils::getDirectoryRunNumbers(runs,dpath)) {
        return (unsigned long)-1;
      } 
    } else { // Sort by file ctime
      if (!DSHUtils::getDirectoryRunCTimes(runs,dpath)) {
        return (unsigned long)-1;
      }
    }

    // Make sure we found some run files
    if (runs.length <= 0) return (unsigned long)-1;

    // Sorted in ascending order -- most recent is last
    unsigned long crun = runs.elements[runs.length-1];
    return crun; 
  }

  /*==============================================================*/
  /** @fn long get_current_segment(String& dpath,unsigned long rnum)
  * @brief Find the most recent segment in the specified directory.
  *
  * Find the most recent segment in the specified directory, belonging
  * to the specified run and return the segment number.
  *
  * @param dpath The directory to search.
  * @param rnum The desired run number.
  * @return The most recent segment number or -1 on error.
  */
  private: long get_current_segment(String& dpath,unsigned long rnum) {
    long lsegnum = -1;
    long segnum = -1;
    String segfile;

    // Loop until we cannot find another segment 
    do { 
      // Move on to next segment
      lsegnum = segnum;
      segnum++;

      // Add the directory path
      segfile = dpath;

      // File name work space
      char filename[SEGMENTER_SEGFILE_FSIZE]; 

      // Add the run and sequence numbers
      snprintf(filename,SEGMENTER_SEGFILE_FSIZE,SEGMENTER_SEGFILE_FMAT,rnum,segnum);

      // Append to the directory path
      segfile += filename;

    } while(FSUtils::pathExists(segfile));

    if (lsegnum == 0) return -1;
    return lsegnum;
  }

  /*==============================================================*/
  /** @fn void waitfor_next_segment()
  * @brief Wait for the next segment to be created.
  *
  * Wait for the next segment to be created.
  *
  * @param None
  * @return None
  */
  private: void waitfor_next_segment() {
    long segnum = cur_seq_number + 1;

    // Loop until the next segment appears
    for (;;) {
      // Current segment file name work space
      String segfile(dirpath);

      // File name work space
      char filename[SEGMENTER_SEGFILE_FSIZE]; 

      // Add the run and sequence numbers
      snprintf(filename,SEGMENTER_SEGFILE_FSIZE,SEGMENTER_SEGFILE_FMAT,cur_run_number,segnum);

      // Append to the directory path
      segfile += filename;

      // If the path exists break out of the loop, else sleep a bit
      if (FSUtils::pathExists(segfile)) break; 
      else usleep(SLEEP_DELAY);
    }
  }

  /*==============================================================*/
  /** @fn void waitfor_next_run()
  * @brief Wait for the next run to be started.
  *
  * Wait for the next run to be started.  This will set 
  * the cur_run_number class variable to the next run number.
  *
  * @param None
  * @return None
  */
  private: void waitfor_next_run() {
    unsigned long rnum = get_current_run(dirpath);

    // Loop until the next run shows up
    while (rnum == cur_run_number) {
      rnum = get_current_run(dirpath);
      if (rnum == cur_run_number) usleep(SLEEP_DELAY);
    }

    // Set cur_run_number and cur_seq_number
    cur_run_number = rnum;
    cur_seq_number = -1;
  }

  /*==============================================================*/
  /** @fn void waitfor_more_data()
  * @brief Wait for more data on the current segment.
  *
  * Wait for more data to be written to the current segment
  * file.  This is determined by testing a mark position 
  * against the current file size.
  *
  * @param None
  * @return None
  */
  private: void waitfor_more_data() {
    // Input stream is not available
    if (finput == NULL) return; 
    off_t mark = finput->mark();
    off_t fsiz = FSUtils::fileSize(finput->getFD());

    while(mark == fsiz) {
      usleep(SLEEP_DELAY);
      fsiz = FSUtils::fileSize(finput->getFD());
    }

    // Check for file truncation
    if (fsiz < mark) {
      throw daqtail_rtexception.format(CSTR("DAQTail::waitfor_more_data() file has been truncated."));
    }

    // Was not truncated so clear the eof on this stream.
    finput->cleareof();
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
  /** @fn FileInputStream *open_input_stream()
  * @brief Open a file for input.
  *
  * Open a file for input.
  *
  * The input stream returned needs to be deleted by the caller
  * when the caller is finished with the stream.
  *
  * @param None
  * @return A new file input stream or NULL if the file does not exist.
  */
  private: FileInputStream *open_input_stream() {
    // Current file name work space
    String datafile(dirpath);

    // If the file does not exits return NULL
    if (!FSUtils::pathExists(datafile)) return NULL;

    // Create the file input stream
    FileInputStream *fin = new FileInputStream(datafile);

    return fin;
  }

  /*==============================================================*/
  /** @fn void tail_stream(FileInputStream *finput,FileOutputStream *foutput)
  * @brief Tail process a single input stream.
  *
  * Tail a single input stream.
  *
  * @param finput The input stream to tail process.
  * @param foutput The output stream.
  * @return None
  */
  private: void tail_stream(FileOutputStream *foutput) {
    if (finput == NULL) {
      throw daqtail_rtexception.format(CSTR("DAQTail::tail_stream() input stream is NULL"));
      return;
    }

    // If we're not catching up we need to fast forward
    bool ffwd = !catchup;

    // If fast forwarding we need the current file size
    off_t fsiz = 0;

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // We need to read until we exceed the current file size
    if (ffwd) fsiz = FSUtils::fileSize(finput->getFD());

    // Create the reader
    if (reader != NULL) delete reader;
    reader = new BufferedRecordReader(*finput);

    // Tail the file and wait for a record to be ready
    while (!reader->tailReady()) {
      waitfor_more_data();
    }

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
      throw daqtail_rtexception.format(CSTR("DAQTail::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw daqtail_rtexception.format(CSTR("DAQTail::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The daqtail main loop does the following.
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
    // If we're not catching up we need to fast forward
    bool ffwd = !catchup;

    // If fast forwarding we need the current file size
    off_t fsiz = 0;

    // Attach stdout.
    FdOutputStream foutput(1);

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // First find the current run according to the collating sequence
    if (!filemode) {
      cur_run_number = get_current_run(dirpath);
      // If we didn't find a run then wait for one to appear
      if (cur_run_number == (unsigned long)-1) waitfor_next_run();

      // If we're not in catchup mode then find the current segment
      if (!catchup) {
        cur_seq_number = get_current_segment(dirpath,cur_run_number);
        // Subtract 1 since next_seg_file will increment cur_seq_number
        if (cur_seq_number >= 0) cur_seq_number--;
      }
    }

    // Start reading records.
    for (;;) { // Break this loop explicitly
      // If finput is NULL, get the next segment
      if (finput == NULL) {
        if (filemode) finput = open_input_stream();
        else finput = next_seg_file();

        if (finput == NULL) {
          throw daqtail_rtexception.format(CSTR("DAQTail::mainloop() could not open segment=%d of run=%d in directory \"%s\""),cur_seq_number,cur_run_number,dirpath.c_str());
          return;
        }

        // We need to read until we exceed the current file size
        if (ffwd) fsiz = FSUtils::fileSize(finput->getFD());

        // Create the reader
        if (reader != NULL) delete reader;
        reader = new BufferedRecordReader(*finput);
      }

      // Tail the file and wait for a record to be ready
      while (!reader->tailReady()) {
        waitfor_more_data();
      }

      // Read a record
      int recsiz = reader->readRecord(rechdr,recdata);
      if (recsiz < 0) continue;  // EOF on reader
     
      // If fast forwarding, skip records until we exceed the
      // number of bytes available when this loop began is
      // exceeded, and then process the last record.
      if (ffwd && (fsiz > 0)) {
        if (fsiz > recsiz) {
          // Need to keep fast forwarding.
          fsiz -= recsiz;
          continue;
        } else {
          // Don't need to ffwd anymore, output current record.
          fsiz = 0;
          ffwd = false; 
        }
      } 
    
      if ((rechdr.record_type == Record::type_endrun)|| 
          (rechdr.record_type == Record::type_badend)) { // ENDRUN || BADEND
        write_record(foutput,rechdr,&recdata);
        foutput.flush();
        finish_input();   // Finish up with the input stream

        if (!filemode) {
          waitfor_next_run();  // Wait for the next run to show up
          continue; // Skip rest of the loop 
        } else {
          break;  // Exit the loop
        }
      } else if (rechdr.record_type == Record::type_continue) {
        foutput.flush();
        finish_input();   // Finish up with the input stream
  
        if (!filemode) {
          waitfor_next_segment(); // Wait for the next segment to be created
          continue; // Skip rest of the loop
        } else {
          break; // Exit the loop
        }
      } 

      // All records are simply written to stdout.  We open existing 
      // segment files as input as needed along the way.
      if (recsiz > 0) { // Write header and data
        write_record(foutput,rechdr,&recdata);
        foutput.flush();
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

DAQTail daqtail;
