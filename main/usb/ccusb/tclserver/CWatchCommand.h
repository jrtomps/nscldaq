/*
     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef __CWATCHCOMMAND_H
#define __CWATCHCOMMAND_H


#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_SET
#include <set>
#ifndef __STL_SET
#define __STL_SET
#endif
#endif

#ifndef __TCL_H
#include <tcl.h>
#ifndef __TCL_H
#define __TCL_H
#endif
#endif

class TclServer;
class CTCLObject;
class CTCLInterpreter;


/**
 * Implements the 'watch' command. The watch command has the following
 * form:
 * \verbatim
 *    watch name
 * \endverabatim
 * 
 * Where name is the name of a tcl variable or array.   This causes
 * the array or variable to be monitored by the control server.
 * Whenever the run is active, changed variables are sent to the
 * Router as a buffer that is then turned into an string text list
 * ring item of type MONITORED_VARIABLES
 * 
 * If name is an array name, the entire array is monitored.
 * name can also be the name of an array element e.g. (somearray(someelement)).
 *
 * Note tracing single elements requires that the array already exist
 * although not the requested element.  This can be ensured within the script:
 *
 * \verbatim
 *      if {![info array exists monitorMe]} {
 *          array set monitorMe [list]
 *      }
 *      watch monitorMe(someElement)
 */
class CWatchCommand : public CTCLObjectProcessor
{
public:
  typedef std::pair<std::string, std::string> TCLVariableName; // Name+optional index.

private:
  static std::set<TCLVariableName>  m_ModifiedVariables; // Shared over all instances.
  static std::set<std::string>      m_WatchedVariables;	 // All watched variables.

public:
  CWatchCommand(CTCLInterpreter&   interp,
		TclServer&         server);
  virtual ~CWatchCommand();
private:
  CWatchCommand(const CWatchCommand& rhs);
  CWatchCommand& operator=(const CWatchCommand& rhs);
  int operator==(const CWatchCommand& rhs) const;
  int operator!=(const CWatchCommand& rhs) const;
  
  // Access to he modified variable names:
public:
  static std::vector<TCLVariableName> getModifications();
  static void clearModifications();
  static std::vector<TCLVariableName> getWatchedVariables(CTCLInterpreter& interp);


  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);

private:
  std::string usage() const;

  char*  logModification(CTCLInterpreter* pInterp, 
			 const char* name1, const char* name2, int flags);
  static char* traceRelay(ClientData object, Tcl_Interp* pInterp, 
		    const char* name1, const char* name2, int flags);


  // Special purpose visitor that is used to build up the result returned
  // by getWatchedVariables().

  class CVisitor {
  private:
    std::vector<TCLVariableName> m_variables;
    CTCLInterpreter&             m_interp;
  public:
    CVisitor(CTCLInterpreter& interp);
    void operator()(std::string varName);
    std::vector<TCLVariableName> get() const;
  };

};


#endif
