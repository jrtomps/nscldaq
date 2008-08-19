
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
#ifndef __CMADCCOMMAND_H
#define __CMADCCOMMAND_H

#ifndef __CMODULECOMMAND_H
#include <CModuleCommand.h>
#endif


/*!
  The madc command creates and configures Mesytec adc modules.

  The format of this command i:

\verbatim
   madc  create module ?config_params?
   madc  config module ?config_param?
   madc  cget   module
\endverbatim
  

   - create is used to create a new module, and optionally configure it.
   - config is used to configure an existing module
   - cget returns a module's configuration as a property list.

   See the CMADC32.h header for information about the configuration options supported.

*/

class CMADCCommand : public CDAQModuleCommand
{

public:
  CMADCCommand(CTCLInterpreter& interp,
	       CConfiguration& config,
	       std::string commandName = std::string("madc"));
  virtual ~CMADCCommand();

private:
  CMADCCommand(const CMADCCommand& rhs);
  CMADCCommand& operator=(const CMADCCommand& rhs);
  int operator==(const CMADCCommand& rhs) const;
  int operator!=(const CMADCCommand& rhs) const;

  // We need to supply the members below to make the base class work.

protected:
  virtual CConfigurableObject* createObject();
  virtual std::string          getType();

};




#endif
