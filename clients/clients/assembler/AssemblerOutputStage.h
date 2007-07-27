#ifndef __ASSEMBLEROUTPUTSTAGE_H
#define __ASSEMBLEROUTPUTSTAGE_H
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


#ifndef __TCLOBJECTPROCESSOR_h
#include <TCLObjectProcessor.h>
#endif


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CTCLInterpreter;
class CNSCLPhysicsBuffer;
class CNSCLOutputBuffer;
class AssembledEvent;


/*!
   The AssemblerOutputStage is the output stage of the assembler
   that interfaces with spectrodaq-lite.  This member is a
   command processor for Tcl implementing the outputstage
   command ensemble that allows one to probe output stage statistics
   as well as to generate data for testing the subsystem:
\verbatim

outputstage  clear
outputstage  statistics
outputstage  event  size
outputstage  control type node entitycount body

\endverbatim

  Programmatically note that we are assuming that spdaq-lite's data store
  has already been set up and we can just go ahead and make DAQWordBuffer's
  to our heart's content.

*/
class AssemblerOutputStage : public CTCLObjectProcessor
{
private:
  typedef int (AssemblerOutputStage::*CommandProcessor)(CTCLInterpreter&,
						       std::vector<CTCLObject>&);
  typedef void (AssemblerOutputStage::*EventProcessor)(AssembledEvent&);
  typedef struct _DispatchTableEntry {
    const char*      pKeyword;	       // Subcommand keyword
    int              parameterCount;   // Number of command parameters allowed
    CommandProcessor processor;        // member function pointer to processor.

  } DispatchTableEntry, *pDispatchTableEntry;
private:

  static const int SUBCOMMANDCOUNT = 4;
  static const int VALIDBUFFERTYPES = 16;

  static DispatchTableEntry m_commandDispatchTable[SUBCOMMANDCOUNT];
  static EventProcessor     m_eventProcessors[VALIDBUFFERTYPES];


  CNSCLPhysicsBuffer*        m_pPhysicsBuffer; // Buffer being built from phys events.
  unsigned long              m_eventsReceived;
  unsigned long              m_buffersSubmitted;
  unsigned long              m_buffersByType[VALIDBUFFERTYPES];
  unsigned long              m_eventsByNode[0x10000];
  unsigned int               m_runNumber;

public:
  AssemblerOutputStage(CTCLInterpreter& interp);
  ~AssemblerOutputStage();

  // The canonicals below are not implemented as they make no sense.

private:
  AssemblerOutputStage(const AssemblerOutputStage& rhs);
  AssemblerOutputStage& operator=(const AssemblerOutputStage& rhs);
  int operator==(const AssemblerOutputStage& rhs) const;
  int operator!=(const AssemblerOutputStage& rhs) const;
public:



public:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
  void submitEvent(AssembledEvent& event);

protected:
  int clearStatistics(CTCLInterpreter& interp,
		      std::vector<CTCLObject>& objv);
  int reportStatistics(CTCLInterpreter& interp,
		       std::vector<CTCLObject>& objv);
  int physicsEvent(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);
  int controlEvent(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);

  std::string Usage();
private:
  void appendPhysicsEvent(AssembledEvent& event);
  void makeScalerBuffer(AssembledEvent& event);
  void makeStateChangeBuffer(AssembledEvent& event);
  void makeDocumentationBuffer(AssembledEvent& event);
  void invalidEvent(AssembledEvent& event);

  void commitPhysicsBuffer();
  void commitBuffer(CNSCLOutputBuffer& buffer);
  void clearCounters();


  bool isStateTransitionEvent(int type);
  bool isScalerEvent(int type);
  bool isDocumentationEvent(int type);

  int submitFakeStateTransition(CTCLInterpreter& interp,
				int              type,
				int              node,
				CTCLObject&      body);
  int submitFakeScalerEvent(CTCLInterpreter& interp,
			    int              type,
			    int              node,
			    CTCLObject&      body);
  int submitFakeDocumentationEvent(CTCLInterpreter& interp,
				   int              type,
				   int              node,
				   CTCLObject&      body);
  void newPhysicsBuffer();
  void initializeHeader(CNSCLOutputBuffer& buffer);
};
#endif
