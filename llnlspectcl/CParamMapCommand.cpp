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
#include "CParamMapCommand.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <Exception.h>

#include <tcl.h>
#include <SpecTcl.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// static data declarations:

CParamMapCommand::ParameterMap  CParamMapCommand::m_theMap;

/*!
   Create the command.. All the real work is done by the base class
   constructor which registers us on the interpreter.
*/
CParamMapCommand::CParamMapCommand(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "paramMap")
{}
/*!
  Destruction is also taken care of by the base class:

*/
CParamMapCommand::~CParamMapCommand()
{}

/*!
  Return a const reference to the parameter map so that the
  unpacker can figure out mappings.
*/
const CParamMapCommand::ParameterMap&
CParamMapCommand::getMap()
{
  return m_theMap;
}
/*!
   Process the paramMap command.
*/
int
CParamMapCommand::operator()(CTCLInterpreter& interp,
			     vector<CTCLObject>& objv)
{
  // Need to have 4 command words:

  if (objv.size() != 4) {
    string error = "paramMap - incorrect parameter word count\n";
    error       += Usage();
    setResult(interp, error);
    return TCL_ERROR;
  }

  // Now get the following items:
  //    - slot (integer)
  //    - channel (integer)
  //    - name (string)
  //
  // failure to do these conversions is an exception.
  // note that the objects are not bound to an interpreter yet so we must bind them.

  objv[1].Bind(interp);
  objv[2].Bind(interp);
  objv[3].Bind(interp);


  int    slot;
  int    channel;
  string name;
  try {
    slot    = objv[1];
    channel = objv[2];
    name    = (string)(objv[3]);
  }
  catch (CException& reason) {
    string error = "Invalid parameter: ";
    error += reason.ReasonText();
    error += " while ";
    error += reason.WasDoing();
    error += "\n";
    error += Usage();
    setResult(interp, error);
    return TCL_ERROR;
  }
  catch (...) {
    string error = "Invalid parameter\n";
    error += Usage();
    setResult(interp, error);
    return TCL_ERROR;
  }

  // The parameter name must translate to a SpecTcl parameter.
  
  SpecTcl* pApi = SpecTcl::getInstance();
  CParameter* pParam = pApi->FindParameter(name);
  if (!pParam) {
    string error = "Parameter: ";
    error   += name;
    error   += " has not been defined yet and must be\n";
    error   += Usage();
    setResult(interp, error);
    return TCL_ERROR;
  }

  //  Now make the entry:

  createEntry(slot);
  m_theMap[slot][channel] = pParam->getNumber();
  setResult(interp, string(""));
  return TCL_OK;
}

/* 
  Create a map entry.  If  the map entry already exists, this is a no-op.
  if not we create enough 'empty' map entries to ensure that it does exist.
  An empty map entry is one where all elements are -1.

*/
void
CParamMapCommand::createEntry(unsigned slot)
{
  while (m_theMap.size() < slot+1) { // until m_theMap[slot] exists...
    AdcMapping item;
    m_theMap.push_back(item);	// Pushing a copy of this empty item.
  }
}

/*
    Return a usage string.
*/
string
CParamMapCommand::Usage() 
{
  string result = "Usage: \n";
  result       += "   paramMap slot channel name\n";
  result       += "      slot    - An ADC Virtual slot number\n";
  result       += "      channel - A channel number within the module\n";
  result       += "      name    - name of SpecTcl parameter to stuff data into\n";

  return result;
}
/*
   Set the interpreter result from a string.
*/
void
CParamMapCommand::setResult(CTCLInterpreter& interp, string result)
{
  Tcl_Obj* objr = Tcl_NewStringObj(result.c_str(), -1);
  Tcl_SetObjResult(interp.getInterpreter(), objr);
}
