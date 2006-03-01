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

#ifndef ASCIIONRAMP_VERSION_H
#include <dshusage/asciionramp_version.h>
#endif

#ifndef ASCIIONRAMP_HELP_H
#include <dshusage/asciionramp_help.h>
#endif

using namespace daqhwyapi;

/**
* @var asciionramp_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException asciionramp_rtexception;

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
* @class ASCIIOnRamp
* @brief Inject ASCII data into the data stream.
*
* Inject ASCII data into the data stream.  ASCII data
* is a series of data records that consist of data followed
* a separator character.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class ASCIIOnRamp: public Main {
  private: unsigned long my_debug_delay;
  private: unsigned int my_packet_number;
  private: int my_retry_count;
  private: String execname;
  private: String execargs;
  private: pid_t my_child_pid;
  private: String my_separators;

  /*==============================================================*/
  /** @fn ASCIIOnRamp()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: ASCIIOnRamp() { 
    my_debug_delay = 0;
    my_packet_number = 0;
    my_retry_count = -1;  // Only start once, 0 == restart forever
    execname = "";
    execargs = "";
    my_child_pid = -1;
    my_separators = '\n';
  }

  /*==============================================================*/
  /** @fn ~ASCIIOnRamp()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~ASCIIOnRamp() { }

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
    bool parsing_exec = false;
    bool have_packet_number = false;

    // Need at least 2 parameters
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
            my_packet_number = StringUtils::parseInteger(parmstr);
            have_packet_number = true;
          } else {
            cerr << "Argument \"--packet\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--oneliner")) { 
          my_separators = '\n';
        } else if (argstr.equals("--separator")) { 
          if (parmstr.size() > 0) {
            my_separators = parmstr;
            DSHUtils::convertEscapeCharacters(my_separators);
          } else {
            cerr << "Argument \"--separator\" requires a character parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--restart")) { 
          if (parmstr.size() > 0) {
            my_retry_count = StringUtils::parseInteger(parmstr);
            if (my_retry_count < 0) {
              cerr << "Argument \"--restart\" requires an integer parameter >=0." << endl;
              return false;
            }
          } else {
            my_retry_count = 0;
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
    else if (!have_packet_number) return false;
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
  /** @fn void startProgram(PipeInjector& injector,String& file,String& args)
  * @brief Fork and start an injector program.
  *
  * Fork and start an injector program.  
  *
  * @param injector The injector to use.
  * @param file The program to execute.
  * @param args Arguments to pass to the program.
  * @return None.
  * @throw RuntimeException On fork or other system error.
  */
  private: void startProgram(PipeInjector& injector,String& file,String& args) {
    int childout[2] = {-1,-1}; // Child stdout 
    int childerr[2] = {-1,-1}; // Child stderr 

    // Create the child's stdout pipe
    if (::pipe(childout) < 0) {
      int eno = errno;
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::startProgram() failed to create pipe msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
    } 

    // Create the child's stderr pipe
    if (::pipe(childerr) < 0) {
      int eno = errno;
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::startProgram() failed to create pipe msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
    }

    // Fork off a child
    if ((my_child_pid = ::fork()) < 0) {
      int eno = errno;
      char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
      ::close(childout[0]); 
      ::close(childout[1]); 
      ::close(childerr[0]); 
      ::close(childerr[1]); 
      throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::startProgram() failed to fork child process pipe msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
    } else {
      if (my_child_pid > 0) { // Parent process
        ::close(childout[1]); // Close the write-side so we get an eof
        ::close(childerr[1]);  // Close the write-side

        // Connect stderr
        // FdOutputStream fmainerr(2,0); // unbuffered
        FdOutputStream fmainerr(2); 
        FdInputStream fchildinerr(childerr[0]);
        injector.connect(fchildinerr,fmainerr);

        // And a LineRecordReader
        FdInputStream fchildout(childout[0]);
        LineRecordReader linereader(fchildout);
        linereader.setSeparators(my_separators);
        injector.connect(linereader);

        // Start the injector  
        injector.start();

        // Flush the error output stream 
        fmainerr.flush(); 

        // Injector has finished, make sure child is dead.
        killChild(my_child_pid); 
        my_child_pid = -1;

        // Close remaining (read-side) unneeded file descriptors
        ::close(childerr[0]);
        ::close(childout[0]);
      } else {  // Child process
        // Duplicate the child's stdout on a pipe
        if (::dup2(childout[1],1) < 0) {
          int eno = errno;
          char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
          throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::startProgram() failed to dup2 child's stdout msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
        }

        // Duplicate the child's stderr on a pipe
        if (::dup2(childerr[1],2) < 0) {
          int eno = errno;
          char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
          throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::startProgram() failed to dup2 child's stderr msg=\"%s\" rc=%d"),strerror_r(eno,ebuf,DAQHWYAPI_STRERROR_SIZE),eno);
        }

        // Close the unused file descriptors
        ::close(childerr[0]);
        ::close(childout[0]);

        // Start the program. 
        System.executeProgram(file,args);
      } 
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
      throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::write_record() failed to write record header."));
    }

    // Write the record data
    if (data != NULL) {
      if (data->length > 0) {
        rc = aStream.write(data->elements,data->length);
        if (rc < data->length) {
          throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::write_record() failed to write record data."));
        }
      }
    }
  }

  /*==============================================================*/
  /** @fn void mainloop()
  * @brief Run the main loop.
  *
  * Run the main loop. The asciionramp main loop reads records
  * from stdin and writes records with the specified packet types
  * to stdout.
  * 
  * @param None
  * @return None
  */
  private: void mainloop() {
    bool have_beginrun = false;  // True when we see a BEGINRUN record.
    bool ignore_injector = false;  // True when injector should not be started.
    int retry_count = my_retry_count;

    if (retry_count < 0) retry_count = 1; // Restart once

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

      // No need to...
      if (recsiz > 0) { // Write header and data...
        if ((!have_beginrun)&&(rechdr.record_type != Record::type_beginrun)) {
          String thetyp = Record::packetTypeToString(rechdr.record_type);
          throw asciionramp_rtexception.format(CSTR("ASCIIOnRamp::mainloop() First record read from stdin is not a BEGINRUN record.  Record has type of %d \"%s\""),rechdr.record_type,thetyp.c_str());
        } else if (!have_beginrun) { // First record
          have_beginrun = true;
          write_record(foutput,rechdr,&recdata);
          foutput.flush();
          continue; // Go around again
        }

        // If we are ignoring the injector, just read and write records.
        if (ignore_injector) {
          write_record(foutput,rechdr,&recdata);
          foutput.flush();
        } else { // Let an injector do the dirty work
          ::signal(SIGCHLD,SIG_IGN); // No zombies, please

          while ((!ignore_injector)&&(!finread.eof())) {
            // Now create and start the injection process.
            PipeInjector injector;

            // Set packet type
            injector.setRecordType(my_packet_number);

            // Connect the in/out streams and the reader
            injector.connect(fstdin);
            injector.connect(foutput);

            // Start the program
            startProgram(injector,execname,execargs);

            // Flush the output stream 
            foutput.flush(); 

            if ((retry_count > 0)&&(my_retry_count != 0)) {
              retry_count--;
              if (retry_count <= 0) ignore_injector = true;
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
  
    // Go!
    mainloop();
  }
};

ASCIIOnRamp asciionramp;
