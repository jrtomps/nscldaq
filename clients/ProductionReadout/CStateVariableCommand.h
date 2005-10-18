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


//////////////////////////CStateVariableCommand.h file//////////////////////////////////

#ifndef __CSTATEVARIABLECOMMAND_H  
#define __CSTATEVARIABLECOMMAND_H

#ifndef __CDAQTCLPROCESOR_H
#include <CDAQTCLProcessor.h>
#endif
                               
#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif
                                                        
/*!
   Encapsulates the Tcl Readout Core command to manage the 
   run state variables.  Run state variables are similar to run variables,
   however:
   - They are locked (readonly) when the run is not in the halted state.
   - They are only written to buffers at run state change time (e.g. start
      stop pause, resume).
 */		



class CStateVariable;


typedef STD(pair)<STD(string), CStateVariable*>  StateVariableEntry;
typedef STD(list)<StateVariableEntry> StateVariableMap;
typedef StateVariableMap::iterator  StateVariableIterator; 

class CStateVariableCommand : public CDAQTCLProcessor     
{ 
private:

  StateVariableMap m_StateVariables;
  
public:
  // Constructors, destructors and other cannonical operations: 
  
  CStateVariableCommand ();                      //!< Default constructor.
  ~ CStateVariableCommand ( );                     //!< Destructor.
  
private:
  CStateVariableCommand(const CStateVariableCommand& rhs); //!< Copy constructor.
  
  CStateVariableCommand& operator= (const CStateVariableCommand& rhs); //!< Assignment
  int         operator==(const CStateVariableCommand& rhs) const; //!< Comparison for equality.
  int         operator!=(const CStateVariableCommand& rhs) const;
  
public:
  
  // Class operations:
  
public:
  virtual   int operator() (CTCLInterpreter& rinterp, CTCLResult& rResult, 
			    int argc, char** argv)  ;
  int Create (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	      int  argc, char** argv)  ;
  int Delete (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	      int  argc, char** argv)  ;
  int List (CTCLInterpreter& rInterp, CTCLResult& rRestul, 
	    int  argc, char** argv)  ;

  // Programmatic interfaces to the database:

  CStateVariable* Create(const STD(string)& rName);
  void Delete(const STD(string)& rName);
  void Delete(StateVariableIterator& rIter);

  // Iteration support.

  StateVariableIterator begin ()  ;
  StateVariableIterator end ()  ;
  size_t size ()  ;
  StateVariableIterator find(const STD(string)& rName);

protected:
  void Usage(CTCLResult& rResult);
};

#endif
