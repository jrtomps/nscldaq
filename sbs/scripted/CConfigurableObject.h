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



/*!

Base class of all objects that have a TCL configurable
 configuration. The configuration object autonomously processes the
config an cget subcommands to maintain a configuration parameter 
database.  Configuration consists of a set of configuration parameter 
objects.

Each of these represents a keyword/value pair. 

*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CCONFIGURABLEOBJECT_H  //Required for current class
#define __CCONFIGURABLEOBJECT_H

//
// Include files:
//

#ifndef __TCLPROCESSOR_H
#include <TCLProcessor.h>
#endif

#ifndef __TCLRESULT_H
#include <TCLResult.h>        //Required for include files  
#endif


#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


// forward definitions. 

class CConfigurationParameter;
class CTCLInterpreter;
class CTCLResult;
class CIntArrayParam;


class CConfigurableObject : public  CTCLProcessor     
{
  // Public data types.
public:
  typedef STD(list)<CConfigurationParameter*> ConfigArray;
  typedef ConfigArray::iterator          ParameterIterator;
private:
  
  STD(string)          m_sName;	//!< Name of the command associated with the object.
  ConfigArray     m_Configuration; //!< The configuration.


  // Constructors and other canonical operations.
public:
  CConfigurableObject (const STD(string)& rName,
		       CTCLInterpreter& rInterp);
  virtual  ~ CConfigurableObject ( );  

  // The copy like operations are not supported on tcl command processing
  // objects:
private:
  CConfigurableObject (const CConfigurableObject& aCConfigurableObject );
  CConfigurableObject& operator= (const CConfigurableObject& aCConfigurableObject);
  int operator== (const CConfigurableObject& aCConfigurableObject) const;
public:

  // Selectors:

  //!  Retrieve a copy of the name:

  STD(string) getName() const
  { 
    return m_sName;
  }   



  // Member functions:

public:

  virtual  int      operator() (CTCLInterpreter& rInterp, 
				CTCLResult& rResult, 
				int nArgs, char** pArgs)   ; //!< Process commands.
  virtual  int      Configure (CTCLInterpreter& rInterp, 
			       CTCLResult& rResult, 
			       int nArgs, char** pArgs)   ; //!< config subcommand 
  virtual  int      ListConfiguration (CTCLInterpreter& rInterp, 
				       CTCLResult& rResult, 
				       int nArgs, char** pArgs); //!< list subcommand 
  ParameterIterator AddParameter(CConfigurationParameter* pConfigParam);
  ParameterIterator AddIntParam (const STD(string)& sParamName, 
				 int nDefault=0)   ; //!< Create an int.
  ParameterIterator AddBoolParam (const STD(string)& rName,
				  bool          fDefault)   ; //!< Create a boolean. 
  ParameterIterator AddStringParam (const STD(string)& rName)   ; //!< Create string param. 
  ParameterIterator AddIntArrayParam (const STD(string)&  rParameterName, 
				      int nArraySize, 
				      int nDefault=0)   ; //!< Create array of ints.
  ParameterIterator AddStringArrayParam (const STD(string)& rName, 
					 int nArraySize)   ; //!< Create array of strings.
  ParameterIterator AddEnumParam(STD(string) name,
				 STD(vector)<STD(pair)<STD(string), int> > values,
				 STD(string) defaultValue);
  ParameterIterator Find (const STD(string)& rKeyword)   ; //!< Find a param 
  ParameterIterator begin ()   ; //!< Config param start iterator.
  ParameterIterator end ()   ;   //!< Config param end iterator.
  int size ()   ;                //!< Config param number of items.
  STD(string) ListParameters (const STD(string)& rPattern)   ; //!< List configuration 
  STD(string) ListKeywords ()   ;     //!< List keyword/type pairs.

  // Incomplete access to some configuratin parameter types by name.

  int getIntegerValue(STD(string) name); //!< Get value of an integer config param.
  bool getBoolValue(STD(string) name); //!< Get value of a bollean config param.
  CIntArrayParam* getIntArray(STD(string) name); //!< Get ptr to int array param.
  int             getEnumValue(STD(string) name);
protected:
  STD(string) Usage();
private:
  void              DeleteParameters ()   ; //!< Delete all parameters. 
  ParameterIterator FindOrThrow(STD(string) name, STD(string) type);
  
  
};

#endif
