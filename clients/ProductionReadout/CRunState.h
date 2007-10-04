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

//////////////////////////CRunState.h file//////////////////////////////////

#ifndef __CRUNSTATE_H  
#define __CRUNSTATE_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_SET
#include <set>
#ifndef __STL_SET
#define __STL_SET
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

                               
/*!
   Encapsulates a run state variable 
   and all of the transitions which are allowed.
   See tee page labelled:
   Run State Diagram 
   for allowed states and transitions as well
   as documented actions on state transitions.
 */		
class CRunState      
{ 
public:
  typedef enum _RunState_ {
    Inactive,
    Active,
    Paused
  } RunState;
private:
  RunState m_eState; //!< Value of the state
  STD(map)<int,STD(string)> 
         m_StatesByValue; //!< State names looked up by state value.
  STD(map)<STD(string), int> 
         m_StatesByName; //!< State Values looked up by run state.

  STD(map)<int, STD(set)<int> > 
         m_Transitions; //!< For each state, allowed transitions.
  
public:
	// Constructors, destructors and other cannonical operations: 

    CRunState ();                      //!< Default constructor.
    CRunState(const CRunState& rhs); //!< Copy constructor.
    ~CRunState ( ) { } //!< Destructor.

    CRunState& operator= (const CRunState& rhs); //!< Assignment
    int         operator==(const CRunState& rhs) const; //!< Comparison for equality.
    int         operator!=(const CRunState& rhs) const {
       return !(operator==(rhs));
    }

	// Selectors for class attributes:
public:

    RunState getState() const {
       return m_eState;
    }

    STD(map)< int,STD(string)> getStatesByValue() const {
       return m_StatesByValue;
    }

    STD(map)<STD(string), int> getStatesByName() const {
       return m_StatesByName;
    }

    STD(map)<int, STD(set)<int> > getTransitions() const {
       return m_Transitions;
    }

	// Mutators:
protected:  

  // Class operations:
public:  
  void Begin ()  ;
  void End ()  ;
  void Pause ()  ;
  void Resume ()  ;
  bool Allowed(RunState newstate);
  
  STD(string) getStateName ()  ;
  STD(string) getStateName(RunState state);
  void   SetStateVariable(RunState state);	
};

#endif
