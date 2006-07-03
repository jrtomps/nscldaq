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

#ifndef __CVMUSBREADOUTLIST_H
#define __CVMUSBREADOUTLIST_H



#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif


/*!
   The best way to use the VM-USB involves building lists of VME
   operations, called \em stacks.  These stacks can either be submitted
   for immediate execution or stored inside the VM-USB for triggered
   execution.  In this way, several events will be autonomously handled
   by the VM-USB with no computer intervention.

   This class allows application programs to painlessly build a stack.
   Stacks are built up by creating an instance of this class, 
   invoking menber functions  to add list elements, and then 
   passing the list to either CVMUSB::loadList or CVMUSB::executeList

   There is nothing sacred about a list with respect to copy construction,
   assignment, or comparison.  Those operations are simply delegated to 
   member data.

   \note Not all VMUSB list operations are supported by this class.

*/

class CVMUSBReadoutList
{
private:
    std::vector<uint32_t> m_list; // Stack lines are all 32 bits wide.
public:
    CVMUSBReadoutList();
    CVMUSBReadoutList(const CVMUSBReadoutList& rhs);
    virtual ~CVMUSBReadoutList();

    CVMUSBReadoutList& operator=(const CVMUSBReadoutList& rhs);
    int operator==(const CVMUSBReadoutList& rhs) const;
    int operator!=(const CVMUSBReadoutList& rhs) const;


    // Operations on the list as a whole:

    void                  clear();
    size_t                size() const;
    std::vector<uint32_t> get()  const;

    // Register operations 

public:
    void addRegisterRead(unsigned int address);
    void addRegisterWrite(unsigned int address, uint32_t data);
};


#endif
