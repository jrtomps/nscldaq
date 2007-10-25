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
#include "InputStageCommand.h"
#include "InputStage.h"
#include <TCLInterpreter.h>
#include <TCLProcessor.h>
#include <TCLObject.h>
#include "AssemblerErrors.h"
#include "EventFragment.h"
#include "PhysicsFragment.h"
#include "AssemblerCommand.h"
#include "InvalidNodeException.h"
#include "EventTooSmallException.h"
#include "AssemblerUtilities.h"

#include <list>
#include <buftypes.h>
#include <stdint.h>

using namespace std;


/*
 * Initialize the dispatch table.  Each
 * element of the dispatch table is used to
 * determine which member function will actually
 * process a subcommand.
 */

InputStageCommand::DispatchTable InputStageCommand::m_dispatchTable[] =
{
		{"create",    2,  &InputStageCommand::createInputStage},
		{"destroy",   2,  &InputStageCommand::destroyInputStage},
		{"start",     2,  &InputStageCommand::startInputStage},
		{"stop",      2,  &InputStageCommand::stopInputStage},
		{"statistics",2,  &InputStageCommand::statistics},
		{"clear",     2,  &InputStageCommand::clearStatistics},
		{"inject",    3,  &InputStageCommand::inject},
		{"monitor",    3,  &InputStageCommand::monitor},
		{"unmonitor", 2,  &InputStageCommand::unmonitor},
		{"get",       3,  &InputStageCommand::get},
		{"pop",       3,  &InputStageCommand::pop},
		{"empty",	  3,  &InputStageCommand::empty},
		{"",          0,  (InputStageCommand::CommandProcessor)(NULL)}
};
////////////////////////////////////////////////////////////////
/*!
 *   Construct the object by registering the command.
 *  The command keyword is unconditionally "inputstage"
 */
InputStageCommand::InputStageCommand(CTCLInterpreter& interp,
									AssemblerCommand& config) :
	CTCLObjectProcessor(interp, "inputstage"),
	m_pInputStage(0),
	m_pConfiguration(&config),
	m_pScript(0)
{}

//////////////////////////////////////////////////////////////
/*!
 *   The destructor is probably never going to be called, but
 * we will run down the inputstage if it exists and is active.
 */
InputStageCommand::~InputStageCommand()
{
	if(m_pInputStage) {
		if (m_pInputStage->isRunning()) {
			m_pInputStage->stop();
		}
		delete m_pInputStage;
		m_pInputStage = 0;
	}
	if (m_pScript) {
	  delete m_pScript;
	  m_pScript = 0;
	}
}

/////////////////////////////////////////////////////////////
/*!
 *   The function call  operator gains control when the
 * Tcl interpreter is told to execute the 'inputstage' command.
 * The command requires a subcommand, as it is a Tcl command\
 * ensemble.  The subcommand is located in the m_dispatchTable
 * and, if the number of command line words is correct.
 * 
 * \param interp - The TCL interpreter that is executing this command.
 * \param objv   - Array of object wrapped Tcl_Obj that make up
 *                 the words on the command line
 * \return int
 * \retval TCL_OK - The command was dispatched and returned
 *                  successfully.
 * \retval TCL_ERROR -Either the command couldn't be dispatched,
 *                    or the command handler detected an error.
 * \note When TCL_ERROR is returned, the error text and the
 *       interpreter result string are filled in with the
 *       error message that describes why the command failed.'
 *  
 * \note Specific AseemblerErrors this function directly
 *       provides include:
 *       InvalidSubcommand - no match to the command table.
 *       TooFewParameters  - Too few command line parameters provided
 *       TooManyParameters - Too many command line parameters provided
 * 
 * In all cases this function detects the error, command usage
 * information is embedded in the error message.
 */
int
InputStageCommand::operator()(CTCLInterpreter& interp,
			      std::vector<CTCLObject>& objv)
{
  // Too few commands if there is no subcommand:
  
  if (objv.size() < 2) {
    return AssemblerErrors::setErrorMsg(interp, 
					AssemblerErrors::TooFewParameters,
					Usage());
  }
  // Extract the command keyword and hunt through the dispatch table
  // for a match:
  
  string subcommand = (string)objv[1];
  int    index      = 0;
  while (m_dispatchTable[index].m_processor) {
    if (subcommand == m_dispatchTable[index].m_keyword) {
      if (objv.size() == m_dispatchTable[index].m_parameterCount) {
	return (this->*m_dispatchTable[index].m_processor)(interp, objv);
      } 
      else {
	return AssemblerErrors::setErrorMsg(interp,
					    objv.size() < m_dispatchTable[index].m_parameterCount ?
					    AssemblerErrors::TooManyParameters :
					    AssemblerErrors::TooFewParameters,
					    Usage());
      }
    }
    
    index++;
  }
}
////////////////////////////////////////////////////////////
/*!
 * Create the input stage.
 * At this point there better not be an input stage in existence.
 * if there is already an input stage this is an
 * AlreadyExists error.
 * 
 * \param interp   - The interpreter running the command.
 * \param objv     - List of words that make up the command.
 * \return int
 * \retval TCL_OK  - The assembler input stage was created.
 * \retval TCL_ERROR - The assembler input stage already exists.
 */
int
InputStageCommand::createInputStage(CTCLInterpreter& interp,
				    std::vector<CTCLObject>& objv)
{
  if (m_pInputStage) {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::AlreadyExists,
					"(Configuration)");
  }
  else {
    list<AssemblerCommand::EventFragmentContributor> nodeList;
    nodeList = m_pConfiguration->getConfiguration();
    if(nodeList.size() == 0) {
      return AssemblerErrors::setErrorMsg(interp,
					  AssemblerErrors::NoNodesSpecified,
					  "(Configuration)");

    }

    m_pInputStage = new InputStage(*m_pConfiguration);
    return TCL_OK;
  }
}
///////////////////////////////////////////////////////////////
/*!
 * Delete the input stage. 
 * \pre The input stage exists.
 * \pre The input stage is in the stopped state.
 * \post m_pInputStage -> NULL.
 * \param interp     - The interpreter running this command.
 * \param objv       - The object encapsulated Tcl_Obj that are
 *                     the command parameters.
 * \return int
 * \retval TCL_OK   - Command completed correctly.
 * \retval TCL_ERROR - One of the following errors  occured:
 *                      - InputStageRunning
 *                      - DoesNotExist
 */
int
InputStageCommand::destroyInputStage(CTCLInterpreter& interp,
		      						 std::vector<CTCLObject>& objv)
{
  if(m_pInputStage) {
    if (!m_pInputStage->isRunning()) {
      delete m_pInputStage;
      m_pInputStage = static_cast<InputStage*>(0);
      return TCL_OK;
    }
    else {
      return AssemblerErrors::setErrorMsg(interp,
					 AssemblerErrors::Running,
					 "Stop it and try again");
    }
  }
  else {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::DoesNotExist,
					"Need to create before destroying");
  }
  
}
/////////////////////////////////////////////////////////////
/*!
 * Start the input stage
 * \pre m_pInputStage is non zero.
 * \pre m_pInputStage->isRunning() returns false.
 * \post m_pInputStage->isRunning() returns true.
 * 
 * \param interp		- Tcl interpreter running this command.
 * \param objv          - Vector of object encapsulated Tcl_Obj
 *                        that make up the command.
 * \return int
 * \retval TCL_OK    - The input stage started.
 * \retval TCL_ERROR - One of the following conditions:
 *                     DoesNotExist
 *                     Running.
 */
int
InputStageCommand::startInputStage(CTCLInterpreter& interp,
				   std::vector<CTCLObject>& objv)
{
  if (m_pInputStage) {
    if (!m_pInputStage->isRunning()) {
      m_pInputStage->start();
      return TCL_OK;
    }
    else {
      return AssemblerErrors::setErrorMsg(interp,
					  AssemblerErrors::Running,
					  "(InputStage)");
      
    }
  } 
  else {
    return	AssemblerErrors::setErrorMsg(interp,
					     AssemblerErrors::DoesNotExist,
					     "(InputStage)");
  }
}
////////////////////////////////////////////////////////////////
/*!
 *  Stop the input stage.
 * \pre m_pInputStage is non zero.
 * \pre m_pInputStage->isRunning() is true.
 * \post m_pInputStage->isRunning() is false
 * \param interp		- Tcl interpreter running this command.
 * \param objv          - Vector of object encapsulated Tcl_Obj
 *                        that make up the command.
 * \return int
 * \retval TCL_OK    - The input stage stopped.
 * \retval TCL_ERROR - One of the following conditions:
 *                     DoesNotExist
 *                     Stopped
 */
int
InputStageCommand::stopInputStage(CTCLInterpreter& interp,
				  std::vector<CTCLObject>& objv)
{
  if (m_pInputStage) {
    if (m_pInputStage->isRunning()) {
      m_pInputStage->stop();
      return TCL_OK;
      
    }
    else {
      return AssemblerErrors::setErrorMsg(interp,
					  AssemblerErrors::Stopped,
					  "(InputStage)");
    }
  }
  else {
    return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::DoesNotExist,
					"(InputStage)");
  }
}
///////////////////////////////////////////////////////////////
/*!
 *  Return a Tcl list of input stage statistics.
 *  See the discussion of the return value for more information
 *  on the format of this list.
 *  \pre m_pInputStage is non zero.
 * \param interp - Interpreter that's running this command
 * \param objv   - Vector of object encapsulated Tcl_Obj's that
 *                 are the command line words (ignored)
 * \return int
 * \retval TCL_ERROR - A "DoesNotExist" error occured becuse
 *                     the precondition described above was not
 *                     met.
 * \retval TCL_OK    - The command succeeded.  In this case, the
 *                     interpreter result is set to a Tcl List
 *                     that contains the statistics. This is a three
 *                     element list.  The list elements are as follows:
 *                   - List of node fragment count pairs
 *                   - List of type fragment count pairs.
 *                   - List of  lists, one for each node from which
 *                     fragments have been received. Each node list
 *                     contains the node number followed by 
 * 	                   type count fragment pairs.
 */
int
InputStageCommand::statistics(CTCLInterpreter& interp,
	 	   					  std::vector<CTCLObject>& objv)
{
	if (m_pInputStage) {
		vector<AssemblerUtilities::typeCountPair> nodefrags = 
				m_pInputStage->nodeFragmentCount();
		vector<AssemblerUtilities::typeCountPair> typefrags = 
				m_pInputStage->perTypeFragmentCount();
		vector<pair<uint16_t, std::vector<AssemblerUtilities::typeCountPair> > >
			nodebyfrags = m_pInputStage->nodePerTypeFragmentCount();
		
		// Now build the lists:
		
		CTCLObject result;
		result.Bind(interp);
		
		// The first element of the list, fragment counts per node:
		
		CTCLObject* nodeFragcountList =
			AssemblerUtilities::typeValuePairToList(interp, nodefrags);
		result += *nodeFragcountList;
		delete nodeFragcountList;
		
		// The second element of the list is fragment counts per type:
		
		CTCLObject* typeCountList = AssemblerUtilities::typeValuePairToList(interp, typefrags);
		result += *typeCountList;
		delete typeCountList;
		
		//  Now do the types by node:
		
		CTCLObject nodeTypeCountList;
		nodeTypeCountList.Bind(interp);

		for (int i =0; i < nodebyfrags.size(); i++) {
			uint16_t node = nodebyfrags[i].first;
			CTCLObject* nodeList =
				AssemblerUtilities::typeValuePairToList(interp, 
						    							nodebyfrags[i].second);
			CTCLObject nodeItem;
			nodeItem.Bind(interp);
			nodeItem += node;
			nodeItem += *nodeList;
			nodeTypeCountList += nodeItem;
			delete nodeList;
		}
		result += nodeTypeCountList;

		// All done set the result and return normal:
		
		interp.setResult(result);
		return TCL_OK;
		
	}
	else {
		return AssemblerErrors::setErrorMsg(interp,
							AssemblerErrors::DoesNotExist,
											"(InputStage)");
	}
	
}
////////////////////////////////////////////////////////////
/*!
 *   Clear the input stage statistics.
 * \pre m_pInputStage is non null.
 * \param interp - Interpreter that's running this command
 * \param objv   - Vector of object encapsulated Tcl_Obj's that
 *                 are the command line words (ignored)
 * \return int
 * \retval TCL_ERROR - A "DoesNotExist" error occured becuse
 *                     the precondition described above was not
 *                     met.
 * \retval TCL_OK    - Command succeeded.
 */
int
InputStageCommand::clearStatistics(CTCLInterpreter& interp,
									std::vector<CTCLObject>& objv)
{
	m_pInputStage->clearStatistics();
}
////////////////////////////////////////////////////////////////
/*!
 *  Injects a buffer into the input stage for testing.
 *  The command requires a parameter that is the buffer contents.
 *  The buffer contents is a set of numbers.
 *  Each number is considered to be a 16 bit number that
 *  will be put in the buffer without any interpretation.
 *  Each non number will be a string that is padded out to an odd
 *  number of bytes and placed null terminated (requiring an even
 *  number of bytes) in the buffer.
 *  
 *  It's the script's responsibility to ensure that the
 *  buffer is acutally sensible.  nonsense buffers can cause
 *  the InputStage to toss errors that are caught here and reported
 * as an ExceptionEvent
 * 
 * \pre m_pInputStage is non null.
 * \pre m_pInputStage->isRunning is true.
 * \param interp - Interpreter that's running this command
 * \param objv   - Vector of object encapsulated Tcl_Obj's that
 *                 are the command line words
 * \return int
 * \retval TCL_ERROR - One of the following:
 *                    - DoesNotExist,
 *                    - Stopped,
 *                    - ExceptionEvent (exceptions from input stage)
 * \retval TCL_OK    - The command succeeded. 
 */
int
InputStageCommand::inject(CTCLInterpreter& interp,
		   				  std::vector<CTCLObject>& objv)
{
	// Check the preconditions:
	
	if (!m_pInputStage) {
		return AssemblerErrors::setErrorMsg(interp,
											AssemblerErrors::DoesNotExist,
											"(InputStage)");
	}
	if (!m_pInputStage->isRunning()) {
		return AssemblerErrors::setErrorMsg(interp,
							AssemblerErrors::Stopped,
							"(InputStage)");
	}
	// The Buffer must at least be a full buffer:
	
	objv[2].Bind(interp);
	if (objv[2].llength() < 16) {
		string result;
		result += "Attempting to inject and event that doesn't\n";
		result += "even have a full header. Giving up";
		interp.setResult(result);
		return TCL_ERROR;
	}
	// Now build the buffer:
	
	uint16_t   buffer[BUFFERSIZE];
	uint16_t*  pDest(buffer);
	
	for (int i=0; i < objv[2].llength(); i++) {
		CTCLObject bufferElement = objv[2].lindex(i);
		bufferElement.Bind(interp);
		// If an integer install as an integer..otherwise
		// install as a string:
		
		try {
			int value = bufferElement;
			pDest = static_cast<uint16_t*>(installInt(pDest, value));
		}
		catch(...)  {
			// Not an int...
		  string value = bufferElement;
		  pDest = static_cast<uint16_t*>(installString(pDest, value));
		  
		}
	}
	
	// Dispatch it to the right buffer processor based on type.
	
	uint16_t type = buffer[1];   // Buffer type...

	
	try {
	  m_pInputStage->processBuffer(buffer);
	}
	catch(InvalidNodeException err) {
	  return AssemblerErrors::setErrorMsg(interp,
					      AssemblerErrors::NoSuchId,
					      "Injecting data");
	}
	catch (EventTooSmallException err) {
	  return AssemblerErrors::setErrorMsg(interp,
					      AssemblerErrors::TooSmallForTimestamp,
					      "Injecting data");
	}
	catch (...) {
		return AssemblerErrors::setErrorMsg(interp,
				         AssemblerErrors::ExceptionEvent,
				         "InputStage threw exception on injected buffer");
	}
	return TCL_OK;
	
}
////////////////////////////////////////////////////////////////
/*!
 *  Add a script to get callbacks dispatched from the
 * InputStage.  Any existing established script is removed.  
 * \pre m_pInputStage is not null
 * \param interp      - Tcl interpreter that is executing this command
 *                      and that we will make execute the script.
 * \param objv        - words that make up the command. objv[2]
 *                      is the script.
 * \return int
 * \retval TCL_OK     - command succeeded.
 * \retval TCL_ERROR  - Command failed:
 *                      DoesNotExist
 */
int
InputStageCommand::monitor(CTCLInterpreter& interp,
		                   std::vector<CTCLObject>& objv)
{
	if (m_pInputStage) {
		// Only make a new object if necessary..otherwise,
		// replacing the old one is good enough.
		
		if (!m_pScript) {
			m_pScript = new CTCLObject;
			m_pScript->Bind(interp);
		}
		objv[2].Bind(interp);
		*m_pScript = objv[2];
		m_pScript->Bind(interp);    // Probably don't need this.
		
		// always unregister/register.
		
		m_pInputStage->removeCallback(dispatchMonitorScript, m_pScript);
		m_pInputStage->addCallback(dispatchMonitorScript, m_pScript);
		return TCL_OK;
	}
	else {
		return AssemblerErrors::setErrorMsg(interp,
						    AssemblerErrors::DoesNotExist, 
						    "(InputStage)");
	}
}
/////////////////////////////////////////////////////////////
/*!
 * Remove the monitor script.  We'll delete the script too.
 * If the input stage does not exist, we'll delete the script
 * object.
 * \param interp     - TCL Interpreter running the command.
 * \param objv       - Command line words.
 * \return int
 * \retval TCL_OK  - This one can't fail.
 */
int
InputStageCommand::unmonitor(CTCLInterpreter& interp,
		                     std::vector<CTCLObject>& objv)
{
  if (m_pInputStage && m_pScript) {
    m_pInputStage->removeCallback(dispatchMonitorScript, m_pScript);
  }
  delete m_pScript;
  m_pScript=0;
  return TCL_OK;
}
///////////////////////////////////////////////////////////
/*!
 *    Returns an event from a specified node.
 * \pre m_pInputStage not null.
 * \pre m_pInputStage->isRunning() is true.
 * \param interp   - Tcl interpreter running the command.
 * \param objv     - objv[2] is the word containing the node
 * \return int
 * \retval TCL_OK  - success.  In this case,
 *                   the result conatins a 4 element list
 *                   consisting of the node, timestamp event type,
 *                   and the body of the event.
 * \retval TCL_ERROR - error:
 *                        DoesNotExist,
 *                        Stopped,
 *                        NoSuchId,
 *                        BadId,
 *                        Empty
 */
int
InputStageCommand::get(CTCLInterpreter& interp,
   					   std::vector<CTCLObject>& objv)
{
	return getEvent(interp, objv, &InputStage::peek);
}

/*!
 * Returns an event from a specified node queue, destructively,
 * that is the event is not on the queue after we've returned.
 * Same preconditions, parameters and return specifications
 * as the previous member.
 */
int
InputStageCommand::pop(CTCLInterpreter& interp,
   		               std::vector<CTCLObject>& objv)
{
	return getEvent(interp, objv, &InputStage::pop);
}

/*!
 * Clear an event queue of data.
 * This is not done very efficiently, we are just
 * call pop until we get an error...then fix up the
 * result to empty and return value to TCL_OK.
 * This is not strictly by the book but it does
 * re-use code to beat the band.
 */
int
InputStageCommand::empty(CTCLInterpreter& interp,
						vector<CTCLObject>& objv)
{
	while (getEvent(interp, objv, &InputStage::pop) == TCL_OK)
		;
	// What's not by the book about the following is that errors
	// are not reported but perverted into a successful return:
	//
	interp.setResult("");
	return TCL_OK;
}
///////////////////////////////////////////////////////////
/*!
 * Return the usage string for the command.
 */
string
InputStageCommand::Usage()
{
	string usage;
	usage   = "Usage\n";
	usage  += "   inputstage create\n";
	usage  += "   inputstage destroy\n";
	usage  += "   inputstage start\n";
	usage  += "   inputstage stop\n";
	usage  += "   inputstage statistics\n";
	usage  += "   inputstage clear\n";
	usage  += "   inputstage inject >buffer<\n";
	usage  += "   inputstage monitor >script<\n";
	usage  += "   inputstage umonitor\n";
	usage  += "   inputstage get >node<\n";
	usage  += "   inputstage pop >node<\n";
	usage  += "   inputstage empty >node<\n";
	
	return usage;
}


/*
**  Install an integer int a test buffer.
**    This is assumed to be a 16 bit integer.
*/
void*
InputStageCommand::installInt(void* dest,
			      int   value)
{
  uint16_t* p = reinterpret_cast<uint16_t*>(dest);
  *p++        = value;
  return reinterpret_cast<void*>(p);
}

/*
 *   Install a text string in a test buffer.
 *   The string is padded with an extra null if needed
 *   so that the string ends on an even boundary.
 */
void*
InputStageCommand::installString(void*   dest,
				 string  value)
{
  char* p   = reinterpret_cast<char*>(dest);

  size_t nchar = value.size() +1; //  include the null.

  strcpy(p, value.c_str());
  p += nchar;			// Include the null.

  if (nchar % 2) {			// Padd if odd..
    *p++ = '\0';
  }


  return reinterpret_cast<char*>(p);
}

/*
 * Dispatch to a monitor script.
 *   pData - Is  a pointer to the CTCLObject that
 *           contains the script.
 *   evt   - Is the event that caused the callback.
 *   node  - Is the node that the event occured on.
 *
 *  The script is called with the following trailing parameters.
 *     evt   - Text version of evt which can be one of:
 *             new      - NewFragments.
 *             shutdown - ShuttingDown
 *             startup  - Starting
 *             error    - Error.
 *             unknown  - Should never happen, means the eventToString
 *                        function is not keeping pace with the
 *                        event type codes.
 *
 *     node - Node on which the event occured.
 * Errors are reported via
 *   Tcl_BackgroundError.
 * The script object must have already been bound to an interpreter
 * and that interpreter will be used for all operations that need interps.
 *
 *
 */
void 
InputStageCommand::dispatchMonitorScript(void* pData, 
					InputStage::event evt,
					uint16_t node)
{
  CTCLObject* pObject = reinterpret_cast<CTCLObject*>(pData);

  // commands are lists so we can add the parameters as arguments by
  // appending their stringification to the object...to prevent
  // perversions of the existing object we copy the original (of course).

  CTCLObject* pScript = new CTCLObject(*pObject);

  string eventName = InputStage::eventToString(evt);
  (*pScript)      += eventName;
  (*pScript)      += node; 

  try {
    (*pScript)();
  }
  catch (...) {
    Tcl_BackgroundError(pScript->getInterpreter()->getInterpreter());
  }
  delete pScript;		// Get rid of temp storage.
}


/*
 * Execute one of the get event operations using the
 * specified InputStage:: member function to actually
 * get the itme.
 */
int
InputStageCommand::getEvent(CTCLInterpreter& interp,
			    vector<CTCLObject>& objv,
			    fragGetter member)
{
	if (!m_pInputStage) {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::DoesNotExist,
				"(InputStage)");
	}
	if(!m_pInputStage->isRunning()) {
	  return AssemblerErrors::setErrorMsg(interp,
					      AssemblerErrors::DoesNotExist,
					      "(InputStage)");
	}
	// The node parameter must be a valid unsigned integer.
	
	int node;
	try {
		objv[2].Bind(interp);
		node = objv[2];
		if (node < 0) throw "<0";
	}
	catch (...) {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::BadId,
				"Node ids must be integers >= 0");
	}
	// The node parameter must be < 0x1000.
	
	if (node < 0x1000) {
	  try {
	    EventFragment* pEvent = (m_pInputStage->*member)((uint16_t)node);
	    if (pEvent) {
	      CTCLObject* pDecodedEvent = eventToList(interp, *pEvent);
	      interp.setResult(*pDecodedEvent);
	      delete pDecodedEvent;
	      return TCL_OK;
	    }
	    else {
	      return AssemblerErrors::setErrorMsg(interp,
						  AssemblerErrors::Empty,
						  "(InputStage::get/pop)");
	    }
	  } catch(...) {
	    return AssemblerErrors::setErrorMsg(interp,
						AssemblerErrors::NoSuchId,
						"(InputStage::get/pop)");
	  }
	}
	else {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::NoSuchId,
				"Id must be < 0x1000 to exist");
	}	
}

/*
 * Convert an event fragment to a list as specified in the 
 * requirements.  The list contains, in order, 
 *   - The node id.
 *   - The timestamp, which is 0 for non event fragments but
 *     the 32 bit words at offset 1 for event fragments.
 *   - The body as a list of 16 bit words.
 */
CTCLObject* 
InputStageCommand::eventToList(CTCLInterpreter& interp,
				EventFragment& fragment)
{
  CTCLObject* pResult = new CTCLObject();
  pResult->Bind(interp);

  // Start building the list:

  (*pResult) += fragment.node();
  
  if (fragment.type() == DATABF) {
    
    PhysicsFragment* pFrag = reinterpret_cast<PhysicsFragment*>(&fragment);
    (*pResult) += static_cast<int>(pFrag->getTimestamp());

  }
  else {
    (*pResult) += 0;
  }
  (*pResult) += fragment.type();

  // Do the body now.


  vector<uint16_t> body = fragment.body();
  CTCLObject bodyList;
  bodyList.Bind(interp);

  for (int i=0; i < body.size(); i++) {
    bodyList += static_cast<int>(body[i]);
  }

  (*pResult)+= bodyList;

  return pResult;

}
