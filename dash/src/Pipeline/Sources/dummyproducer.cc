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

#ifndef DUMMYPRODUCER_VERSION_H
#include <dshusage/dummyproducer_version.h>
#endif

#ifndef DUMMYPRODUCER_HELP_H
#include <dshusage/dummyproducer_help.h>
#endif

using namespace daqhwyapi;

/**
* @var dummyproducer_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException dummyproducer_rtexception;

/**
* @var sigint_caught
* @brief Set to true when a SIGINT has been caught.
*
* Set to true by the signal handler when a SIGINT has been caught.
*/
static bool sigint_caught = false;

#define MAX_RECORD_SIZE 64536

static char *fillarray = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int fillarraylen = strlen(fillarray);

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
  sigint_caught = true;
  sigint_count++;

  // We count the number of sigints received.  If this count
  // exceeds the maximum number we want to allow, we assume that
  // the program is really stuck and abort.  Notably, this may
  // cause the generation of a BadEnd record.  
  if (sigint_count > SIGINT_MAXCNT) ::abort();
}

/*==============================================================*/
/**
* @class DummyProducer
* @brief Produces dummy DAQ records for testing.
*
* Produces dummy DAQ records for testing pipeline elements.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DummyProducer: public Main {
  private: unsigned long my_packet_number;
  private: unsigned long my_debug_delay;
  private: unsigned long my_delay;
  private: unsigned long my_run_number;
  private: unsigned long my_record_size;
  private: String my_title;
  private: FdOutputStream my_fdout;
  private: BufferedRecordWriter *writer;

  /*==============================================================*/
  /** @fn DummyProducer()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: DummyProducer() { 
    my_packet_number = Record::type_physics;
    my_debug_delay = 0;
    my_delay = 0;
    my_run_number = 1;
    my_record_size = 0;  // Random sized buffers
    my_title = "TITLE UNSET";
    writer = NULL;
  }

  /*==============================================================*/
  /** @fn ~DummyProducer()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~DummyProducer() {
    if (writer != NULL) {
      delete writer;
      writer = NULL;
    }

    if (my_fdout.getFD() >= 0) {
      my_fdout.flush();
      my_fdout.close();
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
    bool haverun = false;

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
        } else if (argstr.equals("--size")) { 
          if (parmstr.size() > 0) {
            my_record_size = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--size\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--delay")) { 
          if (parmstr.size() > 0) {
            my_delay = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--delay\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--run")) { 
          if (parmstr.size() > 0) {
            my_run_number = StringUtils::parseInteger(parmstr);
            haverun = true;
          } else {
            cerr << "Argument \"--run\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--title")) { 
          if (parmstr.size() > 0) {
            my_title = parmstr;
          } else {
            cerr << "Argument \"--title\" requires a string parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--packet")) {
          if (parmstr.size() > 0) {
            const char *p = parmstr.c_str();
            if (!isdigit(*p)) {  // Must be a symbolic value
              my_packet_number = Record::stringToPacketType(parmstr);
            } else { // Must be an integer
              my_packet_number = StringUtils::parseInteger(parmstr);
            }
          } 
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } 
      i++;
    }

    // Need a run number
    if (!haverun) {
      return false;
    }

    return true;
  }

  /*==============================================================*/
  /** @fn void write_beginrun()
  * @brief Write a BEGINRUN record to the output stream.
  *
  * Write a BEGINRUN record to an output stream.  
  *
  * @param None
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_beginrun() {
    // Declare a buffer for encoding records.
    record_header_t hdr;
    Record::initHeader(hdr);
    ByteArray data; 
  
    // Pack the record 
    Record::packTimeStamped(hdr,data,Record::type_beginrun,my_run_number,my_title);

    // Add data to writer
    if (writer->write(data.elements,data.length) != data.length) {
      throw dummyproducer_rtexception.format(CSTR("DummyProducer::write_beginrun() failed to write BEGINRUN record data."));
    }

    // Write the record
    if (writer->writeRecord(hdr) != hdr.record_size) {
      throw dummyproducer_rtexception.format(CSTR("DummyProducer::write_beginrun() failed to write BEGINRUN record."));
    }
  }

  /*==============================================================*/
  /** @fn void write_endrun()
  * @brief Write an ENDRUN record to the output stream.
  *
  * Write an ENDRUN record to an output stream.  
  *
  * @param None
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_endrun() {
    // Declare a buffer for encoding records.
    record_header_t hdr;
    Record::initHeader(hdr);
    ByteArray data; 
  
    // Pack the record 
    Record::packTimeStamped(hdr,data,Record::type_endrun,my_run_number,my_title);

    // Add data to writer
    if (writer->write(data.elements,data.length) != data.length) {
      throw dummyproducer_rtexception.format(CSTR("DummyProducer::write_endrun() failed to write ENDRUN record data."));
    }

    // Write the record
    if (writer->writeRecord(hdr) != hdr.record_size) {
      throw dummyproducer_rtexception.format(CSTR("DummyProducer::write_endrun() failed to write ENDRUN record."));
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. Create records and write them to
  * stdout.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
    // Attach stdout
    my_fdout.open(1); 

    // Create the writer
    writer = new BufferedRecordWriter(my_fdout);
  
    // Create a record header
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Create a record data buffer
    ByteArray data;

    // Seed random
    ::srandom(System.currentTimeMillis());

    // Write BEGINRUN
    write_beginrun();

    // Set up the SIGINT signal handler
    ::signal(SIGINT,sigfunc_interrupt); 

    // And begin writing data
    int fcnt = 0;
    while (!sigint_caught) {
      // Wait a bit
      if (my_delay > 0) usleep(my_delay);

      // If SIGINT hasn't been caught we're not at an ENDRUN
      if (!sigint_caught) {
        int recsiz = my_record_size;
        if (recsiz <= 0) recsiz = (::random() % MAX_RECORD_SIZE);
        data.renew(recsiz); 
        memset(data.elements,fillarray[fcnt],recsiz);
        writer->write(data.elements,recsiz);
        Record::initHeader(rechdr);
        rechdr.record_type = my_packet_number;
        writer->writeRecord(rechdr);
      }
     
      fcnt++;
      if (fcnt >= fillarraylen) fcnt = 0;
    }

    // Write ENDRUN
    write_endrun();
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

DummyProducer dummyproducer;
