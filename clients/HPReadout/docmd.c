/*
**++
**  FACILITY:
**
**      Data acquisition system front end, master processes, RCTL
**
**  ABSTRACT:
**
**      This module contains command dispatching procedures for the
**	run control command processor.  The first token in the command
**	string is used to select a command processor from a call table.
**	All command processors are passed the residual line.
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  CREATION DATE:     14-Oct-1987
**  Ported to Linux    16-Feb-1999     Ron Fox.
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include    <stdio.h>
#include    <ctype.h>
#include    "daqinterface.h"
#include    "cmdio.h"
#include    "daqerrno.h"


#include <string.h>

static char* pCopyright = 
"docmd.c: (c) Copyright NSCL 1999, All rights reserved\n";

#ifndef MAXLINE
#define MAXLINE   256
#endif

/*	    Command tables		*/

#define    NCMDS    7				/* Number of defined cmds   */


static const char   *cmdtbl[NCMDS] = {		/* Command match tbl.	    */
			   "BEGIN",		/* Begin run.	*/
			   "END",		/* End run.	*/
			   "PAUSE",		/* Pause run.	*/
			   "RESUME",		/* Resume run.	*/
			   "SET",		/* Set something    */
			   "SHOW",		/* Show something   */
			   "EXIT"               /* Stop the system.  */
		       };
static int    (*cmddsp[NCMDS])() = 
{
  bgrun,
  endrun,
  pauserun,
  resrun,
  setdsp,
  showdsp,
  doexit
};			/* Table of processor ptrs. */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      docmd - Parse first token from command line and dispatch to command
**		processing routines.  Action is as follows:
**
**		Get first command line token.
**		If NOT NULL THEN
**		  Look up token in command table
**		  IF found THEN
**		    Call command processor via table
**		      Passing residual string as argument.
**		  ELSE
**		    putmsg(ILLCMD)
**		  ENDIF
**		ENDIF
**
**
**  FORMAL PARAMETERS:
**
**      cmdline	- Command line to process.
**
**  IMPLICIT INPUTS:
**
**      cmdtbl	- Command lookup table.
**	cmddsp	- Command dispatch table.
**
**  IMPLICIT OUTPUTS:
**
**      Various depending on command.
**
**  SIDE EFFECTS:
**
**      Various depending on command.
**
**--
**/
int docmd(char* cmd)
{
/*	    External functions	    */

/*	    Local variables	    */

    char    *resid;		/* Residual of command line */
    char    keywd[MAXLINE];	/* Will hold command keyword.	*/
    int    cmdidx;		/* Will hold command index.	*/

    int    usrcmd();

    resid = strtoken(cmd, " \t", keywd);	/* Get the token.   */
    if (strlen(keywd))	{	/* Just ignore if no keyword.	*/
	
      cmdidx = fndkey(keywd, (const char**)cmdtbl, NCMDS);
      if (cmdidx >= 0) {
	int retstat;
	retstat = (*cmddsp[cmdidx])(resid); /* Dispatch */
	fflush(stdout);
	fflush(stderr);
	return retstat;
      }
      else {
	putmsg(ILLCMD);
      }
    }
    else
      putmsg(ABKEYW);
    
    return ((int)rctl_NoOp);	/* At present no exit cmd */
}












