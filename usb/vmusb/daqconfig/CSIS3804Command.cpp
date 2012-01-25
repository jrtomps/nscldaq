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
#include "CSIS3804Command.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <C3804.h>
#include <CReadoutModule.h>
#include <CConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////
/////////////////// Canonicals that are implemented ////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command and register it (base constructor does this
   by default.
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of ADCs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CSIS3804Command::CSIS3804Command(CTCLInterpreter& interp,
				 CConfiguration&  config) :
  CTCLObjectProcessor(interp, string("sis3804")),
  m_config(config)
{
}

/*!
  Destructor is a no-op but chains to the base class which unregisters
  etc. etc.
*/
CSIS3804Command::~CSIS3804Command()
{
}

////////////////////////////////////////////////////////////////////////

// Command dispatching

/*!
   The command entry point.
   - Ensure there are at least 3 parameters, the command, subcommand
     and the module name.
   - Dispatch to the subcommand processor which returns the result or...
   - Error if the subcommand is not recognized.

  Parameters:
    CTCLInterpreter& interp        - Interpreter running the command.
    std::vector<CTCLObject>& objv  - Command line words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The interpreter's result will be set in a manner that depends on 
     success/failure and the subcommand's operation.

*/
int
CSIS3804Command::operator()(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 3) {
    Usage("Insufficient command parameters", objv);
    return TCL_ERROR;
  }
  // Get the subcommand keyword and dispatch or error:

  string subcommand = objv[1];
  if (subcommand == string("create")) {
    return create(interp, objv);
  }
  else if (subcommand == string("config")) {
    return config(interp, objv);
  } 
  else if (subcommand == string("cget")) {
    return cget(interp, objv);
  }
  else {
    Usage("Invalid subcommand", objv);
    return TCL_ERROR;
  }
}

/*
  Process the create command.  We allow configuration information to occur
  after the name of the module.
  The main deal is to ensure we have a valid, unique scaler device, create,
  configure it and add it to the scaler list.

  Parameters and return values are the same as for operator()
*/
int
CSIS3804Command::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // We already know we have at least 3  parameters:
  // Ensure the name is unique:


  string name = objv[2];

  CReadoutModule* pModule = m_config.findScaler(name);
  if (pModule) {
    Usage("Attempted to create a duplicate scaler module", objv);
    return TCL_ERROR;
  }

  // Create and configure:

  CReadoutHardware* pHardware = new C3804;
  pModule                     = new CReadoutModule(name, *pHardware);
  int status = configure(interp, objv, pModule);
  if(status == TCL_OK) {
    m_config.addScaler(pModule);
  }
  else {
    delete pModule;
    delete pHardware;
  }
  return status;

}
/*
  Process the config command.  We locate the scaler module, then we
  invoke the configure option.  It's going to do all the needed error
  checking.  If we can't find the module, of course we'll report that error.
  parameters and return values are the same as for operator()
*/
int
CSIS3804Command::config(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  string          name    = objv[2];
  CReadoutModule* pModule = m_config.findScaler(name);
  if (!pModule) {
    Usage("No such scaler module", objv);
    return TCL_ERROR;;
  }
  // 

  return configure(interp, objv, pModule);
}
/*
  Get the configuration as a TCL list and return it as the command result.
  it is an error if:
  - Number of command parameters != 3.
  - The module does not exist.

  Parameters and return values are the same as for operator()

*/
int
CSIS3804Command::cget(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // Validate parameter count.

  if (objv.size() != 3) {
    Usage("Incorrect number of command parameters", objv);
    return TCL_ERROR;
  }
  // validate module name:

  string name = objv[2];
  CReadoutModule* pModule = m_config.findScaler(name);
  if (!pModule) {
    Usage("No such scaler module", objv);
    return TCL_ERROR;
  }
  // Create the return list.

  CConfigurableObject::ConfigurationArray config = pModule->cget();

  Tcl_Obj* pResult = Tcl_NewListObj(0, NULL);

  for (int i =0; i < config.size(); i++) {
    Tcl_Obj* key   = Tcl_NewStringObj(config[i].first.c_str(), -1);
    Tcl_Obj* value = Tcl_NewStringObj(config[i].second.c_str(), -1);

    Tcl_Obj* sublist[2] = {key, value};
    Tcl_Obj* sl = Tcl_NewListObj(2, sublist);
    Tcl_ListObjAppendElement(interp.getInterpreter(), pResult, sl);
  }
  Tcl_SetObjResult(interp.getInterpreter(), pResult);
  return TCL_OK;
  
}

/*
   Configure a module:  Common operations to process a configuration.
   - Ensure there are an even number of configuration key/values.
   - For each pair, configure the hardware.

   Parameters:
       interp   - The interpreter running this command.
       objv     - The command words.
       pModule  - The hardware module that will be configured.
       startAt  - Which index of objv contains the first config keyword (defaults to 3).

   Returns:
      TCL_OK   - Everything worked.
      TCL_ERROR- The configuration failed.


*/
int
CSIS3804Command::configure(CTCLInterpreter& interp,
			    std::vector<CTCLObject>& objv,
			    CReadoutModule* pModule,
			    int startAt)
{
  // See if there are an even number of keyword value pairs:

  int nWords        = objv.size();
  int nConfigItems  = nWords - startAt;
  if(nConfigItems % 2) {
    Usage("For each configuration keyword there must be a value too", objv);
    return TCL_ERROR;
  }
  try {
    while(startAt < nWords) {
      string  key = objv[startAt];
      string  val = objv[startAt+1];
      pModule->configure(key, val);
      startAt += 2;
    }
  }
  catch (string msg) {		// a config option failed.
    Usage(msg, objv);
    return TCL_ERROR;
  }
  return TCL_OK;
}
/*
   Process an error by setting the result to a message that includes the
   entire command as well as usage information.
*/
void
CSIS3804Command::Usage(string msg, vector<CTCLObject>& objv)
{
  string result("ERROR: ");
  result += msg;
  result += "\n";
  for (int i = 0; i < objv.size(); i++) {
    result += string(objv[i]);
    result += ' ';
  }
  result += "\n";
  result += "Usage\n";

  result += "    sis3804 create name ?config-options? \n";
  result += "    sis3804 config name ?config-options? \n";
  result += "    sis3804 cget name";

}
