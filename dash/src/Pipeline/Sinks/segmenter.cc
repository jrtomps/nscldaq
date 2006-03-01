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

#ifndef SEGMENTER_VERSION_H
#include <dshusage/segmenter_version.h>
#endif

#ifndef SEGMENTER_HELP_H
#include <dshusage/segmenter_help.h>
#endif

using namespace daqhwyapi;

/**
* @var segmenter_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException segmenter_rtexception;

#define SEGMENTER_SEGFILE_FMAT "run%04d_%04d.evt"
#define SEGMENTER_SEGFILE_FSIZE 64

// Maximum SIGINTs before we abort.
#define SIGINT_MAXCNT 5 

/*==============================================================*/
/** @fn static void sigfunc_interrupt(int sig)
* @brief SIGINT signal handler.
*
* SIGINT signal handler.  Set the sigint_caught variable to 
* true and returns.  Also, ignores further SIGINTs.
*
* @param sig The signal number.
* @return None
*/
static void sigfunc_interrupt(int sig)
{
  static int sigint_count = 0;
  sigint_count++;

  // We count the number of sigints received.  If this count
  // exceeds the maximum number we want to allow, we assume that
  // the program is really stuck and abort.  Notably, this may
  // cause the generation of a BadEnd record.  
  if (sigint_count > SIGINT_MAXCNT) ::abort();
}

/*===================================================================*/
/**
* @class Segmenter
* @brief Segment a DAQ record stream into segments for disk storage.
*
* Segment a DAQ pipeline record stream into segments for storage on
* disk.  The program does the following:
*
*   1. Monitors stdin for records.
*   2. Upon receipt of a BEGINRUN record, segmenter extracts the
*      run number and create a file named run{run_number}_000.evt.
*   3. The BEGINRUN record is then written to this file.
*   4. Upon receipt of an ENDRUN record, segmenter writes the
*      ENDRUN record to file, closes the file and exits.
*   5. In response to a broken pipe, segmenter creates a BADEND
*      record, signifying that the run has ended, writes this record
*      to file and exits.
*   6. All other records are written to file without interpretation. 
*   7. If any write would create a file larger than 2GB, segmenter
*      closes the currently open file, increment the segment number
*      and create a new file named 
*      run{run_number}_{sequence as %04d}.evt and continues writing
*      records.  
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Segmenter: public Main {
  private: unsigned long my_debug_delay;
  private: String dirpath;
  private: unsigned long cur_run_number;
  private: long cur_seq_number;
  private: unsigned long long cur_byte_count;
  private: unsigned long long my_segsize;
  private: FileOutputStream *foutput;
  private: String titlestr;
    
  /*==============================================================*/
  /** @fn Segmenter()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: Segmenter() {
    my_debug_delay = 0;
    cur_run_number = (unsigned int)-1;
    cur_seq_number = -1;
    cur_byte_count = 0;
    my_segsize = 1024*1024*1024; // 2GB
    my_segsize *= 2;
    foutput = NULL;
    titlestr = "";
  }

  /*==============================================================*/
  /** @fn ~Segmenter()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~Segmenter() {
    // In case the output file has been left open and should be
    // flushed and closed.
    if (foutput != NULL) {
      foutput->flush();
      foutput->close();
      delete foutput;
      foutput = NULL;
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
        } else if (argstr.equals("--segsize")) { 
          if (parmstr.size() > 0) {
            my_segsize = StringUtils::parseInteger(parmstr);
            my_segsize *= (1024*1024); // Convert to bytes
          } else {
            cerr << "Argument \"--segsize\" requires an integer parameter." << endl;
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

          if (FSUtils::isaDirectory(dirpath)) {
            return true;
          } else {
            cerr << "Specified path \"" << (dirpath.c_str()) << "\" is not an accessible directory." << endl;
            return false;
          }
        }
      }
      i++;
    }

    if (dirpath.size() <= 0) return false; // Must have a directory
    else return true;
  }

  /*==============================================================*/
  /** @fn FileOutputStream *create_seg_file()
  * @brief Create a new segment output file.
  *
  * Create a new segment output file using the current run number.
  * The current segment number gets incremented and the current byte
  * count is reset.
  *
  * The output stream returned needs to be deleted by the caller
  * when the caller is finished with the stream.
  *
  * @param None
  * @return A new file output stream or NULL.
  * @throw RuntimeException if the new file already exists.
  */
  private: FileOutputStream *create_seg_file() {
    // Increment the sequence number
    cur_seq_number++;

    // Zero the current byte count
    cur_byte_count = 0;
    
    // Current segment file name work space
    String segfile(dirpath);

    // File name work space
    char filename[SEGMENTER_SEGFILE_FSIZE]; 

    // Add the run and sequence numbers
    snprintf(filename,SEGMENTER_SEGFILE_FSIZE,SEGMENTER_SEGFILE_FMAT,cur_run_number,cur_seq_number);

    // Append to the directory path
    segfile += filename;

    // Avoid overwrites of existing files
    if (FSUtils::pathExists(segfile)) {
      throw segmenter_rtexception.format(CSTR("Segmenter::create_seg_file() attempted to overwrite existing file \"%s\""),segfile.c_str());
      return NULL;
    }

    // Create the file output stream
    FileOutputStream *fout = new FileOutputStream(segfile);

    return fout;
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
      throw segmenter_rtexception.format(CSTR("Segmenter::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw segmenter_rtexception.format(CSTR("Segmenter::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void write_badend(FdOutputStream& aStream)
  * @brief Write an BADEND record to the output stream.
  *
  * Write an BADEND record to an output stream.  
  *
  * @param aStream The output stream to which to write.
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_badend(FdOutputStream& aStream) {
    // Declare a buffer for encoding records.
    record_header_t hdr;
    Record::initHeader(hdr);
    ByteArray data; 
  
    // Pack the record 
    Record::packTimeStamped(hdr,data,Record::type_badend,cur_run_number,titlestr);

    // Write the record
    write_record(aStream,hdr,&data);  
  }

  /*==============================================================*/
  /** @fn void write_continue(FdOutputStream& aStream)
  * @brief Write an CONTINUENEXTFILE record to the output stream.
  *
  * Write an CONTINUENEXTFILE record to an output stream.  
  *
  * @param aStream The output stream to which to write.
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_continue(FdOutputStream& aStream) {
    // Declare a buffer for encoding records.
    record_header_t hdr;
    Record::initHeader(hdr);
    ByteArray data; 
  
    // Pack the record 
    Record::packTimeStamped(hdr,data,Record::type_continue,cur_run_number,titlestr);

    // Write the record
    write_record(aStream,hdr,&data);  
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The segmenter main loop does the following:
  * 
  *   1. Monitors stdin for records.
  *   2. Upon receipt of a BEGINRUN record, segmenter extracts the
  *      run number and create a file named run{run_number}_000.evt.
  *   3. The BEGINRUN record is then written to this file.
  *   4. Upon receipt of an ENDRUN record, segmenter writes the
  *      ENDRUN record to file, closes the file and exits.
  *   5. In response to a broken pipe, segmenter creates a BADEND
  *      record, signifying that the run has ended, writes this record
  *      to file and exits.
  *   6. All other records are written to file without interpretation. 
  *   7. If any write would create a file larger than 2GB, segmenter
  *      writes a CONTINUENEXTFILE record, closes the currently open 
  *      file, increments the segment number and creates a new file named 
  *      run{run_number as %04d}_{sequence as %04d}.evt and continues writing
  *      records.  
  *
  * @param None
  * @return None
  * @throw RuntimeException On missing BEGINRUN record or other errors. 
  */
  private: void mainloop() {
    ::signal(SIGPIPE,SIG_IGN); // ignore sigpipes -- just check for eof
    ::signal(SIGINT,SIG_IGN); // ignore sigint -- wait for producer

    // We need to see a begin run FIRST to create a segment file...
    bool have_runnumber = false;

    // Did we get an ENDRUN record?
    bool have_endrun = false;
 
    // Attach stdin and create a record reader. 
    FdInputStream fstdin(0);
    BufferedRecordReader finread(fstdin);

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // Declare a time structure to use with record unpacking.
    struct tm rectime;

    // Set up the SIGINT signal handler
    ::signal(SIGINT,sigfunc_interrupt); 

    // Start reading records.
    while (!finread.eof()) {
      int recsiz = finread.readRecord(rechdr,recdata);
     
      // Process a BEGINRUN record
      if (rechdr.record_type == Record::type_beginrun) {  // BEGINRUN
        if (foutput != NULL) { // BEGINRUN w/o ENDRUN?
          // Write a BADEND record and close the current output file.
          write_badend(*foutput);
          foutput->flush();
          foutput->close(); // Close the output stream
          delete foutput; foutput = NULL; 
          throw segmenter_rtexception.format(CSTR("Segmenter::mainloop() Received a BEGINRUN record after run has already begun."));
          return;
        }

        cur_run_number = (unsigned int)-1;
        Record::unpackTimeStamped(rechdr,recdata,rectime,cur_run_number,titlestr);

        if (cur_run_number == (unsigned int)-1) {
          throw segmenter_rtexception.format(CSTR("Segmenter::mainloop() Incomplete BEGINRUN record received: no run number."));
          return;
        }

        have_runnumber = true;
        foutput = create_seg_file();  // Create a segment file
        if (foutput == NULL) {
          throw segmenter_rtexception.format(CSTR("Segmenter::mainloop() Failed to create a segment file for run=%d segment=%d"),cur_run_number,cur_seq_number);
        }
      } else if (rechdr.record_type == Record::type_endrun) { // ENDRUN
        if (foutput == NULL) { // ENDRUN w/o BEGINRUN?
          throw segmenter_rtexception.format(CSTR("Segmenter::mainloop() Received an ENRUN record without a first receiving a BEGINRUN record for run=%d segment=%d"),cur_run_number,cur_seq_number);
        }
        write_record(*foutput,rechdr,&recdata);
        foutput->flush();
        foutput->close();  // Close the ouput stream
        delete foutput; foutput = NULL; 
        finread.close();  // Close the reader as well
        have_endrun = true; // We got an ENDRUN record
        return; // Get out of here -- program should exit.
      } else if (!have_runnumber) { // First record is not a BEGINRUN
        throw segmenter_rtexception.format(CSTR("Segmenter::mainloop() First record was not a BEGINRUN record; received a type=%d record"),rechdr.record_type);
        return;  // Cannot continue 
      }  

      // All records are simply written out.  We create new
      // segment files as needed along the way.
      if (recsiz > 0) { // Write header and data
        // If we would exceed the maximum per file segment size,
        // then write out a CONTINUENEXTFILE record and then
        // close this file and create a new segment.
        if ((cur_byte_count+recsiz) > my_segsize) {
          write_continue(*foutput);
          foutput->flush();
          foutput->close();  // Close the ouput stream
          delete foutput; foutput = NULL; 
          foutput = create_seg_file();  // Create a segment file
        }

        write_record(*foutput,rechdr,&recdata);
        cur_byte_count += recsiz;
      }
    }

    // How did we get here...  If we didn't get an ENDRUN record
    // we need to write out a BADEND record.  Otherwise we're done.
    if (!have_endrun) { // We need to send out a BADEND record
      if (foutput != NULL) { // Output should still be around
        // We might need a new segment file...
        if ((cur_byte_count+Record::encode_buffer_size) > my_segsize) {
          foutput->flush();
          foutput->close();  // Close the ouput stream
          delete foutput; foutput = NULL; 
          foutput = create_seg_file();  // Create a segment file
        }
        write_badend(*foutput);
        foutput->flush();
        foutput->close();  // Close the ouput stream
        delete foutput; foutput = NULL; 
        finread.close();  // Close the reader as well
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

Segmenter segmenter;
