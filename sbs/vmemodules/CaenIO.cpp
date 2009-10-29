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
#include <CaenIO.h>
#include <CVME.h>

#include <stdlib.h>
using namespace std;


/*!
  \fn CCaenIO(UInt_t base)

  Purpose:
     Basic constructor for type CCaenIO. Definees a
     VmeModule with CVME mapped at location 'base'
     and with length 'length'. Initializes the output
     mask to zero.

  \param UInt_t base - offset into the VME module at which to begin mapping
  \param int   nCrate - VME Crate number.
*/
CCaenIO::CCaenIO(UInt_t base, int nCrate) :
  CVmeModule(CVmeModule::a24d16, base, LENGTH, nCrate),
  m_nOutputMask(0)
{

}

/*!
  \fn CCaenIO(const CCaenIO& aCCaenIO)

  Purpose:
     Copy constructor method for CCaenIO. Constructs
     a CCaenIO object from a previously constructed
     CCaenIO object.
  
  \param const CCaenIO& aCCaenIO - the object to copy for this
*/
CCaenIO::CCaenIO(const CCaenIO& aCCaenIO) :
  CVmeModule(aCCaenIO)
{
  m_nOutputMask = aCCaenIO.m_nOutputMask;
}

/*!
  \fn CCaenIO& CCaneIO::operator=(const CCaenIO& aCCaenIO)

  Purpose:
     Assigns a CCaenIO object to this object.

  \param const CCaenIO& aCCaneIO - the object to assign to this
*/
CCaenIO&
CCaenIO::operator=(const CCaenIO& aCCaenIO)
{
  if(this == &aCCaenIO) return *this;
  CVmeModule::operator= (aCCaenIO);
  m_nOutputMask = aCCaenIO.m_nOutputMask;

  return *this;
}

/*!
  \fn Int_t CCaneIO::operator== (const CCaenIO& aCCaenIO)

  Purpose:
     Compares this to aCCaenIO and checks for equality.

  \param const CCaenIO& aCCaenIO - the object to check for equality with

  \return 1 if they are equal
          0 if they are not equal
*/
Int_t
CCaenIO::operator== (const CCaenIO& aCCaenIO)
{
  return ((CVmeModule::operator== (aCCaenIO)) &&
	  (m_nOutputMask == aCCaenIO.m_nOutputMask));
}

/*!
  \fn UShort_t CCaenIO::ReadInput(UInt_t input)

  Purpose:
     Reads the value on the inputs of one of the
     NIM level inputs. Which inputs it reads is
     specified by its parameter. Inputs must be
     between 1 and 4.

  \param UInt_t input - the input (0-3) from which to read a value.

  \return The value of the input.
*/
UShort_t
CCaenIO::ReadInput(UInt_t input)
{
  if((input < 0) || (input > 3)) {
    throw string("\nInvalid input on read operation. Must be between 0 and 3.\n");
  }
  return ((peekw(5) & (1 << input)) ? 1 : 0);
}
/*!
       Read all inputs and return them as a mask.
 */
UShort_t
CCaenIO::ReadInputs() 
{
  return peekw(5);
}

/*!
  \fn void CCaenIO::PulseOutput(UInt_t output)
  
  Purpose:
     Pulse one of the pulse outputs.

  \param UInt_t output - the output to pulse
*/
void
CCaenIO::PulseOutput(UInt_t output)
{
  if((output < 0) || (output > 3)) {
    cerr << "\nInvalid output on assert operation. Must be between 0 and 3.\n";
    exit(0);
  }

  pokew((1 << output), 4);
}

/*!
  \fn void CCaenIO::SetLevel(UInt_t output)

  Purpose:
     Set one of the level outputs. If the outputs is
     already set, then this function is a no-op.
  

  \param UInt_t output - the output to assert.
  Must be in the range 0 - 3.
*/
void
CCaenIO::SetLevel(UInt_t output)
{
  if((output < 0) || (output > 3)) {
    cerr << "\nInvalid output on assert operation. Must be between 0 and 3.\n";
    exit(0);
  }

  m_nOutputMask = m_nOutputMask | (1 << output);
  pokew(m_nOutputMask, 3);
}

/*!
  \fn void CCaenIO::ClearLevel(UInt_t output)

  Purpose:
     Clear one of the level outputs. If the output is already
     cleared, then this function is a no-op.

  \param UInt_t output - the output to deassert
     Must be in the range of 0-3
*/
void
CCaenIO::ClearLevel(UInt_t output)
{
  UInt_t clear_mask = ~(1 << output);
  m_nOutputMask = m_nOutputMask & clear_mask;
  pokew(m_nOutputMask, 3);
}

/*!
  \fn void CCaenIO::SetECL(UInt_t output)

  Purpose:
     Set the ECL output

  \param UShort_t value - the value to place in the ECL out register.
        The outputs of the ECL out connector will reflect the bit 
	encoding of this parameter.

*/
void
CCaenIO::SetECL(UShort_t value)
{
  pokew(value, 2);
}

/*!
  \fn void CCaenIO::ClearECL()

  Purpose:
     Set all ECL outputs to logical 0
*/
void
CCaenIO::ClearECL()
{
  pokew(0, 2);
}

/*!
  \fn void CCaenIO::ClearAll()

  Purpose:
     Set all NIM levels and ECL outputs to logical 0
*/
void
CCaenIO::ClearAll()
{
  pokew(0, 2);
  pokew(0, 3);
}

/*!
   Get a pointer to the input register.  This is
designed for high performance software that does
not want to  crawl through the layers of abstraction:
Note that his is only available for VME
controllers that support memory mapping.

*/
short*
CCaenIO::getInputPointer()
{
  CVME<UShort_t>& map(getCVME());
  return ((short*)(map.getStart())+5);

}
/*!
  Get a pointer to the pulsed output register.
  This is only available for controllers that support
  memory mapping.
*/
short* 
CCaenIO::getPulsedOutputPointer()
{
  CVME<UShort_t>& map(getCVME());
  return ((short*)(map.getStart()) + 4);
}
/*!
   Get a pointer to the level output register:
   This is only available for controllers that support
   memory mapping.
*/
short* 
CCaenIO::getLatchedOutputPointer() 
{
  CVME<UShort_t>& map(getCVME());
  return ((short*)(map.getStart()) + 3);
}
/*!
  Get a pointer to the ECL output register:
  This is only avaialable for controllers that support
  memory mapping.

*/
short* 
CCaenIO::getECLOutputPointer() 
{
  CVME<UShort_t>& map(getCVME());
  return ((short*)(map.getStart()) + 2);
}


