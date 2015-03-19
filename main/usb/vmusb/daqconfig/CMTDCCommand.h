/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     CAEN Technologies
	     1140 Bay Street 2C
	     Staten Island, NY 10305
*/

#ifndef __CMTDCCOMMAND_H
#define __CMTDCCOMMAND_H

/**
 *  @file CMTDCCommand.h
 *  @brief Describes the CMTDCCommand class which provides the mtdc command to 
 *         VMUSBReadout.
 *  @author Ron Fox <ron@caentech.com>
 */

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

/**
 * The mtdc command crates and configures Mesytech MTDC32 modules.
 * THe form of this command is:
 * 
 * \verbatim
 *    mtdc create  name ?config_params?
 *    mtdc config  name ?config_params?
 *    mtdc cget    name
 *
 *  \endverbatim
 *
 *    *  create is used to create a new module and, optionally, configure it.
 *    *  config is used to configure an existing module.
 *    -  cget is used to retrieve the configuration of a module as a property list.
 *
 *    See the CMTDC32 header for configuration parameter information.
 */

class CMTDCCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

  // constructors and canonicals:

public:
  CMTDCCommand(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CMTDCCommand();

private:
  CMTDCCommand(const CMTDCCommand& rhs);
  CMTDCCommand& operator=(const CMTDCCommand& rhs);
  int operator==(const CMTDCCommand& rhs) const;
  int operator!=(const CMTDCCommand& rhs) const;

  // the command interface:

  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  CConfiguration* getConfiguration();

  // Dispatch targets:

private:
  void create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  void config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  void cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);
  std::string Usage(std::string msg, std::vector<CTCLObject>& objv);
  void  configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv,
		CReadoutModule* pModule,
		int startAt = 3);
  void throwNoSuchName(std::string command, std::string name, std::vector<CTCLObject>& objv);
};


#endif
