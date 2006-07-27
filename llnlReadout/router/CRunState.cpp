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
#include "CRunState.h"

using namespace std;

// Class level member data:

CRunState* CRunState::m_pTheInstance(0);

/*!
   Construction  For now the default initial state is:
   - Title == "Change this title"
   - Run NUmber 0
   - State idle.

   \note as befits a singleton pattern, this constructor is private!
*/
CRunState::CRunState() : 
  m_title("Change this title"),
  m_runNumber(0),
  m_state(CRunState::Idle)
{
  m_pTheInstance = this;
}
/*!
   Destructor does nothing and, in general should not get called.
*/
CRunState::~CRunState()
{
}

/*!
   Select the currenst state:
   \return RunState
   \retval current run state value.
*/
CRunState::RunState
CRunState::getState() const
{
  Enter();
  RunState state = m_state;
  Leave();
  return state;
}
/*!
  Set a new value for the run statel
  \param newState : CRunState::RunState
     New runstate can be one of CRunState::Idle or CRunState::Active
*/
void
CRunState::setState(CRunState::RunState)
{
  Enter();
  m_state = RunState;
  Leave();
}
/*!
   Get the current title.
   \return std::string
   \retval  String containing the run title.
*/
string
CRunState::getTitle() const
{
  Enter();
  string title = m_title;
  Leave();
  return title;
}
/*!
   Set a new value for the title:
   \param newTitle :: std::string
      New title string.
*/
void
CRunState::setTitle(string newTitle)
{
  Enter();
  m_title = newTitle;
  Leave();
}

/*!
    Get the current run number.
    \return uint16_t
    \retval  The current run number.
*/
uint16_t
getRunNumber() const
{
  Enter();
  uint16_t run = m_runNumber;
  Leave();
  return run;
}
/*!
  Set a new run number value:
  \param newRunNumber : uint16_t
      New value for the run number.
*/
void
setRunNumber(uint16_t newRunNumber)
{
  Enter();
  m_runNumber = newRunNumber;
  Leave();

}
