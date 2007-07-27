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
#include "PhysicsFragment.h"
#include <buftypes.h>


using namespace std;

/////////////////////////////////////////////////////////////////////////
/*!
  Constructs a physics fragment from a soup of words with an optional
  offset
  \param node    Node that originated the fragment.
  \param body    The body of the event.
  \param words   Number of words in the event body.
  \param offset  Word offset relative to body of the start of the event body.
                 defaults to 0.
*/

PhysicsFragment::PhysicsFragment(uint16_t node,
				 void*    body,
				 size_t   words,
				 off_t    offset=0) : 
  EventFragment(node, DATABF
		static_cast<uint16_t*>(body) + offset,
		words)
{
  
}
//////////////////////////////////////////////////////////////////////////
/*!
   Constructs a physics fragment from a vector of data items.
   \param node    Originating gnode.
   \param body    The vector that makes up the event body.
*/
PhysicsFragment:: PhysicsFragment(uint16_t node,
				  std::vector<uint16_t> body) : 
  EventFragment(node, DATABF, body)
{}
