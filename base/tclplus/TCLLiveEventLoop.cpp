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
#include "TCLLiveEventLoop.h"
#include "TCLInterpreter.h"
#include "TCLVariable.h"
#include <StateException.h>
#include "TCLApplication.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

using namespace std;

// Externals:

extern CTCLApplication* gpTCLApplication;


/*  The default stop latency.          */

static const long DEFAULT_STOP_LATENCY(100); // in ms.
static const char* defaultPrompt1 = "\n% ";
static const char* defaultPrompt2 = "-- ";

/*  Pointer to the singleton instance: */

CTCLLiveEventLoop* CTCLLiveEventLoop::m_pTheInstance(0);



/*-----------------------------------------------------------------*/
/*   Construction like operations                                  */
/*-----------------------------------------------------------------*/

/*!
   Return a pointer to the one and only CTCLLiveEventLoop object.
   If this is the first call, the object is created.
*/

CTCLLiveEventLoop*
CTCLLiveEventLoop::getInstance()
{
  if(!m_pTheInstance) {
    m_pTheInstance = new CTCLLiveEventLoop;
  }
  return m_pTheInstance;
}

/*!
   Construction does require some initialization of 
   object variables.
*/
CTCLLiveEventLoop::CTCLLiveEventLoop() :
  m_pStdinTarget(0),
  m_prompt1(0),
  m_prompt2(0),
  m_isRunning(false),
  m_stopLatency(DEFAULT_STOP_LATENCY)
{
}


/*--------------------------------------------------------------*/
/*  Starting and stopping event processing                      */
/*--------------------------------------------------------------*/

/*!
   Start the event loop on the application's Tcl interpreter.
   note that this function will only return to the caller when
   another thread or an event handler invokes the stop member.

   \throw CStateException 
     - There is no application object.
     - The event loop is already running.

*/
void
CTCLLiveEventLoop::start()
{
  // We are really just going to find the CTCLInterpreter that
  // is associated with the application and call the 'other'
  // start function.  It will do all the state checking etc.
  // The only thing we have to worry about is whether or not
  // this is being run from a program that does not create
  // a CTCLApplication.
  //

  if (gpTCLApplication) {
    CTCLInterpreter* pInterp = gpTCLApplication->getInterpreter();
    start(pInterp);
  }
  else {
    throw CStateException("No CTCLApplication", "CTCLApplication",
			  "Attempting to start the live event loop");
  }
		      
}
/*!
   Start the evetn loop on the specified interpreter.
   \param pInterp - Pointer to the CTCLApplication that will receive
                    commands from stdin

   \throw CStateException 
   - Event loop is already active.
*/
void
CTCLLiveEventLoop::start(CTCLInterpreter* pInterp)
{
  if (!m_isRunning) {
    m_pStdinTarget = pInterp;
    delete m_prompt1;
    delete m_prompt2;
    m_prompt1 = m_prompt2 = 0;
    m_command = "";
    m_isRunning = true;

    setupEvents();
    prompt1();
    eventLoop();


  }
  else {
    throw CStateException("running", "stopped",
			  "Attempting to start live event loop");
  }

}

/*!
   This stops the event loop.  The event loop waits for events up to
   some time out specified by setStopLatency. Between events and after
   waits timeout, the software will check to see if m_isRunning is still
   true.  If not the event loop function returns.

   Thus there can be a  latency between stopping the event loop and it
   actually stopping.

   Since serveral mutually unaware actors could be attempting to stop the event loop,
   it is not an error to stop an already stopped event loop.

*/

void
CTCLLiveEventLoop::stop()
{
  m_isRunning = false;
}


/*--------------------------------------------------------------------------------*/
/* Event loop timing/stop latency                                                 */
/*--------------------------------------------------------------------------------*/

/*!
   Set the stop latency, event loop timing.  This determines how long the
   event loop will wait for an event before checking to see if it's time to stop
   running.  The latency is expressed in milliseconds. Note, however that it is
   subject to the clock granularity, and that the less latency you request, 
   the higher the CPU utilization of the loop. 

   \param ms  - Maximum number of milliseconds between checks of
                the stop flag.
   \return long
   \retval Prior latency value.

   \note Changes to the latency to an active event loop have the prior latency
         before becoming effective.
   \note It is legal to set the latency of a stopped event loop.
   \note The latency is not reinitialized to the default value when the event loop is
         started, but retains any prior value.
   
*/
long 
CTCLLiveEventLoop::setStopLatency(long ms)
{
  long prior    = m_stopLatency;
  m_stopLatency = ms;
  return prior;
}
/*!
   Return the stop latency.  See setStopLatency for a description of this parameter.
   \return long
   \retval Stop latency value.

   \note  If the stop latency was recently changed, the value returned may not have
          taken effect yet.  Sorry, that's the way it goes.
*/
long
CTCLLiveEventLoop::getStopLatency() const
{
  return m_stopLatency;
}
/*-----------------------------------------------------------------------------------*/
/*   Utilities:                                                                      */
/*      Prompting                                                                    */
/*-----------------------------------------------------------------------------------*/

/*
** prompt1:
**   Issues an initial command prompt.  This is issued prior to the first
**   command word. The default is "\n% ".  If the variable tcl_prompt1 is defined
**   it is assumed to be a command that can be run to output the prompt.
*/

void
CTCLLiveEventLoop::prompt1() 
{
  // If necessary, we need a new CTCLVariable pointing at tcl_prompt

  if (!m_prompt1) {
    m_prompt1 = new CTCLVariable("tcl_prompt1", false);
    m_prompt1->Bind(m_pStdinTarget);
  }

  prompt(m_prompt1, defaultPrompt1);




}

/*
** prompt2:
**   Same as for prompt1, however the prompt is issued for
**   command continuations.  tcl_prompt2 is the
**   variable that might have a prompt command.
*/
void
CTCLLiveEventLoop::prompt2()
{
  if (!m_prompt2) {
    m_prompt2 = new CTCLVariable("tcl_prompt2", false);
    m_prompt2->Bind(m_pStdinTarget);
  }

  prompt(m_prompt2, defaultPrompt2);

}
/*
** prompt
**   Utility used to prompt either from a script in a variable
**   of from a default prompt if that is not defined.
**Parameters:
**  pVar    -  Pointer to the CTCLVariable wrapper around the variable.
**  default - const char* default prompt if the var isn't defined.
*/
void
CTCLLiveEventLoop::prompt(CTCLVariable* pVar, const char* defaultPrompt) const
{
  // Try to issue the prompt from the var.

  const char* pPromptCommand = pVar->Get();
  string result;
  if (pPromptCommand) {
    try {
      result = m_pStdinTarget->Eval(pPromptCommand);
      return;			// Successful proc based prompt.
    }
    catch(...) {
      result = m_pStdinTarget->GetResultString();
      write(STDOUT_FILENO, result.c_str(), result.size());
    }
  }
  write(STDOUT_FILENO, defaultPrompt, strlen(defaultPrompt));

}

/*----------------------------------------------------------------------*/
/*  Utilities:                                                          */
/*    Event loop related functions and the event loop                   */
/*----------------------------------------------------------------------*/

/*
** The event loop. This only returns to the caller when the event loop
** is stopped by call or when the event loop has been destroyed
** by some meddler.
*/
void
CTCLLiveEventLoop::eventLoop()
{
  prompt1();
  while (m_isRunning) {
    struct Tcl_Time timeout;
    timeout.sec = m_stopLatency/1000;
    timeout.usec= (m_stopLatency % 1000) * 1000;

    if (Tcl_WaitForEvent(&timeout) == -1) {
      // Tcl event loop destroyed.

      m_isRunning = false;
      stopEvents();
      return;
    }
    // Process all queued events.

    while (Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT)) 
      ;

  }
  stopEvents();
}
/*
** Handles the condition that data can be read from stdin.
** We'll read a bunch of bytes and append it to the m_command string.
** when that's a 'complete' command, we'll attempt to execute it.
** Parameters:
**    pData  - Client data passed to file handling functions.
**             This is actually a pointer to the singleton.
**    mask   - Mask of fired events (ignored).
*/
void
CTCLLiveEventLoop::stdinHandler(ClientData pData, int mask)
{
  char input[80];
  memset(input, 0, sizeof(input)); //  ensure this is a C string.
  ssize_t bytes = read(STDIN_FILENO, input, sizeof(input) - 1 );
  if (bytes == 0) {
    Tcl_Exit(0);		// EOF fired, so we exit.
  }
  if (bytes < 0) {
    int e = errno;
    perror("Stdin read failed"); // Maybe stdin closed?  For now exit.
    Tcl_Exit(e);
  }

  CTCLLiveEventLoop* pObject = reinterpret_cast<CTCLLiveEventLoop*>(pData);
  string&            command(pObject->m_command);
  command     += input;


  if(Tcl_CommandComplete(command.c_str())) {
    Tcl_Interp* pInterp = pObject->m_pStdinTarget->getInterpreter();

    Tcl_Eval(pInterp, command.c_str());
    CONST char* pResult = Tcl_GetStringResult(pInterp);
    if (pResult) {
      write(STDOUT_FILENO, pResult, strlen(pResult));
    }
    command = "";
    pObject->prompt1();
  }
  else {
    pObject->prompt2();
  }
  
}
/*
** Enables event processing.
** In this case, we're just going to create a file handler on STDIN_FILENO
** sensitive to readability and exceptional conditions.
** 
*/
void
CTCLLiveEventLoop::setupEvents()
{
  Tcl_CreateFileHandler(STDIN_FILENO, TCL_READABLE | TCL_EXCEPTION,
			stdinHandler, reinterpret_cast<ClientData>(this));
}
/*
 *  Turns off event processing by calling Tcl_DeleteFileHandler on STDIN_FILENO.
 * This may be a bad thing if the Tk event loop got started (via e.g..
 * package require Tk.
 */
void
CTCLLiveEventLoop::stopEvents()
{
  Tcl_DeleteFileHandler(STDIN_FILENO);
}




