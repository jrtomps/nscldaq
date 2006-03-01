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

#ifndef RECORDDUMP_VERSION_H
#include <dshusage/recorddump_version.h>
#endif

#ifndef RECORDDUMP_HELP_H
#include <dshusage/recorddump_help.h>
#endif

using namespace daqhwyapi;

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
* @class RecordDump
* @brief Reads a DAQ record stream from stdin and prints the headers.
*
* Reads a DAQ record stream from stdin and prints the headers.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class RecordDump: public Main {
  private: unsigned long my_debug_delay;
  private: unsigned int my_dump_length;
  private: bool my_print;

  /*==============================================================*/
  /** @fn RecordDump()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: RecordDump() { 
    my_debug_delay = 0;
    my_dump_length = 20;
    my_print = false;
  }

  /*==============================================================*/
  /** @fn ~RecordDump()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~RecordDump() { }

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
        } else if (argstr.equals("--dump")) { 
          if (parmstr.size() > 0) {
            my_dump_length = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--dump\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--debug")) { 
          if (parmstr.size() > 0) {
            my_debug_delay = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--debug\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--print")) { 
          my_print = true;
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
  /** @fn void dump_timestamped(record_header_t& aHdr,ByteArray& data)
  * @brief dump a time stamped record.
  *
  * Dump a time stamped record.  Used for printing BEGINRUN, ENDRUN
  * and BADEND records.
  *
  * @param aHdr The decoded record header to use. 
  * @param data A pointer to the record data.
  * @return None
  */
  private: void dump_timestamped(record_header_t& aHdr,ByteArray& data) {
    struct tm timestamp;
    unsigned long runnum = 0;
    String titlemsg;

    // Unpack the record 
    Record::unpackTimeStamped(aHdr,data,timestamp,runnum,titlemsg);

    // First get the time
    char timestr[100]; // Needs to be at least 26 bytes, but I'll be generous
    if (asctime_r(&timestamp,timestr) != NULL) {
      fprintf(stderr,"Time stamp: %s",timestr);
    } else {
      fprintf(stderr,"Time stamp: ** Could not decode **\n");
    } 

    // Now the run number
    if (runnum == (unsigned long)-1) {
      fprintf(stderr,"Run number: -1\n");
    } else { 
      fprintf(stderr,"Run number: %u\n",runnum);
    }

    // Now print the title (or message)
    if (titlemsg.size() > 0) {
      fprintf(stderr,"Title/Message: %s\n",titlemsg.c_str());
    } else {
      fprintf(stderr,"Title/Message: ** NONE **\n");
    }
  }

  /*==============================================================*/
  /** @fn void dump_record(record_header_t& aHdr,ByteArray& data)
  * @brief dump a record header to stderr.
  *
  * Dump a record header to stderr.
  *
  * @param aHdr The decoded record header to use. 
  * @param data A pointer to the record data.
  * @return None
  */
  private: void dump_record(record_header_t& aHdr,ByteArray& data) {
    String rectyp_str("");
    char *recstat_str = "Unrecognized status code";
    char *endian_str = "Little Endian";
    char dumpchars[31]; 

    // Covert type codes to human readable strings
    rectyp_str = Record::packetTypeToString(aHdr.record_type);

    // Covert status codes to human readable strings
    if (aHdr.status_code == Record::status_ok) recstat_str = "Ok";
    else if (aHdr.status_code == Record::status_err) recstat_str = "Error";
    else if (aHdr.status_code == Record::status_trunc) recstat_str = "Record truncated";
    else if (aHdr.status_code == Record::status_runshort) recstat_str = "Run short";
    else recstat_str = "Unrecognized status code";

    // Human readable Endian
    ubyte *bo = (ubyte*)&(aHdr.byte_order);
    if ((bo[0] == 4)&&(bo[1] == 3)&&(bo[2] == 2)&&(bo[3] == 1)) {
      endian_str = "Little Endian";
    } else if ((bo[0] == 1)&&(bo[1] == 2)&&(bo[2] == 3)&&(bo[3] == 4)) {
      endian_str = "Big Endian";
    } else {
      endian_str = "Byte Swap?";
    }

    fprintf(stderr,"version = %d\n",aHdr.version);
    fprintf(stderr,"record_size = %d\n",aHdr.record_size);
    fprintf(stderr,"record_type = %d %s\n",aHdr.record_type,rectyp_str.c_str());
    fprintf(stderr,"status_code = %d %s\n",aHdr.status_code,recstat_str);
    fprintf(stderr,"byte_order = %x%x%x%x %s\n",bo[0],bo[1],bo[2],bo[3],endian_str);
    fprintf(stderr,"extended_header_size = %d\n",aHdr.extended_header_size);
    fprintf(stderr,"data_size = %d\n",aHdr.data_size);
    fprintf(stderr,"entity_count = %d\n",aHdr.entity_count);
 
    if (data.length > 0) {
      unsigned int nmax = (my_dump_length <= data.length) ? my_dump_length : data.length; 
      fprintf(stderr,"data = ");
      unsigned int ncnt = 0;
      while (nmax > 0) {
        unsigned int n = (nmax <= 30) ? nmax : 30;
        nmax -= n;
        for (int i = 0; i < n; i++) {
          fprintf(stderr,"%02x",data.elements[i+ncnt]);
          if (my_print) dumpchars[i] = data.elements[i+ncnt];
        } 

        // Print as readable characters (if possible).
        if (my_print) {
          fprintf(stderr,"\n       ");
          for (int i = 0; i < n; i++) {
            if (isprint(dumpchars[i])&&(!iscntrl(dumpchars[i]))) {
              fprintf(stderr," %c",dumpchars[i]);
            } else {
              fprintf(stderr,"  ");
            }
          }
        }

        fprintf(stderr,"\n");
        if (nmax > 0) fprintf(stderr,"       ");
        ncnt += n;
      }

      if ((aHdr.record_type == Record::type_beginrun)||
          (aHdr.record_type == Record::type_endrun)||
          (aHdr.record_type == Record::type_badend)||
          (aHdr.record_type == Record::type_continue)) {
        dump_timestamped(aHdr,data);
      }
    } else {
      fprintf(stderr,"data = Header only record\n");
    }

    fprintf(stderr,"---------------------------------------------------\n");
    fflush(stderr);
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The recorddump main loop reads records
  * from stdin and writes human consumable header and some data
  * to stderr.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
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
      if (recsiz > 0) dump_record(rechdr,recdata);
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

RecordDump recorddump;
