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

#ifndef __CSCALERCOMMAND_H
#define __CSCALERCOMMAND_H

#ifndef __CMODULECOMMAND_H
#include "CModuleCommand.h"
#endif


/*!
   This class creates and configures SIS3820 scalers for the LLNL neutrons daq
   system.  The command supports the following syntaxes:
\verbatim
   sis3820 create module-name base-address
\endverbatim
    Unlike the 'normal' scripted readout. this software will read all defined
    scalers. Currently scaler stacks are supported as first class stacks.
    This module does not require configuration.

*/
class CScalerCommand : public CDAQModuleCommand
{
public:
  CScalerCommand(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CScalerCommand();
private:
  CScalerCommand(const CScalerCommand& rhs);
  CScalerCommand& operator=(const CScalerCommand& rhs);
  int operator==(const CScalerCommand& rhs) const;
  int operator!=(const CScalerCommand& rhs) const;
public:


  // The derived concrete classes must supply implementations for the following
  // methods:

protected:
  virtual CConfigurableObject* createObject();
  virtual std::string          getType();
	     
};
   

#endif
