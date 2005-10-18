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


//////////////////////////CRunVariableCommand.h file//////////////////////////////////

#ifndef __CRUNVARIABLECOMMAND_H  
#define __CRUNVARIABLECOMMAND_H
                               
#ifndef __CRUNVARIABLE_H
#include "CRunVariable.h"
#endif

#ifndef __CDAQTCLPROCESSOR_H
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
   Maintains the set of run variables and provides Tcl
   command access to them.  Run variables are simply
   Tcl scalar variablew which are periodically written during
   the run.
 */	

typedef STD(pair)<STD(string), CRunVariable*> RunVariableListEntry;
typedef STD(list)<RunVariableListEntry> RunVariableList;
typedef RunVariableList::iterator RunVariableIterator;
	
class CRunVariableCommand : public  CDAQTCLProcessor
{ 
private:

  RunVariableList m_RunVariables;


public:
	// Constructors, destructors and other cannonical operations: 

    CRunVariableCommand ();	        //!< Default constructor.
    virtual  ~ CRunVariableCommand ( ); //!< Destructor.

private:
    CRunVariableCommand(const CRunVariableCommand& rhs); //!< Copy constructor.
    CRunVariableCommand& operator= (const CRunVariableCommand& rhs); //!< Assignment
    int         operator==(const CRunVariableCommand& rhs) const; //!< Comparison for equality.
    int         operator!=(const CRunVariableCommand& rhs) const;



	// Class operations:

public:
  virtual   int operator() (CTCLInterpreter& rInterp, CTCLResult& rResult, 
			    int  argc, char** argv)  ;
  int Create (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	      int argc, char** argv)  ;
  int Delete (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	      int argc, char** argv)  ;
  int List (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	    int argc, char** argv)  ;
  
  // Programmatic interfaces to the database:
  
  CRunVariable* Create(const STD(string)& rName, 
	      const STD(string)& rInitialValue = STD(string)("-uninitialized"));
  void Delete(const STD(string)& rName);
  void Delete(RunVariableIterator& rIter);


  // iteration support:
  
  RunVariableIterator begin ()  ;
  RunVariableIterator end ()  ;
  size_t size ()  ;
  RunVariableIterator find(const STD(string)& rName);

  
protected:
  void Usage(CTCLResult& rResult);

};

#endif
