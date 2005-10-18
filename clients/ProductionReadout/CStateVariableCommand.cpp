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
//////////////////////////CStateVariableCommand.cpp file////////////////////////////////////
#include <config.h>
#include "CStateVariableCommand.h"                  
#include "CStateVariable.h"
#include <algorithm>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Local classes:

class ListMatches {             //!< Variables matching patterns -> result.
private:
  string m_Pattern;
  CTCLResult& m_rResult;
public:
  ListMatches(CTCLResult& rResult, const string& rPattern) :
    m_Pattern(rPattern), m_rResult(rResult) {}

  void operator()(pair<string, CStateVariable*> i) {
    if(Tcl_StringMatch(i.first.c_str(), m_Pattern.c_str())) {
      m_rResult.AppendElement(i.first);
    }
  }

};


class MatchStateVar {
private:
  string m_Name;
public:
  MatchStateVar(string name) :
    m_Name(name) {}
  bool operator()(StateVariableEntry e) {
    return e.first == m_Name;
  }
};

/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CStateVariableCommand  object;
   are performed.

   \note  pInterp in the base class is set to null with the knowledge
   that our creator will at some point do a CTCLProcessor::Bind() command.

*/
CStateVariableCommand::CStateVariableCommand () :
  CDAQTCLProcessor("statevar", 0)
{
}

/*!  
  Destructor.  Iterate through the state variable map, destroying each var.
*/
CStateVariableCommand::~CStateVariableCommand() 
{
  StateVariableIterator i = begin();
  while (i != end()) {
    delete i->second;
    i->second = (CStateVariable*)NULL;
    i++;
  }
}

// Functions for class CStateVariableCommand

/*!
    Provides the dispatching for the various subcommands
    of the statevariable core command.  This command
    has the following forms and handlers:
    - statevariable   name           Create()
    - statevaraible -delete name     Delete()
    - statevariable -list [pattern]  List()

	\param rInterp   - The interpreter running the command.
	\param rResult   - Result string returned to the interpreter.
	\param argc,argv - The command parameters. Note that argv[0] is
                           the command name all over again.
   \return TCL_OK    -  If the everything worked.
   \return TCL_ERROR -  On error.

\note - The return values for rResult will depend in general not only on 
        the 'subcommand' executed, but on whether or not it worked.

*/
int 
CStateVariableCommand::operator()(CTCLInterpreter& rInterp, 
				  CTCLResult& rResult, 
				  int argc, char** argv)  
{
  // The first parameter is just the command name so it's skipped.

  argv++; argc--;

  // There must be at least one parameter left.. this would
  // be either a subcommand keyword switch, or a new variable name.

  if(!argc) {
    Usage(rResult);
    return TCL_ERROR;
  }
  // Branch out to the appropriate executor function depending on
  // the value of the next paramter:

  int status;
  if(string("-delete") == *argv) {
    argc--; argv++;
    status = Delete(rInterp, rResult, argc, argv);
  } 
  else if (string("-list") == *argv) {
    argc--; argv++;
    status = List(rInterp, rResult, argc, argv);
  }
  else {
    status = Create(rInterp, rResult, argc, argv);
  }

  return status;
}  

/*!
    Creates a new run variable.  The optional parameters
    specify the name and the optional initial value for the
    variable.   The syntax of this command is:

    statevar name

    Since the name can be an array base name, there's no mechanism
    to provide an initial value.

    \param rInterp   - Interpreter on which the command is running.
    \param rResult   - Result string to fill in.
    \param argc,argv - Parameters following command name.

     \note - It is a successful no-op to create a statevar which already 
             exists.
     \note - On success, rResult will be empty, otherwise it will contain
             a descriptive error  message.  rResult can't be built to have
             the variable value because, once more, the variable could be
	     an array base name.
*/
int 
CStateVariableCommand::Create(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			      int  argc, char** argv)  
{
  // The caller has ensured that a variable name exists.

  if(argc > 1) {		// Too many parameters.
    Usage(rResult);
    return TCL_ERROR;
  }

  // Only do anything if the variable has not been created yet:

  Create(string(*argv));



  return TCL_OK;
}  

/*!
    Deletes an existing state variable. If the underlying TCL variables
    are still defined, they will remain defined, but won't figure
    in the production of StateVariableBuffers, nor will readonly-ness be
    enforced if the run is not halted.
    

    \param rInterp    - The interpreter which is running this command.
    \param rResult    - The result string to fill in.
    \param argc,argv  - The parameters following the -delete switch.

    \return TCL_OK - on success.
    \return TCL_ERROR - on failure.

    \note rResult will be empt on TCL_OK, but will contain a descriptive
          error message if TCL_ERROR is returned.
     \note it is an error to attempt to delete a nonexistent state variable.

*/
int 
CStateVariableCommand::Delete(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			      int  argc, char** argv)  
{


  // There must be only one parameter

  if(argc != 1) {
    Usage(rResult);
    return TCL_ERROR;
  }

  // And that paramteer is the name of the variable to delete:
  
  char* pName = *argv;

  StateVariableIterator i = find(string(pName));
  if ( i == end()) { // No such variable:
    rResult  =  "State Variable: ";
    rResult += pName;
    rResult += " Does not exist.";
    return TCL_ERROR;
  }
  Delete(i);

  return TCL_OK;

}  

/*!
    Lists the set of state variables which glob
    match the pattern.  This work is all done in the internal
    class: ListMatches which provides a function object to
    the STL for_each generic algorithm.

    Syntax:
 
    statevar -list pattern

    pattern can contain glob substitution special characters.

    \param rInterp   - Interpreter under which the command is running.
    \param rResult   - Result to be returned from the command.
    \param argc,argv - Parameters after the -list option.

    \note On success, rResult will contain a potentially empty Tcl
    formatted list of state variable names which match the pattern.
   
    \note  The default pattern value is *
*/
int 
CStateVariableCommand::List(CTCLInterpreter& rInterp, CTCLResult& rResult, int  argc, char** argv)  
{
  string pattern("*");
  
  // If there are parameters must be exactly 1:

  if(argc > 1) {
    Usage(rResult);
    return TCL_ERROR;
  }

  if(argc == 1) {
    pattern = *argv;
  }

  // Now we're ready to fire off the for_each:

  for_each(begin(), end(), ListMatches(rResult, pattern));

  return TCL_OK;

}  

/*!
    Returns a looping iterator which will,
    on repeated increments, traverse the set of
    state variables.



*/
StateVariableIterator 
CStateVariableCommand::begin()  
{
  return m_StateVariables.begin();
}  

/*!
    Returns an end of iteration sentinel iterator
    for the set of state variables being maintained.
    


*/
StateVariableIterator 
CStateVariableCommand::end()  
{
  return m_StateVariables.end();
}  

/*!
    Returns the number of variables which have been defined.

	\param 

*/
size_t 
CStateVariableCommand::size()  
{
  return m_StateVariables.size();
}

/*!
   Append usage information to the result string.
   */

void
CStateVariableCommand::Usage(CTCLResult& rResult)
{
  rResult += "Usage:\n";
  rResult += "   statevar newname\n";
  rResult += "   statevar -list ?globpattern?\n";
  rResult += "   statevar -delete name\n";

}
/*!
   Create a new state variable given its name.  This interface allows
   creating variable programmatically.  It is also used by the command
   interface. 

   \param rName const string& [in] - Name of the new variable.

   \note
    This is a NOOP if the name already exists.
   
    \return One of:
    - A pointer to the newly created state variable.
    - Null if the state variable already exists.

*/
CStateVariable*
CStateVariableCommand::Create(const string& rName)
{
  CStateVariable* pVar(0);
  if(find(rName) == end()) {
    pVar = new CStateVariable(getInterpreter(),rName);
    StateVariableEntry entry(rName, pVar);
    m_StateVariables.push_back(entry);
  }
  return pVar;
}
/*!
   Delete a state variable given its name.  This function is a no-op if the
   name does not exist.

   \param rName const string& [in] - Name of the variable to erase.

*/
void
CStateVariableCommand::Delete(const string& rName)
{
  StateVariableIterator i = find(rName);
  if(i != end()) {
    Delete(i);
  }
}
/*!
   Delete a state variable given an iterator that points to it.
   
   \param rIter StateVariableIterator[in] - Iterator that 'points' to
                the variable to delete.
*/
void
CStateVariableCommand::Delete(StateVariableIterator& rIter)
{
  CStateVariable* pVar = rIter->second;
  m_StateVariables.erase(rIter);
  delete(pVar);

}       
/*!
   Locate the state variable that corresponds to the specified name.
   
   \param rName const string& [in] Name of the variable to look for.

   \return iterator to the item, or end() if not found.
*/
StateVariableIterator
CStateVariableCommand::find(const string& rName)
{
  return find_if(begin(), end(), MatchStateVar(rName));

}
