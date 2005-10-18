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


static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CRunVariableCommand.cpp file////////////////////////////////////

#include <config.h>
#include "CRunVariableCommand.h"  
#include "CRunVariable.h"

#include <TCLProcessor.h>

#include <assert.h>
#include <string>
#include <algorithm>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Local classes:


class ListMatches {		//!< Variables matching patterns -> result.
private:
  string m_Pattern;
  CTCLResult& m_rResult;
public:
  ListMatches(CTCLResult& rResult, const string& rPattern) :
    m_Pattern(rPattern), m_rResult(rResult) {}

  void operator()(pair<string, CRunVariable*> i) {
    if(Tcl_StringMatch(i.first.c_str(), m_Pattern.c_str())) {
      m_rResult.AppendElement(i.first);
    }
  }

};

// Predicate for CRunVariableCommand::find

class MatchRunVar {
private:
  string m_Name;
public:
  MatchRunVar(const string& rName) :
    m_Name(rName)
  {}
  bool operator()(RunVariableListEntry& e) {
    return e.first == m_Name;
  }
};


/*!
  Construct the command object.  The object is bound to no interprater
  the command name, which will eventually be registered is:

  runvar


*/
CRunVariableCommand::CRunVariableCommand () :
  CDAQTCLProcessor("runvar", 0)
 
{

} 

/*!
   Destructor... we need to delete all the items pointed to:
   */
CRunVariableCommand::~CRunVariableCommand()
{
  RunVariableIterator i = begin();
  while(i != end()) {
    delete i->second;		// Delete the variable object.
    i->second = (CRunVariable*)NULL;
    i++;
  }
  // m_RunVariables destructor will later empty the map.
}
// Functions for class CRunVariableCommand

/*!
    Executes the runvariable command.  This command has
    the following subcommands:
    - add Create a new run variable.
      syntax: runvar newname ?initialvalue?
    - list    List the existing run variables.
      syntax: runvar -list ?pattern?
    - delete Delete a run variable.
      syntax: runvar -delete name
     
    A run variable is simply a Tcl/Tk variable which
    is put in the RunVariable registry.  A run variable can
    be changed at any time.  A run variable can also be
    a state variable.  Run Variables are periodically written
    to buffers during a run.
    

	\param rInterp  - Refers to the interpreter which is running this 
	                  command
	\param rResult  - Refers to the result variable into which the
	                  command result is put.  The value of this
			  will depend on the subcommand executed.
	\param argc,argv - Parameter count and array.


	\return  TCL_OK    - If success else
	\return  TCL_ERROR - If there was a problem with rResult having
	                     a reason message.

    \note - The variable may already exist or not as a Tcl Variable.
            If the variable is new, and no initial value is supplied,
	    The variable will be initialized to the value "-uninitialized-"
	   

    \note - Each subcommand is farmed out it its own execution function:
    - Create - handles variable creation.
    - Delete - handles variable deletion.
    - List   - handles listing variables.
*/
int 
CRunVariableCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult,
				int  argc, char** argv)  
{
  // The first parameter is just the command name:

  argv++; argc--;

  // There must be at least one more parameter:

  if(argc <=0 ) {
    Usage(rResult);
    return TCL_ERROR;
  }

  // Branch out based on the first parameter:

  int status;

  if(string("-list") == *argv) { // -list switch
    argv++; argc--;
    status = List(rInterp, rResult, argc, argv);
  }
  else if (string("-delete") == *argv) {	// -delete switch.
    argv++; argc--;
    status = Delete(rInterp, rResult, argc, argv);
  }
  else {			// Create operation.
    status = Create(rInterp, rResult, argc, argv);
  }

  return status;
}  

/*!
    Dispatched to from the operator()
    member function.  This member function
    has the form:
    
    runvar name [value]
    
    If the variable does not exist it is created,
    and, if no initializer is specified, is set to "-uninitialized-"
    If the variable exists and no initializer is specified,
    the value is not changed.
    

	\param rInterp   - Reference to the interpreter object which is running
	                   this command.
	\param rResult   - Reference to the result string which will be filled
	                   in either with the value of the variable if 
			   success or contain an error message on failure.
	\param argc,argv - Parameters after the runvar command.

    \return TCL_OK    - If success.
    \return TCL_ERROR - If failure

    \note - It is a successful no-op to attempt to create a runvar which
            already exists.
    \note - There's no mechanism to create an array, however individual
            elements can be created; e.g. runvar george(harry) 1234

*/
int 
CRunVariableCommand::Create(CTCLInterpreter& rInterp, CTCLResult& rResult, int argc, char** argv)  
{

  if(!argc) {			// Too few parameters.
    Usage(rResult);
    return TCL_ERROR;
  }
  if(argc > 2) {		// Too many parameters.
    Usage(rResult);
    return TCL_ERROR;
  }

  // 1 or 2 is just right.

  string vName = *argv;
  argv++; argc--;

  if(find(vName) == end()) { // New variable.. .must create.
    CRunVariable* pVar = new CRunVariable(&rInterp, vName);
    RunVariableListEntry entry(vName, pVar);
    m_RunVariables.push_back(entry);
  }
  else {
    rResult = vName;
    rResult += " already exists";
    return TCL_ERROR;
  }
  // At this point we're gaurenteed there's a variable of the name 
  // requested in the list.
  // If there is an additional parameter, it's the initial value of the
  // variable:

  RunVariableIterator i = find(vName);
  assert(i != end());

  //  If there' an initial value  set the variable:
  if(argc) {
    i->second->Set(*argv, TCL_GLOBAL_ONLY);
  }

  rResult = i->second->Get(TCL_GLOBAL_ONLY);

  return TCL_OK;
  
}  

/*!
    Removes  a run variable from the list
    of run variables.  The variable continues
    to be a Tcl/Tk variable;  however it is no longer
    written to buffers when the run is active.  
    The form of this command is:
    runvariable -delete name

	\param rInterp,  - Refers to the interpreter running this command.
	\param rResult   - Refers to the result string.  On success this will
	                  be empty, otherwise, an instructive error message.
	\param argc,argv - Command parameters following the -delete switch.

    \return TCL_OK - Everything worked and the result string is empty.
    \return TCL_ERROR - something failed and the result string tells you what.

    \note it is not possible to directly delete entire arrays, however you
         can delete the individual elements in foreach script where the
         list comes from foreach item [runvar -list arrayname(*)] ...
    
*/
int 
CRunVariableCommand::Delete(CTCLInterpreter& rInterp, CTCLResult& rResult,
			    int argc, char** argv)  
{
  if(argc != 1) {
    Usage(rResult);
    return TCL_ERROR;
  }

  // The only remaining parameter is the name of the var to delete.
  //
  RunVariableIterator i = find(string(*argv));
  if(i != end()) {		// Matched one...
    Delete(i);
    return TCL_OK;
  }
  else {			// No match ... error:
    rResult  = *argv;
    rResult += " is not a runvariable";
    return TCL_ERROR;
  }
}  

/*!

  Lists the set of run variables which match a patter which can contain
  glob wildcard specials.  This is sort of like info var but for the set
  of variables which are run variables.

  \param rInterp - the interpreter running this command.
  \param rResult - The result string.  If all goes well, this will be a
                   TCL List containing the names of variables which match
		   the pattern or empty if none do.  If there's a problem,
		   this will contain an informative error message.
  \param argc,argv - The remaining parameters after the -list switch.

  \return  TCL_OK - Everything worked and rResult contains the
                      (possibly empty) list of matching patterns).
  \return  TCL_ERROR - if there was an error parsing the rest of the command.
                        (not really possible).

  \note This function makes use of the STL for_each visitor function to
        visit each variable in the list and match it against the pattern.
        
   \note If no pattern is supplied by the user (argc == 0), the default
         pattern "*" will be used.

   \note If the variable corresponding to a run variable was unset, it will
         still be listed.. it is therefore a bad idea to unset a run variable
	 without also deleting it.



*/
int 
CRunVariableCommand::List(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			  int argc, char** argv)  
{
  string pattern("*");		// Default pattern is exhaustive list.
  if(argc) {
    pattern = *argv;
    argc--; argv++;
    if(argc) {			// Too many parameters..
      rResult = "Too many parameters: \n";
      Usage(rResult);
      return TCL_ERROR;
    }
  }

  // Ready to do the match:

  for_each(begin(), end(), ListMatches(rResult, pattern));
  
  return TCL_OK;
}  

/*!
    Returns an iterator to the list of variables
    which are considered to be run variables.

	\param 

*/
RunVariableIterator 
CRunVariableCommand::begin()  
{
  return m_RunVariables.begin();
}  

/*!
    Returns an end loop interator for the
    set of run variables.

	\param 

*/
RunVariableIterator 
CRunVariableCommand::end()  
{
  return m_RunVariables.end();
}  

/*!
    Returns the number of run variables currently 
    defined.

	\param 

*/
size_t 
CRunVariableCommand::size()  
{
  return m_RunVariables.size();
}
/*!
   Appends usage information to the rResult string.
   */
void
CRunVariableCommand::Usage(CTCLResult& rResult)
{
  rResult += "Usage:\n";
  rResult += "   runvar newvar ?value?\n";
  rResult += "   runvar -list  glob-pattern\n";
  rResult += "   runvar -delete varname\n";


}
/*!
   Create a run variable programmatically
  
   \param rName - const string& [in] - Name of the variable.
   \param rInitialValue const string& [in] - Initial value of the variable.

   \note If the variable rName already exists, this function is a no-op (for
   now) future implementations reserve the right to throw an exception for 
   duplicate object.

   \return  One of:
   - a pointer to the new variable on success.
   - NULL if the variable could not be created (e.g. one with that name
   already exists.
*/
CRunVariable*
CRunVariableCommand::Create(const string& rName,
			    const string& rInitialValue)
{
  CRunVariable* pVar(0);
  if(find(rName) == end()) { // Doesn't exist so we can create:
    string Name(rName);
    pVar    = new CRunVariable(getInterpreter(), Name);
    RunVariableListEntry entry(Name, pVar);
    m_RunVariables.push_back(entry);
    pVar->Set(rInitialValue.c_str());
  }
  return pVar;

}
/*!
   Delete (programmatically) a run variable given its name.
   \param rName - const string& [in] the name of the variable to delete.
   
   \note If rName does not exist, this function currently is a no-op, future
   implementations reserve the right to throw an exception derived from
   CException.

*/
void
CRunVariableCommand::Delete(const string& rName)
{
  RunVariableIterator i = find(rName);
  if(i != end()) {
    delete i->second;
    i->second = (CRunVariable*)NULL;
    m_RunVariables.erase(i);
  }
}
/*!
   Delete programmatically a run variable given an iterator to it:
   \param rIter -  RunVariableIterator& iterator ``Pointing'' to the
   variable object in the map.
*/
void
CRunVariableCommand::Delete(RunVariableIterator& rIter)
{
  delete rIter->second;
  rIter->second = (CRunVariable*)NULL;
  m_RunVariables.erase(rIter);
}
/*!
    Locate an element by name in the run variable list.
    \param string   -  Name of the variable.
    \return iterator
    \retval end  - if not found.
*/
RunVariableIterator
CRunVariableCommand::find(const string& rName) 
{
  return find_if(begin(), end(), MatchRunVar(rName));
}
