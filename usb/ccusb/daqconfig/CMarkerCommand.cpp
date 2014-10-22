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
 * @file CMarkerCommand.cpp
 * @brief Implementation of the CCUSBReadout's marker daqconfig command.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include "CMarkerCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CMarker.h>
#include <Exception.h>

/**
 *------------------------------------------------------------------------------------
 * Canonical methods.  Note those that were declared as private are actually forbidden.
 */


/**
 * constructor
 *   Create the command and save the global configuration so we can add'
 *   hardware modules to it.
 *
 * @param interp - The interpreter that will run this command (the command gets 
 *                 auto-registered on.
 * @param config - The global configuration that we manipulate.
 *
 */
CMarkerCommand::CMarkerCommand(CTCLInterpreter& interp, CConfiguration& config) :
  CTCLObjectProcessor(interp, "marker", true),
  m_Config(config)
{}

/**
 * destructor
 *   The base destructor will take care of all the heavy lifting.
 */
CMarkerCommand::~CMarkerCommand() 
{}


/*------------------------------------------------------------------------------------
 *  Command processing.
 */

/**
 * operator()
 *
 *   Called in response to the marker command:
 *   *  Ensure there's a subcommand.
 *   *  Dispatch to the sub command processor depending on the keyword or error for illegal subcommands.
 *
 *  @param interp - The Tcl Interpreter that is exectuting the command.
 *  @param objv   - The vector of command words that make up the command.
 *
 * @return int
 * @retval  TCL_OK - the command succeedded.
 * @retval  TCL_ERROR - the command failed.
 */
int
CMarkerCommand::operator()(CTCLInterpreter& interp,
			  std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);

  // We'll use string exceptions to report errors.  The string will become the 
  // result:

  try {
    // All commands must have at least a subcommand and the name of the module the affect:

    requireAtLeast(objv, 3, Usage("Incorrect Number of command Parameters", objv).c_str());

    std::string subcommand = objv[1];
    if (subcommand == "create") {
      create(interp, objv);
    } else if (subcommand == "config") {
      config(interp, objv);
    } else if (subcommand == "cget") {
      cget(interp, objv);
    } else {
      throw Usage("Invalid subcommand", objv);
    }
  } 
  catch(std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/**
 * create
 *
 *   Create a new object and store it in the configuration where it can be found and manipulated.
 *   It is an error to create a device with the same name as an existing object.
 *
 *  @param interp - The Tcl Interpreter that is exectuting the command.
 *  @param objv   - The vector of command words that make up the command.
 *
 * @throws std::string - error message.
 */
void
CMarkerCommand::create(CTCLInterpreter& interp,  std::vector<CTCLObject>& objv)
{
  // There can be additional parameters.  We're going to pretend we don't know
  // how many options there are so we require only an odd number of parameters:


  if((objv.size() % 2) == 0) {
    throw Usage("Invalid number of parameters", objv);
  }

  std::string name = objv[2];
  
  // check for duplicate name and throw if so:

  CReadoutModule* pModule = m_Config.findAdc(name);
  if(pModule) {
    throw Usage("Duplicate module", objv);
  }
  // Create it and configure it.


  CMarker* pMarker = new CMarker;
  pModule          = new CReadoutModule(name, *pMarker);
  try {
    configure(interp, pModule, objv);
    m_Config.addAdc(pModule);
    m_Config.setResult(name);
  }
  catch(...) {
    delete pModule;
    delete pMarker;
    throw;
  }

}
/**
 * config
 *    Process the config subcommand.
 *    * ensure there are an odd number of parameters.
 *    * Locate the module (complain if we can't).
 *    * configure.
 *
*
 *  @param interp - The Tcl Interpreter that is exectuting the command.
 *  @param objv   - The vector of command words that make up the command.
 *
 * @throws std::string - error message.
 */
void
CMarkerCommand::config(CTCLInterpreter& interp,  std::vector<CTCLObject>& objv)
{
  if ((objv.size() % 2) == 0) {
    throw Usage("Invalid number of parameters", objv);
  }

  std::string name =objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if(!pModule) {
    throw Usage("No such module", objv);
  }

  configure(interp, pModule, objv);
  interp.setResult(name);
}
/**
 * cget
 *   Process the cget subcommand.
 *   * Locate the module.
 *   * Fetch its configuration 
 *   * organize it into a list of name/value pairs.
 *
 *  @param interp - The Tcl Interpreter that is exectuting the command.
 *  @param objv   - The vector of command words that make up the command.
 *
 * @throws std::string - error message.
 */
void
CMarkerCommand::cget(CTCLInterpreter& interp,  std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, Usage("Invalid number of command parameters", objv).c_str());
    
    std::string name = objv[2];
    CReadoutModule* pModule = m_Config.findAdc(name);
    if (!pModule) {
        throw Usage("No such module", objv);
    }
    CConfigurableObject::ConfigurationArray config = pModule->cget();
    CTCLObject result;
    result.Bind(interp);
    
    for (int i =0; i < config.size(); i++) {
        CTCLObject key;
        CTCLObject value;
        CTCLObject sublist;
        key.Bind(interp); value.Bind(interp); sublist.Bind(interp);
        
        key   = config[i].first;
        value = config[i].second;
        
        sublist += key;
        sublist += value;
        
        result += sublist;
    }
    interp.setResult(result);
    
}
/**
 * Usage:
 *    Returns command usage information preceded by an error message.
 *
 *  @param msg - The error message.
 *  @param objv - Vector of objects that make up the command words.
 *  @return std::string - fully constructed error message.
 */
std::string
CMarkerCommand::Usage(std::string msg, std::vector<CTCLObject>& objv)
{
    // Prefix:
    
    std::string result = "Error: ";
    result            += msg;
    result            += "\n";
    
    // The full command:
    
    for (int i = 0; i < objv.size(); i++) {
        result += std::string(objv[i]);
        result += " ";
    }
    result += "\n";
    
    // Helpful usage information:
    
    result += "Usage:\n";
    result += "   marker create name ?options?\n";
    result += "   marker config name ?options?\n";
    result += "   marker cget name\n";
    
    return result;
}

/**
 * configure
 *   Does the actual work of extracting key/value pairs from command line
 *   words and using them to configure the object.  Note that configuration
 *   is not atomic.  If there's an error/failure, all options up until the
 *   failed on succeed.
 *
 * @param interp  - Interpreter running the command.
 * @param pModule - Module being configured.
 * @param objv    - Vector of command words
 * @param first   - Index into objv of the first configuration keywords.
 *
 * @note the caller must have ensured that [first:end] are an even number
 *       of elements.
 *  @throw std::string on error.
 */
void
CMarkerCommand::configure(
    CTCLInterpreter& interp, CReadoutModule* pModule, std::vector<CTCLObject>& objv,
    int firstPair
)
{
    // The try block below converts all exceptions to string exceptions.
    
    std::string failingPair;
    
    try {
        for (int i = firstPair; i < objv.size(); i +=2) {
            std::string key = objv[i];
            std::string value = objv[i+1];
            failingPair = "Failing pair was: ";
            failingPair += key;
            failingPair += value;
            
            pModule->configure(key, value);
        }
    }
    catch (std::string msg) {
        msg += " ";
        msg += failingPair;
        throw Usage(msg, objv);
    }
    catch (CException& e) {
        std::string msg = e.ReasonText();
        msg += " ";
        msg += failingPair;
        throw Usage(msg, objv);
    }
    catch (const char* msg) {
        std::string message = msg;
        message += " ";
        message += failingPair;
        throw Usage(message, objv);
    }
    catch (...) {
        std::string msg = "Some unanticiapted exception type: ";
        msg += failingPair;
        throw Usage(msg, objv);
    }
}
