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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CVMEStatusModule.cpp file////////////////////////////////////

#include <config.h>
#include "CVMEStatusModule.h"                  
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

static const unsigned int GoingBusy=0;
static const unsigned int GoingFree=1;

/*!
  Construct a status module.  The status module is a CAEN VME262 modle.
  The module is fully encapsulated  here.
  \param base  - Base address in switches of the module.
*/
CVMEStatusModule::CVMEStatusModule (UInt_t base, int nCrate) :
  CStatusModule(),
  m_IOModule(base, nCrate)
{
} 

/*!
  Construct a status module given that a CAEN VME262 module object has 
  already been created.  THis constructor will allow the module to be
  shared amongst other functions without wasting mapping resources.

  \param module - Reference to the CAEN V262 object.
  */
CVMEStatusModule::CVMEStatusModule(const CCaenIO& module) :
  CStatusModule(),
  m_IOModule(module)
{}

/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CVMEStatusModule::CVMEStatusModule(const CVMEStatusModule& rhs) :
  CStatusModule(),
  m_IOModule(rhs.m_IOModule)
{

}

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CVMEStatusModule& CVMEStatusModule::operator= (const CVMEStatusModule& aCVMEStatusModule)
{ 
    if (this != &aCVMEStatusModule) {
       CStatusModule::operator= (aCVMEStatusModule);
       m_IOModule = aCVMEStatusModule.m_IOModule;
    }
    return *this;
}

/*!
  Equality comparison:
  \param rhs = The rhs of the == operator.
  */
int
CVMEStatusModule::operator==(const CVMEStatusModule& rhs) 
{
  return ((m_IOModule == rhs.m_IOModule) &&
	  CStatusModule::operator==(rhs));
}

// Functions for class CVMEStatusModule

/*!
    Interface: Pulse the going busy output.

*/
void 
CVMEStatusModule::GoBusy()  
{
  m_IOModule.PulseOutput(GoingBusy);
}  

/*!
    Pulse the going not busy output.

*/
void 
CVMEStatusModule::GoClear()  
{
  m_IOModule.PulseOutput(GoingFree);
}
/*!
   Pulse by hand the module clears which are the NIM level
   outputs of the module.
   */
void
CVMEStatusModule::ModuleClear() 
{
  m_IOModule.PulseOutput(3);

}
