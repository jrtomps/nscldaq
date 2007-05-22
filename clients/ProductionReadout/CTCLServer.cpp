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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";
//////////////////////////CTCLServer.cpp file////////////////////////////////////
#include <config.h>
#include "CTCLServer.h"    
#include <CSocket.h>     
#include <TCLInterpreter.h>
#include "CReaper.h"     
#include <Exception.h>
#include <CTCPConnectionLost.h> 
#include "CReadoutMain.h"
#include <CInterpreterStartup.h>
#include "CInterpreterShell.h"
#include "CInterpreterCore.h"
#include <assert.h>
#include <tcl.h>   
#include <Iostream.h>
#include <sys/poll.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Local data:

static const int MAXLINE=80;

	//Default constructor alternative to compiler provided default constructor
	//Association object data member pointers initialized to null association object 
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   \verbatim
	   CTCLServer server(pSocket);
   \endverbatim
   
   \param pSocket - CSocket* Pointer to the socket on which we will be conversing with
				our client.
*/
CTCLServer::CTCLServer (CSocket* pSocket) :
	CServerInstance(pSocket)     // Anonymous socket object.
 
{

}
int
CTCLServer::operator==(const CTCLServer& rhs) const
{
  return (int)false;
} 
// Functions for class CTCLServer


/*!
    Called each time data is readable on a server. 
    -  Read the data append it to the tcl command being built.  
    -  Check to see if we have a complete command.
    -  If the command is complete submit it to the interpreter.
    -  Return the interpreter result string to the peer.
    -  If the socket indicatse that it is closing, shutdown and
       exit.  The reaper will take care of deleting our object.
    

	\param CSocket* pPeer

*/
void 
CTCLServer::OnRequest(CSocket* pPeer)  
{
  try {
    string  chunk = GetChunk();
    m_Command += chunk;
    if(isComplete()) {
      CTCLInterpreter* pInterp = getInterpreter();
      string result;
      try {
	result = pInterp->GlobalEval(m_Command);
      }
      catch(...) {                      // Probably a CTCLException.
	result = pInterp->GetResultString();
	cerr << "Error on TCL server received Command: " 
	     << m_Command
	     << " " << result << endl;
      }
      // Clean up from command execution:
      
      m_Command = "";              // Empty the command string.
      result         += '\n';          // Make result a 'line'.
      try {
	CSocket* pSocket = getSocket();
	pSocket->Write((char*)result.c_str(), 
		       result.size());
      }
      catch(...) {
	// If disconnected, the next read will catch that immediately.
      }
    }
    
  }
  // Deal with common exception types.
  //
  catch (CTCPConnectionLost& rExcept) {
    string prefix("Lost tcl client connection ");
    prefix += m_Peer;
    string suffix(" Shutting down server instance");
    ReadException(prefix.c_str(), rExcept.ReasonText(),
		  suffix.c_str());		
  }
  catch (CException& rExcept) {
    ReadException("NSCL Exception caught in TCLserver read",
		  rExcept.ReasonText(),
		  "Shutting down server instance");
  }
  catch (string& rExcept) {
    ReadException("TCLServer read: string exception in read",
		  rExcept.c_str(),
		  "Shutting down server instance");
  }
  catch (char* pExcept) {
    ReadException("TCLServer read: char* exception in read",
		  pExcept,
		  "Shutting down server instance");
    
  }
  catch (...) {
    ReadException("TCLServer: Unanticipated exception in read",
		  "- unknown reason -",
		  "Shutting down server instance");
  }
}  

/*!
    Add ourselves to the CReaper thread's table
    and invoke the base class's operator() to get
    started.   The Reaper object ensures that our
    object and its exit status will be deleted when the 
    thread exits.

	\param int nArgs, char** pArgs

*/
int 
CTCLServer::operator()(int nArgs, char** pArgs)  
{
	 CReaper* pReaper(CReaper::getInstance());
	assert(pReaper);
	
	pReaper->Add(this);      // Add us as an instance.
	m_Peer = getPeername();

	cout << "Accepted tcl client connection from " 
	        << m_Peer << endl;
		
	// Delegate the main loop to our parent class. it does all the
	// right stuff already.
	
	CServerInstance::operator()(nArgs, pArgs);

}

// Implementation of local utility functions:
//
/*!     Determines if the command buffer contains a complete command yet.
     \return true  - m_Command is a complete tcl command.
     \return false - m_Command is not a complete tcl command.
*/
bool
CTCLServer::isComplete()
{
	return (bool)Tcl_CommandComplete((char*)m_Command.c_str());
	
}
/*!
     Gets the interpreter object for the application.
     
     \return CTCLInterpreter*  Pointer to the application's interpreter object.
     
     */
CTCLInterpreter*
CTCLServer::getInterpreter()
{
	CReadoutMain*         pMain    = CReadoutMain::getInstance();
	CInterpreterShell*    pStartup = pMain->getInterpreter();
	CInterpreterCore*     pCore    = pStartup->getInterpreterCore();
	CInterpreterStartup*  pIStartup= pCore->getStartup();
	CTCLInterpreter*      pInterp  = pIStartup->getInterpreter();
	return pInterp;
}

/*!
     Provides common handling of exceptions thrown while reading
     the socket:
     - A message is emitted on stderr,
     - The socket is shutdown preventing further communication and
        releasing socket resources.
     - The enable flag is set false scheduling the interpreter thread
         to exit. 
	 
	\note
	    The interpreter thread has been registered with a reaper thread
	     once the active flag goes false, the reaper will join and delete this
	     object

	\param pPrefix - const char* [in] 
	                        Prefixes the error message.
	\param pReason - const char* [in]
	                        Contains the error message corresponding to the exception.
	\param pSuffix - const char* [in]
	                        Contains a suffix to the error message.
*/
void
CTCLServer::ReadException(const char* pPrefix,
				      const char* pReason,
				      const char* pSuffix)
{
  cerr << pPrefix <<endl << pReason << endl << pSuffix << endl;
  try {
    getSocket()->Shutdown(); // This will throw if already shutdown...
  }
  catch(...) {		// So ignore the exception.
  }
  setEnable(false);
}
	                        
/*!
   Utility function to get a chunk of data from the socket.
   We will read from the socket until either:
   - We received a newline (\n) (which is appended to the string).
   - We understand there is no more data waiting for us.
   When either of these two conditions is met, the string retrieved from
   the socket up until then is returned.
   \return string
      data gotten from socket.
*/
string
CTCLServer::GetChunk()
{
  CSocket*      pSocket(getSocket());
  int           fd = pSocket->getSocketFd(); // For poll..
  struct pollfd pollinfo;	        // Struct in poll.
  string        result;		        // Result is built up here.
  char          c;		        // Characters are read into this.

  pollinfo.fd    = fd;
  pollinfo.events= POLLIN;	// Only interested in readability.
  while(poll(&pollinfo, 1, 1) == 1) { // as long as pollable.
    if(pollinfo.revents & POLLIN) {
      pSocket->Read(&c, 1);	// Exceptions are handled by our caller.
      result += c;
    }
    else {			// Not readable.
      break;
    }
  }
  return result;
  
}
