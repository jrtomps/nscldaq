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
#include "CScaler.h"

/*!
   It is expected that derived classes can override this to 
   provide any initialization required for the module that is being
   read by this object.

*/
void
CScaler::initialize()
{
}

/*!
  It is expected that derived classes may override this to provide
  the operations required to clear the scaler counters.
*/
void
CScaler::clear()
{
}

/*!
  It is expected that derived classes will override this to provide
  the operations needed to disable this module.
*/
void
CScaler::disable()
{
}

//
// The C++ standard for some perverse reason allows the implementation
// of asbstract members, they can be called only via explicit scoping
// and still must be overridden in derived classes.  I suppose this
// allows common code in the tree to be implemented here but forces that
// to be bracketed by code in concrete classes.
// Here we use this to provide a doxygen comment for the intent of the
// read member.

/*!
  This method is supposed to read and clear the module (atomically
  if possible, by calling clear as soon after the readout as possible
  otherwise).  

  \return std::vector<uint32_t>
  \retval A vector containing the scaler values.

*/
std::vector<uint32_t>
CScaler::read()
{
  std::vector<uint32_t> result;

  return result;
}
  
