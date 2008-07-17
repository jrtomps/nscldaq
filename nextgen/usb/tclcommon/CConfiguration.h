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


#ifndef __CCONFIGURATION_H
#define __CCONFIGURATION_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

class CTCLInterpreter;
class CConfigurableObject;
class CTCLObjectProcessor;


/*!
   This class encapsulates the configuration of
   Readout and scaler modules.  It makes available
   services to:
   - Read a configuration.
   - To locate a module by name.
   - To add new modules to the system.

   The idea is that when a run starts, the software will
   create an instance of the configuration and then
   configure it.  At run end, we destroy that instance,
   so that the new run is reconfigured according to any 
   changes in the configuration file.
*/
class CConfiguration 
{
public:
  struct ConfigItem {
    std::string           s_name;	// Name of the item.
    std::string           s_type;       // Type of the item.
    CConfigurableObject*  s_pObject;    // Pointer to the object.
  };
  typedef std::vector<ConfigItem>     Configuration;
  typedef Configuration::iterator     ConfigurationIterator;
private:
  Configuration           m_Objects; // This is what we load.

  CTCLInterpreter*                    m_pInterp;
  std::vector<CTCLObjectProcessor*>   m_Commands;
  //
  // Canonicals:
  //
public:
  CConfiguration();
  virtual ~CConfiguration();

  // lazy so:
private:
  CConfiguration(const CConfiguration& rhs);
  CConfiguration& operator=(const CConfiguration& rhs);
  int operator==(const CConfiguration& rhs) const;
  int operator!=(const CConfiguration& rhs) const;
public:

  // Reading configurations:

  void processConfiguration(std::string configFile);
  void addObject(std::string name, std::string type, 
		 CConfigurableObject* pObject);
  void setResult(std::string);

  // Interrogating the configuration.

  std::vector<ConfigItem>           getObjectsOfType(std::string type);
  ConfigurationIterator             findObjectByName(std::string name);
  ConfigurationIterator             end();

  // For derived classes:

protected:
  void addCommand(CTCLObjectProcessor& processor);


  
};










#endif
