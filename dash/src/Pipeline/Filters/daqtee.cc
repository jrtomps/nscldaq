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

#ifndef DAQTEE_VERSION_H
#include <dshusage/daqtee_version.h>
#endif

#ifndef DAQTEE_HELP_H
#include <dshusage/daqtee_help.h>
#endif

using namespace daqhwyapi;

/**
* @var daqtee_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException daqtee_rtexception;

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
* @class DAQTee
* @brief Inject binary data into the data stream.
*
* Inject binary data into the data stream.  Binary data
* is a series of data records that consist of a leading length 
* specified as an int in host byte order. 
* This length is followed by the data.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DAQTee: public Main {
  private: unsigned long my_debug_delay;
  private: unsigned int my_packet_number;
  private: int my_retry_count;
  private: String execname;
  private: String execargs;
  private: pid_t my_child_pid;
  private: bool my_typed;
  private: range_t *my_ranges;
  private: int my_rangecnt;
  private: bool my_hasrange;

  /*==============================================================*/
  /** @fn DAQTee()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: DAQTee() { 
    my_debug_delay = 0;
    execname = "";
    execargs = "";
    my_ranges = NULL;
    my_rangecnt = 0;
    my_hasrange = false;
  }

  /*==============================================================*/
  /** @fn ~DAQTee()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~DAQTee() { }

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
  /** @fn bool in_range(record_header_t& aHdr)
  * @brief Check if a record is one of the specified packet types.
  *
  * Check if a record is one of the specified packet types.
  *
  * @param aHdr The decoded record header to use. 
  * @return True if the record is one of the specified packet types.
  */
  private: bool in_range(record_header_t& aHdr) {
    if (!my_hasrange) return true; // Pass everything

    for (int i = 0; i < my_rangecnt; i++) {
      if ((aHdr.record_type >= my_ranges[i].low)&&
          (aHdr.record_type <= my_ranges[i].high)) {
        return true;
      }
    }

    return false;
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
    bool parsing_exec = false;

    // Need at least 1 parameter
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
      if ((!parsing_exec)&&(argstr[0] == '-')) { // Option argument
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
            uint32_t prng[2];
            DSHUtils::parsePacketParam(parmstr,prng);
            addrange(prng);
            my_hasrange = true;
          } else {
            cerr << "Argument \"--packet\" requires an integer, symbolic or range parameter." << endl;
            return false;
          }
        } else { // Unknown paramter
          cerr << "Argument \"" << (argstr.c_str()) << "\" is unrecognized." << endl;
          return false; 
        }
      } else { // Must be command name
        if (!parsing_exec) {
          parsing_exec = true;
          execname = argstr;
        } else {
          if (execargs.size() > 0) execargs += " ";
          execargs += argstr;
          if (parmstr.size() > 0) {
            execargs += "=";
            execargs += parmstr;
          } 
        }
      }
      i++;
    }

    if (execname.size() <= 0) return false;
    else return true;
  }

  /*==============================================================*/
  /** @fn void killChild(pid_t pid)
  * @brief Kill a child process.
  *
  * Kill a child process.  This method starts by sending a 
  * SIGTERM to the specified process.  If the child still exists,
  * a SIGKILL is sent after a short periord.
  *
  * @param pid The pid to kill.
  * @return None.
  */
  private: void killChild(pid_t pid) {
    if (::kill(pid,0) >= 0) { // Still exists
      ::kill(pid,SIGTERM);
      ::sleep(1);  // one second
      if (::kill(pid,0) >= 0) { // Still exists
        ::kill(pid,SIGKILL);  // It's either dead or we can not kill it.
      }
    }
  }
 
  /*==============================================================*/
  /** @fn int startProgram(pid_t& childpid,String& file,String& args)
  * @brief Fork and start a program.
  *
  * Fork and start a program.  
  *
  * @param childpid The child's pid or -1.
  * @param file The program to execute.
  * @param args Arguments to pass to the program.
  * @return The file descriptor attached to the child's stdin or -1 on error.
  * @throw RuntimeException On fork or other system error.
  */
  private: int startProgram(pid_t& childpid,String& file,String& args) {
    int childin[2] = {-1,-1}; // Child stdin
    childpid = -1;

    // Create the child's stdin pipe
    if (::pipe(childin) < 0) {
      int eno = errno;
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      throw daqtee_rtexception.format(CSTR("DAQTee::startProgram() failed to create pipe msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
    } 

    // Fork off a child
    if ((my_child_pid = ::fork()) < 0) {
      int eno = errno;
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      ::close(childin[0]); 
      ::close(childin[1]); 
      throw daqtee_rtexception.format(CSTR("DAQTee::startProgram() failed to fork child process pipe msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
    } else {
      if (my_child_pid > 0) { // Parent process
        childpid = my_child_pid; 
        ::close(childin[0]); // Close the read-side so we get an eof
        return(childin[1]);
      } else {  // Child process
        // Duplicate the child's stdin on a pipe
        if (::dup2(childin[0],0) < 0) {
          int eno = errno;
          char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
          throw daqtee_rtexception.format(CSTR("DAQTee::startProgram() failed to dup2 child's stdin msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
        }

        // Close the unused file descriptors
        ::close(childin[1]);

        // Start the program. 
        System.executeProgram(file,args);
      } 
    }

    return(-1);
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
      throw daqtee_rtexception.format(CSTR("DAQTee::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw daqtee_rtexception.format(CSTR("DAQTee::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The daqtee main loop reads records
  * from stdin and writes records with the specified packet types
  * to stdout.
  * 
  * @param None
  * @return None
  * @throw RuntimeException On child startup and other errors.
  */
  private: void mainloop() {
    bool have_beginrun = false;  // True when we see a BEGINRUN record.
    bool ignore_tee = false;  // True when injector should not be started.

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

    // No zombies, please
    ::signal(SIGCHLD,SIG_IGN); 

    // Start the program
    pid_t childpid = -1;
    int childin = startProgram(childpid,execname,execargs);

    if (childin < 0) {
      throw daqtee_rtexception.format(CSTR("DAQTee::mainloop() Failed to start child program \"%s\" with arguments \"%s\""),execname.c_str(),execargs.c_str());
    }

    // Ok, create an output stream
    FdOutputStream fchildin(childin);

    // Start reading records.
    while (!finread.eof()) {
      int recsiz = finread.readRecord(rechdr,recdata);

      // No need to...
      if (recsiz > 0) { // Write header and data...
        if ((!have_beginrun)&&(rechdr.record_type != Record::type_beginrun)) {
          String thetyp = Record::packetTypeToString(rechdr.record_type);
          throw daqtee_rtexception.format(CSTR("DAQTee::mainloop() First record read from stdin is not a BEGINRUN record.  Record has type of %d \"%s\""),rechdr.record_type,thetyp.c_str());
        } else if (!have_beginrun) { // First record
          have_beginrun = true;
        }

        // Write to the main output stream
        write_record(foutput,rechdr,&recdata);
        foutput.flush();

        // Try to write to the child
        if (!ignore_tee) {
          // Is the record is in range?
          if (in_range(rechdr)) {
            try {
              write_record(fchildin,rechdr,&recdata);
              fchildin.flush();
            } catch(...) {
              // Got an error, assume child has died... clean up.
              ignore_tee = true; 
              killChild(childpid); 
              childpid = -1;
              // Close remaining file descriptors
              ::close(childin);
              childin = -1;
            }
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
 
    // Do we want to attach a debugger?
    if (my_debug_delay > 0) {
      System.attachDebugger(argv[0],my_debug_delay);
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
  
    // Go!
    mainloop();
  }
};

DAQTee daqtee;
