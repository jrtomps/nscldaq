
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
#ifndef __CV977COMMAND_H
#define __CV977COMMAND_H


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

   See the CV97732.h header for information about the configuration options supported.

*/

class CV977Command : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

  // Constructors and canonicals.
public:
  CV977Command(CTCLInterpreter& interp,
	       CConfiguration& config,
	       std::string commandName = std::string("v977"));
  virtual ~CV977Command();

private:
  CV977Command(const CV977Command& rhs);
  CV977Command& operator=(const CV977Command& rhs);
  int operator==(const CV977Command& rhs) const;
  int operator!=(const CV977Command& rhs) const;

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
