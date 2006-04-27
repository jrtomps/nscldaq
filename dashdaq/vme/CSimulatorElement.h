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

#ifndef __CSIMULATORELEMENT_H
#define __CSIMULATORELEMENT_H


// Forward class definitions:

class CVMEPio;
class CSimulatedVMEList;

/*!
   An abstract base class for the simulator instruction set.
   Each simulator instruction is an object from a class that is
   derived from this.  Simulation is really just the process of
   an ordered visitation of the objects in the 'program'.
*/
class CSimulatorElement 
{
public:
  virtual ~CSimulatorElement() {}; // So we don't have to have an implementation file.

  // Evaulate the insruction:

  virtual void* operator()(CVMEPio& pio, CSimulatedVMEList& program, void* outBuffer) = 0;
};


#endif
