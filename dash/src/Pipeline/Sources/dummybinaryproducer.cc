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
#include <netinet/in.h>

#ifndef DAQHWYAPI_H
#include <dshapi/daqhwyapi.h>
#endif

#ifndef DUMMYBINARYPRODUCER_VERSION_H
#include <dshusage/dummybinaryproducer_version.h>
#endif

#ifndef DUMMYBINARYPRODUCER_HELP_H
#include <dshusage/dummybinaryproducer_help.h>
#endif

using namespace daqhwyapi;

/**
* @var dummybinaryproducer_rtexception
* @brief Exception to throw for general runtime errors.
*
* Exception to throw for for general runtime errors.
*/
static RuntimeException dummybinaryproducer_rtexception;

#define MAX_RECORD_SIZE 64536

static char *fillarray = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int fillarraylen = strlen(fillarray);

/*==============================================================*/
/**
* @class DummyBinaryProducer
* @brief Produces dummy DAQ records for testing.
*
* Produces dummy DAQ records for testing pipeline elements.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DummyBinaryProducer: public Main {
  private: unsigned long my_packet_number;
  private: unsigned long my_debug_delay;
  private: unsigned long my_delay;
  private: unsigned long my_record_size;
  private: int my_iter_count;
  private: bool my_typed;

  /*==============================================================*/
  /** @fn DummyBinaryProducer()
  * @brief Default constructor.
  *
  * Default constructor.
  *
  * @param None
  * @return this
  */
  public: DummyBinaryProducer() { 
    my_packet_number = 0;
    my_debug_delay = 0;
    my_delay = 0;
    my_record_size = 0;  // Random sized buffers
    my_iter_count = -1; // Run forever
    my_typed = false;
  }

  /*==============================================================*/
  /** @fn ~DummyBinaryProducer()
  * @brief Destructor
  *
  * Destructor.
  *
  * @param None
  * @return None
  */
  public: virtual ~DummyBinaryProducer() { }

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
        } else if (argstr.equals("--size")) { 
          if (parmstr.size() > 0) {
            my_record_size = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--size\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--iterations")) { 
          if (parmstr.size() > 0) {
            my_iter_count = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--iterations\" requires an integer parameter." << endl;
            return false;
          }
        } else if (argstr.equals("--delay")) { 
          if (parmstr.size() > 0) {
            my_delay = StringUtils::parseInteger(parmstr);
          } else {
            cerr << "Argument \"--delay\" requires an integer parameter." << endl;
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
            my_typed = true;
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
    // Create a data buffer
    ByteArray data;

    // Seed random
    ::srandom(System.currentTimeMillis());

    // And begin writing data
    int fcnt = 0;
    for (;;) {
      // Wait a bit
      if (my_delay > 0) usleep(my_delay);

      // Write length and data
      int recsiz = my_record_size;
      if (recsiz <= 0) recsiz = (::random() % MAX_RECORD_SIZE);
      data.renew(recsiz); 
      memset(data.elements,fillarray[fcnt],recsiz);
      ::write(1,(void*)(&recsiz),sizeof(int));  // Length

      if (my_typed) { // Typed records
        ::write(1,(void*)(&my_packet_number),sizeof(int)); 
      }

      ::write(1,(void*)(data.elements),recsiz); // Data 

      fcnt++;
      if (fcnt >= fillarraylen) fcnt = 0;

      // If we are iterating a fixed number of times...
      if (my_iter_count > 0) {
        my_iter_count--;
        if (my_iter_count <= 0) break;  // We are done
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

DummyBinaryProducer dummybinaryproducer;
