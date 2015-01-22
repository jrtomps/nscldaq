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
#ifndef __CMASECOMMAND_H
#define __CMASECOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;
class CReadoutModule;

/*!
  The mase command creates and configures the mase readout XLM FPGA module.
  the format of this command is:

\verbatim
   mase  create module ?config_params?
   mase  config module ?config_param?
   mase  cget   module
\endverbatim

- create is used to create a new module give it a name and optionally cofigure it.
- config is used to configure an existing module.
- cget returns the module's configuration.

See the CMASECommand.h header for information about the configuration options supported by mase.
*/
class CMASECommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

  // Canonicals:
public:
  CMASECommand(CTCLInterpreter& interp,
	       CConfiguration&  config,
	       std::string commandName = std::string("mase"));
  virtual ~CMASECommand();
private:
  CMASECommand(const CMASECommand& rhs);
  CMASECommand& operator=(const CMASECommand& rhs);
  int operator==(const CMASECommand& rhs) const;
  int operator!=(const CMASECommand& rhs) const;

  // Implementng the normal command interface:

public:

  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  CConfiguration* getConfiguration();


public:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);
  void Usage(std::string msg, std::vector<CTCLObject>& objv);

private:
  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv,
		CReadoutModule* pModule,
		int startAt = 3);

};

#endif
