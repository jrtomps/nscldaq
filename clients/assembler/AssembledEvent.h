#ifndef __ASSEMBLEDEVENT_H
#define __ASSEMBLEDEVENT_H

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


/*!
   The AssembledEvent class is the base class for the hierarchy of 
   complete events that can be submitted to the buffer management
   system via the assembler's output stage.  
*/
class AssembledEvent {
  // Public data types:
public:
  enum BufferType {
    Physics        = 1,
    Scaler         = 2,
    SnapshotScaler = 3,
    StateVariables = 4,
    RunVariables   = 5,		// For now these are the same as
    Packets        = 6,		// corresponding  buffer types in
    BeginRun       = 11,	// buftypes.h
    EndRun         = 12,
    PauseRun       = 13,
    ResumeRun      = 14
  };
  // Object state data:
private:
  unsigned short m_node;	// Originating node number.
  BufferType     m_type;        // Type of the event.
public:
  AssembledEvent();
  AssembledEvent(unsigned short node,
		 BufferType     type);
  virtual ~AssembledEvent() {}
  // Selectors:

  unsigned short node() const;
  BufferType     type() const;

  void           setNode(unsigned short node);
};


#endif
