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
#include "CVMESubsystem.h"
#include "CVMEInterface.h"


#include <RangeError.h>
#include <CInvalidInterfaceType.h>
#include <CVMEInterfaceFactory.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// Static class members:

bool CVMESubsystem::m_lockCreated(false);
int  CVMESubsystem::m_lockId;
bool CVMESubsystem::m_lockHeld(false);

/*!
   Construction of the VME subystem. This requires no action.
*/
CVMESubsystem::CVMESubsystem()
{
}

/*!
   Destruction requires that we destroy all elements
   that have been marked as must delete:
*/
CVMESubsystem::~CVMESubsystem()
{
  for(int i=0; i < m_Interfaces.size(); i++) {
    if (m_Interfaces[i].s_mustDelete) {
      delete m_Interfaces[i].s_pInterface;
    }
      
    
  }
}
/*!
   Install a new interface.  A crate number is allocated and returned
   for the interface that is added.
   \param interface : CVMEInterface& [in]
      A reference to an object derived from the abstract base class
      CVMEInterface of all VME interfaces.
   \param deleteWhenDone : bool [in];  defaults to: false
      If this is true, then when the VME Subsystem no longer needs
      the interface, it will be deleted.  This happens when:
      - The VME subsystem is destroyed.
      - The interface is replaced via a call to replaceInterface.
   \return int
   \retval Number to use to retrieve this VME crate from the subsystem.
*/
int
CVMESubsystem::installInterface(CVMEInterface& interface,
				bool deleteWhenDone)
{
  Interface i = {deleteWhenDone, &interface};
  m_Interfaces.push_back(i);

  return (m_Interfaces.size() - 1);
}

/*!
   Replace an existing interface with another one.
   \param index : unsigned int [in]
       Index of the interface to replace. 
   \param replacement : CVMEInterface& 
       The new interface that will take over that crate.
   \param deleteWhenDone : bool, defaults to false
       Determines if the interface is deleted when the subsystem
       no longer needs it.
   \return CVMEInterface*
   \retval The previous interface for this index.  Note that
           if the interface was added with deleteWhenDone true,
           this will point to an object that has been deleted.
           If you want to do any pre-replacement cleanup you should
	   get hold of the interface prior to this call by e.g.
           indexing do the cleanup and then do the replacement.
*/
CVMEInterface*
CVMESubsystem::replaceInterface(unsigned int   index,
				CVMEInterface& replacement,
				bool           deleteWhenDone)
{
  // Make sure the index is in range.

  if (index >= m_Interfaces.size()) {
    throw CRangeError(0, m_Interfaces.size()-1, index,
		      "CVMESubsystem::replaceInterface - index to replace");
  }
  // Get the old interface record by reference so we can modify it:

  Interface&     old(m_Interfaces[index]);

  /// Take care of the old interface:

  CVMEInterface* pPrior  = old.s_pInterface;

  if (old.s_mustDelete) {
    delete pPrior;		// If here prior is now invalid!!!!!!!!
  }
  // replace with the new

  old.s_pInterface = &replacement;
  old.s_mustDelete = deleteWhenDone;

  //// Warning pPrior may be invalid!!!!

  return pPrior;
}

/*!

   \return size_t
   \retval Number of interfaces that are currently installed.
*/
size_t
CVMESubsystem::size() const
{
  return m_Interfaces.size();
}
/*!
   Return a begin of iteration iterator to the interfaces.
   The iterator can be treated as a pointer to a structure
   that contains the following elements:
   - s_mustDelete - true if the interface is deleted after
                    being used by the subsystem (see installInterface).
   - s_pInterface - Pointer to the CVMEInterface* object.
   
   The iterator can be incremented or decremented.  If after an
   increment it is equal to the value returned by end(), you've run off
   the end of the array.
   \return InterfaceIterator
*/
CVMESubsystem::InterfaceIterator
CVMESubsystem::begin()
{
  return m_Interfaces.begin();
}
/*!
    Returns an end of iteration iterator.  For more on interators,
    see the Musser, Derge, Saini book on the Standard Template Library.
    See also begin earlier in this file.
*/
CVMESubsystem::InterfaceIterator
CVMESubsystem::end()
{
  return m_Interfaces.end();
}

/*!
   Index into the subsystems and return a reference to the 
   interface.  Note that many interfaces don't support copy construction.
   Furthermore you don't want to deal with slicing down to the
   base class which, after all is abstract.
   Thus the only ways to safely index are:
   - subsystem[index].method();  Invoke a method from the indexed element
     directly
   - CVMEInterface& interface(subsystem[index]);
   - CVMEInterface* pInterface = &(subsystem[index]);
   
   \param index : unsigned int [in]
        Number of VME crate for which you want an interface object reference.
   \return CVMEInterface&
   \retval A reference to the selected interface.  See above for
           safe ways to use this.
   \throw CRangeError - if the index is not in range.
*/
CVMEInterface& 
CVMESubsystem::operator[](unsigned int index)
{
  // range check the index:

  if(index >= size()) {
    throw CRangeError(0, size()-1, index,
		      "CVMESubsystem::[] index is too large");
  }
  return *(m_Interfaces[index].s_pInterface);
}
 

/*!
    Process an interface description line to create a new interface.
    The interface type is the first word extracted from the
    description, the remainder is considered to be configuration.

    \param description std::string [in]
       Description string of the form "type device dependent configuration"
    \return int
    \retval Crate number assigned to this interface.

    \throw CInvalidInterfaceType   if the factory returned a null.
          
*/
int
CVMESubsystem::processDescription(string description)
{
  CVMEInterface* pInterface = CVMEInterfaceFactory::create(description);
  if (!pInterface) {
    throw CInvalidInterfaceType(description, 
	       string("CVMESubsystem::processDescription Creating an interface"));
  }

  return installInterface(*pInterface, true);
  
}
