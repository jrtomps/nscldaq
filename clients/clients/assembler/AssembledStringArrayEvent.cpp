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
#include "AssembledStringArrayEvent.h"
#include <RangeError.h>

using namespace std;

///////////////////////// Constructors and explicit canonicals

/*!
  \param node - Originating node for this event.
  \param type - The type of documentation event.
  \throws CRangeError - If the type is not a valid
          documentation event type.
*/

AssembledStringArrayEvent::AssembledStringArrayEvent(
						unsigned short             node,
					        AssembledEvent::BufferType type) :
  AssembledEvent(node, type)
{
  if ((type != AssembledEvent::StateVariables)     &&
      (type != AssembledEvent::RunVariables)       &&
      (type != AssembledEvent::Packets)) {
    throw CRangeError(AssembledEvent::StateVariables,
		      AssembledEvent::RunVariables,
		      type,
 "Invalid documentation event type while constructing AssembledStringArray event");
  }

}
///////////////////////////  Adding strings and getting all of them: /////////

/*!
    Adds a single string:
    \param pString  - pointer to the string to add
*/
void
AssembledStringArrayEvent::addString(const char* pString)
{
  m_strings.push_back(string(pString));
}
/*!
  Adds a single string:
  \param string - string object to addk
*/
void
AssembledStringArrayEvent::addString(string item)
{
  m_strings.push_back(item);
}

/*!
  Add a bunch of strings:
  \param strings - vector of strings to add
*/
void
AssembledStringArrayEvent::addStrings(vector<string> strings)
{
  m_strings.insert(end(),
		   strings.begin(), strings.end());
}
/*!
  \return std::vector<std::string>
  \retval a copy of the array of strings in this event.
*/
vector<string>
AssembledStringArrayEvent::getStrings() const
{
  return m_strings;
}
///////////////////////// Delegations to m_strings //////////////////////

///

size_t
AssembledStringArrayEvent::size() const
{
  return m_strings.size();
}

///

vector<string>::iterator
AssembledStringArrayEvent::begin()
{
  return m_strings.begin();
}

///

vector<string>::iterator
AssembledStringArrayEvent::end()
{
  return m_strings.end();
}

///

string&
AssembledStringArrayEvent::operator[](unsigned int index)
{
  return m_strings[index];
}
