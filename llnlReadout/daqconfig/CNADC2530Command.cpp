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
#include "CNADC2530Command.h"
#include <TCLInterpreter.h>
#include <CConfiguration.h>
#include <CNADC2530.h>
#include <CReadoutModule.h>
#include <CConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>


using std::string;
using std::vector;


////////////////////////////////////////////////////////////////////////////////////
//   Constructors and implemented canonical functions


/*!
   Construct the command and register it.  The base constructor does this by default.
   \param interp   : CTCLInterpreter& 
       Tcl interpreter on which the command is registered.
   \param config   : CConfiguration&
       The configuration of ADC's etc. that are manipulated by this command.
   \param commandName : std::string
       Name of the command to register.  This defaults to "hytec"

*/
CNADC2530Command::CNADC2530Command(CTCLInterpreter&   interp,
				   CConfiguration&    config,
				   std::string        commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{
}
/*!  
   The constructor only exists to maintain a chain of destructors back to the ultimate base class.
*/
CNADC2530Command::~CNADC2530Command()
{
}

///////////////////////////////////////////////////////////////////////////////////
//    Command processing.

/*!
   The command entry just:
   - Ensures there are at least three parameters, the command, the subcommand and the
     name of an item on which to operate.
   - Dispatches to the appropriate sub-command processor... or
   - Returns an error if the subcommand keyword is not recognized.


   \param CTCLInterpreter&  interp       - Interpreter running the command
   \param  std::vector<CTCLObject>& objv  - Command line words.

   \return int
   \retval TCL_OK    - Command was successful.
   \retval TCL_ERROR - Command failed.

  \note The interpreter's result will be set in a mannter that depends on the
        success/failure of the specific subcommand's operation.
*/

int
CNADC2530Command::operator()(CTCLInterpreter& interp, 
			     vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 3) {
    Usage(string("Insufficient command parameters"), objv);
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
/*!
  Return the configuration database that we are
building.
*/
CConfiguration*
CNADC2530Command::getConfiguration()
{
  return &m_Config;
}
/*
  Process the create subcommand:
  - ensure we have enough values on the command line.
  - ensure we have a valid name that is not yet used.
  - create the module
  - if there are additional configuration parameters
  - configure the module using them.

  Parameters and return value are as for operator()
*/
int
CNADC2530Command::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // We already know we have at least three elements...

  string name = objv[2];

  // ensure the name is unique:

  CReadoutModule* pModule = m_Config.findAdc(name);
  if(pModule) {
    Usage("Duplicate moduel creation attempted", objv);
    return TCL_ERROR;
  }

  // Tentatively create the module, and bind it to a configuration.

  CNADC2530* pAdc = new CNADC2530;
  pModule         = new CReadoutModule(name, *pAdc);

  // If there are additional parameters, attempt to configure the module
  

  int status = TCL_OK;		// If no config, then configuration worked ;-)

  if (objv.size() > 3) {
    status = configure(interp, objv, pModule);
  }

  // If the configuration failed, back off; otherwise enter the module in the configuration 
  // database.

  if (status != TCL_OK) {
    delete pModule;
    delete pAdc;
  }
  else {
    m_Config.addAdc(pModule);
    m_Config.setResult(name);
  }

  return status;

}
/*
   Process the configure subcommand:
   - Ensure that we have more than three parameters.
   - Ensure that the name is valid by locating it in the configuration database.
   - pass control to the configure function.\
*/

int
CNADC2530Command::config(CTCLInterpreter& interp,
			 vector<CTCLObject>& objv)
{
  if (objv.size() < 4) {
    Usage("Insufficient parameters to do a configuration", objv);
    return TCL_ERROR;
  }


  // Locate the module:

  string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    Usage("Module does not exist", objv);
    return TCL_ERROR;
  }
  // Configure it according to the remaining parameters.

  return configure(interp, objv, pModule);
}
/*
   Process the cget command.  
  - There must be exactly three parameters.
  - Locate the module.
  - Create a TCL list of the configuration array.

*/
int
CNADC2530Command::cget(CTCLInterpreter& interp,
		       vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    Usage("Invalid command parameter count for cget", objv);
    return TCL_ERROR;
  }
  string           name    = objv[2];
  CReadoutModule *pModule = m_Config.findAdc(name);
  if (!pModule) {
    Usage("No such  module", objv);
    return TCL_ERROR;
  }
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
   set the return value with an error message,
   the full command, and a usage string:
*/
void
CNADC2530Command::Usage(string msg, std::vector<CTCLObject>& objv)
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
  result += "    hytec create name base-address\n";
  result += "    hytec config name config-params...\n";
  result += "    hytec cget name";

  m_Config.setResult(result);

}

/*
   Configure the module - common code shared by create and config.
   - Ensure there are an even number of configuration key/values.
   - For each pair, configure the hardware.

   Parameters:
       interp   - The interpreter running this command.
       objv     - The command words.
       pModule  - The hardware module that will be configured.
       startAt  - Which index of objv contains the first config keyword (defaults to 3).

   Returns:
      TCL_OK   - Everything worked.
      TCL_ERROR- The configuration failed.*/

int
CNADC2530Command::configure(CTCLInterpreter& interp,
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
