
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
#ifndef __CMADCSCALERCOMMAND_H
#define __CMADCSCALERCOMMAND_H


#ifndef __CMODULECOMMAND_H
#include "CModuleCommand.h"
#endif

/*!
  The madcscaler command creates and configures Mesytec adc modules
  so that tht daq time and time couners are read as scalers.

  The format of this command is:

\verbatim
   madcscaler  create module ?config_params?
   madcscaler  config module ?config_param?
   madcscaler  cget   module
\endverbatim
  

   - create is used to create a new module, and optionally configure it.
   - config is used to configure an existing module
   - cget returns a module's configuration as a property list.

   See the CMADCScaler.h header for information about the configuration options supported.

*/

class CMADCScalerCommand : public CModuleCommand
{

public:
  CMADCScalerCommand(CTCLInterpreter& interp,
	       CConfiguration& config,
	       std::string commandName = std::string("madcscaler"));
  virtual ~CMADCScalerCommand();

private:
  CMADCScalerCommand(const CMADCScalerCommand& rhs);
  CMADCScalerCommand& operator=(const CMADCScalerCommand& rhs);
  int operator==(const CMADCScalerCommand& rhs) const;
  int operator!=(const CMADCScalerCommand& rhs) const;



  // We need to supply the members below to make the base class work.

protected:
  virtual CConfigurableObject* createObject();
  virtual std::string          getType();


};




#endif
