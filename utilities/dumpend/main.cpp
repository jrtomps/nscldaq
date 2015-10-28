/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   mian.cpp
# @brief  entry point for the dumpend program.
# @author <fox@nscl.msu.edu>
*/

#include "options.h"
#include "Application.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
/**
 * main
 *   Entry point. Process the parameters, create an application functor and
 *   invoke it.
 */

int main(int argc, char** argv)
{
  gengetopt_args_info info;
  if (cmdline_parser(argc, argv, &info)) {
    std::cerr << "Failed to parse the command line parameters\n";
    exit(EXIT_FAILURE);
  }
  
  Application app(info);
  app();
  exit(EXIT_SUCCESS);
}