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
#include "CModuleCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CConfigurableObject.h>
#include <CItemConfiguration.h>

using std::string;
using std::vector;


//////////////////////////////////////////////////////////////////////////////////////////


/**
 * Constructors and canonicals.
**/

/*!
   \param interp  - Reference to the interpreter that will execute this command object.
                    The object resulting from this construction will be 'hooked up'
                    so that its operator() is called when the command specified by
                    name is invoked.
   \param config  - Reference to the configuration object.  This configuration object
                    is what the command will manipulate when it creates and configures
                    module objects.
   \param name    - The name of the command.
*/
CDAQModuleCommand::CDAQModuleCommand(CTCLInterpreter& interp, CConfiguration& config, string name) :
  CTCLObjectProcessor(interp, name),
  m_config(config)
{}

/*!
   Chain to the base class destructor which will unregister the command.
*/
CDAQModuleCommand::~CDAQModuleCommand()
{}

//////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Implementations of virtual methods that are basic to the operation of the object:
*/


/*!
  The command entry just:
  - Ensures there is at least 3 parameters, the command, subcommand and
    module name.
  - Dispatches to the appropriate subcommand processor... or
  - Returns an error if the subcommand keyword is not recognized.
 
    \param interp        - Interpreter running the command.
    \param objv  - Command line words.

    \return int
    \retval TCL_OK      - Command was successful.
    \retval TCL_ERROR   - Command failed.

  Side effects:
     The interpreter's result will be set in a manner that depends on 
     success/failure and the subcommand's operation.

*/
int
CDAQModuleCommand::operator()(CTCLInterpreter& interp, vector<CTCLObject>& objv)
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
/*!
   Process the create subcommand:
   - ensure we have enough values on the command line.
   - If there are 4 command words, insert a -base in front of it.
   - ensure we have a valid module name.
   - ensure that there is no other module with the same name.
   - Create the new module module
   - Add it to the configuration.
   - If there are additional command words use them to configure the object.

     \param    interp   - Interpreter that is executing this command.
     \param    objv     - Vector of command words.

    \return int
    \retval TCL_OK      - Command was successful.
    \retval TCL_ERROR   - Command failed.

  Side effects:

     The result for the interpreter is set as follows:
     - On error this is an error message of the form ERROR: message
     - On success, this is the name of the module. allowing e.g.
       adc config [adc create adc1 0x80000000] ....
*/
int
CDAQModuleCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if (objv.size() < 3) {
    Usage("not enough parameters for the create subcommand", objv);
    return TCL_ERROR;
  }
  // If we have exactly 4 elements, we need to insert a -base in front of the last one.
  // after that we must have an odd number of command words so that config works correctly:

  CTCLObject base;
  base = string("-base");
  if (objv.size() == 4) {
    base.Bind(interp);
    vector<CTCLObject>::iterator i = objv.begin(); // [0].
    i++; i++; i++;		                   // [3]
    objv.insert(i, base);	                   // I think this pushes the value back.
  }
  if ((objv.size() % 2) != 1) {
    Usage("Incorrect number of parameters for the create subcommand", objv);
    return TCL_ERROR;
  }

  // Ensure there's not already a module with my type:

  string name = objv[2]; 
  if (findModule(name)) {
    Usage("Duplicate module for the create subcommand", objv);
    return TCL_ERROR;
  }
  // Create configure and insert the module in the configuration.

  CConfigurableObject* pModule = createObject();
  pModule->Attach(new CItemConfiguration(name), true);

  addObjectToConfiguration(m_config, name, getType(), pModule);


  interp.setResult(name);
  if (objv.size() > 3) {
    return Configure(interp, 	// might fail.
		     objv,
		     pModule,
		     3);
  }
  return TCL_OK;
  

}
/*!
    Configure an existing module.
    - Ensure that there are enough command line parameters.  These means
      at least 5 parameters in order to have at least one configuration
      keyword value pair... and that there are an odd number of params
      (to ensure that all keywords have values).
    - Ensure the module exists, and matches this type.
    - Use the Configure utility to configure the object.

     \param interp   - Interpreter that is executing this command.
     \param objv     - Vector of command words.

    \return int
    \retval TCL_OK      - Command was successful.
    \retval TCL_ERROR   - Command failed.

  Side effects:

     The interpreter result is set with an error message if the return value
     is TCL_ERROR, otherwise it is set with the module name.
     Note that all error messages will start with the text "ERROR:"
*/
int
CDAQModuleCommand::config(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if ((objv.size() < 5) || ((objv.size() % 2) != 1)) {
    Usage("Incorrect number of command parameters in ", objv);
    return TCL_ERROR;
  }

  // Locate the module ensuring it's one of ours:

  string name = objv[2];
  CConfigurableObject* pObject = findModuleOfMyType(name);
  if (!pObject) {
    Usage("Module specified does not exist or is of the wrong type", objv);
    return TCL_ERROR;
  }


  return Configure(interp, objv, pObject);
  
}
/*!
   Get the configuration of a module and return it as a list of
   keyword/value pairs.
   - ensure we have enough command line parameters (exactly 3).
   - Ensure a module of this type  exists and get its pointer.
   - Fetch the module's configuration.
   - Map the configuration into a list of 2 element lists and set the
     result accordingly.

     \param interp   - Interpreter that is executing this command.
     \param objv     - Vector of command words.

     \return int
     \retval TCL_OK      - Command was successful.
     retval  TCL_ERROR   - Command failed.

  Side effects:

     The interpreter result is set.  If the command returned an error, 
     This is a string that begins with the text ERROR:  otherwise it is a 
     list of 2 element sublists where each sublist is a configuration keyword
     value pair...e.g. {-base 0x80000000} ...
*/
int
CDAQModuleCommand::cget(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    Usage("Invalid command parameter count for cget", objv);
    return TCL_ERROR;
  }
  string               name    = objv[2];
  CConfigurableObject* pModule = findModuleOfMyType(name);
  if (!pModule) {
    Usage("No such  module", objv);
    return TCL_ERROR;
  }
  CItemConfiguration::ConfigurationArray config = pModule->cget();

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

/*!
   Create error message text, and set it as the interpreter result.
*/
void
CDAQModuleCommand::Usage(std::string msg, std::vector<CTCLObject>& objv)
{
  string result("ERROR: ");
  string cmd = objv[0];
  result += msg;
  result += "\n";
  for (int i = 0; i < objv.size(); i++) {
    result += string(objv[i]);
    result += ' ';
  }
  result += "\n";
  result += "Usage\n";
  result += "    ";
  result += cmd;
  result += "  create name base-address\n";
  result += "    ";
  result += cmd;
  result += " config name config-params...\n";
  result += "    ";
  result += cmd;
  result += " cget name";

  getInterpreter()->setResult(result);
}


////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Hook that allows concrete classes to override how modules are added to the configuration.
*/

/*!
  Add an configurable object to the configuration.
  This can be overriden for special needs classes.

  \param config   - Reference to the config in which the object should be registered.
  \param name     - Name to be associated with the object.
  \param type     - String that defines the type of the object to register.
  \param object   - Pointer to the object to register.

  \note Implementors can be assured that there is no duplicate module in the configuration
        at this time.
*/
void
CDAQModuleCommand::addObjectToConfiguration(CConfiguration& config,
					 std::string     name,
					 std::string     type,
					 CConfigurableObject* object)
{
  config.addObject(name, type, object);
}


////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Internal utility functions.
*/

/*
  Locate a module in the configuration given its name, and not caring about its type.
  If found, a pointer to the module is returned, if not a NULL.
*/
CConfigurableObject*
CDAQModuleCommand::findModule(string name)
{
  CConfiguration::ConfigurationIterator p = m_config.findObjectByName(name);
  if (p == m_config.end()) {
    return reinterpret_cast<CConfigurableObject*>(0);
  }

  return p->s_pObject;
}
/*
  Same as above, but also returns null if the module's type is not a match for the
  type of module we're managing
*/
CConfigurableObject*
CDAQModuleCommand::findModuleOfMyType(string name)
{  CConfiguration::ConfigurationIterator p = m_config.findObjectByName(name);
  if (p == m_config.end()) {
    return reinterpret_cast<CConfigurableObject*>(0);
  }

  return (p->s_type == getType()) ? p->s_pObject :  reinterpret_cast<CConfigurableObject*>(0);
}
/*
   Configure a module using pairs in the objv array starting at some point and
   continuing to the end of objv.  The assumption is that there are an even number
   of remainig elements in objv.
   Parameters:
      interp  - Interpreter doing the config... the result will be set on error.
      objv    - Command words.
      pObject - Pointer to the object being configured.
      startAt - which objv to start at.
   Returns:
     TCL_OK    - success.
     TCL_ERROR - failure.
*/
int
CDAQModuleCommand::Configure(CTCLInterpreter&         interp,
		std::vector<CTCLObject>& objv,
		CConfigurableObject*     pObject,
		unsigned                 startAt)
{
  try {
    for (int i = startAt; i < objv.size(); i += 2) {
      string key   = objv[i];
      string value = objv[i+1];
      pObject->configure(key, value);
    }
  }
  catch (string msg) {		// This may partially configure object... but what else can I do.
    Usage(msg, objv);
    return TCL_ERROR;
  }
  return TCL_OK;
}
