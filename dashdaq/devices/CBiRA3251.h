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

#ifndef __CBIRA3251_H
#define __CBIRA3251_H



#ifndef __CCAMACMODULE_H
#include <CCAMACModule.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

/*!
   Class that supports a BiRA 3251 module.
   This class is relatively simple as the only thing that module
   can do is to write a mask to an output register (F16.A0).
*/
class CBiRA3251 : public CCAMACModule
{
  // Construtors and canonicals
public:
  CBiRA3251(CCAMACCrate& crate, size_t slot);
  CBiRA3251(const CBiRA3251&   rhs);
  CBiRA3251& operator=(const CBiRA3251& rhs);
  int operator==(const CBiRA3251& rhs) const;
  int operator!=(const CBiRA3251& rhs) const;
  
  // Operations on th emodule:

  void write(uint16_t mask);

};



#endif
