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
#include <config.h>
#include "CMTDCCommand.h"
#include "CMTDC32.h"

#include <TCLInterpreter.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>


/**
 * Construction and other implemented canonicals.
 */
CMTDCCommand::CMTDCCommand(CTCLInterpreter& interp, CConfiguration& config) : 
  CTCLObjectProcessor(interp, "mtdc"),
  m_Config(config)
{}

/**
 * Destructor:
 */
CMTDCCommand::~CMTDCCommand() {} // At present nothing to do.

/*--------------------------------------------------------------------------
 *  Command processing.
 */

/**
 * operator()
 *   Called when the mtdc command is invoked.  
 *   *  Ensure there are the minimum number of command words (3)
 *   *  dispatch to the proper handler.
 *
 * @note This implementation will use the newer style I've been using of
 *       making all errors into string exceptions.  The thrown string should
 *       be an error message as it becomes the command result string.
 *
 * @param interp - Reference to the  encapsulated interpreter that is running
 *                 the command.
 * @param objv   - Vector of encapsulated command words.
 *
 * @return int
 * @retval TCL_OK   - Success command value is determined by the executor.
 * @retval TCL_ERROR - failure.  THe command value is a human readable error message.
 */
int
CMTDCCommand::operator()(CTCLInterpreter& interp,  std::vector<CTCLObject>& objv)
{
  try {
    // Validate the argument count and bind the parameters:

    bindAll(interp, objv);
    requireAtLeast(objv, 3, 
		   Usage("Insufficient command parameters", objv).c_str());

    // Extract the keyword and dispatch:

    std::string subcommand = objv[1];
    if (subcommand == "create" ) {
      create(interp, objv);
    } else  if (subcommand == "config") {
      config(interp, objv);
    } else if (subcommand == "cget") {
      cget(interp, objv);
    } else {
      throw Usage("Invalid command keyword", objv);
    }

  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }

  return TCL_OK;
}
/**
 * create
 *    Create a new module.  The form of this command is:
 * \verbatim
 *    mtdc create module-name ?options?
 * \endverbatim
 *   where the optional options are assumed to be configuration option
 *   value pairs.
 *
 * @param interp  - Interpreter in which the command is running.
 * @param objv    - objectified command line words.
 *
 * @note - errors are thrown strings while the result on success is the name of the
 *         created object so one could do:
 * \verbatim
 *   mtdc config [mtdc create george] options....
 * \endverbatim
 */
void
CMTDCCommand::create(CTCLInterpreter& interp,  std::vector<CTCLObject>& objv)
{
  // We need at least 3 command words:

  requireAtLeast(objv, 3, Usage("mtdc: Insufficient command parameters", objv).c_str());
  
  // If configuration parameters are provided, there must be an even number of them
  // which means an odd total number of parameters:

  if( (objv.size() % 2) == 0) {
    throw Usage("mtdc create - there must be an even number of configuration words", objv);
  }

  // cannot have duplicate names:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    throw Usage("mtdc create - duplicate module name", objv);
  }

  // Create the module and bind it ot the configuration.
  // after that we're in a try/catch block where the catch block can destroy the stuff we create
  // and re-throw to our caller:

  CMTDC32* pTdc = new CMTDC32;
  pModule       = new CReadoutModule(name, *pTdc);

  try {
    if (objv.size() > 3) {
      configure(interp, objv, pModule);
    }
    // 
  }
  catch (...) {
    delete pModule;
    delete pTdc;
    throw;
  }
  // Wire the module into the known set:

  m_Config.addAdc(pModule);
  interp.setResult(name);

}
/**
 * config
 *   Configure an existing module.  The form of this command is:
 * \verbatim
 *   mtdc config name options...
 * \endverbatim
 *
 * @param interp - Interpreter that is running the command.
 * @param objv   - Vector of objectified command words.
 *
 * @note all errors are thrown to be caught by the caller.
 */
void
CMTDCCommand::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // There must be at least 5 parameters to supply a configuration, and an odd number at that:

  requireAtLeast(objv, 5, Usage("mtdc config: Insufficient command parameters", objv).c_str());
  if ((objv.size() % 2) == 0) {
    throw Usage("mtdc config: there must be an even number of configuration words", objv);
  }
  // find the module:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    throwNoSuchName("mtdc config", name, objv);
  }
  configure(interp, objv, pModule);
  interp.setResult(name);	// For command chaining.

}
/**
 * cget
 *   set as a result the entire device configuration as a list of
 *   keyword value pairs.
 * @param interp - the interpreter running this command.
 * @param objv   - vector containing the command line words.
 */
void
CMTDCCommand::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // There are only three command words;

  requireExactly(objv, 3, Usage("mtdc cget should only have 3 command parameters: ", objv).c_str());

  std::string name = objv[2];
  CReadoutModule* pModule= m_Config.findAdc(name);
  if (!pModule) {
    throwNoSuchName("mtdc cget", name, objv);

  }

  CConfigurableObject::ConfigurationArray config = pModule->cget();

  CTCLObject result;
  result.Bind(interp);

  for (int i =0; i < config.size(); i++) {
    CTCLObject key;
    CTCLObject value;
    key.Bind(interp); value.Bind(interp);
    key = config[i].first;
    value = config[i].second;

    CTCLObject sublist;
    sublist.Bind(interp);
    sublist+= key;
    sublist+= value;
    result += sublist;
  }
  interp.setResult(result);

}

/**
 * configure
 *   Does the actual configuration of a module:
 *
 * @param interp  - Interpreter running the command.
 * @param objv    - vector of command line words.
 * @param pModule - Pointer to the CReadoutModule being configured.
 * @param startAt - Index into objv at which the first configuration parameter keyword will be found.
 * 
 * @note the caller has validated the length of the objv.
 * @note all exceptions get a usage string added to them and then are rethrown.
 *
 */
void
CMTDCCommand::configure(CTCLInterpreter& interp, std::vector<CTCLObject>& objv, CReadoutModule* pModule, int startAt)
{
  int nWords        = objv.size();
  int nConfigItems  = nWords - startAt;

  try {
    while(startAt < nWords) {
      std::string  key = objv[startAt];
      std::string  val = objv[startAt+1];
      pModule->configure(key, val);
      startAt += 2;
    }
  }
  catch (std::string msg) {	      
    throw Usage(msg, objv);
  }
  
}






/**
 * throwNoSuchName
 *   Constructs and throws a string appropriate to an inability to find a module by name.
 *
 * @param command - command being run (e.g. mtdc config).
 * @param name    - Name of the module being looked for
 * @param objv    - The command words.
 */
void
CMTDCCommand::throwNoSuchName(std::string command, std::string name, std::vector<CTCLObject>& objv)
{
  std::string baseMsg = command;
  baseMsg += ": NO such module: ";
  baseMsg += name;
  baseMsg += ": ";
  throw Usage(baseMsg, objv);
}

/**
 * Usage:
 *   Returns a string with an error message, the command line and some helpful
 *   usage information.
 *
 * @param msg  - The error message part of the final string
 * @param objv - The vector of CTCLObject's that make up the command line words.
 *
 * @return std::string final message.
 *
 */
std::string
CMTDCCommand::Usage(std::string msg, std::vector<CTCLObject>&  objv)
{

  // The error message:

  std::string result = msg;
  msg += "\n";

  // The command line:

  msg += "Command was: ";
  for (int i =0; i < objv.size(); i++) {
    msg += std::string(objv[i]);
    msg += " ";
  }
  msg += "\n";

  // The usage information:

  msg += "Usage: \n";
  msg += "   mtdc create name ?options?\n";
  msg += "   mtdc config name option...\n";
  msg += "   mtdc cget\n";


  return result;
}
