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

static const char* Copyright=  Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
//   NSCL All rights reserved.
//
// See CTCPBadSocketState.h for a description of this class.

#include <config.h>
#include <CTCPBadSocketState.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// Constructors and related methods.

/*!
    "Normal Constructor"  This constructor is normally used prior to
    throwing a CTCPBadSocketState object as an exception. 

    \param badState - the state of the CSocket object which threw the
                      exception which was objectionable.
    \param okStates - A vector of states which would have been ok
                      for the socket to have been in at the time it threw.
    \param pDoing   - A textual description of what the CSocket object
                      was asked to do when it detected the invalid state.
 */

CTCPBadSocketState::CTCPBadSocketState(CSocket::SocketState         badState,
				       vector<CSocket::SocketState> okStates,
				       const char*            pDoing) :
  CException(pDoing),
  m_BadState(badState),
  m_ValidStates(okStates)
{
  
}
/*!
   "Copy constructor" This constructor creates a new object from a
   'reference' object.  This is used by the compiler to create temporaries,
   it is also used by throw to copy the exception to a "spot" where it cannot
   go out of scope as it travels up the call stack in search of a handler.

   \param rhs  - the reference object copied.
   */
CTCPBadSocketState::CTCPBadSocketState(const CTCPBadSocketState& rhs) :
  CException(rhs),
  m_BadState(rhs.m_BadState),
  m_ValidStates(rhs.m_ValidStates)
{
  
}
/*!
  Assignement... little different from copy construction however:
  - Any existing members require cleanup if they are dynamic, since
    we are already fully constructed.
  - We avoid self assignment.
  - We return a reference to ourselves after the copy in.

  \param rhs The right hand side object which is being assigned to us.

  */
CTCPBadSocketState&
CTCPBadSocketState::operator=(const CTCPBadSocketState& rhs)
{
  if(this != &rhs) {
    CException::operator=(rhs);
    m_BadState    = rhs.m_BadState;
    m_ValidStates = rhs.m_ValidStates;

  }
  return *this;
}
/*!
  Equality comparison.  Returns int indicating if for all practical purposes
  *this is the same as the rhs.  m_Message is not compared as it is 
  inconsequential.
  \param rhs The object *this is being compared to.
  */
int
CTCPBadSocketState::operator==(const CTCPBadSocketState& rhs)
{
  return (CException::operator==(rhs)      &&
	  (m_BadState    == rhs.m_BadState)   &&
	  (m_ValidStates == rhs.m_ValidStates));
}

// Operations on the class.

/*!
  Build up and return a text string describing why the 
  exception was thrown.  This is done from the m_BadState, m_ValidStates,
  and base class WasDoing() strings.  The final string is of the form:
  
  "CSocket member called while in state m_BadState, Valid states are
   m_ValidStates, CSocket was attempting to: WasDoing()"

   
 */
const char* 
CTCPBadSocketState::ReasonText() const
{
  m_Message = "CSocket member called while in state: ";
  m_Message += CSocket::StateName(m_BadState);
  m_Message += "\n";
  m_Message += " allowed states were any of.:";
  for(int i = 0; i < m_ValidStates.size(); i++) {
    m_Message += " ";
    m_Message += CSocket::StateName(m_ValidStates[i]);
  }
  m_Message += "\n CSocket was attempting to: ";
  m_Message += WasDoing();
}
