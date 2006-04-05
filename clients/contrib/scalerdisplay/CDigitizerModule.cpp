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


////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CDigitizerModule.h"    				
#include <assert.h>
#include <TCLResult.h>
#include "CIntConfigParam.h"
#include "CIntArrayParam.h"
#include "CBoolConfigParam.h"
#include "CStringConfigParam.h"
#include "CStringArrayparam.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Constructor, creates a digitizer module.
   The configuration parameters can take care
   of themselves.  The string gets initialized
   from the parameter.
   \param sName  const string& [in]
          The name of the module.
*/
CDigitizerModule::CDigitizerModule (const string& rString,
                    CTCLInterpreter& rInterp)
   : m_sName(rString),
     CTCLProcessor(rString.c_str(), &rInterp)
{
  Register();
}
/*!
  Destructor.  The configuration parameters pointed to by
  the elements of the list are assumed to be
  dynamically allocated.  Therefore they must be deleted.
  The list elements themselves are assumed taken care of by
  the list destructors.
*/
CDigitizerModule::~CDigitizerModule ( )  //Destructor - Delete dynamic objects
{
  DeleteParameters();
}



// Functions for class CDigitizerModule

/*!  Function: 	
    

Processes the module's command.  The default
implementation is to look for matches of the
pArgs[1] with:
- "config" calls the module's Configure member. member function.
- "cget"    calls the module's ListConfiguration member function.
- "help"    calls the module's Usage member function.

\param rInterp CTCLInterpreter& [in] 
            Interpreter running the command.
\param rResult CTCLResult& [in]
            The result string that will be returned to the
            caller.
\param nArgs int [in]  The number of parameters on the
            command line.  Note that the first one should
            be m_sName.
\param pArgs char** [in] The command parameters.

\return  Either of:
  - TCL_OK  if the command completed properly or
  - TCL_ERROR if the command failed.
  
  \note
      To extend functionaly to support additional keyword,
      override this, check for your own keywords and if you 
      don't find them, call the base class member with
      the unaltered parameters.
*/
int 
CDigitizerModule::operator()(CTCLInterpreter& rInterp, 
                            CTCLResult& rResult, 
                            int nArgs, char** pArgs)  
{ 
  int nStatus = TCL_OK;
  assert(m_sName == string(*pArgs));
  nArgs--;
  pArgs++;
  
  if(nArgs) {
    // Match the command keyword against the ones we understand:
    // config, cget or help
    //
    string sCommand(*pArgs);
    pArgs++; 
    nArgs--;
    if(sCommand == string("config")) {
      nStatus = Configure(rInterp, rResult, nArgs, pArgs);
    }
    else if(sCommand == string("cget")) {
      nStatus = ListConfiguration(rInterp, rResult, nArgs, pArgs);
    }
    else if(sCommand == string("help")) {
      rResult = Usage();
    }
    else {                    // NO command match.
      rResult = m_sName;
      rResult += " unrecognized subcommand \n";
      rResult += Usage();
      nStatus  = TCL_ERROR;
      
    }
  }
  else {
    rResult  = " Insufficient parameters:\n";
    rResult += Usage();
    nStatus  = TCL_ERROR;
  }

  return nStatus;
}  

/*!  Function: 	
   int Configure(CTCLInterpreter& rInterp, CTCLResult& rResult, int nArgs, char** pArgs) 
 Operation Type:
    
Purpose: 	

Handles the "configure" command.  The default 
implementation is to assume configuration is
a set of keyword value pairs. e.g:
- -threshold 5
- -pedestals {1 2, 3 4 5 6 7 8 9}
- -subtraction enable

Each keyword is matched against the parameters in the
following order.  Naturally duplicate command keys are not a
good thing.
- m_IntParameters keys that accept single integer parameters
- m_ArrayParameters keys that accept a fixed size list of integers
- m_BoolParameters keys that accept a boolean flag.

See CConfigurationParameter CIntConfigParam CIntArrayParam CBoolConfigParam
As many configuration options as can be performed get done.
Any failures are reported by returning TCL_ERROR and placing
stuff in the results string.

\param rInterp CTCLInterpreter& [in] Interpreter running the
            command.
\param rResult CTCLResult [in] Result string that is filled in
            either by us or by the configuration dudes.
\param nArgs int [in] Count of remaining parameters. the first
      one should be the first configuration keyword.
\param pArgs char** [in] The text of the parameters.
*/
int 
CDigitizerModule::Configure(CTCLInterpreter& rInterp, 
                            CTCLResult& rResult, 
                            int nArgs, char** pArgs)  
{
  int nStatus = TCL_OK;
  while (nArgs) {
    if(nArgs < 2) {
      rResult += " Keyword without parameters.";
      return TCL_ERROR;
    }
    string   Keyword(*pArgs);     // Extract the command
    string   Parameter(pArgs[1]);   // and its keywords.
    nArgs -= 2;
    pArgs += 2;
    
    ParameterIterator pParam = Find(Keyword);
    if(pParam != m_Configuration.end()) {
      CConfigurationParameter* p = *pParam;
      (*p)(rInterp, rResult, Parameter.c_str());
    }
    else {
      nStatus = TCL_ERROR;
      rResult += "Unrecognized config keyword/param pair\n";
      rResult += " Keyword: ";
      rResult += Keyword;
      rResult += " Parameter: ";
      rResult += Parameter;
      rResult += "\n";
    }
  }
  return nStatus;
}  

/*! 

Lists the current module configuration.  The default implementation
iterates through the set of m_IntParameters, m_ArrayParameters and 
m_BoolParameters producing pairs of {parametername values} such as:

- {threshold 5}
- {pedestals {1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 }}
- {subtraction on}

\note
   In the list, no distinction is made between the types of
    parameter values.  
\note
    The string representationi is used, so the value of the
    bool will not be standardized, but will be whatever was
    used (e.g. on, 1 are both possible values.
    
  \param rInterp CTCLInterpreter& [in] The interpreter that
            is running the command.
  \param rResult CTCLResult& [in] The result string that
            will contain the configuration information.
  \param nArgs int [in]  Number of parameters, should be 0.
  \param pArgs char** [in] text of parameters.
  
  \note  There should be at most 1 parameter. If present it
    is a glob pattern that will be used to filter the output.
    (essentially no parameter results in a pattern of *).
  
  \return Any of:
    -  TCL_OK the configuraton was listed 
    -  TCL_ERROR the configuration could not be listed and
            rResult is an error string.
*/
int 
CDigitizerModule::ListConfiguration(CTCLInterpreter& rInterp,
                                    CTCLResult& rResult, 
                                    int nArgs, char** pArgs)  
{ 
  int nStatus = TCL_OK;
  if(nArgs > 1) {
    rResult += " Too many parameters in cget\n";
    rResult += Usage();
    nStatus = TCL_ERROR;
  }
  else {
    string sPattern = "*";     // Assume no parameters.
    if(nArgs) {
      sPattern = *pArgs;
    }
    string listing = ListParameters(rInterp, sPattern);
    rResult        = listing;
  }
  return nStatus;
} 

/*!

Returns a string describing the command usage.  
The defafult implementation produces a
string of the form:
\verbatim

m_sName config Paramdescription
m_sName cget
m_sName help

\endverbatim

Paramdescription is produced by iterating through the set of
configuration parameter descriptions and for each of them listing
the name and the type expected e.g.:
- theshold int
- pedestal {int[16]}
- subtraction on|off|enable|disable

*/
string 
CDigitizerModule::Usage()  
{
  string help;
  help  = m_sName;
  help += " config ";
  help += ListKeywords();
  help += "\n";
  help += m_sName;
  help += " cget ?pattern?\n";
  help += m_sName;
  help += " help";
  
  return help;
}  

/*!

Adds an integer configuration parameter to the
set recognized by the default configuration parser.

 \param sParamName const string& [in]
      Name of parameter to add.
 \param nDefault int [in] = 0
      The default value of the parameter.
*/
void 
CDigitizerModule::AddIntParam(const string& sParamName,
                              int nDefault)  
{ 
  CIntConfigParam* pNewParam = new CIntConfigParam(sParamName,
                                                   nDefault);
  m_Configuration.push_back(pNewParam);
}  

/*! 

Adds an array of parameters to the set of
configuration parameter that are parsed by the default
Configure function.

  \param rParamName const string& [in]
        Name of the parameter to add.
  \param nArraySize int [in]
        Size of the parameter array expected.
  \param nDefault int [in] = 0
        Default value of the array elements.
*/
void 
CDigitizerModule::AddIntArrayParam(const string& rParamName, 
                                   int nArraySize,
                                   int nDefault)  
{ 
  CIntArrayParam* pNew = new CIntArrayParam(rParamName,
                                            nArraySize,
                                            nDefault);
  m_Configuration.push_back(pNew);
}  

/*!  Function: 	

Adds a boolean parameter to the set of
parameters that are recognized by the
default Configure parser.

  \param rName const string& [in]
      Name of the parameter to create.
  \param fDefault bool [in] = false
      Defaults value of the parameter.
*/
void 
CDigitizerModule::AddBoolParam(const string& rName, 
                               bool fDefault)  
{ 
  CBoolConfigParam* pParam = new CBoolConfigParam(rName, 
                                                  fDefault);
  m_Configuration.push_back(pParam);
}

/*!
   Delete the parameter arrays.
*/
void
CDigitizerModule::DeleteParameters()
{
    // Kill off the bool parameters.
  
  ParameterIterator p = m_Configuration.begin();
    while (p != m_Configuration.end()) {
      CConfigurationParameter *pParam = *p;
      delete pParam;
      p++;
  }
}
/*!
    Locate a parameter matching the configuration parameter
  test string.
  \param rKeyword The keyword to check.
*/
CDigitizerModule::ParameterIterator
CDigitizerModule::Find(const string& rKeyword)
{
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    CConfigurationParameter* pParam = *p;
    if(pParam->Match(rKeyword)) { 
      return p;
    }
    p++;
  }
  return m_Configuration.end();
}
/*!
  Produces a list of the configuration parameters that match 
the input pattern.  
\param rInterp CTCLInterpreter& [in]
      Interpreter excuting this 
\param sPattern const string& [in]
      The glob string pattern.
*/

string
CDigitizerModule::ListParameters(CTCLInterpreter& rInterp,
                               const string& rPattern)
{
  CTCLString result;
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    if(Tcl_StringMatch( ((*p)->getSwitch().c_str()),
                       rPattern.c_str())) {
      result.StartSublist();
      result.AppendElement((*p)->getSwitch());
      result.AppendElement((*p)->getOptionString());
      result.EndSublist();
    }
    p++;
  }
  return string((const char*)result);
}
/*!
    List the allowed configuration keywords.  The words
    are returned as  a string of pairs.  The pairs are 
    \em not a bracketed list, but just a pair of words.
    the first word of each pair is the command keyword. 
    the second word is the parameter format as returned
    from CConfigurationParameter::GetParameterFormat()

  \return string - the results of the list.
*/
string
CDigitizerModule::ListKeywords()
{
  string result;
  ParameterIterator p = m_Configuration.begin();
  while(p != m_Configuration.end()) {
    result += (*p)->getSwitch();
    result += " ";
    result += (*p)->GetParameterFormat();
    result += " ";
    
    p++;
  }
  return result;
}

/*!
   Adds a string parameter to the set of parameters
   recognized by this module.  A string parameter
   is a parameter with a single string valued value.
  
   \param rName (const string& [in]):
      Name of the new parameter.
*/
void
CDigitizerModule::AddStringParam(const string& rName)
{
  CStringConfigParam *p = new CStringConfigParam(rName);
  m_Configuration.push_back(p);

}
/*!
  Adds a string array parameter to the set of parameters
  recognized by this module. A string array parameter has
  a parameter that is a tcl formatted list where each list element
  is an arbitrary string.
  \param rName (const string & [in]):
     The name of the configuration parameter.
  \param nArrayAzie (int [in]):
     The number of elements in the array.
*/
void
CDigitizerModule::AddStringArrayParam(const string& rName,
				     int nArraySize)
{
  CStringArrayparam* p = new CStringArrayparam(rName, nArraySize);
  m_Configuration.push_back(p);
}
