#ifndef __INPUTSTAGECOMMAND_H
#define __INPUTSTAGECOMMAND_H

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
#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __ASSEMBLEROUTPUTSTAGE_H
#include <AssemblerOutputStage.h>
#endif

#ifndef __INPUTSTAGE_H
#include <InputStage.h>
#endif


// Forward class definitions:

class CTCLInterpreter;
class CTCLObject;
class CTCLProcessor;
class AssemblerCommand;
class EventFragment;
/*!
 *  This class provides the command/control for the
 * input stage.  It implements a tcl command that can be used
 * to control and test the input stage software (InputStage class)
 * The format of this command ensemble is:
 * inputstage create
 * inputstage destroy
 * inputstage start
 * inputstage stop
 * inputstage statistics
 * inputstage clear
 * inputstage inject
 * inputstage monitor
 * inputstage umonitor
 * inputstage get
 * inputstage pop
 * inputstage empty
 * 
 */
class InputStageCommand : public CTCLObjectProcessor
{
	// Private data types:
private:
	typedef int (InputStageCommand::*CommandProcessor)(CTCLInterpreter&,
							   std::vector<CTCLObject>&);
	typedef int (InputStage::*EventGetter)(uint16_t node);
	typedef struct {
		std::string 		m_keyword;
		unsigned     		m_parameterCount;
		CommandProcessor	m_processor;
	}DispatchTable, *pDispatchTable;
	
	typedef EventFragment* (InputStage::*fragGetter)(uint16_t);
	// Private data.
private:
	static DispatchTable    m_dispatchTable[];
	InputStage*             m_pInputStage;
	AssemblerCommand*       m_pConfiguration;
	CTCLObject*             m_pScript;
public:	
	// Constructors and canonicals.
	
	InputStageCommand(CTCLInterpreter& interp,
			  AssemblerCommand&   config);
	~InputStageCommand();
	// Invalid canonicals:
	
private:
	InputStageCommand(const InputStageCommand& rhs);
	InputStageCommand& operator=(const InputStageCommand& rhs);
	int operator==(const InputStageCommand& rhs);
	int operator!=(const InputStageCommand& rhs);
public:
	
	// Public methods.
	
	int operator()(CTCLInterpreter& interp,
				   std::vector<CTCLObject>& objv);
	int createInputStage(CTCLInterpreter& interp,
				   		 std::vector<CTCLObject>& objv);
	int destroyInputStage(CTCLInterpreter& interp,
	   		 		      std::vector<CTCLObject>& objv);
	int startInputStage(CTCLInterpreter& interp,
	   		 			std::vector<CTCLObject>& objv);
	int stopInputStage(CTCLInterpreter& interp,
	   		 		   std::vector<CTCLObject>& objv);
	int statistics(CTCLInterpreter& interp,
	   		 	   std::vector<CTCLObject>& objv);
	int clearStatistics(CTCLInterpreter& interp,
	   		 			std::vector<CTCLObject>& objv);
	int inject(CTCLInterpreter& interp,
	   		   std::vector<CTCLObject>& objv);
	int monitor(CTCLInterpreter& interp,
	   		    std::vector<CTCLObject>& objv);
	int unmonitor(CTCLInterpreter& interp,
	   		      std::vector<CTCLObject>& objv);
	int get(CTCLInterpreter& interp,
	   		std::vector<CTCLObject>& objv);
	int pop(CTCLInterpreter& interp,
	   		std::vector<CTCLObject>& objv);
	int empty(CTCLInterpreter& interp,
	   		  std::vector<CTCLObject>& objv);
	static std::string Usage();

	InputStage* getInputStage();
private:
	
	// Private utilities.
		

	void*   installInt(void* dest, 
			   int   value);
	void*   installString(void* dest,
			      std::string value);
	static  void dispatchMonitorScript(void* pData, 
					   InputStage::event evt,
					   uint16_t node);

	int getEvent(CTCLInterpreter&         interp,
		     std::vector<CTCLObject>& objv,
		     fragGetter member);

	CTCLObject* eventToList(CTCLInterpreter& interp,
				EventFragment& fragment);
	
};

#endif
