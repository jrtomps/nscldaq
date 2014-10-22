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

/**
 * @file CMarkerCommand.h
 * @brief Defines the marker command which creates, configs marker drivers.
 * @author Ron FOx <fox@nscl.msu.edu>
 */
#ifndef __CMARKERCOMMAND_H
#define __CMARKERCOMMAND_H

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
 * @class CMarkerCommand
 *    Defines the command that creates, configures and queries marker device
 *    driver instances.  This command is the base command for an ensemble with the usual
 *    create, config, cget sub commands.
 * @note
 *    This manages a pseudo device that does not occupy any hardware position in the crate.
 */
class CMarkerCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;                    // Global device configuration instance.

  // Canonicals.

public:
  CMarkerCommand(CTCLInterpreter& interp, CConfiguration& config);
  virtual ~CMarkerCommand();

private:
  CMarkerCommand(const CMarkerCommand& rhs);
  CMarkerCommand& operator=(const CMarkerCommand& rhs);
  int operator==(const CMarkerCommand& rhs) const;
  int operator!=(const CMarkerCommand& rhs) const;

public:
  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // Processors for the individual ensemble subcommands.

private:
  void create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  void config(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  void cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);

  // Utilty functions.

  std::string Usage(std::string msg, std::vector<CTCLObject>& objv);
  void    configure(CTCLInterpreter&         interp,
		   CReadoutModule*          pModule,
		   std::vector<CTCLObject>& config,
		   int                      firstPair = 3);

};

#endif
