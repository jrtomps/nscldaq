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
/*! \class CModuleCreator  abstract 
           CLASS_PACKAGE
           Abstract base class of a set of factory like (creational) modules
           that make data acquisition module classes.  Each of these has
           a string module type.  And the following key members:
           - Match - return true if an input string matches the module type.
           - Create - Returns a new module.
           - Help    - Returns stringified help about the type of module
                          created by this class.
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CModuleCreator.h"    	
#include "CReadableObject.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>			

using namespace std;



/*!
	Constructor: The only thing we need to do is initialize
	the module type field:
	
	\param rType const string& [in]
				Type name of the parameter.  This turns out to be the
				module_type for tcl commands of the form:
			module create module_type
*/
CModuleCreator::CModuleCreator (const string& rType)
   : m_sModuleType(rType)
 
{   
} 

/*!
    There is nothing interesting to do for the destructor.
*/    
 CModuleCreator::~CModuleCreator ( )  //Destructor - Delete dynamic objects
{
}

/*!
    Copy construction. We are constructed as an exact copy of the parameter
*/
CModuleCreator::CModuleCreator (const CModuleCreator& rhs) :
	m_sModuleType(rhs.m_sModuleType)
{

} 

/*!
	Assignment.  rhs is copied into this.  The only difference
	between assignment and copy construction is that copy constructed
	objects are being created, and assignment objects have longer duration.
	
	\param rhs ConstCModuleCreator& [in]
		 The rhs of the assignment operation.
		 
	 \return a new object that can be put into the readout
	 subsystem.

*/
CModuleCreator& 
CModuleCreator::operator= (const CModuleCreator& rhs)
{ 
   if(this != &rhs) {
      m_sModuleType = rhs.m_sModuleType;
   }

  return *this;
}

/*!
   Test for equality.  The assumption is that there cannot be
   two modules with the same mdule name.
   
   \param rhs  CModuleCreator& [in] rhs of the == operator.
*/
int 
CModuleCreator::operator== (const CModuleCreator& rhs) const
{ 
   return (m_sModuleType == rhs.m_sModuleType);
}

// Functions for class CModuleCreator

/*!  Function: 	
 

Returns true if the input string 
matches the module type.
	\param rType const string& [in]  The type of module being searched
			for.
	\return  One of:
		- true - If rType == m_sModuleType
		-false - if not.

*/
bool
CModuleCreator::Match(const string& rType)   const
{ 
   return (rType == m_sModuleType);
}  

