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

#include "CSimulatedVMEList.h"

/*!
   Construct a simulated list.  
   - The stored data are initialized to 0..which may or may not make sense
     but is as good as any other random value.
   \param pio  : CVMEPio&
       a Programmed I/O object for the VME crate we will
       run the list on.  This provides access to the hardware
       at list execution time.
*/
CSimulatedVMEList::CSimulatedVMEList(CVMEPio& pio) :
  m_countShift(0),
  m_countMask(0),
  m_pattern(0)
{
}
/*!
   Destruction of a simulated list must destroy the program elements.
*/
CSimulatedVMEList::~CSimulatedVMEList()
{
}

/*!
   set the count field extraction parameters.  Normally this will
   be called from within a list element.  The count field extraction
   parameters are a shift count and a mask.  The are applied to a 
   count bearing item x as follows:
      transferCount = (x >> m_countShift) & m_countMask
   \param shiftCount : uint8_t
      New right shift count for the above equation.
   \param  mask : uint32_t
      New mask for the above equation.
*/
void 
CSimulatedVMEList::setCountExtractionParameters(uint8_t  shiftCount,
						 uint32_t mask)
{
  m_countShift = shiftCount;
  m_countMask  = mask;
}
/*!
  Retrieve the count extraction shift count.  See setCountExtractionParameters
  above for more on this value and how it is used.  
  \return uint8_t
  \retval The current value of m_countShift.

 */
uint8_t
CSimulatedVMEList::getCountRightShift() const
{
  return m_countShift;
}
/*!
   Retrieve the count extraction mask.  See setCountExtractionParameters above
   for more information on this value and how it is used.
   \return uint32_t
   \retval m_countMask
*/
uint32_t
CSimulatedVMEList::getCountMask() const
{
  return m_countMask;
}
/*!
   Set the hit pattern value.  This is normally done by a simulator element
   (program step).  The hit pattern value is then used by later simulator
   elements in the list to conditionalize their operations.
   See documentation of the conditionalized operations e.g.
   CConditionalRead for more information about how this conditionalization
   works.
   \param pattern : uint16_t
      The new conditional pattern.
*/
void
CSimulatedVMEList::setConditionMask(uint16_t pattern)
{
  m_pattern = pattern;
}
/*!
   Return the current hit pattern value.  This is normally called by
   a simulator element (program step) like CConditionalRead to determine
   if the condition for performing the conditional operation is made.
   \return uint16_t
   \retval m_pattern.
*/
uint16_t
CSimulatedVMEList::getConditionMask() const
{
  return m_pattern;
}
