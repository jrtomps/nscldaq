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

/*
** command.c:
**   Provides stubs for interfaces to the data acquisition command distribution
**   and processing facility.
**   This file includes the following exported functions:
**      daq_GetCommand            - Retrieve next command
**      daq_OpenControlPath       - Open and return fid on source of control
**                                  commands (stdin for this).
**      daq_CloseControlPath      - Closes the fid opened with 
**                                  daq_OpenControlPath
**      daq_GetScalerCount        - Retrieve number of scalers to read.
**      daq_SetScalerCount        - Set no. of scalers to read.
**      daq_GetScalerReadoutInterval - Get the current scaler readout interval.
**      daq_SetScalerReadoutinterval - Set current scaler readout interval.
**
** Author:
**   Ron Fox
**   NSCL
**   MichiganState University
**   East Lansing, MI 48824-1321
**   mailto:fox@nscl.msu.edu
**
*/
static char* Copyright = 
"command.c: (c) Copyright NSCL 1999, all rights reserved\n";

/*
** Header files required:
*/
#include <daqinterface.h>
#include <cmdio.h>
#include <daqerrno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
/*
**  Manifest constants:
*/
#define LINESIZE  256		/* Size of the largest accepted input line. */
#ifndef TRUE
#define TRUE      1
#endif
#ifndef FALSE
#define FALSE     0
#endif
/*
**  Local data types:
*/

typedef enum _RunState {
  State_Active,
  State_Paused,
  State_Halted
} RunState;

/*
**  File scoped storage:
*/
static time_t          snScalerInterval = 10;   /* sec between scaler reads. */
static unsigned        snScalers        = 0;    /* scalers to read. */
static unsigned        snRunNumber      = 0; /* Current run number. */
static char            ssRunTitle[81]; /* Current Run title. */
static RunState        rsState = State_Halted;

static unsigned short  cpuNumber = 0;

#define NSETSHOKEYS   6
static const char* ppSetShoKeywords[NSETSHOKEYS] = {
  "FREQUENCY",
  "SCALERS",
  "RUN",
  "TITLE",
  "CPU",
  "ALL",
};


/*  Prototypes for and dispatch tables for SET/SHO commands:  */

static int SetFrequency(const char* pTail);
static int SetScalers(const char* pTail);
static int ShoFrequency(const char* pTail);
static int ShoScalers(const char* pTail);
static int SetRun(const char* pTail);
static int ShoRun(const char* pTail);
static int SetTitle(const char* pTail);
static int ShoTitle(const char* pTail);
static int ShoAll(const char* pTail);
static int InvKeyword(const char* pTail);

static int setCPU(const char* pTail);
static int shoCPU(const char* pTail);

typedef int (*pCommandProcessor)(const char* pTail);

static const pCommandProcessor ppSetRoutines[] = {
   SetFrequency,
   SetScalers,
   SetRun,
   SetTitle,
   setCPU,
   InvKeyword
   };
static const pCommandProcessor ppShoRoutines[] = {
   ShoFrequency,
   ShoScalers,
   ShoRun,
   ShoTitle,
   shoCPU,
   ShoAll
   };

/*-----------------------------------------------------------------------
** Function:
**   void daq_SetScalerCount(nScalers)
** Operation Type:
**   Mutator
*/
void
daq_SetScalerCount(unsigned nScalers)
{
  /*  Sets the the number of scalers to readout.  In theory, this is only
  **  legitimate when the run is inactive, however in this prototype, that
  **  sort of state information is not propagated system wide yet.
  ** Parameters:
  **     unsigned nScalers:
  **         Number of scalers to read.
  */

  snScalers = nScalers;
}
/*------------------------------------------------------------------------
** Function:
**    unsigned daq_GetScalerCount()
** Operation Type:
**    Selector
*/
unsigned
daq_GetScalerCount()
{
  /* Returns the number of scalers to readout.
   */

  return snScalers;
}
/*-----------------------------------------------------------------------
** Function:
**     void daq_SetScalerReadoutInterval(nMs)
** Operation Type:
**     Mutator.
*/
void
daq_SetScalerReadoutInterval(unsigned nSec)
{
  /* Sets the number of milliseconds between scaler readouts.
  ** Formal Parameters:
  **     unsigned nMs:
  **        Number of seconds between scaler readouts.
  */

  snScalerInterval = nSec;
}
/*-----------------------------------------------------------------------
** Function:
**    unsigned daq_GetScalerReadoutInterval()
** Operation type:
**    Selector.
*/
time_t
daq_GetScalerReadoutInterval()
{
  /*  Returns the number of ms between scaler readouts with an active run */

  return snScalerInterval;
}
/*-------------------------------------------------------------------------
** Function:
**    void* daq_OpenControlPath()
** Operation Type:
**    I/O attachment
*/
void*
daq_OpenControlPath()
{
  /*  Opens and returns a stream to stdin.  This is actually a dup
  **  so that the caller can do whatever they want with stdin after this.
  **  The stream is made opaque by casting the file pointer to a void*
  */
  FILE* pControlStream = (FILE*)0;
  int   nFileId;
  nFileId = dup(fileno(stdin));
  if(nFileId < 0) 
    return (void*)0;

  pControlStream = fdopen(nFileId, "r");

  return (void*)pControlStream;
}
/*-------------------------------------------------------------------------
** Function:
**    void daq_CloseControlPath(void* pPath)
** Operation Type:
**   I/O connection
*/
void
daq_CloseControlPath(void* pPath)
{
  /*  Closes the channel open on the run control device like thing.
  **  Note that in this prototype, pPath is a FILE* disguised as a void*
  ** Parameters:
  **    void* pPath:
  **      Path open on the run control stream,  returned from
  **      daq_OpenControlPath.
  */

  FILE* f = (FILE*)pPath;
  fclose(f);
}
/*------------------------------------------------------------------------
**  Function:
**      daq_CheckForCommand(void* pCtl)
**  Operation Type:
**      I/O Query
*/
int
daq_CheckForCommand(void *pCtl)
{
  /* Checks to see if there is a command available on the control path.
  ** Formal Parameters:
  **    void* pCtl:
  **       Pointer to the control path.  This is really just a FILE* in 
  **       disguise.
  ** Returns:
  **      TRUE  - A read on the command path won't block.
  **      FALSE - A read onthe command path will block.
  */
  FILE*     f = (FILE*)pCtl;
  int       fd = fileno(f);	/* Select needs a set of fd's not FILE's */
  fd_set    reads,writes,excepts;
  struct
    timeval poll;
  int       nfds;		/* Number of ready fds from select. */
  
  /*  Set up the file descriptor sets so that the only thing we care about
  **  is readability on fd:
  */
  FD_ZERO(&reads);
  FD_ZERO(&writes);
  FD_ZERO(&excepts);
  FD_SET(fd, &reads);

  memset(&poll, 0, sizeof(poll)); /* Poll without blocking on select. */

  /*  Now we're set to issue the select.  The return value from select
  **  maps to our function return value as follows:
  **  > 0    Return TRUE
  **  = 0    Return FALSE
  **  < 0    This is an error in something that went into the select and
  **         is a bug which is either our fault or the caller's so we'll
  **         blow away with an assertion failure.
  */
  nfds = select(fd+1, &reads, &writes, &excepts, &poll);

  if(nfds < 0) {
    perror("daq_CheckForCommand - select failed"); /* give some explanation */
    assert(nfds >= 0);		/* then blow the program away/ */
  }
  return ((nfds > 0) ?  TRUE : FALSE);
  
  
}
/*--------------------------------------------------------------------
**  Function:
**     RunControlCmd daq_GetCommand(void* pCtl)
**  Operation:
**     I/O
*/

RunControlCmd
daq_GetCommand(void* pCtl)
{
  /*  Retrieves a command from the control path. 
  **  Commands are mapped to a RunControlCmd enumerator
  **  which specifies which run control operation should be performed.
  **  Some commands are completely internal and therefore don't require
  **  action from the run control state machines.  These return
  **  rctl_NoOp and are processed by this software.
  **    This function really just gets the command line in from the path
  **  and then uses the table driven parser to pass control to the 
  **  actual execution function.  
  **
  **  Formal Parameters:
  **      void* pCtl:
  **         Really a FILE* which is the stream open on the control path.
  **  Returns:
  **    Appropriate run control operation.
  **  Acknowledgements:
  **    The keyword parser and so forth is shamelessly stolen from the
  **    MASH front end's RCTL parser.  Tables are reduced somewhat however.
  **  NOTE:
  **     If necessary, this function blocks until a line of input is available.
  */
  FILE* f = (FILE*)pCtl;
  char szCommand[LINESIZE];
  RunControlCmd code;
  
  getlin(f, szCommand, sizeof(szCommand)); /* Get the input line.            */
  if(strlen(szCommand) == 0) return rctl_NoOp;
  upcase(szCommand);		          /* Convert to upper case.     */

  code = (RunControlCmd)docmd(szCommand); 
  prompt(stdout, "RunCtl> ");
		
  return code; 
}
/*---------------------------------------------------------------------
** Function:
**    static int NoParams(const char* ptail, int Okcode)
** Operation Type:
**    Command Processor
*/
static int
NoParams(const char* pTail, int nOkCode)
{
  /* Processes commands with no additional parameters. 
  ** Formal Parameters:
  **   const char* pTail:
  **       Pointer to the rest of the command.
  **   int nOkCode:
  **       Code returned if the parse finishes correctly.
  ** Returns:
  **     nOkCode   - If the parse succeeded.
  **     rctl_NoOp - If the parse failed, and emits MAXPAR error message
  **                 to stderr.
  */
  char szToken[LINESIZE];
  strtoken(pTail, " \t", szToken);
  if(strlen(szToken) > 0) {
    putmsg(MAXPAR);
    return (int)rctl_NoOp;
  }
  return nOkCode;
}
/*----------------------------------------------------------------------
** Function:
**    int bgrun(const char* tail)
** Operation type:
**    Command Processor
*/
int
bgrun(const char* tail)
{
  /*  Begin a run.
   **    Ensure that there is no tail on the command.  If there is, then
   **    emit an error message, if not, then return rctl_Begin.
   ** Formal Parameters:
   **     const char* pTail:
   **        Points to the unprocessed part of the command string.
   */
  return NoParams(tail, (int)rctl_Begin);
}
/*-----------------------------------------------------------------------
** Function:
**    int endrun(const char* pTail)
** Operation type:
**   Command Processor.
*/
int
endrun(const char* pTail)
{
  /* Ends the run if the command parse succeeds */

  return NoParams(pTail, (int)rctl_End);
}
/*----------------------------------------------------------------------
** Function:
**    int pauserun(const char* pTail)
** Operation Type:
**    Command Processor
*/
int
pauserun(const char* pTail)
{
  /* Pauses the run: */

  return NoParams(pTail, (int)rctl_Pause);
}
/*----------------------------------------------------------------------
** Function:
**    int resrun(const char* pTail)
** Operation Type:
**    Command Processor
*/
int
resrun(const char* pTail)
{
  /* Resumes a paused run. */

  return NoParams(pTail, (int)rctl_Resume);
}
/*---------------------------------------------------------------------
** Function:
**   int doexit(const char* tail)
** Operation type:
**    Command Processor.
*/
int
doexit(const char* pTail)
{
  /* Exit from program.  */

  return NoParams(pTail, rctl_Exit);
}
/*---------------------------------------------------------------------
** Function:
**   static int Dispatch(const char* pTail, unsigned nkeys, 
**                       const char** ppKeytbl, 
**                       const pCommandProcessor* pFunctions) 
** Operation Type:
**    Command Dispatch.
*/
static int 
Dispatch(const char* pTail, unsigned nKeys,
	 const char** ppKeytbl,const pCommandProcessor* pFunctions)
{
  /*  Dispatches to an appropriate command handling routine based on the
  **  next keyword in the command line.  Returns the value returned
  **  by the command dispatcher, or reports an IVKEYW error message
  **  and returns rctl_NoOp.
  **
  ** Formal Parameters:
  **    const char* pTail:
  **       Remaining chunk of the command line.
  **    unsigned nKeys:
  **       Number of keys in the keyword lookup table.
  **    unsigned nKeySize:
  **       Number of characters in each keyword slot.
  **    const char* ppKeytbl:
  **       Pointer to the table of legal keywords.
  **    pCommandProcessor* pFunctions:
  **       Pointer to the array of functions to execute for each
  **       corresponding keyword in the ppKeytbl.
  ** Returns:
  **    The value returned from the appropriate pFunctions function or
  **    rctl_NoOp if there isn't a match between the keyword and the
  **    table.
  */

  char szKeyword[LINESIZE];
  char *pRemnant;
  int  nMatched;
  int  nReturnValue;
  /* Get the next keyword in the command line:
   */
  pRemnant = strtoken(pTail, " \t", szKeyword);
  if(strlen(szKeyword) == 0) {
    putmsg(INSFPAR);
    return rctl_NoOp;		/* No keyword on command line. */
  }
  /* Hunt for the keyword in the ppKeytbl list   */

  nMatched = fndkey(szKeyword, ppKeytbl,  nKeys);
  switch(nMatched) {
  case -1:			/* No match */
    putmsg(IVKEYW);
    nReturnValue = rctl_NoOp;
    break;
  case -2:			/* Ambiguous match */
    putmsg(ABKEYW);
    nReturnValue = rctl_NoOp;
    break;
  default:			/* Unambiguous match. */
    nReturnValue = (*pFunctions[nMatched])(pRemnant);
    break;
  }
  return nReturnValue;
}

/*----------------------------------------------------------------------
** Function:
**    int setdsp(const char* pTail)
** Operation Type:
**    Command Processor.
*/
int 
setdsp(const char* pTail)
{
  /* Performs second level dispatching of the SET command.
  ** uses Dispatch to do this.
  ** Formal Parameters:
  **    const char* pTail:
  **       The remainder of our command line.
  */
  return Dispatch(pTail, NSETSHOKEYS,
		  (const char**)ppSetShoKeywords, ppSetRoutines);
}
/*---------------------------------------------------------------------
** Function:
**    int shodsp(const char* pTail)
** Operation Type:
**    Command Processor.
*/
int 
showdsp(const char* pTail)
{
  /* Perform second level dispatching of the SHOW command.
  ** Uses Dispatch to do this.
  ** Formal Parameters:
  **     const char* pTail:
  **         Remainder of the command line.
  */
  return Dispatch(pTail, NSETSHOKEYS, 
		  (const char**)ppSetShoKeywords, ppShoRoutines);
}
/*-----------------------------------------------------------------------
** Function:
**    static int ShoFrequency(const char* pTail)
** Operation Type:
**    Command Processor.
*/
static int
ShoFrequency(const char* pTail)
{
  /* Show the scaler readout frequency to stdout. The Tail must not
  ** contain any additional parameters.  We play a bit of a trick to
  ** be sure that this is the case while re-using the NoParams code.
  **
  ** Formal Parameters:
  **    const char* pTail:
  **       Pointer to the command tail.
  ** Returns:
  **    rctl_NoOp
  */
  assert((int)rctl_NoOp != 0);

  if(NoParams(pTail, 0) == (int)rctl_NoOp) { /* TRUE if too many parameters. */
    return (int)rctl_NoOp;
  }
  else {
    printf("Scalers read every  %u seconds.\n", snScalerInterval);
    fflush(stdout);
    return (int)rctl_NoOp;
  }
}
/*---------------------------------------------------------------------
 * Function:
 *    ShoCPU
 *      Show the number of the CPU.
 */
static int
shoCPU(const char* pTail)
{
  if (NoParams(pTail, 0) == rctl_NoOp) {
    return (int)rctl_NoOp;
  }
  else {
    printf("CPU Node number is: %u\n", cpuNumber);
    fflush(stdout);
    return (int)rctl_NoOp;
  }
}
/*----------------------------------------------------------------------
** Function:
**   int ShoScalers(const char* pTail)
** Operation Type:
**   Command Processor.
*/
static int
ShoScalers(const char* pTail)
{
  /* Show the number of scalers being readout to stdout.  The Tail must
  ** not contain any additional parameters (trailing whitespace is allowed).
  ** We play a bit of a trick to be sure that this is the case while re-using
  ** the NoParams function.
  **
  ** Formal Parameters:
  **    const char* pTail:
  **        Pointer to the tail of the command string.
  ** Returns:
  **        rctl_NoOP cast as integer.
  */
  assert((int)rctl_NoOp != 0);	/* Must be able to distinguish 0 from no-op */

  if(NoParams(pTail, 0) == (int)rctl_NoOp) { /* Too many parameters... */
    return (int)rctl_NoOp;
  }
  else {
    printf("%u Scalers are being read out.\n", snScalers);
    fflush(stdout);
    return (int)rctl_NoOp;
  }

}
/*----------------------------------------------------------------------
** Function:
**   int SetUnsignedPar(unsigned* pnParameter, const char* pTail, int nOk)
** Operation Type:
**   Generic command processor.
*/
static int
SetUnsignedPar(unsigned* pnParameter, const char* pTail, int nOk)
{
  /*  Generic command processor to handle the tail of commands which
  **  process SET someunsignedparam  value.  This function is called with
  **  just the value left in the command tail.  It parses the value out of
  **  the tail and sets the associated unsigned variable.
  ** Formal Parameters:
  **    unsigned *pnParameter:
  **        Pointer to the unsigned value to be set.
  **    const char* pTail:
  **        Pointer to the tail of the command line. Should contain exactly
  **        one parseable parameter which is a valid integer value.  When
  **        decoded, this value is placedin *pnParameter.
  **    int nOk:
  **        The value to return on successful parse.
  **        Unsuccessful parses return (int)rctl_NoOp and will
  **        result in various associated putmsg outputs to stderr.
  ** Returns:
  **    nOk             - If the parse was successful and *pnParameter was 
  **                      modified.
  **    (int)rctl_NoOp  - If there was a problem.
  ** Error Messages:
  **    The folowing error messages can be emitted:
  **        IVNUM     - If the tail did not contain a number or the number
  **                    was negative.
  **        INSFPAR   - If there were no parameters in the tail.
  **        MAXPAR    - There were additional parameters beyond the number
  **                    on the command line.\
  */

  char szToken[LINESIZE];	/* Token from the command line.        */
  int  nValue;			/* Value parsed from the command line. */
  int  nStatus;			/* Various status values.              */
  char *pRemainder;		/* Remaining command line tails.       */
  int  nReturnValue;		/* Save return values for common exit code. */

  /* Get the next command line token... It must be a positive integer. */

  pRemainder = strtoken(pTail, " \t", szToken);
  if(strlen(szToken) == 0) {	/* No number found.             */
    putmsg(INSFPAR); 
    return (int)rctl_NoOp;
  }
  nValue = cvtcnt(szToken);
  if(nValue < 0) {
    putmsg(IVNUM);
    return (int)rctl_NoOp;
  }
  /*  There must not be any additional parameters on the command line.
  **  We can use the 'standard' trick with NoParams() to determine if
  **  this is the case.  You might think we could just the nOk value for
  **  the other side of the NoParams function, but imagine what would happen
  **  if the caller *wanted* us to return rctl_NoOp??
  */
  assert((int)rctl_NoOp != 0);	/* Trick requires these to be distinct. */
  if(NoParams(pRemainder, 0) == 0) {
    *pnParameter = nValue;
    nReturnValue = nOk;
  }
  else {
    putmsg(MAXPAR);
    nReturnValue = (int)rctl_NoOp;
  }
  return nReturnValue;
}
/*--------------------------------------------------------------------------
** Function:
**  SetCPU
**    Set the CPU number from the command tail.
*/
static int
setCPU(const char* pTail)
{
  unsigned newNode     = cpuNumber;
  int      returnValue = SetUnsignedPar(&newNode, pTail, (int)rctl_NoOp);

  if (!daq_IsHalted()) {
    putmsg(RUNACTIVE);
    return(int)rctl_NoOp;
  }
  cpuNumber = newNode;
  return returnValue;

}
/*--------------------------------------------------------------------------
** Function:
**   static int SetFrequency(const char* pTail);
** Operation Type:
**   Command Processor.
*/
static int 
SetFrequency(const char* pTail)
{
  /*  Process the SET FREQUENCY n command.  This makes use of the
  **  SetUnsignedPar utility function to parse the remainder of the command
  **  and, if appropriate, set the number of seconds between readouts.
  **  we then convert this to ms and set snScalerIntervalms.
  ** Formal Parameters:
  **    const char* pTail:
  **       Pointer to the tail of the command.
  */


  unsigned nSeconds = snScalerInterval; /* Current rdo time. */
  unsigned nReturnValue = SetUnsignedPar(&nSeconds, pTail, (int)rctl_NoOp);

  if(!daq_IsHalted()) {
    putmsg(RUNACTIVE);
    return (int)rctl_NoOp;
  }

  snScalerInterval = nSeconds;
  return nReturnValue;
}
/*-------------------------------------------------------------------------
** Functions:
**   static int SetScalers(const char* pTail)
** Operation Type:
**   Command Processor.
*/
static int 
SetScalers(const char* pTail) 
{
  /* Process the SET SCALERS n command.  This makes use of the SetUnsignedPar
  ** utility function to parse the remainder of the command and, if 
  ** appropriate, directly set the value of snScalers.
  **
  ** Formal Parameters:
  **    const char* pTail:
  **       Pointer to the command line tail.
  */
  if(!daq_IsHalted()) {
    putmsg(RUNACTIVE);
    return (int)rctl_NoOp;
  }

  return SetUnsignedPar(&snScalers, pTail, (int)rctl_NoOp);

}
/*-------------------------------------------------------------------------
** Function:
**   daq_StartRun()
** Operation Type:
**   State transition marker.
**
*/
void 
daq_StartRun()
{
  /* It's the caller's responsibility to see that this is  legal  */

  rsState = State_Active;
}
/*-------------------------------------------------------------------------
** Function:
**    daq_EndRun()
** Operation Type:
**    State transition marker.
*/
void
daq_EndRun()
{
  /* The caller must ensure this is legal. */

  rsState = State_Halted;
}
/*-----------------------------------------------------------------------
** Function:
**   daq_PauseRun()
** Operation Type:
**   State Transtion marker.
*/
void 
daq_PauseRun()
{
  /* The caller must ensure this is legal. */

  rsState = State_Paused;
}
/*----------------------------------------------------------------------
** Function:
**   daq_ResumeRun()
** Operation Type:
**    State transition marker.
*/
void 
daq_ResumeRun()
{
  /* The caller must ensure this is legal. */

  rsState = State_Active;
}

/*-------------------------------------------------------------------
**  daq_IsActive()
**
** Operation Type:
**    Inquiry
*/
bool
daq_IsActive()
{
  /* Returns TRUE if run state is active */

  return (rsState == State_Active);
}

/*------------------------------------------------------------------
**  daq_IsHalted()()
**
** Operation Type:
**    Inquiry
*/
bool 
daq_IsHalted()
{
  /* Returns TRUE if run state is halted */

  return (rsState == State_Halted);
}
/*---------------------------------------------------------------------
** daq_IsPaused()
**
** Operation Type:
**   Inquiry
*/
bool
daq_IsPaused()
{
  /* Returns FALSE if run state is paused */

  return (rsState == State_Paused);
}
/*--------------------------------------------------------------------
**  daq_GetTitle()
**
** Operation Type:
**    Inquiry
*/
char* 
daq_GetTitle()
{
  return ssRunTitle;
}
/*---------------------------------------------------------------------
** daq_SetTitle()
** 
** Operation Type:
**   Info setting.
*/
void 
daq_SetTitle(const char* pNewTitle)
{
  memset(ssRunTitle, 0, sizeof(ssRunTitle));
  strncpy(ssRunTitle, pNewTitle, sizeof(ssRunTitle)-1);
}
/*---------------------------------------------------------------------
** daq_GetRunNumber:
** 
** Operation type:
**   inquiry
*/
unsigned 
daq_GetRunNumber()
{
  return (unsigned)snRunNumber;
}
/*---------------------------------------------------------------------
** daq_SetRunNumber
**
** Operation Type:
**   state change.
*/
void daq_SetRunNUmber(unsigned nNextRun)
{
  snRunNumber = (unsigned short)nNextRun;
}
void 
daq_IncrementRunNumber()
{
  snRunNumber++;
}

/*------------------------------------------------------------------
** InvKeyw()
**
** Operation type:
**   catchall.
*/
static int 
InvKeyword(const char *pTail)
{
  /* Called when an invalid keyword in a set/sho is found.
  ** puts an IVKEYW error message to stderr and returns a noop.
  */

  putmsg(IVKEYW);
  return (int)rctl_NoOp;
}
/*------------------------------------------------------------------
** ShoAll()
**
** Operation Type:
**   Status output.
*/
static int
ShoAll(const char* pTail)
{
  /* Shows the Run number, the title, scaler count and scaler frequency. */

  /*
  ** deal with cases where there are extra parameters on the line:
  */
  assert((int)rctl_NoOp != 0);
  if(NoParams(pTail, 0) == (int)rctl_NoOp) {
    return (int)rctl_NoOp;
  }
  printf("---------------------------- Status ------------------\n");
  ShoRun(pTail);
  ShoTitle(pTail);
  ShoScalers(pTail);
  ShoFrequency(pTail);

  fflush(stdout);
  return (int)rctl_NoOp;
}
/*------------------------------------------------------------------
** ShoRun()
**
** Operation Type:
**    Displayer.
*/
static int
ShoRun(const char* pTail)
{
  /* Display the current run number. */
  char* pState = "(Unknown)";
  /*
  ** deal with cases where there are extra parameters on the line:
  */
  assert((int)rctl_NoOp != 0);
  if(NoParams(pTail, 0) == (int)rctl_NoOp) {
    return (int)rctl_NoOp;
  }
 
  if(daq_IsHalted()) pState = "(halted)";
  if(daq_IsActive()) pState = "(active)";
  if(daq_IsPaused()) pState = "(paused)";


  printf("Current run number:    %d %s\n", snRunNumber, pState);
  return (int)rctl_NoOp;
}
/*---------------------------------------------------------------------
** SetRun()
**
** Operation Type:
**    Parameter setting.
*/
static int
SetRun(const char* pTail)
{ 
  if(!daq_IsHalted()) {
    putmsg(RUNACTIVE);
    return (int)rctl_NoOp;
  }

  return SetUnsignedPar(&snRunNumber, pTail, (unsigned int)rctl_NoOp);

}
/*-------------------------------------------------------------------------
** ShoTitle
**
** operation type:
**   Display
*/
static int
ShoTitle(const char* pTail)
{

  /*
  ** deal with cases where there are extra parameters on the line:
  */
  assert((int)rctl_NoOp != 0);
  if(NoParams(pTail, 0) == (int)rctl_NoOp) {
    return (int)rctl_NoOp;
  }

  printf("Run Title is            : %s\n", ssRunTitle);
  return (int)rctl_NoOp;

}
/*-----------------------------------------------------------------------
** SetTitle()
**
** Operation Type:
**   Parameter setting.
*/
static int
SetTitle(const char* pTail)
{
  if(!daq_IsHalted()) {
    putmsg(RUNACTIVE);
    return (int)rctl_NoOp;
  }

  memset(ssRunTitle, 0, sizeof(ssRunTitle));
  strncpy(ssRunTitle, pTail, sizeof(ssRunTitle) -1);
  return (int)rctl_NoOp;
}


/*--------------------------------------------------------
**  daq_SetBuferSize:
**    Sets a new buffersize.
*/
static int daqBufferSize = DAQ_BUFFERSIZE; /* maintains current size.*/
void daq_SetBufferSize(int newsize)
{
  daqBufferSize = newsize;
}
/*-----------------------------------------------------------
** daq_GetBufferSize:
**   Returns the current buffer size.
*/
int daq_GetBufferSize()
{
  return daqBufferSize;
}


/*-----------------------------------------------------------
 *  Support setting the CPU node number that is set in all
 *  data buffers.
 */
void daq_setCPUNumber(unsigned short node)
{
  cpuNumber = node;
}
/*-------------------------------------------------------------
 * Retrieve the CPU Node number:
 *
 */
unsigned short
daq_getCPUNumber()
{
  return cpuNumber;
}
