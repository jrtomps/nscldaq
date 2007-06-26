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
#include "BadEventTypeException.h"

#include <stdio.h>

using namespace std;

// Event type name strings:

static const char* eventTypes[] = {
  "Physics Event (1)",
  "Scaler Event (2)",
  "Snapshot Scaler Event (3)",
  "State Variable Event (4)",
  "Run Variable Event (5)",
  "Packet documentation event (6)",
  "Unused Event Type (7)",
  "Unused Event Type (8)",
  "Unused Event Type (9)",
  "Unused Event Type (10)",
  "Begin run event (11)",
  "End run Event (12)",
  "Pause run Event (13)",
  "Resume run Event (14)"
};

static const int eventTypeCount = sizeof(eventTypes)/sizeof(const char*);

/*!
   Construct the exception:

   \param provided   - The provided (bad) event type.
   \param required   - Stringified name of the expected event type or class of
                       event types.
   \param doing      - Execution context information.
*/
BadEventTypeException::BadEventTypeException(unsigned int    provided,
					     string          required,
					     string          doing) :
  CException(doing),
  m_requiredType(required),
  m_providedType(provided)
{
}
/*!
   The reason code is always -1:
*/
Int_t
BadEventTypeException::ReasonCode() const
{
  return -1;
}

/*!
   The reason text is of the form:
   Expected 'required' event type but got 'event type name' (provided) while ...
   We fill in the m_ReasonText mutable so that it lives past our return.
*/
const char*
BadEventTypeException::ReasonText() const
{
  m_ReasonText  = "Expected event type: ";
  m_ReasonText += m_requiredType;
  m_ReasonText += " event type but got: ";
  m_ReasonText += eventTypeString();
  m_ReasonText +=  " while : ";
  m_ReasonText += WasDoing();

  return m_ReasonText.c_str();
}

/*
   Utility to return a string that describes an event number.. of the form
   "Description (integer id).
*/
string
BadEventTypeException::eventTypeString() const
{
  if (m_providedType < eventTypeCount) {
    return string(eventTypes[m_providedType]); // The easy case.
  }
  // Now we construct a string that looks like:
  // Unknown type (%d).

  char resultText[1000];
  sprintf(resultText, "Unknown type (%d)", m_providedType);
  return string(resultText);

}
