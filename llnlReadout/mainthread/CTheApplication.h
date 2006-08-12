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

#ifndef __CTHEAPPLICATION_H
#define __CTHEAPPLICATION_H

using namespace std;
#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif


/*!
   This object is the main thread for the application.
   This thread includes the command processing thread as well
   as the software to start up threads that continuously run like
   - The buffer output thread
   - The TCP/IP listener thread.
   The application will manage serveral Tcl interpreters in order
   to maintain the Tcl apartment model of threading (only the thread
   which starts an interpreter can 'use' it):
   - command - will process commands from stdin.
   - config  - At the start of run, a tcl interpreter will be started
               by the command thread and used to interpret the
               configuration file to create the appropriate set of
               readout objects.
   - Tcp     - Each socket server will create a Tcl interpreter.
               That interpreter will be used to process commands
               sent to the interpreter from the remote system.
    
*/

     

#endif
