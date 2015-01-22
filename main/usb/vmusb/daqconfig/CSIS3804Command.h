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

#ifndef __CSIS3804COMMAND_H
#define __CSIS3804COMMAND_H

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
  This class creates and configures SIS 3804 scalers for the VM-USB readout system.
   the comman supports the following syntaxes:

\verbatim
   sis3804 create module-name ?config-options?
   sis3804 config module-name ?config-options?
   sis3804 cget   module-name
\verbatim

   See the C3804 class for information about the configuration optionst that
   are acceptable.

*/
class CSIS3804Command : public CTCLObjectProcessor
{
  CConfiguration& m_config;
public:
  CSIS3804Command(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CSIS3804Command();
private:
  CSIS3804Command(const CSIS3804Command& rhs);
  CSIS3804Command& operator=(const CSIS3804Command& rhs);
  int operator==(const CSIS3804Command& rhs) const;
  int operator!=(const CSIS3804Command& rhs) const;

public:
  // Command entry point:

  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // Command utilities:

private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);

  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv,
		CReadoutModule* pModule,
		int startAt = 3);

  void Usage(std::string msg, std::vector<CTCLObject>& objv);

};

#endif
  
