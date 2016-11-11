/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file multiaggman.cpp
 * @brief Main program for the multi-node aggregator.
 */
#include "multiaggregator.h"
#include <cstdlib>
/**
 *  For more information about this program and how it works, see
 *  the comments in CMultiaggregator.h
 */

int main(int argc, char** argv)
{
  // Process command line parameters.
  
  gengetopt_args_info parsedArgs;
  
  cmdline_parser(argc, argv, &parsedArgs);  // Exits with error on failure.
  
  const char* subService = parsedArgs.subscribe_arg;
  const char* pubService = parsedArgs.publish_arg;
  int         discoveryInterval = parsedArgs.discovery_period_arg;
  
  // Create the application object and run it.
  
  
  // We should not exit:
  
  return EXIT_FAILURE;
}
