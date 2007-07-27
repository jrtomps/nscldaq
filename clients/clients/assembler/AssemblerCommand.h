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
#ifndef __ASSEMBLERCOMMAND_H
#define __ASSEMBLERCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
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

class CTCLInterpreter;
class CTCLObject;

/*!  
 The AssemblerCommand class implements the assembler 
 ensemble of commands that configures the 
 event assembler.
\verbatim

assembler node dns/ip id
assembler trigger id
assembler window id width ?offset?
assembler list
assembler validate

\endverbatim
*/


class AssemblerCommand : public CTCLObjectProcessor
{
public:
  typedef struct {
    char*             pNodeName;
    uint32_t          ipAddress;     // Octets of ip.
    unsigned short    cpuId;
    bool              isTrigger;
    bool              windowDefined;
    unsigned int      windowWidth;
    bool              offsetDefined;
    int               offset;
  } EventFragmentContributor, *pEventFragmentContributor;

// Dispatch table structure.. The dispatch table allows
// table driven dispatch of the command subkeywords:

typedef int (AssemblerCommand::*CommandProcessor)(CTCLInterpreter&,
						  std::vector<CTCLObject>&);

typedef struct {
    const char* pKeyword;
    int         minParameters;
    int         maxParameters;
  CommandProcessor processor;

} DispatchEntry, *pDispatchEntry;
  static const int SUBCOMMANDCOUNT = 6;

private:

  EventFragmentContributor   m_nodeTable[0x10000];    // 65K entries.
  std::list<int>             m_definedNodes;          // List of used indices

  static DispatchEntry DispatchTable[SUBCOMMANDCOUNT];

public:
	AssemblerCommand(CTCLInterpreter& interpreter);
	~AssemblerCommand();
private:
	AssemblerCommand(const AssemblerCommand& rhs);
	AssemblerCommand& operator=(const AssemblerCommand& rhs);
	int operator==(const AssemblerCommand& rhs);
	int operator!=(const AssemblerCommand& rhs);
public:
	// Fetch used nodes:

	std::list<EventFragmentContributor> getConfiguration();

	//! Command dispatcher:
	
	virtual int operator()(CTCLInterpreter &interp, 
                           std::vector<CTCLObject>& objv);


	// Command processors for the ensemble commands:
protected:
	int node(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
	int trigger(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
	int window(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
	int list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
	int validate(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
	int clear(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
	pEventFragmentContributor findNode(unsigned short id);
	pEventFragmentContributor findNode(const char* name);
	
	std::string Usage();
	int Dispatch(std::string keyword,
		     CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
	void clearTrigger();
	void describeNode(CTCLObject&               description,
			  EventFragmentContributor& node);
	void clearTables();

  static char* copyString(const char* src);
};

#endif
