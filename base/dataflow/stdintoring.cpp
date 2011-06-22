#include "stdintoringsw.h"
#include "CRingBuffer.h"
#include "Exception.h"

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>


using namespace std;


/**************************************************************
 * integerize:                                                *
 *   Utility function that converts a string to an integer    *
 *   while allowing the suffixes k, m, g to be kilo, mega     *
 *   and giga multipliers respectively.  Computer sizes e.g.  *
 *   k = 1024, m = 1024*1024, g = 1024*1024*1024              *
 *                                                            *
 * On failure, this will print the usage and exit with a fail *
 * code.                                                      *
 *                                                            *
 * Parameters:                                                *
 *   const char* str  - The input string                      *
 * Returns:                                                   *
 *   The integer result                                       *
 *************************************************************/

static int 
integerize(const char* str)
{
  char* endptr;

  // Decode the front part:

  int base = strtol(str, &endptr, 0); // Allow hex etc.
  
  // It's an error to not have at least one digit:

  if (endptr == str) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // If the entire string was converted return it:

  if(strlen(endptr) == 0) {
    return base;
  }
  // If there's more than one extra char, that's an error too:

  if (strlen(endptr) > 1) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // One extra character can be the multiplier

  char multiplier = *endptr;
  int value;

  switch (multiplier) {
  case 'k':
    value = base*1024;
    break;
  case 'm':
    value = base*1024*1024;
    break;
  case 'g':
    value = base*1024*1024*1024;
    break;

  default:
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }
  return value;
}

/********************************************************************
 * mainLoop:                                                        *
 *     Main loop that takes data from stdin and puts it in the ring *
 * Parameters:                                                      *
 *   std::string ring       - Name of the target ring.              *
 *   int         timeout    - Maximum time to wait for data on stdin*
 *   int         mindata    - Chunk size for reads.. which are done *
 *                            with blocking off.                    *
 *******************************************************************/

int
mainLoop(string ring, int timeout, int mindata)
{
  // Attach to the ring:

  CRingBuffer* pSource;
  try {
    pSource = new CRingBuffer(ring, CRingBuffer::producer);
  }
  catch (CException& error) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << error.ReasonText() << endl;
    return (EXIT_FAILURE);
  }
  catch (string msg) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << msg << endl;
    return (EXIT_FAILURE);
  }
  catch (const char* msg) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << msg <<endl;
    return (EXIT_FAILURE);
  }
  catch (...) {
    cerr << "stdintoring Failed to attach to " << ring << endl;
    return (EXIT_FAILURE);
  }

  CRingBuffer& source(*pSource);
  CRingBuffer::Usage use = source.getUsage();
  if (mindata > use.s_putSpace/2) {
    mindata = use.s_putSpace/2;
  }
  // In order to deal with timeouts reasonably well, we need to turn off
  // blocking on stdout.

  // #define NONBLOCKING6
  long flags = fcntl(STDIN_FILENO, F_GETFL, &flags);
  flags   |= O_NONBLOCK;
  int stat= fcntl(STDIN_FILENO, F_SETFL, flags);
  if (stat == -1) {
    perror("stdintoring Failed to set stin nonblocking");
    return (EXIT_FAILURE);
  }
  char* pBuffer = new char[mindata];

  while (1) {

    // Wait for stdin to be readable:

    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(STDIN_FILENO, &readfds);
    timeval selectTimeout;
    selectTimeout.tv_sec = timeout;
    selectTimeout.tv_usec= 0;
    int stat = select(STDIN_FILENO+1, &readfds, &writefds, &exceptfds, &selectTimeout);
    // Three cases:
    // stat = 0... just do the next pass of the loop.
    // stat = -1   an error detected by select, but could be EINTR which is ok.
    // stat = 1 .. input is ready.

    if (stat != 0) {
      if (stat < 0) {
	if (errno != EINTR) {
	  perror("Select failed");
	  return (EXIT_FAILURE);
	}
      }
      else if (stat == 1) {
	int nread = read(STDIN_FILENO, pBuffer, mindata);
	if (nread > 0) {
	  source.put(pBuffer, nread);
	}
	if (nread < 0) {
	  perror("read failed");
	  return (EXIT_FAILURE);
	}
	if (nread == 0) {
	  cerr << "Exiting due to eof\n";
	  return (EXIT_SUCCESS);	// eof on stdin.
	}
      }
      else {
	cerr << "Exiting due to select fail " << errno << endl;
	return (EXIT_FAILURE);
      }
    } 
  }
  

}

/*!
   Entry point.
*/

int main(int argc, char** argv)
{
  struct gengetopt_args_info parsed;

  int status = cmdline_parser(argc, argv, &parsed);
  if (status == EXIT_FAILURE) {
    return (status);
  }

  // There should be exactly one parameter, that is not a switch,
  // the ring name:

  if (parsed.inputs_num != 1) {
    cmdline_parser_print_help();
    exit (EXIT_FAILURE);
  }
  // extract the rin name, the timeout and the size from the 
  // parameters/defaults:
  //

  string ringname = parsed.inputs[0];
  int    timeout  = parsed.timeout_arg;
  size_t mindata  = integerize(parsed.mindata_arg);


  int exitStatus = mainLoop(ringname, timeout, mindata);

  // If requested, delete the ring on exit:

  if (parsed.deleteonexit_given) {
    CRingBuffer::remove(ringname);
  }
  exit(exitStatus);
}
