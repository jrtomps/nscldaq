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
//////////////////////////CConstVariableCommand.cpp file////////////////////////////////////
#include <config.h>
#include "CConstVariableCommand.h"                  
#include "CConstVariable.h"
#include <algorithm>
#include <string>
#include <tcl.h>
#include <TCLString.h>
#include <TCLResult.h>
#include <TCLInterpreter.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
// static data and type definitions:

enum SwitchValue {	
  SW_LIST =0,
  SW_DELETE,
  SW_INVALID
};


// Local Classes:


/*!  Function object class that is a visitor to each variable
      in the const dictionary.  If the variable name
      matches a glob string it is added as a list element to 
      a CTCLString

      Note that since function objects are passed by value, the string must
      be built up in data external to the object, hence the reference to a
      CTCLString rather than the string itself.
*/
class CMatchConstCommand { 
private:
	CTCLString&   m_VariableList;      //!< List of vars built here.
	string        m_nMatchPattern;    //!< Pattern to match against.
public:
	CMatchConstCommand(const char* pPat, CTCLString& rString) :
	  m_VariableList(rString),
	  m_nMatchPattern(pPat)
	{}

		
	void operator()(pair<string, CConstVariable*> p)
	{
		if(Tcl_StringMatch(p.first.c_str(), m_nMatchPattern.c_str())) {
			m_VariableList.AppendElement(p.first.c_str());
		}
	}
};

/*!
   Constructor:  Create the command processor given a string
   command name and an interpreter pointer.

   \param rCommand - const string& [in] - Command keyword (defaults
                     to string("const").
   \param pInterp - CTCLInterpreter* [in] - Pointer to the
                    interpreter object on which this command will
                    be registered.  (defaults to 0).  If pInterp
                    is null, it is necessary to invoke the Bind()
		    member function to attach the object to an
		    interpreter after construction.
*/
CConstVariableCommand::
CConstVariableCommand(const string& rCommand,
		      CTCLInterpreter* pInterp) :
  CDAQTCLProcessor(rCommand, pInterp)
{}
/*!
   Constructor: Create the command processor givne a C string and a
   pointer to the interpreter.
   \param pCommand - const char* [in]
                       C String command keyword. (Defaults to 
                       "const").
   \param pInterp - CTCLInterpreter* [in]
                      Pointer to the TCL Interpreteron which the 
		      command will be installed.  (Defaults to 0).
		      If this value is null, the command must
		      be registerd on an interpreter later via the
		      Bind member function.
*/
CConstVariableCommand::
CConstVariableCommand(const char* pCommand, CTCLInterpreter* pInterp) :
  CDAQTCLProcessor(pCommand, pInterp)
{}

// Functions for class CConstVariableCommand

/*!
    Processes subcommands of the const command.
    The const command creates and manipulates Tcl constants.
    A Tcl constant is a 'variable' with a trace that prevents 
    modification.  The const allows c++ software to modify it
    so that initial values can be set and so that consts can also
    be used to represent status information that is internal and should
    not be script modifiable.

	\param CTCLInterpreter& rInterp, CTCLResult&  rResult, char** pArgs, int nArgs

*/
int 
CConstVariableCommand::operator()(CTCLInterpreter& rInterp, 
						   CTCLResult&  rResult, 
						   int nArgs, char** pArgs)  
{
	int status = TCL_OK;
	nArgs--; pArgs++;	//  The first parameter is the command.

	// There must be at least one parameter:
	
	if(!nArgs) {
		rResult   = "const command requires at least one parameter\n";
		rResult += Usage();
		return TCL_ERROR;
	}
	// Parse a switch and determine which function to dispatch to.

	vector<string> switches;
	switches.push_back("-list");
	switches.push_back("-delete");
	
	SwitchValue sw = (SwitchValue)MatchKeyword(switches, string(*pArgs),
									 (int)SW_INVALID);
									 
	switch(sw) {
	case SW_LIST:
		nArgs--;
		pArgs++;
		status = List(rInterp, rResult, pArgs, nArgs);
		break;
	case SW_DELETE:
		nArgs--;
		pArgs++;
		status = Delete(rInterp, rResult, pArgs, nArgs);
		break;
	case SW_INVALID:			// Create, pArg is name.
		status = Create(rInterp, rResult, pArgs, nArgs);
		break;
	default:                                            // Should never happen.
		rResult = "BUG Unexoected case in CConstVariableCommand::()";
		status = TCL_ERROR;
		break;
	}
	return status;
}  

/*!
    Creates a const.  This is in response to a command of the form
    const name ?value? as decoded by operator()
    
 
	\param Interp    - CTCLInterpreter& [in]
			   Reference to the interpreter on which this command is executing.
	 \param rResult  - CTCLResult& [out] 
			   Reference to the interpreter result string that we will fill in.
			   On normal completion, this will contain the initial value of the var
			   just like a \b set command.  If the command fails, this will 
			   contain the reason for the failure.
	 \param pArgs	char** [in]
			   The remaining parameters on the command line.  See nArgs
			   below for information about the meaning of the parameters.
			   Each parameter is a C string.
	 \param nArgs	int [in]
			   Number of remaining parameters on the command line. This
			  can be as many as 2 and as few as 1.  The first value will be the
			    name of the const to create and is mandatory.  The second,
			   optional, parameter is an initial value.  If the initial value is not
			   supplied, The const will be set to zero.

*/
int 
CConstVariableCommand::Create(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			      char** pArgs, int nArgs)  
{
   if(!nArgs) {				// Require at least one addtional param...
      rResult   = "const creation requires at least a variable name\n";
      rResult += Usage();
      return TCL_ERROR;
   }
   if(nArgs > 2) {			// but no more than the name, and value.
	rResult   = "const creation requires at most a name and value\n";
        rResult += Usage();
        return TCL_ERROR;
   }
   // Pull out the command variable name.
      
   char*       pValue   = "0";
   const char* pName    = *pArgs++;
   nArgs--;
   
   // If there's an additional parameter pull it out as the initial value.

   if(nArgs) {
      pValue = *pArgs++;
      nArgs;
   }
   // Check for name duplication, and return an error if the const already exists.
   ConstVariableIterator p = Find(string(pName));
   if(p != m_Dictionary.end()) {
      rResult  = "Attempt to create a duplicate const: ";
      rResult += pName;
      return TCL_ERROR;
   }
   
   // Create the variable and set its initial value and insert it into the dictionary so we can
   // keep track of it:
   
   m_Dictionary[pName] = new CConstVariable(&rInterp,
					     string(pName),
					     string(pValue));
 
   // Return the result string and TCL_OK.
   
   rResult = pValue;
   return TCL_OK;
}  

/*!
    Called in response to a tcl command of the form:
    
    \verbatim
    const -delete name
    \endverbatim
    
    Where name is the name of a const to destroy.  The
    variable is unset and the variable object destroyed.
    

	\param rInterp - CTCLInterpreter& [in] 
	                the interpreter that is running the
			command.
	\param rResult - CTCLResult& [out]
	                The result string that will be returned
			to the caller by this command.  If the
			return from this function is \b TCL_OK 
			the result string will be empty.  If the
			value returned is \b TCL_ERROR the result
			string will describe while the command failed
			result string values can be:
			- No such Variable.

        \param pArgs  - char** [in] 
	                The remaining parameters on the
	                command line.  In this case, the remaining
			characters should be the name of a variable
			to delete.
        \param nArgs  - int [in]
	                The number of parameters remaining on the
			command line.
     \return
         An int value that is one of:

	 - \b TCL_OK    - If the function worked.
	 - \b TCL_ERROR - If the function failed for some reason.
   
*/
int 
CConstVariableCommand::Delete(CTCLInterpreter& rInterp, 
			      CTCLResult& rResult,
			       char** pArgs, int nArg)
{
  int nStatus(TCL_OK);    
  if(nArg != 1) {		// Check for valid usage.
	rResult = "const deletion requires a const name\n";
        rResult += Usage();
        nStatus  = TCL_ERROR;
  }
  else {			// The single parameter is the name
    string Name(*pArgs);	// of the const to delete.
    ConstVariableIterator i = Find(Name);
    if(i != end()) {		// Found it so we can delete it.
	CConstVariable *pVar = (i->second);
        m_Dictionary.erase(i);
       delete pVar;
        Tcl_UnsetVar(rInterp.getInterpreter(), (char*)Name.c_str(),
			   0);		// can define consts in procs.
    }
    else {			// Not found, error:
	rResult += "Attempt to delete nonexistent const: ";
        rResult += Name;
        nStatus = TCL_ERROR;
   }
 }
  return nStatus;
}  

/*!
    Called in response to a tcl command of the
    form:
    \verbatim
    const -list ?pattern?
    \endverbatim

    The set of constant variable names matching the optional 
    glob pattern is returned as the result string.  If no
    pattern is supplied, all names are printed out (the pattern
    is *).

    \param rInterp - CTCLInterpreter& [in] 
                    The interpreter on which
                    the command is executing.
    \param rResult - CTCLResult& [out] 
                    The result string (see  the
                    function description above for more information.
    \param pArgs  - char** [in] 
                    Pointers to the remaining parameters on the 
		    command line. This can either be a pattern
		    or nothing.
    \param nArgs  - Number of parameters on the line.

    \return Any of:
    - TCL_OK    - Everything worked out.
    - TCL_ERROR - If there was an error to report.  


*/
int 
CConstVariableCommand::List(CTCLInterpreter& rInterp,
			    CTCLResult&      rResult,
			    char** pArgs,
			    int    nArgs)  
{
  int nStatus(TCL_OK);
  if(nArgs > 1) {		// Too many args:
	rResult   = "const list requires at most a pattern\n";
	rResult += Usage();
	nStatus = TCL_ERROR;
  } 
  else {			// If no args, pattern is *:
	char* pPattern="*";
	if(nArgs) pPattern = *pArgs;
     
	// Use for_each to build up the list...
	
	CTCLString Matches;
	CMatchConstCommand lister(pPattern, Matches);
	for_each(begin(), end(), lister);
	rResult += (const char*)Matches;
    
  }
  return nStatus;
}  

/*!
   \return
    Returns an iterator to the first element of the const dictionary.  Dereferencing the iterator produces a
    pair<string, CConstVarialble*> since the current implementation stores the dictionary in a map.
    
    \note If the internal representation of the dictionary changes, we'll need to design a class that wraps
            an iterator with a pair that is pointer-like to keep this interface fixed.
	
    
*/
CConstVariableCommand::ConstVariableIterator 
CConstVariableCommand::begin()  
{
	 return m_Dictionary.begin();
}  

/*!
    \return 
	    Returns an interator past the last element of the const dictionary. Dereferencing the iterator produces a
    pair<string, CConstVarialble*> since the current implementation stores the dictionary in a map.
    
    \note If the internal representation of the dictionary changes, we'll need to design a class that wraps
            an iterator with a pair that is pointer-like to keep this interface fixed.

*/
CConstVariableCommand::ConstVariableIterator 
CConstVariableCommand::end()  
{
	 return m_Dictionary.end();
}  

/*!
    \return  The number of elements in the const dictionary.

*/
int 
CConstVariableCommand::size()  
{
	 return m_Dictionary.size();
}

/*!
    Locates a const variable by name in the const dictionary.  Dereferencing the iterator returned retrieves a
    pair<string, CConstVariable*>.
    
    \return Any of:
    -   An iterator that dereferences a pair where the second element is a matching const.
    -  end() if the item is not found.
    
    \note At present, this exposes the map find interface.  If the internal representation is changed, this wll
    have to be recast as a generic algorithm and the return value redefined as a wrapper arount both an iterator 
    and a dynamically built pair.
*/
CConstVariableCommand::ConstVariableIterator
CConstVariableCommand::Find(const string& rName)
{
	return m_Dictionary.find(rName);
}

/*!
    \return
    Returns a string that contains usage inormation about the const command.  This is a protected utility member.
    
*/
string 
CConstVariableCommand::Usage()
{
	string usage =  "Usage:\n";
	usage         +="    const name ?value?\n";
	usage         +="    const -list   ?pattern?\n";
	usage         +="    const -delete name\n";
	return usage;
}

