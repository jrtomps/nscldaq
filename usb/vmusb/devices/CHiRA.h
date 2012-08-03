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

#ifndef __CHIRA_H
#define __CHIRA_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/**
 * A HiRA device is just CHINP and a 
 * sis330x module that are read together.
 * Configuration options are:
 *
 * - -xlm - Name of the HINP XLM used in this supermodule.
 * - -fadc - Name of the FADC modle used in this supermodule.
 * -  id   - Integer tag used to mark the data readout by the two 
 *           drivers.  This can be any integer and defaults to 0x618A
 *           closest to hIRA I can get to with hex digits.
 *
 */

class CHiRA : public CReadoutHardware
{
private:
  CReadoutModule* m_pConfiguration;

public:
  // Class canonicals:

  CHiRA();
  virtual ~CHiRA();
private:
  CHiRA(const CHiRA&);
  CHiRA& operator=(const CHiRA&);
  int operator==(const CHiRA&) const;
  int operator!=(const CHiRA&) const;

  // overridable : operations on constructed objectgs:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

  // Utility modules:

private:
  static bool isModule(std::string name, std::string value, void* pArg);

};



#endif
