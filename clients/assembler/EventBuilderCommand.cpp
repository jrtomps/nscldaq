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
#include "EventBuilderCommand.h"
#include "EventBuilder.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "AssemblerOutputStage.h"
#include "AssemblerCommand.h"
#include "InputStage.h"
#include "AssemblerUtilities.h"


using namespace std;
////////////////////////////////////////////////////////////////////////
/*!
 *  Create ourself and the associated event builder.  Note that
 * most of our parameters are for the event builder constructor.
 * \param interpreter - TCL Interpreter that can run us as a command.
 *                       The command is hard-coded to "eventbuilder"
 * \param configuration  Reference to the assembler command that holds the
 *                       configuration.  This is passed to the event builder
 *                       constructor without interpretation.
 * \param fragmentSource Reference to the source for event fragments that will be
 *                       assembled into events.
 * \param eventSink Reference to the output stage to which the assembled events will be
 *                  submitted.
 */
EventBuilderCommand::EventBuilderCommand(CTCLInterpreter&      interpreter,
					 AssemblerCommand&     configuration,
					 InputStageCommand&    fragmentSource,
					 AssemblerOutputStage& eventSink) :
    CTCLObjectProcessor(interpreter, "eventbuilder"),
    m_pBuilder(new EventBuilder(configuration, fragmentSource, eventSink))
{
}
////////////////////////////////////////////////////////////////////
/*!
 *   Cleanup consists of killing of the event assembler.
 */
EventBuilderCommand::~EventBuilderCommand()
{
	delete m_pBuilder;
	
}
/////////////////////////////////////////////////////////////////////////////////
/*!
 *   The Tcl interpreter and TCL++ dispatch the "eventbuilder" command here.
 * The command is an ensemble that supports clearing and fetching the event builder's
 * statistics arrrays.  This top level dispatcher just checks that there are no further
 * parameters than the ensemble command and dispatches to the appropriate execution
 * function.  Since there are only two elements to the ensemble, the
 * dispatch is via if-else chains rather than by a table.
 * \param interpreter - Reference to the interpreter that is executing us.
 * \param objv        - Reference to a vector of encapsulated Tcl_Obj's that define
 *                      the words on the command line.
 * \return int
 * \retval TCL_OK - the command succeeded.
 * \retval TCL_ERROR -the command failed and the reason is in the interpreter's
 *                    result object.
 */
int 
EventBuilderCommand::operator()(CTCLInterpreter& interpreter,
								std::vector<CTCLObject>& objv)
{
	if (objv.size() != 2) {
		string result = "Invalid number of command words.\n";
		result       += usage();
		interpreter.setResult(result);
		return TCL_ERROR;
		
	}
	objv[1].Bind(interpreter);
	string ensembleCommand = objv[1];
	
	if(ensembleCommand == "clear") {
		return clear(interpreter);
	}
	else if (ensembleCommand == "stats") {
		return stats(interpreter);
	}
	else if (ensembleCommand == "reload") {
		return reload(interpreter);
	}
	else {
		string result = "Invalid command keyword\n";
		result += usage();
		return TCL_ERROR;
	}
}
////////////////////////////////////////////////////////////////////////////
/*!
 * Clears the event builder's statistics.
 * \param interpreter - the Tcl intpreter that is executing this command.
 * \return int  
 * \retval TCL_OK
 * \note - does not set the result string (leaving it clear).
 */
int
EventBuilderCommand::clear(CTCLInterpreter& interpreter)
{
	m_pBuilder->clear();
	return TCL_OK;
}
////////////////////////////////////////////////////////////////////////////
/*!
 * Returns the event builder's statistics as a Tcl list.
 * The list consists of a set of type/value pair lists so that items
 * with no counts are removed:
   - Breakdown of fragments by node.
   - Number of complete events built, by type.
   - Number of unmatched event fragments received discarded by node.
   - Number of unmatched state transition fragments received, discared by node.
   
   \param interpreter The Tcl interpreter that is executing this command.
   \return int
   \retval TCL_OK
   \note the information is returned in the interpeter's result.
   
  */
int
EventBuilderCommand::stats(CTCLInterpreter& interpreter)
{
	CTCLObject result;
	result.Bind(interpreter);
	
	EventBuilder::Statistics stats = m_pBuilder->statistics();
	
	// Sub lists...
	
	CTCLObject fragmentsByNode;
	fragmentsByNode.Bind(interpreter);
	CTCLObject completeEventsByType;
	completeEventsByType.Bind(interpreter);
	CTCLObject unmatchedFragmentsByNode;
	unmatchedFragmentsByNode.Bind(interpreter);
	CTCLObject discardedDueToTransitionsByNode;
	discardedDueToTransitionsByNode.Bind(interpreter);
	
	// Get the information and convert into the lists.

	vector<AssemblerUtilities::typeCountPair> statsSet;
	CTCLObject*                               statsList;
	
	statsSet = AssemblerUtilities::makeTypeCountVector(stats.fragmentsByNode, 0x10000);
	statsList= AssemblerUtilities::typeValuePairToList(interpreter, statsSet);
	result += (*statsList);
	delete statsList;
	
	statsSet = AssemblerUtilities::makeTypeCountVector(stats.eventsByType, 0x100);
	statsList = AssemblerUtilities::typeValuePairToList(interpreter, statsSet);
	result += (*statsList);
	delete statsList;
	
	statsSet = AssemblerUtilities::makeTypeCountVector(stats.unmatchedByNode, 0x10000);
	statsList= AssemblerUtilities::typeValuePairToList(interpreter, statsSet);
	result += (*statsList);
	delete statsList;
	
	statsSet = AssemblerUtilities::makeTypeCountVector(stats.discardedByNode, 0x10000);
	statsList=AssemblerUtilities::typeValuePairToList(interpreter, statsSet);
	result += (*statsList);
	delete statsList;

	interpreter.setResult(result);
	
	return TCL_OK;
}
///////////////////////////////////////////////////////////////////////////////
/*!
 *   Reload the event builder configuration.
 */
int
EventBuilderCommand::reload(CTCLInterpreter& interpreter)
{
	m_pBuilder->reloadConfiguration();
	return TCL_OK;
}
string
EventBuilderCommand::usage()
{
	string result = "Usage:\n";
	result       += "  eventbuilder clear\n";
	result       += "  eventbuidler stats\n";
	result       += "  eventbuilder reload\n";

	return result;
}

