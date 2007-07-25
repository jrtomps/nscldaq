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
	m_pConfiguration(&config)
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
 * \retval TCL_OK    - The input stage stopped.
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
							AssemblerErrors::DoesNotExist);
	}
}
////////////////////////////////////////////////////////////////
/*!
 * 