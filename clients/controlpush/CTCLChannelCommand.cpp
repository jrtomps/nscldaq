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
#include <CTCLChannelCommand.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "TCLVariable.h"
#include "CChannel.h"
#include "CTCLEpicsPackage.h"

#ifdef HAVE_STD_NAMESPACE 
using namespace std;
#endif

/*!
   Construct a channel command.
   \param interp  : CTCLInterpreter&
       The interpreter on which the command is registered.
   \param name    : std::string
       The name of both the channel and the command.
       Note that the name could also be the field-name of a device record.
       For example, to get the units (engineering units) of a device
       named 'george' you would ask for george.EGU.
*/
CTCLChannelCommand::CTCLChannelCommand(CTCLInterpreter& interp,
				       string          name) :
  CTCLObjectProcessor(interp, name),
  m_pChannel(0),
  m_pLinkedVar(0)
{
  m_pChannel = new CChannel(name);
  m_pChannel->Connect();
}
/*!
   Destroy a channel command and the associated channel resources:
*/
CTCLChannelCommand::~CTCLChannelCommand()
{

  delete m_pChannel;
  removeLinkage(this);
  delete m_pLinkedVar;
}

/*!
   Called when the command for the channel has been issued.  The channel command
   is an ensemble (see the class level comments in the header and the
   comments for each of the utility functions that implements the commands.
   \param interp : CTCLInterpreter&
      Reference to the interpreter that is executing the command.
   \param objv   : vector<CTCLObject>& 
      Reference to a vector of OO encapsulated Tcl_Object*s  that 
      are the command words.
*/
int
CTCLChannelCommand::operator()(CTCLInterpreter&    interp,
			       vector<CTCLObject>& objv)
{
  // Must have at least the subcommand of the ensemble:

  if (objv.size() < 2) {
    string result = objv[0];
    result       += " -- incorrect number of command parameters\n";
    result       += Usage();
    interp.setResult(result);

    return TCL_ERROR;
  }

  // Branch out based on the sub command:

  string subcommand = objv[1];
  if (subcommand == string("get")) {
    return Get(interp);
  }
  else if (subcommand == string("set")) {
    return Set(interp, objv);
  }
  else if (subcommand == string ("delete")) {
    return Delete(interp);
  }
  else if (subcommand == string("updatetime")) {
    return Updatetime(interp);
  }
  else if (subcommand == string("link")) {
    return Link(interp, objv);
  }
  else if (subcommand == string("unlink")) {
    return Unlink(interp);
  }
  else {
    string result = objv[0];
    result       += " ";
    result       += objv[1];
    result       += " -- Bad subcommand for channel command\n";
    result       += Usage();
    interp.setResult(result);

    return TCL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////
//////////////////// Utility functions //////////////////////////
/////////////////////////////////////////////////////////////////

/*!
   Get the value of a channel and return it in the result.
   Errors:
   - Channel is not connected at this time.
   Note that the CChannel class is structured so that all gets are a string
   which fits in well with Tcl's EIAS model.
   \param interp : CTCLInterpreter&
      Interpreter executing the command.
*/
int
CTCLChannelCommand::Get(CTCLInterpreter& interp)
{
  interp.setResult(m_pChannel->getValue()); // do this to force an update...

  if (!m_pChannel->isConnected()) {
    string result = m_pChannel->getName();
    result       += " is not connected at this time";
    interp.setResult(result);

    return TCL_ERROR;
  }

  return TCL_OK;
}
/*!
   Set the value of an epics channel.  The format of the set command is

   channelname set value ?type?

   where the optional type parameter is the data type (string, int, real)
   of the channel.  string in general will work. Errors include:
   - channel not connected
   - incorrect number of parameters.
   - The 'value' is not representabler in the desired type.
   \param interp : CTCLInterpreter&
      Reference to the interpreter that is running this command.
   \param objv : vector<CTCLObject>&
      Reference to the vector of OO Wrapped Tcl_Obj*s that make up the
      command.
*/
int
CTCLChannelCommand::Set(CTCLInterpreter&    interp, 
			vector<CTCLObject>& objv)
{
  string datatype("string");	// default data type.

  // check the number of parameters.

  if ((objv.size() < 3) || (objv.size() > 4)) {
    string result = objv[0];
    result       += " ";
    result       += objv[1];
    result       += " -- insufficient parameters\n";
    result       += Usage();
    interp.setResult(result);

    return TCL_ERROR;
  }

  // Get the value string and, if there is a datatype string update the
  // datatype variable contents:

  string value = objv[2];
  if (objv.size() == 4) {
    datatype = objv[3];
  }

  // The rest of this is inside a try block so that any conversion errors
  // are caught and turned into normal Tcl errors.

  try {
    if (datatype == string("string")) {
      (*m_pChannel) = value;
    }
    else if (datatype == string("int")) {
      objv[2].Bind(interp);
      int ivalue = objv[2];
      (*m_pChannel) = ivalue;
    }
    else if (datatype == string("real")) {
      objv[2].Bind(interp);
      double dvalue = objv[2];
      (*m_pChannel) = dvalue;
    }
    else {
      string result = objv[0];
      result += " ";
      result += objv[1];
      result += " ";
      result += objv[2];
      result += " ";
      result += datatype;
      result += " -- invalid data type\n";
      result += Usage();
      interp.setResult(result);

      return TCL_ERROR;
    }
  }
  catch (...) {
    string result = objv[0];
    result       += " ";
    result       += objv[1];
    result       += " ";
    result       += objv[2];
    result       += " ";
    result       += datatype;
    result       += " -- data does not convert to selecte data type\n";
    interp.setResult(result);

    return TCL_ERROR;

  }
  return TCL_OK;
}
/*!
   Get the time at which the channel was last updated.
   This will be the epoch if the channel has never been updated.
   This value is returned in a form suitable for use with [clock format]
   \param interp : CTCLInterpreter&
     References the interpreter on which this command is running

*/
int
CTCLChannelCommand::Updatetime(CTCLInterpreter& interp)
{
  time_t Time = m_pChannel->getLastUpdate();
  CTCLObject result;
  result.Bind(interp);
  result = (int)Time;
  interp.setResult(result);
  return TCL_OK;
  
}

/*!
  Delete the command.
*/
int
CTCLChannelCommand::Delete(CTCLInterpreter& interp)
{
  delete this;
  return TCL_OK;
}

/*!
   Link a variable to the channel.  This is sort of like many
   Tk widget's -textvariable option  The variable is assumed to
   specify a global variable.
*/
int
CTCLChannelCommand::Link(CTCLInterpreter& interp,
			 vector<CTCLObject>& objv)
{
  // Need a variable name:

  if (objv.size() != 3) {
    string result = getName();
    result       += " - insufficient parameters\n";
    result       += Usage();
    interp.setResult(result);

    return TCL_ERROR;
  }
  string tclVarName = objv[2];

  // If necessary get rid of the old one:

  if (m_pLinkedVar) {
    removeLinkage(this);
    delete m_pLinkedVar;
  }
  m_pLinkedVar = new CTCLVariable(&interp, tclVarName, kfFALSE);
  m_pLinkedVar->Set(m_pChannel->getValue().c_str(), TCL_GLOBAL_ONLY);
  addLinkage(this);
  
  
  return TCL_OK;
}

/*!
  Unlink a variable from the channel.
*/
int
CTCLChannelCommand::Unlink(CTCLInterpreter& interp)
{
  if (m_pLinkedVar) {
    removeLinkage(this);
    delete m_pLinkedVar;
  }
}

/*
  Provide command usage.
*/
string
CTCLChannelCommand::Usage()
{
  string result = "Usage: \n";
  result       += getName();
  result       += "  get\n";
  result       += getName();
  result       += " set value ?string|int|real\n";
  result       += getName();
  result       += " link tclVariableName\n";
  result       += getName();
  result       += " unlink\n";
  result       += getName();
  result       += " updatetime\n";
  result       += getName();
  result       += " delete\n";


  return result;
}


void
CTCLChannelCommand::UpdateLinkedVariable()
{
  if (m_pLinkedVar) {
    m_pLinkedVar->Set(m_pChannel->getValue().c_str(), TCL_GLOBAL_ONLY);
  }
}
