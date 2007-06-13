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
#include <AssembledEvent.h>

//////////////////////////// Constructors /////////////////////////////
/*!
   The default constructor builds a physics event from node 0,
   that's an assembled physics event.
*/
AssembledEvent::AssembledEvent() :
  m_node(0),
  m_type(AssembledEvent::Physics)
{
}
/*!
   Since some events are not assembled but passed through unassembled,
   we'll also need a constructor that can explicitly set the
   values of the node and type member data:
   \param node   - Originating node id.
   \param type   - Type of the data buffer
*/
AssembledEvent::AssembledEvent(unsigned short             node,
			       AssembledEvent::BufferType type) :
  m_node(node),
  m_type(type)
{}

////////////////////////// Selectors /////////////////////////////

/*!

  \return unsigned short
  \retval the node number.
*/
unsigned short
AssembledEvent::node() const 
{
  return m_node;
}
/*!
   \return AssembledEvent::BufferType
   \retval The type of data in the event.
*/
AssembledEvent::BufferType
AssembledEvent::type() const
{
  return m_type;
}
