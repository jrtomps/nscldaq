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
#include <time.h>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif

#ifndef SIEVE_VERSION_H
#include <dshusage/ratemeter_version.h>
#endif

#ifndef SIEVE_HELP_H
#include <dshusage/ratemeter_help.h>
#endif

using namespace daqhwyapi;

/**
* @var ratemeter_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException ratemeter_rtexception;

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
* @class RateMeter
* @brief A pass through filter that measures data rates. 
*
* Reads a DAQ record stream from stdin and either cosume the
* record or pass it through to stdout.  Calculate rate 
* statistics and print them to stderr.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RateMeter: public Main {
  private: unsigned long my_debug_delay;
  private: unsigned long my_every_secs;
  private: bool my_consume;

  /*==============================================================*/
  /** @fn RateMeter()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: RateMeter() { 
    my_debug_delay = 0;
    my_every_secs = 10; // 10 seconds
    my_consume = false;
  }

  /*==============================================================*/
  /** @fn ~RateMeter()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~RateMeter() { }

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
        } else if (argstr.equals("--consume")) { 
          my_consume = true;
        } else if (argstr.equals("--every")) { 
          if (parmstr.size() > 0) {
            my_every_secs = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--every\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--debug")) { 
          if (parmstr.size() > 0) {
            my_debug_delay = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--debug\" requires an integer parameter." << endl;
            return false;
          }
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } 
      i++;
    }

    return true;
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
      throw ratemeter_rtexception.format(CSTR("RateMeter::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw ratemeter_rtexception.format(CSTR("RateMeter::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The ratemeter main loop reads records
  * from stdin and writes records with the specified packet types
  * to stdout.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
    // A few timing variables
    unsigned long tstart = 0;
    unsigned long tend = 0;
    double telapsed = 0;

    // Number of bytes seen so far
    unsigned long bytesread = 0;

    // Attach stdout.
    FdOutputStream foutput(1);

    // Attach stdin and create a record reader. 
    FdInputStream fstdin(0);
    BufferedRecordReader finread(fstdin);

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // If testing more than one stream a pid is useful
    pid_t pid = getpid(); 

    // Set up the SIGINT signal handler
    ::signal(SIGINT,sigfunc_interrupt); 

    // Start reading records.
    while (!finread.eof()) {
      tstart = System.currentTimeMillis();
      int recsiz = finread.readRecord(rechdr,recdata);
      
      if (recsiz > 0) { // Write header and data...
        // Add data size
        bytesread += rechdr.data_size;
        // If we are not consuming records then write them out.
        if (!my_consume) {
          write_record(foutput,rechdr,&recdata);
          foutput.flush();
        } 
      }
 
      // Include the time to write 
      tend = System.currentTimeMillis();

      // How much time has passed in seconds
      telapsed += ((double)(tend - tstart) / 1000.0);

      // Should we print some statistics?
      if (telapsed >= my_every_secs) {
        // Megabytes read
        double avgmeg = (double)bytesread / (1024.0*1024.0);

        // Megabytes per second
        avgmeg /= telapsed;

        fprintf(stderr,"%u: Average megabytes/second: %g\n",pid,avgmeg);
        bytesread = 0;
        telapsed = 0.0;
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
  
    // Go!
    mainloop();
  }
};

RateMeter ratemeter;
