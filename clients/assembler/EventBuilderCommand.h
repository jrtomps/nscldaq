#ifndef __EVENTBUILDERCOMMAND_H
#define __EVENTBUILDERCOMMAND_H
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

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class EventBuilder;
class CTCLInterpreter;
class CTCLObject;
class AssemblerOutputStage;
class AssemblerCommand;
class InputStageCommand;

/*!
 *   Provides a Tcl command interface to the event builder class.
 * furhtermore, the constructor of this command is also responsible for
 * creating the event builder object itself.
 * Here's the command ensemble supported by this class:
 \verbatim
   eventbuilder clear      ;# clears event builder statistics
   eventbuilder stats      ;# Gets the event builder statistics.
 \endverbatim
 */
class EventBuilderCommand : public CTCLObjectProcessor
{
private:
	EventBuilder*  m_pBuilder;
public:
	// Canonicals
	
	EventBuilderCommand(CTCLInterpreter&      interpreter,
			    AssemblerCommand&     configuration,
			    InputStageCommand&    fragmentSource,
			    AssemblerOutputStage& eventSink);
	virtual ~EventBuilderCommand();
private:
	EventBuilderCommand(const EventBuilderCommand& rhs);
	EventBuilderCommand& operator=(const EventBuilderCommand& rhs);
	int operator==(const EventBuilderCommand& rhs) const;
	int operator!=(const EventBuilderCommand& rhs) const;
public:
	int operator()(CTCLInterpreter& interpreter,
					std::vector<CTCLObject>& objv);
	int clear(CTCLInterpreter& interpreter);
	int stats(CTCLInterpreter& interpreter);
	int reload(CTCLInterpreter& interpreter);
private:
	static std::string usage();
};

#endif /*EVENTBUILDERCOMMAND_H_*/
