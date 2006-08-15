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


///////////////////////////////////////////////////////////
//  CCAENV977Status.cpp
//  Implementation of the Class CCAENV977Status
//  Created on:      07-Jun-2005 04:42:55 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV977Status.h"
#include "CCAENV977.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


/*

   These masks define the output usage:
*/
static const UShort_t GOINGREADY(1);
static const UShort_t GOINGBUSY(2);
static const UShort_t CLEARS(0xfffc);	// All the other bits.

/*!
   Construct given addressing:
   @param lBase:
       Base address of the module.
   @param nCrate:
       crate number (Defaults to 0)
*/
CCAENV977Status::CCAENV977Status(ULong_t lBase, UShort_t nCrate) :
  m_Module(*(new CCAENV977(lBase, nCrate)))
{
  
}
/*!
   Construct given an existing  V977 module.

*/
CCAENV977Status::CCAENV977Status(CCAENV977& module) :
  m_Module(*(new CCAENV977(module)))
{

}
/*!
   Copy construction.. copy construct in the m_Module of the
   rhs.
*/
CCAENV977Status::CCAENV977Status(const CCAENV977Status& rhs) :
  m_Module(*(new CCAENV977(rhs.m_Module)))
{

}
/*!
  Destroying an object requires destruction of it's V977 object.:

 */
CCAENV977Status::~CCAENV977Status()
{
  delete &m_Module;
}

/*!
  Assignment:
 */
CCAENV977Status&
CCAENV977Status::operator=(const CCAENV977Status& rhs)
{
  if(this != &rhs) {
    m_Module = rhs.m_Module;	// Modules assign.
  }
  return *this;
}
/*!
  equality comparison:
 */
int
CCAENV977Status::operator==(const CCAENV977Status& rhs) const
{
  return m_Module == rhs.m_Module;
}
/*!
   Inequality is the logical negation of equality
*/
int
CCAENV977Status::operator!=(const CCAENV977Status& rhs) const
{
  return !(*this == rhs);
}


/**
 *   Pulses the busy output.
 */
void 
CCAENV977Status::GoBusy()
{
  PulseOutputs(GOINGBUSY);
}


/**
 * Pusle the clear output.
 */
void 
CCAENV977Status::GoClear()
{
  PulseOutputs(GOINGREADY);
}
`

/**
 * Clear both the busy and ready outputs.
 */
void 
CCAENV977Status::ModuleClear()
{
  PulseOutputs(CLEARS);
}
//   Utility to pulse an output mask.
//   The mask is written to the outputs then 0 is written.
//   the assumption is that output are only pulsed.
//
void
CCAENV977Status::PulseOutputs(UShort_t mask)
{
  m_Module.outputSet(mask);
  m_Module.outputSet(0);
}


