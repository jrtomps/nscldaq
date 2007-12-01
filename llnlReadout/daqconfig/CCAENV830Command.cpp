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
#include "CCAENV830Command.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"

#include <C830.h>
#include <CReadoutModule.h>

#include <stdlib.h>
#include <errno.h>

using std::string;
using std::vector;


////////////////////////////////////////////////////////////////////////
///////////////////////////////// Canonicals ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command.
   \param interp : CTCLInterpreter&
      Reference to the interpreter on which the command will be registered
   \param config : CConfiguration&
       Configuration we will be maintaining.
*/
CCAENV830Command::  CCAENV830Command(CTCLInterpreter& interp, CConfiguration& config,
				     string name) :
  CTCLObjectProcessor(interp, name),
  m_Config(config)
{
}
/*!
   The destructor is just needed to provide a virtual chain back to the
   base class destructor.
*/
CCAENV830Command::~CCAENV830Command()
{
}

///////////////////////////////////////////////////////////////////////
////////////////////////// Command processing /////////////////////////
///////////////////////////////////////////////////////////////////////


/*!
   Process the command:
   - Ensure there are exactly 4 command parameters.
   - Ensure the subcommand is create.
   - Ensure the module name does not already exists.
   - Ensure the base address is a valid uint32_t.
   - Create and configure the scaler.
   - Add it to the configuration.

   \param interp : CTCLInterpreter&
       Referes to the interpreter that is running this command.
   \param objv    : std::vector<CTCLObject>&
       reference to a vector of TCL Objects that are the command words.

   \return int
   \retval TCL_ERROR - If the command failed for some reason.
   \retval TCL_OK    - If the command succeeded.

  \note Side effects:   result is modified.
  - If the command succeeded, the result is the name of the new module.
  - If the command failed (returned TCL_ERROR), the result is an error
    messages string that begins with the text "ERROR"
*/
int
CCAENV830Command:: operator()(CTCLInterpreter& interp, 
			 std::vector<CTCLObject>& objv)
{
  if (objv.size() < 4) {
    Usage("Invalid parameter count", objv);
    return TCL_ERROR;
  }
  string subcommand = objv[1];
  string name       = objv[2];
  string sBase      = objv[3];



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
   Handle the creation of the module:

*/
int
CCAENV830Command::create(CTCLInterpreter& interp, 
			 std::vector<CTCLObject>& objv)
{
  string subcommand = objv[1];
  string name       = objv[2];
  string sBase      = objv[3];

  CReadoutModule*  pModule = m_Config.findScaler(name);
  if (pModule) {
    Usage("Attempted to create a duplicate scaler", objv);
    return TCL_ERROR;
  }
  
  uint32_t base = strtoul(sBase.c_str(), NULL, 0);
  if ((base == 0) && (errno != 0)) {
    Usage("Invalid base address", objv);
    return TCL_ERROR;
  }
  CReadoutHardware* pHardware = new C830;
  pModule  = new CReadoutModule(name, *pHardware);
  pModule->configure(string("-base"), sBase);
  
  m_Config.addScaler(pModule);

  m_Config.setResult(name);
  return TCL_OK;
}

/*!
  Configure the module:
*/

int
CCAENV830Command::config(CTCLInterpreter& interp,
			 vector<CTCLObject>& objv)
{
  string subcommand = objv[1];
  string name       = objv[2];

  CReadoutModule* pModule = m_Config.findScaler(name);
  if (!pModule) {
    Usage("caenv830 config unknonwn module name", objv);
    return TCL_ERROR;
  }
  // There must be an even number of configuration parameters...
  // that means a total odd number of command words:

  if ((objv.size() & 1) == 0) {
    Usage("caenv830 config - improper nubmer of command parameters", objv);
    return TCL_ERROR;
  }

  // Do the configuration within a try/catch so that e.g. invalid
  // parameters get caught.
  //
  try {
    for (int i = 3; i < objv.size(); i +=2) {
      string key   = objv[i];
      string value = objv[i+1];
      pModule->configure(key, value);
    }
  }
  catch (string msg) {		// issue - the configuration may have been partially done.
    
    Usage(msg, objv);
    return TCL_ERROR;
  }

  m_Config.setResult(name);
  return TCL_OK;
}
/*! 
  Get the configuration of the module into the interpreter result.
*/
int
CCAENV830Command::cget(CTCLInterpreter& interp,
		       vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    Usage("Invalid parameter count: ", objv);
    return TCL_ERROR;
  }

  string name             = objv[2];
  CReadoutModule* pModule = m_Config.findScaler(name);
  if (!pModule) {
    Usage("Module name does not correspond to an existing scaler", objv);
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

/////////////////////////////////////////////////////////////////////////
////////////////////// Utility(s) ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*
   Set the result to  a usage message that is headed by a messages and
   a reconstruction of the command following substitution.
   Parametrs:
    string msg                : error message.  This will be preceded by ERROR:
    vector<CTCLObject>& objv  : The command line words.
*/
void
CCAENV830Command::Usage(string msg, vector<CTCLObject>& objv)
{
  string result = "ERROR: ";
  result       += msg;
  result       += "\n";
  for (int i =0; i < objv.size(); i++) {
    msg += string(objv[i]);
    msg += ' ';
  }
  msg   += "\n";
  msg   += "Usage:\n";
  msg   += "   scaler create name base";

  m_Config.setResult(msg);
}
