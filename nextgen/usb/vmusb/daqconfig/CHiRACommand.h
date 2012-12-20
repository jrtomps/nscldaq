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

#ifndef __CHIRACOMMAND_H
#define __CHIRACOMMAND_H


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


/**
 * this class creates and configures hira modules.  A hira module is just an
 * XLM and a FADC module.
 *
 */
class CHiRACommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

public:
  
  // Canonicals:
  
  CHiRACommand(CTCLInterpreter& interp,
	       CConfiguration&  config,
	       std::string      commandName = std::string("hira"));
  virtual ~CHiRACommand();

private:
  CHiRACommand(const CHiRACommand&);
  CHiRACommand& operator=(const CHiRACommand&);
  int operator==(const CHiRACommand&) const;
  int operator!=(const CHiRACommand&) const;

 // command entry point:
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  CConfiguration* getConfiguration();

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
  virtual int config(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv);
  virtual int cget(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);
  virtual void Usage(std::string msg, std::vector<CTCLObject>& objv);
};


#endif
