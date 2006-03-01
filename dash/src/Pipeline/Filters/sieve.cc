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
#include <dshusage/sieve_version.h>
#endif

#ifndef SIEVE_HELP_H
#include <dshusage/sieve_help.h>
#endif

using namespace daqhwyapi;

/**
* @var sieve_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException sieve_rtexception;

// A structure for storing packet type ranges
typedef struct range_struct {
  uint32_t low;
  uint32_t high;
} range_t;

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
* @class Sieve
* @brief Selectively output records from the data stream.
*
* Reads a DAQ record stream from stdin and selectively outputs
* records to stdout.  Record selection is specified using the
* --packet command line flag to indicate the specific packet types
* to output.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Sieve: public Main {
  private: unsigned long my_debug_delay;
  private: range_t *my_ranges;
  private: int my_rangecnt;
  private: bool my_invert;
  private: bool my_hasrange;

  /*==============================================================*/
  /** @fn Sieve()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: Sieve() { 
    my_debug_delay = 0;
    my_ranges = NULL;
    my_rangecnt = 0;
    my_invert = false;
    my_hasrange = false;
  }

  /*==============================================================*/
  /** @fn ~Sieve()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~Sieve() { 
    if (my_ranges != NULL) delete[] my_ranges;
    my_ranges = NULL;
    my_rangecnt = 0;
    my_invert = false;
    my_hasrange = false;
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
  /** @fn void addrange(uint32_t rng[])
  * @brief Add a new packet range to the list of ranges.
  *
  * Add a new packet range to the list of ranges.
  *
  * @param rng The new range to add.
  * @return None
  */
  private: void addrange(uint32_t rng[]) {
    if (my_ranges == NULL) {
      my_ranges = new range_t[1];
    } else {
      range_t *oldrng = my_ranges;  
      my_ranges = new range_t[my_rangecnt+1];
      for (int i = 0; i < my_rangecnt; i++) {
        my_ranges[i] = oldrng[i];
      }
      delete[] oldrng;
    }

    my_ranges[my_rangecnt].low = rng[0]; 
    my_ranges[my_rangecnt].high = rng[1]; 
    my_rangecnt++;
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
    // Need at least one --packet parameter
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
        } else if (argstr.equals("--packet")) { 
          if (parmstr.size() > 0) {
            uint32_t prng[2];
            DSHUtils::parsePacketParam(parmstr,prng);
            addrange(prng);
            my_hasrange = true;
          } else {
            cerr << "Argument \"--packet\" requires an integer, symbolic or range parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--invert")) { 
          my_invert = true;
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

    if (!my_hasrange) {
      cerr << "sieve requires at least one packet argument." << endl;
      return false;
    }

    return true;
  }

  /*==============================================================*/
  /** @fn bool in_range(record_header_t& aHdr)
  * @brief Check if a record is one of the specified packet types.
  *
  * Check if a record is one of the specified packet types.
  *
  * @param aHdr The decoded record header to use. 
  * @return True if the record is one of the specified packet types.
  */
  private: bool in_range(record_header_t& aHdr) {
    for (int i = 0; i < my_rangecnt; i++) {
      if ((aHdr.record_type >= my_ranges[i].low)&&
          (aHdr.record_type <= my_ranges[i].high)) {
        return true;
      }
    }

    return false;
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
      throw sieve_rtexception.format(CSTR("Sieve::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw sieve_rtexception.format(CSTR("Sieve::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The sieve main loop reads records
  * from stdin and writes records with the specified packet types
  * to stdout.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
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

    // Set up the SIGINT signal handler
    ::signal(SIGINT,sigfunc_interrupt); 

    // Start reading records.
    while (!finread.eof()) {
      int recsiz = finread.readRecord(rechdr,recdata);
      if (recsiz > 0) { // Write header and data...
        if (in_range(rechdr)) { // In rage
          // If the record is in range and noninverted logic.
          if (!my_invert) { // Regular matching logic
            write_record(foutput,rechdr,&recdata);
            foutput.flush();
          }
        } else {  // Not in range
          // If the record is in range and inverted logic.  
          if (my_invert) {  // Inverted matching logic
            write_record(foutput,rechdr,&recdata);
            foutput.flush();
          }
        }
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

    // Add BEGINRUN, ENDRUN, BADEND and CONTINUE to the ranges.
    // This enables proper parsing of the data stream by consumers.
    uint32_t prng[2];
    prng[1] = prng[0] = Record::type_beginrun;
    addrange(prng);
    prng[1] = prng[0] = Record::type_endrun;
    addrange(prng);
    prng[1] = prng[0] = Record::type_badend;
    addrange(prng);
    prng[1] = prng[0] = Record::type_continue;
    addrange(prng);
    
    // Do we want to attach a debugger?
    if (my_debug_delay > 0) {
      System.attachDebugger(argv[0],my_debug_delay);
    }
  
    // Go!
    mainloop();
  }
};

Sieve sieve;
