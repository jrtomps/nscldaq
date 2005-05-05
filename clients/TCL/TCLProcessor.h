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


//  CTCLProcessor.h:
//
//    This file defines the CTCLProcessor class.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
//  Copyright 1999 NSCL, All Rights Reserved.
//
/////////////////////////////////////////////////////////////

#ifndef __TCLPROCESSOR_H  //Required for current class
#define __TCLPROCESSOR_H
                               //Required for base classes
#ifndef __TCLINTERPRETEROBJECT_H
#include "TCLInterpreterObject.h"
#endif                               

#ifndef __TCLINTERPRETER_H
#include "TCLInterpreter.h"
#endif

#ifndef __TCLRESULT_H
#include "TCLResult.h"
#endif
  
#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __STL_VECTOR
#include <vector>
#define __STL_VECTOR
#endif
                             
typedef STD(vector)<CTCLInterpreter*> TCLInterpreterList;
typedef TCLInterpreterList::iterator TCLInterpreterIterator;

class CTCLProcessor  : public CTCLInterpreterObject        
{

  STD(string) m_sCommandName;                     // Name of the command.
  TCLInterpreterList m_vRegisteredOn;  // Set of interpreters 
				                  // we've been registered to.
public:
  
  //Constructors with arguments

  CTCLProcessor(const STD(string)& sCommand, CTCLInterpreter* pInterp);
  CTCLProcessor(const char* pCommand, CTCLInterpreter* pInterp);

  ~ CTCLProcessor ( ) {
    UnregisterAll();
  }
  	
			//Copy constructor [ illegal ]
private:
  CTCLProcessor (const CTCLProcessor& aCTCLProcessor );
public:
			//Operator= Assignment Operator [ illegal ]
private:
  CTCLProcessor& operator= (const CTCLProcessor& aCTCLProcessor);
public:

			//Operator== Equality Operator
                        // Legal, but pretty useless

  int operator== (const CTCLProcessor& aCTCLProcessor) const
  { 
    return (
	    (CTCLInterpreterObject::operator== (aCTCLProcessor)) &&
	    (m_sCommandName == aCTCLProcessor.m_sCommandName) &&
	    (m_vRegisteredOn == aCTCLProcessor.m_vRegisteredOn) 
	    );
  }                             
  // Selectors:

  STD(string) getCommandName() const
  {
    return m_sCommandName;
  }
  TCLInterpreterIterator begin() { 
    return m_vRegisteredOn.begin();
  }
  TCLInterpreterIterator end() {
    return m_vRegisteredOn.end();
  }
  // Mutators:
protected:

  void setCommandName (const STD(string)& am_sCommandName)
  { m_sCommandName = am_sCommandName;
  }
  void setRegisteredOn (const STD(vector)<CTCLInterpreter*>& am_vRegisteredOn)
  { m_vRegisteredOn = am_vRegisteredOn;
  }
  // Operations and overrides:

public:
  
  virtual   int operator() (CTCLInterpreter& rInterpreter, 
			    CTCLResult& rResult, 
			    int nArguments, 
			    char* pArguments[])   = 0;
  
  static  STD(string) ConcatenateParameters (int nArguments, 
					      char* pArguments[])  ;
  static  int EvalRelay (ClientData pData,
			 Tcl_Interp* pInterp, 
			 int Argc, 
#if (TCL_MAJOR_VERSION > 8) || ((TCL_MAJOR_VERSION ==8) && (TCL_MINOR_VERSION > 3))
			 const char *Argv[])  ;
#else
                         char *Argv[]);
#endif
  virtual   void OnDelete ( )  ;
  static void DeleteRelay (ClientData pObject)   ;

  int ParseInt (const char* pString, int* pInteger)  ;
  int ParseInt (const STD(string)& rString, int* pInteger) {
    return ParseInt(rString.c_str(), pInteger);
  }

  int ParseDouble (const char* pString, double* pDouble)  ;
  int ParseDouble (const STD(string)& rString, double* pDouble) {
    return ParseDouble(rString.c_str(), pDouble);
  }

  int ParseBoolean (const char* pString, Bool_t* pBoolean)  ;
  int ParseBoolean (const STD(string)& rString, Bool_t* pBoolean) {
    return ParseBoolean(rString.c_str(), pBoolean);
  }

  void Register ()  ;
  int Unregister ()  ;
  void UnregisterAll ()  ;

  static int MatchKeyword(STD(vector)<STD(string)>& MatchTable, 
			  const STD(string)& rValue, 
			  int NoMatch = -1);
};

#endif
