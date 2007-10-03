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
#include "InvalidNodeException.h"
#include <stdio.h>

using namespace std;

/////////////////////////////////////////////////////////////
/*!
 *  Create the exception
 * \param node  - the node that was being referenced.
 * \param reason- Context information about what was going on
 *                when the exception was thrown.
 */
InvalidNodeException::InvalidNodeException(uint16_t node,
					   string reason) : 
  CException(reason),
  m_node(node)
{

}
///////////////////////////////////////////////////////////////
/*!
 *   Actually don't need a destructor.
 */
InvalidNodeException::~InvalidNodeException()
{
}
////////////////////////////////////////////////////////////////
/*!
 *   Return the node that was the culprit.
 */
uint16_t
InvalidNodeException::getNode() const
{
	return m_node;
}

////////////////////////////////////////////////////////////
/*!
 *   Fabricate some reason for the failure.
 * our reason will include the reason text, the
 * node that was attempted as well as some explanatory text.
 */
const char*
InvalidNodeException::ReasonText() const
{
	m_reason = "Attempted to use the event queue from  unconfigured node: ";
	char nodeString[100];
	sprintf(nodeString, "%u", m_node);
	m_reason += nodeString;
	m_reason += "\n";
	m_reason += WasDoing();
	
	return m_reason.c_str();
}

/////////////////////////////////////////////////////////////
/*!
 *   The reason code will always be -1.
 */
int
InvalidNodeException::ReasonCode() const
{
	return -1;
}
