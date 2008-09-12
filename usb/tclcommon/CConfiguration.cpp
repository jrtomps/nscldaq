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
#include "CConfiguration.h"
#include "CConfigurableObject.h"


#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <Exception.h>

#include <tcl.h>
#include <algorithm>

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonicals //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
  Construct the object.  This consists of 
  creating a new Tcl interpreter, wrapping into an object and
  the adding the commands to the interpreter needed to manage the
  configuration as well as saving those command processor objects for destruction
  lateron.
*/
CConfiguration::CConfiguration() :
  m_pInterp(0)
{

  m_pInterp = new CTCLInterpreter();

}
/*!
   Destruction must:
   - Destroy all the CReadoutModule object pointers that were added to m_Adcs
   - Destroy all the CReadoutModule object pointers that were added to m_Scalers.
   - Destroy all readout stacks.
   - Destroy all the CTCLObjectProcessors that were added to m_Commands.
   - Destroy m_pInterp which will implicitly invoke Tcl_DeleteInterp() on
     the interpreter it wraps.
*/
CConfiguration::~CConfiguration()
{
  // Delete the configurable objects that have been created.
  // vector is perfectly capable of cleaning itself up.

  clearConfiguration();

  delete m_pInterp;
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Support for configuration /////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Process a configuration file.  Since configuration files are just
   Tcl scripts for our enhanced interpreter, we just EvalFile.
   errors are allowed to propagate upwareds.
   \param configFile : std::string
       Name of the configuration file to process.
*/
void
CConfiguration::processConfiguration(string configFile)
{

  // Clear the configuration:

  clearConfiguration();


  // Process the file:

  try {
    m_pInterp->EvalFile(configFile);
  }
  catch (string msg) {
    cerr << "CConfiguration::processConfiguration caught string exception: "
	 << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "CConfiguration::processConfiguration caught char* exception: "
	 << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "CConfiguration::processConfiguration caught CExcpetion : "
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    cerr << Tcl_GetStringResult(m_pInterp->getInterpreter()) << endl;
    throw;
  }
  catch (...) {
    cerr << "CConfiguration::processConfiguration caught an unknown exception type\n";
    throw;
  }
}

/*!
   Add an object to the configuration:
   \param name   - Name of the object in the configuration.
   \param type   - Type of the object in the configuration
                   One use of this is to distinguish between objects of type
		   e.g. Readout and objects of type stack.
   \param pObject- Popinter to the object to add.
*/
void 
CConfiguration::addObject(string name, string type, CConfigurableObject* pObject)
{
  ConfigItem item = {name, type, pObject};
  m_Objects.push_back(item);
}


/*!
  set the interpreter result.
  \param msg : std::string
*/
void
CConfiguration::setResult(string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(m_pInterp->getInterpreter(), result);
}


////////////////////////////////////////////////////////////////////////////
/////////////////////// Support for config retrieval ///////////////////////
////////////////////////////////////////////////////////////////////////////


/*!
   Return the objects of a specific type.
   This might be used e.g. to get all the stacks so that they can be
   configured and downloaded.
   \param type - the type of item we want.

   \return vector<CConfiguration::ConfigItem>
   \retval the vector of items that match the requested type
     (possibly empty).
*/
vector<CConfiguration::ConfigItem>
CConfiguration::getObjectsOfType(string type)
{
  vector<ConfigItem> result;
  for (int i =0; i < m_Objects.size(); i++) {
    if (m_Objects[i].s_type == type) {
      result.push_back(m_Objects[i]);
    }
  }
  return result;
}

/*!
  Return a pointer to an object that matches the name of the requested item.
  This might be used by a configuration command to locate an existing object to
  configure.
  \param name - Name of the item to lookup.
  \return ConfigurationIterator
  \retval end - item not found.
  \retval other - iterator 'pointing' to the item that was found.

*/
CConfiguration::ConfigurationIterator
CConfiguration::findObjectByName(string name)
{
  ConfigurationIterator p = m_Objects.begin();
  while (p != m_Objects.end()) {
    if (p->s_name == name) break;
    p++;
  }
  return p;
}


/*!
  \return CConfigurationIterator
  \retval end iterator against which find iterators can be compared
          to determine if the object was not found.
*/
CConfiguration::ConfigurationIterator
CConfiguration::end()
{
  return m_Objects.end();
}






/*!
 Register a dynamically allocated extension command so that it
 will be automatically deleted when we are destroyed:

 \param processor - reference to the processor to add.

*/
void
CConfiguration::addCommand(CTCLObjectProcessor& processor)
{
  m_Commands.push_back(&processor);
}
/*!
  Clear the configuration.
*/
void
CConfiguration::clearConfiguration()
{
  for (int i=0; i < m_Objects.size(); i++) {
    delete m_Objects[i].s_pObject;
  }
  m_Objects.clear();



}
/*!
  \return CTCLInterpreter* 
  \retval Pointer to the interpreter that this configuration object runs on.
*/
CTCLInterpreter*
CConfiguration::getInterpreter()
{
  return m_pInterp;
}
