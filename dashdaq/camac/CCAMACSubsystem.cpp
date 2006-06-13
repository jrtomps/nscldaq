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

#include <CCAMACSubsystem.h>
#include <CCAMACInterface.h>
#include <CCAMACInterfaceFactory.h>
#include <RangeError.h>
#include <CInvalidInterfaceType.h>
#include <parseUtilities.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace descriptionFile;

// Static data elements of the class:

CCAMACSubsystem*                      CCAMACSubsystem::m_pInstance(0);
CCAMACSubsystem::InterfaceContainer  CCAMACSubsystem::m_interfaces;

/*!
    Construct a camac subsystem.  This is 
    private since the class is singleton.
*/
CCAMACSubsystem::CCAMACSubsystem()
{
  m_pInstance = this;
}
/*!   

    Destructor is just provided in case.
*/
CCAMACSubsystem::~CCAMACSubsystem()
{
  m_pInstance = 0;
}

/*!
  Return the instance pointer to the singleton. 
  \return CCAMACSubsystem*

*/
CCAMACSubsystem*
CCAMACSubsystem::getInstance()
{
  if (!m_pInstance) {
    new CCAMACSubsystem;
  }
  return m_pInstance;		// Set by constructor if need be.
}
/*!
   Add an interface to the subsystem. 
   \param interface : CCAMACInterface& 
       Reference to the item to add.
   \return size_t
   \retval the index of the new element (can be used with operator[]).
*/
size_t
CCAMACSubsystem::addInterface(CCAMACInterface& interface)
{
  size_t index = size();	// Size prior is last index after push.
  m_interfaces.push_back(&interface);
  return index;
}
/*!
   remove an interface from the subsystem.
   The interface is not deleted as we don't know how it should be managed.
   The caller is responsible for knowing what to do with the object once
   removed.
   \param index : size_t
       The index of the item to remove.
   \return CCAMACInterface*
   \retval Pointer to the interface removed.
   \throw CRangeError - if the index is not valid.

   size_t is assumed to be an unsigned type.
*/
CCAMACInterface*
CCAMACSubsystem::removeInterface(size_t index)
{

  CCAMACInterface*  result = (*this)[index]; // takes care of our throw too.

  // wish I could think of a way to delete by index, but vectors don't give
  // that.. in any event I don't expect the CAMAC subsystem to be >that< 
  // dyanmic or large, so this is not a bad performance hit typically.
  //
  for (InterfaceIterator i = m_interfaces.begin(); i != end(); i++) {
    if (result == *i) {
      m_interfaces.erase(i);
      return result;
    }
  }
  //should not ever get here...but this is still the guy we are releasing.

  return result;
}
/*!
   Return a begin of iteration iterator through the interfaces.
   dereferencing this iterator will give an interface pointer.
*/
CCAMACSubsystem::InterfaceIterator
CCAMACSubsystem::begin()
{
  return m_interfaces.begin();
}
/*!
   Return an end of iteration iterator through the interfaces.
   this iterator is not meant to be dereferenced, only compared
   with other iterators that have been incremented to determine
   if iteration is complete.
*/
CCAMACSubsystem::InterfaceIterator
CCAMACSubsystem::end()
{
  return m_interfaces.end();
}
/*!
   Return the number of interfaces currently registered.
*/
size_t
CCAMACSubsystem::size()
{
  return m_interfaces.size();
}
/*!
   Index to a specific interface in the subsystem.
   \param index : size_t
       The interface number to retrieve.
   \return CCAMACInterface*
   \retval  The pointer to the interface stored at that index.
   \throw  CRangeError - if the index is out of range.
*/
CCAMACInterface*
CCAMACSubsystem::operator[](size_t index)
{
  if (index >= size()) {
    throw CRangeError(0, size()-1, index,
		      "Indexing a camac interface in CCAMACSubsystem::operator[]");
  }
  return m_interfaces[index];
}
/*!
  Create an interface based on a description and enter it into the
  subsystem.  
  \param description : string
      Describes the interface. See below for more information.

  The description must have the form:

   interface-type configuration

  where interface-type is a single 'word' that selects the type of interface
  to create and configuration is some interface specific configuration information.
  Example:

   cbd8210  vmecrate 0 branch 3

  - cbd8210  selects a CES CBD8210 camac branch highway driver 
  - the remainder of the line is configuration data interpreted by the creation
    of that interface (in this case saying the interface is in vme crate 0,
    and is set to be branch number 3).

    \return size_t
    \retval index assigned to the interface created.
    \throw  CInvalidInterfaceType - interface is not recognized by factory.

*/
size_t
CCAMACSubsystem::createInterface(string description)
{
  // Strip the crap off the description and extract the
  // type and configuration.

  string d = description;	// preserve description string.
  string type;
  string configuration;

  d = stripLeadingBlanks(d);
  d = stripComment(d);
  d = stripTrailingBlanks(d);

  type=  firstWord(d);
  configuration = d.erase(0, type.size());
  configuration = stripLeadingBlanks(configuration);

  // Now use the factory to create the interface:

  CCAMACInterfaceFactory* pFact = CCAMACInterfaceFactory::getInstance();
  CCAMACInterface*        pInt  = pFact->createInterface(type, configuration);

  if(!pInt) {
    throw CInvalidInterfaceType(description,
				"Creating a CAMAC interface in CCAMACSubsystem::createInterface");
  }
  return addInterface(*pInt);
}
/*!
   Create interfaces from a description file.  This is a front end to
   createInterface above.  We accept a file and line by line hand the
   non comment lines to createInterface.  All exceptions are passed back to the
   caller.
   Adding interfaces from file is an all or nothing thing.  Any errors
   rolls back the subsystem to its original state.
*/
void
CCAMACSubsystem::createInterfaces(istream& descriptionFile)
{

  string line;
  int    originalSize = size();
  try {
    while (!descriptionFile.eof()) {
      line = getLine(descriptionFile);
      line = stripComment(line);
      line = stripLeadingBlanks(stripTrailingBlanks(line));
      
      if (line.size() != 0) {
	createInterface(line);
      }
    }
  }
  catch (...) {
    // An exception fired.. remove/delete any interfaces
    // that were already added:

    while(size() != originalSize) {
      CCAMACInterface* p = m_interfaces.back();
      delete p;
      m_interfaces.pop_back();
    }
    throw;			// Let the caller do the detailed error handling.
  }
}

// This function is only exported to tests it allows tests to
// clear the contents of the subsystem... the caller is responsible
// for destroying the interfaces themselves.

void
CCAMACSubsystem::clearInterfaces()
{
  m_interfaces.clear();
}
