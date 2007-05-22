/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include <spectrodaq.h>
#include <stdlib.h>

using namespace std;

/*!
   Implementation of the non trivial methods
   of DAQWordBufferPtr as a wrapper for the spectrodaq-lite
   Main class.
*/

/*!
   main - just calls operator()

*/
void
DAQROCNode::main(int argc, char* argv[]) 
{
  int status = (*this)(argc, argv);
  exit(status);
}
/*!
  Set a new process title.
*/
void
DAQROCNode::SetProcessTitle(const char* pTitle)
{
  String newTitle(pTitle);
  setName(newTitle);
}
void
DAQROCNode::SetProcessTitle(String title)
{
  setName(title);
}
