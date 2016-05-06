/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
  
/*! \class CConfigurationParameter  abstract 
           This is the base class for all configuration parameter parsers.
           Configuration parameter parsers accept pair of 
           - keyword int
           - keyword arrays of ints.
           - keyword bool
           
    Author: Ron Fox
            Ron's DAQ software.
            (c) 2002, All rights reserved.
    Revision History:
      $Log$
      Revision 8.2  2005/06/24 11:33:24  ron-fox
      Bring the entire world onto the 8.2 line

      Revision 4.2  2004/11/23 15:01:00  ron-fox
      Port to gcc/g++ 3.x

      Revision 4.1  2004/11/08 17:37:56  ron-fox
      bring to mainline

      Revision 1.5  2004/11/08 17:31:43  ron-fox
      mainline

      Revision 1.1.4.1  2004/04/12 17:33:03  ron-fox
      - Packet creation added.

      Revision 1.1  2003/12/09 21:16:27  ron-fox
      Incorporated ScriptedReadout software into the build.

      Revision 1.1.1.1  2003/10/23 11:59:16  see
      Initial CVS repository import

           
*/
////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CConfigurationParameter.h"    				
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <string>

using namespace std;



/*!
   Constructor.  The configuration parameter is constructed
   by assigning a keyword to the object. This keyword will
   be recognized by the Match member function.  At this time
   we cannot define a default for the value since that needs
   to be correlated with any internal representation maintained
   by the derive classes, and virtual functions essentially
   don't work virtually in constructors.
*/   
CConfigurationParameter::CConfigurationParameter (const string& keyword)
   : m_sSwitch(keyword)
{   
    
         //Initialization of array of 1:M association objects to null association objects
    
} 

/*!
    Destructor.  No action is required. Since derived classes
    may need class specific destruction, we provide a virtual
    base class destructor as a placeholder to support 
    destructor virtualization.
*/
 CConfigurationParameter::~CConfigurationParameter ( ) 
{
}


/*!
   Copy constructor.  This constructor is used by the compiler
  to create temporaries (e.g. in pass by value to function
  situtations.

  \param rhs const CConfigurationParameter& [in]
            the object that will be cloned into us.
*/
CConfigurationParameter::CConfigurationParameter (const CConfigurationParameter& rhs ) :
  m_sSwitch(rhs.m_sSwitch),
  m_sValue(rhs.m_sValue)
{
 
} 

/*
   Assignment.  'this' will be made into a copy of the
  \em rhs parameter.  This function differs from copy 
  construction in that it is invoked in expressions of the
  form
  \verbatim
  lhs = rhs;
  \endverbatim

  \param rhs const CConfigurationParameter& rhs [in]
          The object that will be copied to this.

  \return *this.
*/
CConfigurationParameter& 
CConfigurationParameter::operator= (const CConfigurationParameter& rhs)
{ 
  if(this != &rhs) {
    m_sSwitch = rhs.m_sSwitch;
    m_sValue  = rhs.m_sValue;
  }
  return *this;
}

/*!
   Determins if this is functionally equivalent to the \em rhs
  parameter. This will be true if all member data are equal.

  \param rhs const CConfigurationParameter& rhs [in]
              The object to be compared with *this.

  \return Either of:
  - true if there is functional equivalence.
  - false if there is not functional equivalence.
*/
int 
CConfigurationParameter::operator== (const CConfigurationParameter& rhs) const
{ 
  return ( (m_sSwitch == rhs.m_sSwitch)    &&
           (m_sValue  == rhs.m_sValue));

}

// Functions for class CConfigurationParameter

/*!  Function: 	
  Returns true if the input string matches m_sSwitch.
  typically intended to be used in detecting which of
  several configuration parameters should be parsed.

  \param rSwitch - const string& [in]
          The string to match against m_sSwitch.

*/
bool 
CConfigurationParameter::Match(const string & rSwitch)  
{ 
  return (m_sSwitch == rSwitch);
}  

/*!  Function: 	

Called when our keyword matches an option keyword. 
The new value of the parameter is saved.  This is a virtual
member function.  The action is as follows:
- Call SetValue
- If SetValue returned TCL_OK, update the stringified value.
- If SetValue failed, return to the caller without update.

\param rInterp CTCLInterpreter& [in] Interpreter that is runinng
              this command.
\param rResult CTCLResult& [in] The result object that will
              hold any error string if there is a problem.
\param parameter const char* [in] the string containing the
            candidate new value.

\return This function can return:
      TCL_OK - if SetValue claims the parameter string was 
                properl parsed.
      TCL_ERROR - if not.

*/
int 
CConfigurationParameter::operator()(CTCLInterpreter& rInterp,
                                    CTCLResult& rResult, 
                                    const char* parameter)  
{ 
  int status = SetValue(rInterp, rResult, parameter);
  if(status == TCL_OK) {
    setValue(parameter);
  }
  return status;
}  

/*!  Function: 	

Returns the current value of the
parameter value as a string.  We delegate to 
getValue.

*/
string 
CConfigurationParameter::getOptionString() const  
{ 
  return getValue();
}  

