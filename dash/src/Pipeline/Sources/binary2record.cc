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
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <time.h>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif

#ifndef BINARY2RECORD_VERSION_H
#include <dshusage/binary2record_version.h>
#endif

#ifndef BINARY2RECORD_HELP_H
#include <dshusage/binary2record_help.h>
#endif

using namespace daqhwyapi;

/**
* @var binary2record_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException binary2record_rtexception;

/**
* @var sigint_caught
* @brief Set to true when a SIGINT has been caught.
*
* Set to true by the signal handler when a SIGINT has been caught.
*/
static bool sigint_caught = false;

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

/*===================================================================*/
/**
* @class Binary2Record
* @brief Convert binary data to DSH records.
*
* Convert binary data (data with a leading length as a 
* host format int) to DSH records.  This program will also generate
* a BEGINRUN and ENDRUN record for the data.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Binary2Record: public Main {
  private: unsigned long my_debug_delay;
  private: unsigned long my_packet_number;
  private: unsigned long my_run_number;
  private: String my_title;
  private: FdOutputStream my_fdout;
  private: BufferedRecordWriter *writer;
  private: bool my_typed;

  /*==============================================================*/
  /** @fn Binary2Record()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: Binary2Record() { 
    my_debug_delay = 0;
    my_packet_number = 0;
    writer = NULL;
    my_title = "TITLE UNSET";
    my_run_number = 1;
    my_typed = false;
  }

  /*==============================================================*/
  /** @fn ~Binary2Record()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~Binary2Record() { 
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
    bool have_packet_number = false;

    // Need at least 2 parameters
    if (argc < 1) {
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
        } else if (argstr.equals("--packet")) { 
          if (parmstr.size() > 0) {
            const char *p = parmstr.c_str();
            if (!isdigit(*p)) {  // Must be a symbolic value
              if (parmstr.equalsIgnoreCase("typed")) {
                my_typed = true;
              } else {
                my_packet_number = Record::stringToPacketType(parmstr);
              }
            } else { // Must be an integer
              my_packet_number = StringUtils::parseInteger(parmstr);
            }
            have_packet_number = true;
          } else {
            cerr << "Argument \"--packet\" requires an integer parameter." << endl;
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
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } 
      i++;
    }

    if (!haverun) return false;
    else if (!have_packet_number) return false;
    else return true;
  }

  /*==============================================================*/
  /** @fn void write_record(record_header_t& aHdr,ByteArray *data,int len)
  * @brief Write a record to an output stream.
  *
  * Write a record to an output stream.  If the data pointer is NULL, only
  * the record header will be written.
  *
  * @param aHdr The decoded record header to use. 
  * @param data A pointer to the record data.
  * @param len Length to write.
  * @return None
  * @throw RuntimeException On a write failure.
  */
  private: void write_record(record_header_t& aHdr,ByteArray *data,int len) {
    // Add data to writer
    if (data != NULL) {
      if (writer->write(data->elements,len) != len) {
        throw binary2record_rtexception.format(CSTR("Binary2Record::write_record() failed to write record data."));
      }
    }

    // Write the record
    if (writer->writeRecord(aHdr) != aHdr.record_size) {
      throw binary2record_rtexception.format(CSTR("Binary2Record::write_record() failed to write record."));
    }

    writer->flush();
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
      throw binary2record_rtexception.format(CSTR("Binary2Record::write_beginrun() failed to write BEGINRUN record data."));
    }

    // Write the record
    if (writer->writeRecord(hdr) != hdr.record_size) {
      throw binary2record_rtexception.format(CSTR("Binary2Record::write_beginrun() failed to write BEGINRUN record."));
    }

    writer->flush();
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
      throw binary2record_rtexception.format(CSTR("Binary2Record::write_endrun() failed to write ENDRUN record data."));
    }

    // Write the record
    if (writer->writeRecord(hdr) != hdr.record_size) {
      throw binary2record_rtexception.format(CSTR("Binary2Record::write_endrun() failed to write ENDRUN record."));
    }

    writer->flush();
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The binary2record main loop reads records
  * from stdin and writes records with the specified packet types
  * to stdout.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
    // Attach stdout
    my_fdout.open(1); 

    // Create the writer
    writer = new BufferedRecordWriter(my_fdout);

    // Attach stdin and create a record reader. 
    FdInputStream fstdin(0);
    BinaryRecordReader finread(fstdin);
    finread.setTyped(my_typed);

    // Declare and initialize a record header.
    record_header_t rechdr;
    Record::initHeader(rechdr);

    // Declare a byte array for reading the record data
    ByteArray recdata;

    // Write a BEGINRUN record
    write_beginrun();

    // Set up the SIGINT signal handler
    ::signal(SIGINT,sigfunc_interrupt); 

    // Start reading records.
    while ((!finread.eof())&&(!sigint_caught)) {
      int recsiz = finread.readRecord(rechdr,recdata);

      // No need to...
      if (recsiz > 0) { // Write header and data...
        if (!my_typed) rechdr.record_type = my_packet_number;
        write_record(rechdr,&recdata,recsiz);
      }
    } 

    // Write a ENDRUN record
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
  
    // Go!
    mainloop();
  }
};

Binary2Record binary2record;
