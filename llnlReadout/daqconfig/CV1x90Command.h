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
#ifndef __CV1X90COMMAND_H
#define __CV1X90COMMAND_H

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
  This class creates and configures CAEN V1290/V1190 TDC modules for the
  scripted VM-USB readout system (formerly known as the LLNL neutrons DAQ
  system until it got more general.
  The command supports the usual three syntaxes:

\verbatim
tdc1x90  create module-name  ?options?
tdc1x90  config module-name  ?options?
tdc1x90  cget   module-name
\endverabtim

    The create form of the command creates a tdc driver instance and optionally
    configures it using the remaining options.
    The config form configures an existing TDC.
    The cget form returns the configuration as a list of keyword value pairs.
    where the keyword is the configuration switch and the value the
    value of he configuration option associated with that switch.

    For configuration options, see the documentation or the 
    CV1x90 module in the ../devices subdirectory.
*/

class CV1x90Command : public CTCLObjectProcessor
{
private:
  CConfiguration&   m_Config;
public:
  CV1x90Command(CTCLInterpreter&  interp,
		CConfiguration&   config,
		std::string       commandName = std::string("tdc1x90"));
  virtual ~CV1x90Command();

private:  
  CV1x90Command(const CV1x90Command& rhs);
  CV1x90Command& operator=(const CV1x90Command& rhs);
  int operator==(const CV1x90Command& rhs) const;
  int operator!=(const CV1x90Command& rhs) const;

  // Implementing the normal command interface:


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
		CReadoutModule*          pModule,
		int startAt = 3);


};
#endif
