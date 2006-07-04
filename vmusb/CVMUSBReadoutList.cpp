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

#include "CVMUSBReadoutList.h"

using namespace std;

// The following are bits in the mode word that leads off each stack line.
//

static const int modeAMMask(0x3f); // Address modifier bits.
static const int modeAMShift(0);

static const int modeDSMask(0xc0);
static const int modeDSShift(6);

static const int modeNW(0x100);
static const int modeNA(0x400);
static const int modeMB(0x800);
static const int modeSLF(0x1000);
static const int modeBE(0x10000);
static const int modeHD(0x20000);
static const int modeND(0x40000);
static const int modeHM(0x80000);
static const int modeNTMask(0xc00000);
static const int modeNTShift(22);
static const int modeBLTMask(0xf0000000);
static const int modeBLTShift(24);

/////////////////////////////////////////////////////////////////
//  Constructors and canonicals.

/*!
   Constructing a CVMUSBReadoutList is not really doing anything.
   the vector constructs itself by default.
*/
CVMUSBReadoutList::CVMUSBReadoutList()
{}

/*!
   Copy construction requires a copy construction of the list data.
   without this, the bit by bit copy would be very bad.
*/
CVMUSBReadoutList::CVMUSBReadoutList(const CVMUSBReadoutList& rhs) :
    m_list(rhs.m_list)
{
}
/*!
   Destruction is also a no-op but is provided for the sake of subclasses.
*/
CVMUSBReadoutList::~CVMUSBReadoutList()
{}

/*!
   Comparison is based on  comparison of the list contents.
*/
int
CVMUSBReadoutList::operator==(const CVMUSBReadoutList& rhs) const
{
    return m_list == rhs.m_list;
}
int
CVMUSBReadoutList::operator!=(const CVMUSBReadoutList& rhs) const
{
    return !(*this == rhs);
}

////////////////////////////////////////////////////////////////////
//    Operations on the whole of the list:

/*!
   Clear the list.
*/
void
CVMUSBReadoutList::clear()
{
    m_list.clear();
}
/*!
   return the size of the list in longwords.
*/
size_t
CVMUSBReadoutList::size() const
{
    return m_list.size();
}
/*!
     Return a copy of the list itself:
*/

vector<uint32_t>
CVMUSBReadoutList::get() const
{
   return m_list;
}
/////////////////////////////////////////////////////////////////////
//  register operations.

/*!
    Add a register read to a stack.  Register reads are just like
    single shot 32 bit reads but the SLF bit must be set in the 
    mode line.  I am assuming that we don't need stuff like the 
    DS bits either, as those are significant only for VME bus.
   \param address  : unsigned int
       Register address, see 3.4 of the VM-USB manual for a table
       of addresses.
*/
void
CVMUSBReadoutList::addRegisterRead(unsigned int address)
{
    uint32_t mode = modeNW  | modeSLF;
    m_list.push_back(mode);
    m_list.push_back(address);
	
}
/*!
   Add a register write to a stack. Register writes are the same
   as register reads except:
   - There is a data longword that follows the address.
   - The mode word does not have modeNW set.

   \param address : unsigned int
       Address of the register.  See VM-USB manual section 3.4 for
       a table of addresses.
   \param data    : uint32_t 
      The data to write.  Note that some registers are only 16 bits
      wide.  In these cases, set the low order 16 bits of the data.
*/
void
CVMUSBReadoutList::addRegisterWrite(unsigned int address,
				    uint32_t     data)
{
    uint32_t mode = modeSLF;
    m_list.push_back(mode);
    m_list.push_back(address);
    m_list.push_back(data);

}



