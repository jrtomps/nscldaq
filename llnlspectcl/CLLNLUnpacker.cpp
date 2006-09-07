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
#include "CLLNLUnpacker.h"
#include "CParamMapCommand.h"
#include <Event.h>
#include <Analyzer.h>
#include <TCLAnalyzer.h>
#include <BufferDecoder.h>


/*!
  Construct the unpacker. The initial state is...well Initial...
*/
CLLNLUnpacker::CLLNLUnpacker() :
  m_state(Initial),
  m_fragmentResidual(0),
  m_isLastFragment(false),
  m_entityDone(false)
{
}
/*!  
  Dont' need anything for the destructor:
*/
CLLNLUnpacker::~CLLNLUnpacker()
{
}
/*!
   Copy constructor... just copy the member data.
*/
CLLNLUnpacker::CLLNLUnpacker(const CLLNLUnpacker rhs) :
  CEventProcessor(rhs),
  m_state(rhs.m_state),
  m_fragmentResidual(rhs.m_fragmentResidual),
  m_isLastFragment(rhs.m_isLastFragment),
  m_entityDone(rhs.m_entityDone)
{
}

/*! 
    Assignment.. not much different than copy construction.
*/
CLLNLUnpacker&
CLLNLUnpacker::operator=(const CLLNLUnpacker& rhs)
{
  if (this != &rhs) {
    CEventProcessor::operator=(rhs);
    m_state            = rhs.m_state;
    m_fragmentResidual = rhs.m_fragmentResidual;
    m_isLastFragment   = rhs.m_isLastFragment;
    m_entityDone       = rhs.m_entityDone;
  }
  return *this;
}
/*!
   Equality if all members are identical:
*/
int
CLLNLUnpacker::operator==(const CLLNLUnpacker& rhs) const
{
  return  (CEventProcessor::operator==(rhs)                       &&
	   (m_state            == rhs.m_state)                    &&
	   (m_fragmentResidual == rhs.m_fragmentResidual)         &&
	   (m_isLastFragment   == rhs.m_isLastFragment)           &&
	   (m_entityDone       == rhs.m_entityDone));


}
int
CLLNLUnpacker::operator!=(const CLLNLUnpacker& rhs) const
{
  return !(*this == rhs);
}

/*!
   The unpacker.  Most of the detail about this is described in the class
   header.
*/
Bool_t
CLLNLUnpacker::operator()(const Address_t pEvent,
			    CEvent&         rEvent,
			    CAnalyzer&      rAnalyzer,
			    CBufferDecoder& rDecoder)
{
  Uint_t        nWords = 0;		// This will be tallied as we go.
  CTclAnalyzer& analyzer(dynamic_cast<CTclAnalyzer&>(rAnalyzer));



  // End of event processing:

      // Control when the event count is decremented.

  if(! m_entityDone) {
    analyzer.entityNotDone();
  }
     // Report how many bytes were processesd

  analyzer.SetEventSize(nWords * sizeof(UShort_t));

     // Don't abort processing.

  return kfTRUE;
}
