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

#ifndef __CCAMAC_H
#define __CCAMAC_H


/*!
   Responsible for maintaining the CAMAC map.  CCamac maintains a static array 
   of branch highway base addresses, and uses them to minimize CAMAC mapping
   requirements.  Static member functions allow branches to be accessed
   and deaccessed.

   */

static const int MAX_BRANCHCOUNT = 8*32; //!<  Max Number of branches in system.
class CCamac
{
  // Private member data:

   static long* m_BranchBases[MAX_BRANCHCOUNT]; //!< Array of branch base pointers. 
   static void* m_Fds[MAX_BRANCHCOUNT];         //!< Array of device fds.

public:  
  static void BranchInit(int nBranch);
  static void BranchRelease(int nBranch);

  //!  Retrieves the base address of a branch.. Null if not mapped.

  static long* Base(int nBranch) {
    return m_BranchBases[nBranch];
  }

};


#endif
