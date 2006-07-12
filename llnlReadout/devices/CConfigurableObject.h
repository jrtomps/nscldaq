\/*
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

#ifndef __CCONFIGURABLEOBJECT_H
#define __CCONFIGURABLEOBJECT_H

// Necessary includes (kept to a minimum using forward class defs).

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


#ifndef __STL_MAP
#include <map>			// also defines std::pair
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_SET
#include <set>
#ifndef __STL_SET
#define __STL_SET
#endif
#endif

// Typedefs for the parameter checker are in the global namespace:

typedef bool (*typeChecker)(std::string name, std::string value, void* arg);

/*!
   Configurable object consist of a name and a configuration.
   A configuration is a set of name value pairs.  Each name value
   pair can have an associated validity checker who is responsible
   for ensuring that for any configuration value a candidate new value
   is legal.  Type checkers are just unbound functions, or static
   member functions.  See
   the typedef for typeChecker above.
 
   For convenience, some of the static functions of this class provide
   commonly used type checkers.

   How to use this:

   In general, one would subclass this object.  The subclassed object would 
   at some point call addParameter a number of times to provide the configuration 
   parameters needed by the subclass.  

   The configuration subsystem would at some point call configure to set up
   the configuration (note subclassed objects can also call this to set up their
   default configuration).

   The data taking system would then call subclass functions to inform it that
   configuration is complete.  The subclass then would call cget to get and
   react to its configuration.

   This class does not implement any policy about how the configuration is gotten.
   This can be  done by manual parsing of data files, by internal data structures,
   by a Tcl interpreter reading scripts or any other practical means.
*/
class CConfigurableObject {
  // internal typedefs:
private:
  typedef std::pair<typeChecker, void*>         TypeCheckInfo;	// Type checker + arg.
  typedef std::pair<std::string, TypeCheckInfo> ConfigData;
  typedef std::map<std::string, ConfigData>     Configuration;
  typedef Configuration::iterator               ConfigIterator;

  // public typedefs:

  typedef std::vector<std::pair<std::string, std::string> > ConfigurationArray;
  
  // Arg types for the standard type checkers.

  struct  limit {
    bool   s_checkMe;
    long    s_value;
  }
  typedef std::pair<limit, limit>   isIntParameter;
  typedef std::set<std::string> isEnumParameter;
  struct ListSizeConstraint {
    int s_atLeast;
    int s_atMost;
  };
  typedef _isListParameter {
    ListSizeConstraint s_allowedSize;
    typeChecker        s_Checker;
    void*              s_CheckerArg;
  };

private:
  std::string     m_name;	//!< Name of this object.
  Configuration   m_parameters;	//!< Contains the configuration parameters.

public:
  // Canonicals..

  CConfigurableObject(std::string name);
  CConfigurableObject(const CConfigurableObject& rhs);
  virtual ~CConfigurableObject();
  CConfigurableObject& operator=(const CConfigurableObject& rhs);
  int operator==(const CCOnfigurableObject& rhs) const;
  int operator!=(const CConfigurableObject& rhs) const;

  //  Selectors:

public:
  std::string getName() const;
  std::string cget(std::string name) const;
  ConfigurationArray cget()          const;

  // Operations:

public:
  // Establishing the configuration:

  void addParameter(std::string name, typeChecker checker, void* arg 
		    std::string default = std::string(""));

  // Manipulating and querying the configuration:

  void configure(std::string name, std::string value);



  // common type checkers:

  static bool isInteger(std::string name, std::string value, void* arg);
  static bool isBool(   std::string name, std::string value, void* arg);
  static bool isEnum(   std::string name, std::string value, void* arg);
  static bool isList(   std::string name, std::string value, void* arg);
  static bool isBoolList(std::string name,std::string value, void* arg);
  static bool isIntList(std::string name, std::string value, void* arg);
  static bool isStringList(std::string name, std::string value, void* arg);
  
};


#endif
