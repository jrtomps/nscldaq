/* CMS REPLACEMENT HISTORY, Element DIAGNOSE.C*/
/* *5    20-FEB-1990 11:22:38 FOX "Cause bug() calls to crash CPU."*/
/* *4    17-NOV-1989 13:46:08 FOX "Add FE-F-BUG to front of messages"*/
/* *3    12-SEP-1989 14:13:51 FOX "Remove panic routines"*/
/* *2    31-AUG-1989 17:45:47 FOX "Modify to work on IRONICS processor"*/
/* *1     4-FEB-1988 14:13:49 FOX "Diagnostic message reporting routines"*/
/* CMS REPLACEMENT HISTORY, Element DIAGNOSE.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element DIAGNOSE.C*/
/* *2    14-OCT-1987 11:03:30 FOX "Add putmsg function"*/
/* *1    12-OCT-1987 11:27:08 FOX "Various diagnostic routines"*/
/* DEC/CMS REPLACEMENT HISTORY, Element DIAGNOSE.C*/

/*
**++
**  FACILITY:
**
**      Data acquisition system front end master, BFMT process.
**
**  ABSTRACT:
**
**      This module contains procedures which are used to diagnose failures
**	and error conditions in processes.
**
**  AUTHORS:
**
**      Ron Fox	
**
**
**  CREATION DATE:     12-Oct-1987
**  Ported to Linux    16-Feb-1999
**--
**/




/*
**
**  INCLUDE FILES
**
**/

#include    <stdio.h>
#include    <stdlib.h>
#define	PUTMSG							       /*2*/
#include    "daqerrno.h"						       /*2*/
/*
**  Functional Description:
**    panic - put out an error message string and then abort the program
**
**  Formal Parameters:
**     const char* msg:
**           Message to emit before dying.
*/
void
panic(const char* msg)
{
  fprintf(stderr, msg);
  abort();
}  

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      bug - Print out a debugging message and exit.
**
**  FORMAL PARAMETERS:
**
**      stat - A status code which will be printed in hex
**	msg  - Pointer to a character string to print. 
**		Full text of message will be:
**
**	    msg status = 0xstat
**
**
**  SIDE EFFECTS:
**
**      Issuing process suspends. after message is issued.
**
**--
**/
bug(stat, msg)
int    stat;
char    *msg;
{
    char    outstr[80];
    int    consline();

    sprintf(outstr,"FE-F-BUG %s status = 0x%X\r\n", msg, stat);
    panic(outstr);
}

/* */								       /*2*/
/*++ */								       /*2*/
/*  FUNCTIONAL DESCRIPTION: */					       /*2*/
/* */								       /*2*/
/*      putmsg - Outputs an error text associated with a message number. */ /*2*/
/* */								       /*2*/
/*  FORMAL PARAMETERS: */					       /*2*/
/* */								       /*2*/
/*      errno - message number to outpu */			       /*2*/
/* */								       /*2*/
/*  IMPLICIT INPUTS: */						       /*2*/
/* */								       /*2*/
/*      errmsg	- Table of message text. */			       /*2*/
/* */								       /*2*/
/*-- */								       /*2*/
/*  */								       /*2*/
putmsg(errno)							       /*2*/
int    errno;							       /*2*/
{								       /*2*/
    char    *msg;						       /*2*/
								       /*2*/
    if ((errno < 0) || (errno >= NUMERRS))			       /*2*/
	msg = errmsg[NOMSG];					       /*2*/
    else							       /*2*/
	msg = errmsg[errno];					       /*2*/
								       /*2*/
    fprintf(stderr, msg);
}								       /*2*/
