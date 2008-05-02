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

#ifndef __CN2530COMMAND_H
#define __CN2530COMMAND_H

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

/*!
  The hytec command creates and configures hytec NADC 2530 adcs.
 The format of this command is:

\verbatim
   hytec create module_name  ?config_params?...
   hytec config moule_name   ?config_params?
   hytec cget   module_name

\endverbatim
   - create is used to create a new module, and optionally configure it.
   - config is used to configure an existing module
   - cget returns as a Tcl property list, the configuration of an existing module.
   See the header for the CNADC2530 class for information about the
   configuration parameters that are supported by the module.


 Example:

\verbatim
hytec create adc -csr 0x800000 -memory 0x80000000
hytec config adc -ipl 5 -vector 0x100 -lld 1.0 -hld 8.0 -events 5
set hconf [hytec cget adc]
puts "configuration:"
foreach pair $hconf {
  set property  [lindex $pair 0]
  set value     [lindex $pair 1]
  puts "  $property  = $value"
}
\endverbatim


*/

class CNADC2530Command : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

public:
  CNADC2530Command(CTCLInterpreter& interp,
		   CConfiguration&  config,
		   std::string commandName = std::string("hytec"));
  virtual ~CNADC2530Command();

private:
  CNADC2530Command(const CNADC2530Command& rhs);
  CNADC2530Command& operator=(const CNADC2530Command& rhs);
  int operator==(const CNADC2530Command& rhs) const;
  int operator!=(const CNADC2530Command& rhs) const;

  // Implementation of the normal configuration command interface:

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
  void Usage(std::string msg, std::vector<CTCLObjet>& objv);

private:
  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv,
		CReadoutModule* pModule,
		int startAt = 3);

};

#endif
