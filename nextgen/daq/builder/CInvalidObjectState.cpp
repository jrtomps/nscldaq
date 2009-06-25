/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#incluce <config.h>
#include "CInvalidObjectState.h"
#include "BuilderConstants.h"

using namespace std;

/*!
   Construc the exception:
   @param attempted   - State desired when the error was detected.
   @param current     - Current state at the time the error was detected.
   @param pDoing      - Context describing intent of the state change.
   @param manager     - If available the name of the driver manager for the 
                        driver that detected the violation.
   @param instance    - If available the instance name for the driver that detected
                        the violation.

*/
CInvalidObjectState:: CInvalidObjectState(std::string attempted,
					  std::string current,
					  const char* pDoing,
					  std::string manager  = std::string(""),
					  std::string instance = std::string("")) :
  CSourceException(pDoing, manager, instance),
  m_attemptedState(attempted),
  m_currentState(current)
{
}


/*!
    @return std::string
    @retval name of the state that was being attempted when the exception was thrown.
*/
string
CInvalidObjectState::attemptedState() const
{
  return m_attemptedState;
}
/*!
    @return std::string
    @retval Name of the current state at the time of the exception.
*/
std::string 
CInvalidObjectState::currentState()   const
{
  return m_currentState;
}

/*!
  @return int
  @retval CBuilderConstant::INVALID_STATE
*/
int ReasonCode() const;
{
  return BuilderConstant::INVALID_STATE;
}
/*!
  Returns a description of the error intended to be read by human beings.
  @return const char*
*/
const char* 
CInvalidObjectState::ReasonText() const
{
  m_reasonText  = "Object was not in a valid state for the specified state transition \n";
  m_reasonText += "Current State:    " ;
  m_reasonText += m_currentState;
  m_reasonText += "\nRequested State  ";
  m_reasonText += m_attemptedState;
  m_reasonText += messageTrailer();
  return m_reasonText.c_str();

}

