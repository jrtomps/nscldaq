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
#include "AssemblerWrongStateException.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////
/*!

    Constructs the exception.  Since ReasonText method is const,
    we take the time here to construct the m_reasonText variable...
    this also works because we have not really given any method for changing
    the exception mid-stream as our anticipated usecase is:
    throw AssemblerWrongStateException ...

    \param state   - The actual state of the system either
                     AssemblerWrongStateException::inactive or
		     AssemblerWrongStateException::active
     \param attempted - Some text that explains what state was attempted.
     \param wasDoing  - Some text that describes the larger context of the effort.
*/
AssemblerWrongStateException::AssemblerWrongStateException(AssemblerWrongStateException::State state,
							   string    attempted,
							   string    wasDoing) :
  CException(wasDoing.c_str()),
  m_actualState(state)
  
{
  m_reasonText  = "Assembler input stage was in the wrong state while ";
  m_reasonText += attempted;
  m_reasonText += " actual state was: ";
  m_reasonText += ((state == inactive)) ? "inactive" : "active";
  m_reasonText += " while: ";
  m_reasonText += wasDoing;
}
//////////////////////////////////////////////////////////////////////////
/*!
   Return the reason for he exception.  This is just
   m_reasonText.
*/
const char*
AssemblerWrongStateException::ReasonText() const
{
  return m_reasonText.c_str();
}
//////////////////////////////////////////////////////////////////////////
/*!
    Returns the reason code.  This is one of:
   - 0 if the actual state is inactive.
   - 1 if the actual state is active.
*/
Int_t
AssemblerWrongStateException::ReasonCode() const
{
  return (m_actualState == inactive) ? 0 : 1;
}
