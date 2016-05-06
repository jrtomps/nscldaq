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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/* 
 * tkAppInit.c --
 *
 *	Provides a default version of the Tcl_AppInit procedure for
 *	use in wish and similar Tk-based applications.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: tkAppInit.cpp 1322 2007-03-09 14:06:04Z ron-fox $
 */

#include <config.h>
#include <stdio.h>
#include <iostream>
#include "tk.h"

#include <string>
#include <CPortManager.h>

#ifdef WIN32
#include <winsock.h>
#endif

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

#ifdef SOLARIS
extern int matherr();
int *tclDummyMathPtr = (int *) matherr;
#endif

#ifdef TK_TEST
EXTERN int		Tktest_Init _ANSI_ARGS_((Tcl_Interp *interp));
#endif /* TK_TEST */


using namespace std;



/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int serverport=2048;		/* Default server port. */
bool userauth=true;		// Require user authentication.

/*
** GetServerPort:  Search for -pnum and if present, modify serverport
**
*/
static void GetServerPort(int argc, char** argv)
{
  cerr << "Getserver port\n";
  argc--; argv++;
  string AppName("TclServer");
  string thePort = "";

  int    port;
  for(int i =0; i < argc; i++) {
    string param(argv[i]);
    cerr << "Processing parameter: '" << param << "'\n";
    if( (param[0] == '-') && (param[1] == 'a')) {
      cerr << "Setting appname. \n";
      AppName.assign(param, 2, param.size()-1);
      cerr << "Appname is now: " << AppName << endl;
    }
    if (param == "-userauth") {
      cerr << "Setting userauth true\n";
      userauth = true;
    }
    if ((param[0] == '-') && (param[1] == 'p')) {
      thePort.assign(param, 2, param.size() - 1);
    }

  }

  // Now assign the port pased on the values of thePort and
  // if needed appname:

  // Empty port : default value:

  if (thePort == "") {
    return;
  }
  // Port == Managed - use the port manager:
  //        Use new since once destruction happens port manager
  //        drops our port.
  //
  if (thePort == string("Managed")) {
    CPortManager& manager = *(new CPortManager("localhost"));
    cerr << "Allocating a port for " << AppName << endl;
    serverport = manager.allocatePort(AppName);
    return;
  }

  // Otherwise needs to be a number:
  
  if(sscanf(thePort.c_str(), "%d", &port) != 1) {
    fprintf(stderr, 
	    "Warning ignored improperly formatted port switch: '%s'\n",
	    thePort.c_str());
  } else
    serverport = port;
}

int
main(int argc,char** argv)
{
#ifdef WIN32
	WSADATA Implementation;
	WORD wVersionRequested = MAKEWORD(0xff,0xff);
	int nErrorStatus = WSAStartup(wVersionRequested, &Implementation);
	if (nErrorStatus != 0) {
		cerr << "Unable to start winsock\n";
		return nErrorStatus;
	}
	else {
		cerr << "Winsock " << Implementation.wHighVersion 
			 << " started\n" << Implementation.szDescription << endl;
	}
#endif
	GetServerPort(argc,argv);		/* Modify server port as needed. */
	Tk_Main(argc, argv, Tcl_AppInit);
#ifdef WIN32
	WSACleanup();
#endif
    return 0;			/* Needed only to prevent compiler warning. */
}





