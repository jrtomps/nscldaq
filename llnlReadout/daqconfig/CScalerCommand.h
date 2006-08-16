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
   This class creates and configures SIS3820 scalers for the LLNL neutrons daq
   system.  The command supports the following syntaxes:
\verbatim
   scaler create module-name base-address
\endverbatim
    Unlike the 'normal' scripted readout. this software will read all defined
    scalers.  It will read them in the order in which they were defined.
    Furthermore, scalers do not require any configuration.  They are automatically
    set up properly to serve as 'event counters' for periodic scaler readout.
*/
class CScalerCommand : public CTCLObjectProcessor
{
  CConfiguration&     m_Config;
public:
  CScalerCommand(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CScalerCommand();
private:
  CScalerCommand(const CScalerCommand& rhs);
  CScalerCommand& operator=(const CScalerCommand& rhs);
  int operator==(const CScalerCommand& rhs) const;
  int operator!=(const CScalerCommand& rhs) const;
public:

  // Command entry point:

  virtual int operator()(CTCLInterpreter& interp, 
			 std::vector<CTCLObject>& objv);

private:
  int create(CTCLInterpreter& interp, 
	     std::vector<CTCLObject>& objv);

  void Usage(std::string msg, std::vector<CTCLObject>& objv);
	     
};
   

#endif
