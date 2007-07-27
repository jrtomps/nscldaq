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
#include <TCLObject.h>
#include "AssemblerErrors.h"
#include <buftypes.h>
#include <stdint.h>
/*
 * Initialize the dispatch table.  Each
 * element of the dispatch table is used to
 * determine which member function will actually
 * process a subcommand.
 */

InputStageCommand::pDispatchTable InputStageCommand::m_dispatchTable =
{
		{"create",    2,  &InputStageCommand::createInputStage},
		{"destroy",   2,  &InputStageCommand::destroyInputStage},
		{"start",     2,  &InputStageCommand::startInputStage},
		{"stop",      2,  &InputStageCommand::stopInputStage},
		{"statistics",2,  &InputStageCommand::statistics},
		{"clear",     2,  &InputStageCommand::clearStatistics},
		{"inject",    3,  &InputStageCommand::inject},
		{"monitor"    3,  &InputStageCommand::monitor},
		{"unmonitor", 2,  &InputStageCommand::unmonitor},
		{"get",       3,  &InputStageCommand::get},
		{"pop",       3,  &InputStageCommand::pop},
		{"empty",	  3,  &InputStageCommand::empty}
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
			m_pInputStaget->stop();
		}
		delete m_pInputStage;
		m_pInputStage = 0;
	}
	delete m_pScript;
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
InputStageCommand::operator()(CTCLInterprete& interp,
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
				return this->*m_dispatchTable[index].m_processor(interp, objv);
			} 
			else {
				return AssemblerErrors::setErrorMsg(interp,
						objv.size() < m_dispatchTable[index].m_parameterCount ?
									AssemblerErrors::TooManyParameters :
									AssemblerErrors::TooFewParameters,
									Usage());)
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
		m_pInputStage = new InputStage(m_pConfig);
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
			m_pInputStage = static_cast<InputStage*>(m_pInputStage);
			return TCL_OK;
		}
		else {
			return AssemblerErrors:setErrorMsg(interp,
											   AssemblerErrors::InputStageRunning,
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
										AssemblerInputStage::Runnning,
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
								AssemblerInputStage::Stopped,
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
		vector<InputStage::typeCountPair> nodefrags = 
				m_pInputStage->nodeFragmentCount();
		vector<InputStage::typeCountPair> typefrags = 
				m_pInputStage->perTypeFragmentCount();
		vector<pair<uint16_t, std::vector<InputStage::typeCountPair> > >
			nodebyfrags = m_pInputStage->nodePerTypeFragmentCount();
		
		// Now build the lists:
		
		CTCLObject result;
		result.Bind(interp);
		
		// The first element of the list, fragment counts per node:
		
		CTCLObject* nodeFragcountList =
			typeValuePairToList(interp, nodefrags);
		result += *nodeFragcountList;
		delete nodeFragcountList;
		
		// The second element of the list is fragment counts per type:
		
		CTCLObject* typeCountList =
			typeValuePairTolist(interp, typefrags);
		result += *typeCountList;
		delete typeCountList;
		
		//  Now do the types by node:
		
		for (int i =0; i < nodePerTypeFragmentCount.size(); i++) {
			uint16_t node = nodePerTypeFragmentCount[i].first;
			CTCLObject* nodeList =
				typeValuePairToList(interp, 
									nodePerTypeFragmentCount[i].second);
			CTCLObject nodeItem;
			nodeItem.Bind(interp);
			nodeItem += node;
			nodeItem += nodeList;
			result   += nodeItem;
			delete nodeList;
		}
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
 *                 are the command line words (ignored)
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
	if (!m_InputStage->isRunning()) {
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
	uint16_t*  pDest;
	
	for (int i=0; i < objv[2].llength(); i++) {
		CTCLObject bufferElement = objv[2].lindex(i);
		bufferElement.Bind(interp);
		// If an integer install as an integer..otherwise
		// install as a string:
		
		try {
			int value = bufferElement;
			pDest = installInt(pDest, value);
		}
		catch(...)  {
			// Not an int...
			
			string value = bufferElement;
			pDest = installString(pDest, value);
		}
	}
	
	// Dispatch it to the right buffer processor based on type.
	
	uint16_t type = buffer[1];   // Buffer type...
	
	try {
		switch (type) {
		case DATABF:
			m_pInputStage->processPhysicBuffer(buffer);
			break;
		case SCALERBF:
		case SNSAPSCBF:
			m_pInputStage->processScalerBuffer(buffer);
			break;
		case STATEVARBF:
		case RUNVARBF:
		case PKTDOCBF:
		case PARAMDESCRIP:
			m_pInputStage->processStringlistBuffer(buffer);
			break;
		case BEGRUNBF:
		case ENDRUNBF:
		case PAUSEBF:
		case RESUMEBF:
			m_pInputStage->processStateChangeBuffer(buffer);
			break;
		default:
			// Bad buffer type...
			{
				string result = "Injecting an unrecognized buffer type";
				interp.setResult(result);
				return TCL_ERROR;
			}
		}
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
		objv[2].Bind(inter);
		*m_pScript = objv[2];
		m_pScript->Bind(interp);    // Probably don't need this.
		
		// always unregister/register.
		
		m_pInputStage->removeCallback(dispatchMonitor, m_pScript);
		m_pInputStage->addCallback(dispatchMonitor, m_pScript);
		return TCL_OK;
	}
	else {
		return AssemblerErrors(interp,
				AssemblerErrors::DoesNotExist, "(InputStage)");
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
		m_pInputStage->removeCallback(dispatchMonitor, m_pScript);
	}
	delete m_pScript;
	m_pScript=0;
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
	return getEvent(interp, objv, InputStage::peek);
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
	return getEvent(interp, objv, InputStage::pop);
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
	while (getEvent(interp, objv, InputStage::pop) == TCL_OK)
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
InputStageCommand::Usage() const
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
///////////////////////////////////////////////////////////
/*
 * Convert a type/value vector to a Tcl list.
 * The list is dynamically created here and must be deleted
 * by the caller.
 * Parameters:
 *    interp  - Interpreter to which the list will be bound.
 *    stats   - vector of type value pairs to build into the list
 * Returns:
 *    A pointer to the stocked list.
 */
CTCLObject*
InputStage::typeValuePairToList(CTCLInterpreter& interp,
	                            vector<InputStage::typeCountPair>& stats)
{
	CTCLObject* pObject = new CTCLObject;
	pObject->Bind(interp);
	
	for (int i=0; i < stats.size(); i++ ) {
		CTCLObject pair;
		pair.Bind(interp);
		pair += stats[i].first;
		pair += stats[i].second;
		
		(*pObject) += pair;
	}
	return pObject;
}

/*
 * Execute one of the get event operations using the
 * specified InputStage:: member function to actually
 * get the itme.
 */
int
InputStageCommand::getEvent(CTCLInterpreter& interp,
							vector<CTCLObject>& objv,
							InputStageCommand::fragGetter member)
{
	if (!m_pInputStage) {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::DoesNotExist,
				"(InputStage)");
	}
	if(!m_pInputStage->isRunning()) {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::DoesNotExist,
				"(InputStage)")
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
		AssemblerEvent* pEvent = m_pInputStage->*member((uint16_t)node);
		if (pEvent) {
			CTCLObject* pDecodedEvent = eventToList(interp, *pEvent);
			interp.setResult(pDecodedEvent);
			delete pDecodedEvent;
			return TCL_OK;
		}
		else {
			return AssemblerErrors::setErrorMsg(interp,
					AssemblerErrors::Empty,
					"(InputStage::get/pop)");
		}
	}
	else {
		return AssemblerErrors::setErrorMsg(interp,
				AssemblerErrors::NoSuchId,
				"Id must be < 0x1000 to exist");
	}	
}