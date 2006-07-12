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
#include "CConfigurableObject.h"
#include <stdlib.h>
#include <errno.h>


using namespace std;

///////////////////////////////////////////////////////////////////////////
//////////////////////// Canonical member functions ///////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
   Contruct the object:
   \param name : std::string
       Name to give to the object.
*/
CConfigurableObject::CConfigurableObject(string name) :
  m_name(name)
{
}

/*!
  Destruction probably will result in some memory leaks since
  it is possible that the typeChecker's will have parameters that
  are dynamically allocated. In a future life we can provide
  cleanup functions.. for now we just assume that destruction will
  be infrequent, and leaks will be small enough to be tolerated.
  
  I think that destruction is not necessary since all the
  pairs will copyconstruct/assign into the map.
  This is a place holder for later code that can handle deletion of the
  typechecker args.

*/
CConfigurableObject::~CConfigurableObject()
{

}

/*!
   Copy construction.  We can just copy the name and map.
   Copy construction should be rare as normally names are unique.
*/
CConfigurableObject::CConfigurableObject(const CConfigurableObject& rhs)
  m_name(rhs.m_name),
  m_parameters(rhs.m_parameters)
{

}
/*!
   Assignement is similar to copy construction:
*/
CCOnfigurableObject&
CConfigurableObject::operator=(const CConfigurableObject& rhs)
{
  if (this != &rhs) {
    m_name       = rhs.m_name;
    m_parameters = rhs.m_parameters; 
  }
  return *this;
}

/*!
  Equality is via item comparison.
*/
int
CConfigurableObject::operator==(const CConfigurableObject& rhs) const
{
  return ((m_name == rhs.m_name)   &&
	  (m_parameters == rhs.m_parameters));
}
/*!
  Inequality is the logical inverse of equality:
*/
int
CConfigurableObject::operator!=(const CConfigurableObject& rhs) const
{
  return !(*this == rhs);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////// Selectors //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
  \return string
  \retval The name of the object.
 */
string
CConfigurableObject::getName() const
{
  return m_name;
}

/*!
   Get the value of a single configuration item.
   \param name : std::string
      Name of the parameter to retrieve.

   \return std::string
   \retval the value of the configuration parameter.

   \throw string - if name is not a known configuration parameter, afte all,
                   clients should know the configuration parameters.. if they don't
		   see the next cget which retrieves parameters and values.
*/
string
CConfigurationParameter::cget(string name) const
{
  ConfigIterator found = m_parameters.begin();
  if (found == m_parameters.end()) {
    string msg("CConfigurationParameter::cget was asked for parameter: ");
    msg += name;
    msg += " which is not defined";
    throw msg;
  }
  ConfigData data = found->second;
  return data.first;
}
/*!
   Get the values of all the configuration parameters.
   \return CCOnfigurableObject::ConfigurationArray
   \retval A vector of parameter name/value pairs.
           Given an element, ele of the vector,
	   ele.first is the parameter name, ele.second the value.

   \note While at present, the parameters come out sorted alphabetically,
         you should not count on that fact.
*/
CConfigurableObject::ConfigurationArray
CConfigurationParameter::cget() const
{
  ConfigurationArray result;
  ConfigIterator p = m_parameters.begin();
  while(p != m_parameters.end()) {
    
    string name = p->first;
    ConfigurationData data = p->second;
    string value = data.second;
    result.push_back(pair<string, string>(name, value));
    
    p++;
  }
  return result;
}
///////////////////////////////////////////////////////////////////////////
/////////////////////// Establishing he configuration /////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
   Adds a configuration parameter to the configuration.
   If there is already a configuration parameter by this name,
   it is silently ovewritten.

   \param name : std::string
      Name of the parameter to add.
   \param checker : typeChecker
      A type checker to validate the values proposed for the parameter,
      if NULL, no validation is performed.
   \param arg   : void
      Parameter passed without interpretation to the typechecker at validation
      time.
   \param default : string (default = "")
      Initial value for the parameter.
*/
void
CConfigurationParameter::addParameter(string      name, 
				      typeChecker checker,
				      void*       arg,
				      string      default)
{
  TypeCheckInfo checkInfo(checker, arg);
  ConfigData    data(default, checkInfo);
  m_parameters[name] = data;	// This overwrites any prior.
}

/*!
    Configure the value of a parameter.
    \param name : std::string
       Name of the parameter to configure.
    \param value : std::string
       New value of the parameter

   \throw std::string("No such parameter") if the parameter 'name' is not defined.
   \throw std::string("Validation failed for 'name' <- 'value'") if the value
           does not pass the validator for the parameter.
*/
void
CConfigurationParameter::configure(string name, string value)
{
  // Locate the parameter and complain if it isn't in the map:

  ConfigIterator item = m_parameters.find(name);
  if(item == m_parameters.end()) {
    string msg("No such parameter: ");
    msg  += name;
    throw msg;
  }
  // If the parameter has a validator get it and validate:

  TypeCheckInfo checker = item->second->second;
  TypeChecker   pCheck  = checker.first;
  if (pCheck) {			// No checker means no checkig.
    if (! (*pCheck)(name, value, checker.second)) {
      string msg("Validation failed for ");
      msg += name;
      msg += " <- ";
      msg += value;
      throw msg;
    }
  }
  // Now set the new validated value:

  m_parameters[name].second.first = value;

}
////////////////////////////////////////////////////////////////////////////
/////////////////////  Stock type checkers //////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Validate an integer parameter with optional limits.
   \param name : std::string
       Name of the parameter being checked (ignored).
   \param value : std::string
       Proposed new value.
   \param arg : void*
       This is actually a pointer to an isIntParameter structure or NULL.
       - If NULL, no range checking is done.
       - If not null, range checking is done.  Each limit contains a checkme
         flag which allows validation to occur when one or both limits are needed.
         limits are inclusively valid.
    \return bool
    \retval true  - Validation passed.
    \retval false - Validation failed.
*/
bool
CConfigurableObject::isInteger(string name, string value, void* arg)
{
  // first determine the 'integernes' using strtoul.

  errno = 0;			// Some strtoul's don't init this evidently.
  long value = strtoul(value.c_str(), NULL, 0);	// Base allows e.g. 0x.
  if ((value ==0) && (errno != 0)) {
    return false;
  }
  // If there's no validator by definition it's valid:

  if(!arg) return true;

  // Get the validator in the correct form:

  isIntParameter* pRange = dynamic_cast<isIntParameter*>(arg);
  if (!isIntParameter) {
    string msg("BUG: argument for integer validator for parameter: ");
    msg += name;
    msg += " is not a pointer to a isIntParameter type";
  }
  // check lower limit:

  if((pRange->first.s_checkMe) && (value < pRange->first.s_value)) {
    return false;
  }
  if ((pRange->second.s_checkMe) && (value > pRange->second.s_value)) {
    return false;
  }
  return true;
}
