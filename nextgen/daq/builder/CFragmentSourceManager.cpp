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
#include <config.h>
#include "CFragmentSourceManager.h"
#include "CFragmentSource.h"
#include <Exception.h>
#include "CNoSuchConfgurationParameter.h"
#include "CInvalidConfigurationValue.h"
#include "CInvalidObjectType.h"
#include "CInvalidObjectState.h"

using namespace std;
/*!
   Fragment sources are configurable.  The configuration of a source is a set of name/value 
   pairs.  The ability to define valid values for pairs that are checked by the infrastructure
   is also provided.  This function checks the validity of a name/value pair for a specific
   fragment source.
   @param paramName    - Name of the parameter.
   @param parmValue    - Proposed new parameter value.
   @param pInstance    - Driver instance on which the configuration is proposed.
   @return std::string
   @retval std::string("") - The proposed configuration is acceptable.
   @retval anyting else - A stringified error describing why the configuration is not
                          acceptable.

   @note No configuration is actually performed.
*/
std::string 
CFragmentSourceManager::validateConfig(std::string paramName, std::string paramValue,
				       CFragmentSource* pInstance)
{
  throwIfNoInstance(pInstance);
  try {
    pInstance->configIsValid(paramName, paramValue);
    return string("");
  }
  catch(CException& e) {
    return std::string(e.ReasonText());
  }
  catch(std::string s) {
    return s;
  }
  catch (const char* pString) {
    return std::string(pString);
  }
  catch (...) {
    return std::string("Unanticipated exception");
  }
}
/*!
   Sets the value of a configuration item in a fragment source
   @param paramName   - Name of the parameter being configured.
   @param paramValue  - New value for the parameter.
   @param pInstance   - POinter to the driver instance to be configured.
   
   @throw CInvalidObjectType           - pInstance is not managed by this manager.
   @throw CNoSuchConfigurationParmeter - paramName is not a configuration parameter of the
                                         instance.
   @throw CInvalidConfigurationValue   - paramValue is not a legal value for paramName.
*/


void
CFragmentSourceManager::config(std::string paramName, std::string paramValue, 
			       CFragmentSource* pInstance)
{
  throwIfNoInstance(pInstance);

  try {
    pINstance->configure(paramName, paramvalue);
  }
  catch(std::string e) {
    if(e.find("No such parameter") == 0) {
      throw CNoSuchConfigurationParameter(paramName, paramValue,
					  "configuring a driver instance", 
					  managerName());
    }
    if (e.find("Validation failed") == 0) {
      throw CInvalidConfigurationValue(paramName, paramValue,
				       "configuring a driver instance",
				       getName());
    }
    // This is not expected. rethrow.

    throw;
  }

}
 
/*!
  Retrieve the string version of a configuration value.  This method is intended for command
  classes/objects that are producing user readable dumps of configurations... as there
  is no type information.  Fortunately usually only the driver instance itself needs anything
  other than that.

  @param paramName   - Name of the parameter to retrieve
  @param pInstance   - Pointer to the driver instance for whome we are inquiring.

  @return std::string
  @retval Value paramName
  
  @throw CInvalidObjectType pInstance is not an object managed by this manager.
  @throw CNoSuchConfigurationParameter paramName is not the name of a parameter that is
                                       recognized by pInstance.

*/
std::string
CFragmentSourceManager:: cget(std::string paramName,
			      CFragmentSource* pSource)
{
  throwIfNoInstance(pSource);


  try {
    return pSource->cget(paramName);
  }
  catch (...) {
    throw CNoSuchConfigurationParameter(paramName, 
					"Getting a configuration parameter value",
					getName());
  }
}
/*!
   Return the entire configuration paramter set.
   @param pInstance - Pointer to the fragment source instance from which
                      the configuration is to be fetched.
   @return std::map<std::string, std::string>
   @retval Each entry of the map is a name value pair.  The first element of
           the pair is a parameter name.  The second element of each pair
           is its current value (as of the time of this call).
*/
std::map<std::string, std::string> 
CFragmentSourceManager::cget()
{
  throwIfNoInstance(pInstance);
  return pInstance->cget();
}

/*!
    Starts a fragment source instance. It is an error to start one
    that is already running.
    @param pInstance - Pointer to the driver instance.

    @throw CInvalidObjectState - The driver instance is already running.
    @throw CInvalidObjectType - pInstance not managed by this manager.
*/
void 
CFragmentSourceManager::start(CFragmentSource* pInstance)
{
  setState(pInstance, true);
}
/*!
   Stops a fragment source instance. It is an error to stop a fragment
   source instance that is already stopped.
    @param pInstance - Pointer to the driver instance.
    @throw CInvalidObjectState - The driver instance is already stopped.

*/
void 
CFragmentSourceManager::stop(CFragmentSource* pInstance)
{
  setState(pInstance, false);
}

/*!
    @param pInstance fragment sourceinstance pointer.
    @return bool
    @retval true - driver instance is running
    @retval false - Driver instance is not running

    @throw CInvalidObjectType - pInstance not managed by this manager.
*/
bool 
CFragmentSourceManager::isRunning(CFragmentSource* pInstance)
{
  FragmentSourceIterator p = find(pInstance);
  if (p != end) {
    return p->second;
  }
  else {
    throw CInvalidObjectType(pSource,
			     "Getting Run state"
			     getName());
  }
}
/*!
   @return CFragmentSourceManager::FragmentSourceIterator
   @retval start of iteration iterator into the fragment source map.
*/
CFragmentSourceManager::FragmentSourceIterator 
CFragmentSourceManager::begin()
{
  return m_mySources.begin();
}

/*!
   @return CFragmentSourceManager::FragmentSourceIterator
   @retval start of iteration iterator into the fragment source map.
*/
CFragmentSourceManager::FragmentSourceIterator 
CFragmentSourceManager::end()
{
  return m_mySources.end();
}
/*!
   @param pInstance - The instance to locate.
   @return CFragmentSourceManager::FragmentSourceIterator
   @retval iterator pointing to the found item. This is a
           pointer like object to
	   std::pair<CFragmentSource*, bool>  Where the
	   bool part is true if the driver instance is running.
   @retval end() - if pInstance is not being managed by this 
          manager.
*/
CFragmentSourceManager::FragmentSourceIterator
CFragmentSourceManager::find(CFragmentSource* pInstance)
{
  return m_mySources.find(pInstance);
}

/*!
  @return size_t
  @retval number of driver instances we're managing:
*/
size_t
CFragmentSourceManager::size()
{
  return m_mySources.size();
}

/*------------------------------------ non public ---------------------------------*/

/*
**
** protected element that throws a CInvalidObjectType exception if 
** its paramneter is not managed by this object.
** Parameter:
**   @param pInjstance - the instance to check.
*/
void 
CFragmentSourceManager::throwIfNoInstance(CFragmentSource* pInstance)
{
  if (find(pInstance()) == end())) {
  throw CInvalidObjectType(pInstance,
			   "Checking if the instance is being managed by this",
			   getName());
}

/*
** Starts or stops a driver instance throwing appropriate exceptions if needed.
**
** Parameters:
**   @param pInstance - Driver instance variable.
**   @param newState  - True to start the instance, false to stop it. 
**                      This is the desried run state.
*/
void 
CFragmentSourceManager::setState(CFragmentSource* pInstance, bool newState)
{
  FragmentSourceIterator p;

  if (isRunning(pInstance) != newState) {
    p = find(pInstance);	// isRunning will throw if not found.
    newState ? pInstance->start() : 
               pInstance->stop();
    p->second = newState;
  }
  else {
    throw CInvalidObjectState(newState ? "Running" : "Halted",
			      isRunning(pInstance) ? "Running" : "Halted",
			      "Changing run state",
			      getName());
			      
  }
}
