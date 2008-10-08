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
static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";// Implements the CCamac class.  This class is composed soley of static 
// members.
//   See the CCamac.h file for more information.
//

#include <config.h>
#include "CCamac.h"
#include <RangeError.h>
#include <ErrnoException.h>
#include <CVMEInterface.h>


#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;



//
// Manifest constants used by this module;

static const size_t nBranchSize = 0x80000;     // !< '32' slots by 2048 F.A's.
static const off_t  BranchBase  = 0x800000;    //!< VME offset to branch 0.
static const int    BRANCHPERCRATE(8); //!< number of branches in a crate.

//  Provide and initialize the array holding the branch bases:
//  The number of branches is fixed by the CAMAC Hardware.

long* CCamac::m_BranchBases[MAX_BRANCHCOUNT] = {
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL 
};
void* CCamac::m_Fds[MAX_BRANCHCOUNT] = {
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL 
};

/*!
  Initialize access to a branch.  

  - If the associated BranchBases[] element is not null, this is a no-op.
  - If the associated BranchBases[] element is null, the vme device is opened
    and a mapping performed for the specified branch.  The mapping address
    is then stored in the BranchBases array.
  
  \param nBranch - Number of the branch to access.

   \exception CRangeError -  Thrown if nBranch is out of the range of legal
                          branch numbers.
   \exception CErrnoException - If the mapping failed for any reason.
   */
void
CCamac::BranchInit(int nBranch)
{

  // Throw if branch number is bad:

  if((nBranch < 0) || (nBranch >= MAX_BRANCHCOUNT)) {
    throw CRangeError(0, MAX_BRANCHCOUNT-1, nBranch,
		      "Mapping the branch address space");
  }

  // No need to map if this branch has already been mapped.

  if(m_BranchBases[nBranch] != (long*)NULL) return;

  // Open the VME device driver.

   void* nfd = CVMEInterface::Open(CVMEInterface::A24, nBranch/BRANCHPERCRATE);
   m_Fds[nBranch] = nfd;
  
   // Map the branch segment:
   
  m_BranchBases[nBranch] = (long*)CVMEInterface::Map((m_Fds[nBranch]), 
						     BranchBase + nBranch * nBranchSize, 
						     nBranchSize);
  
   
}

/*!
   Releases access to a map. If the branch has not been mapped, this
   is a no-op.  If the branch has been mapped, munmap is called as well as
   the VME_KILL_MMAP ioctl on the device driver.

   \param nBranch - Number of the branch to unmap.

   \exception CRangeError - If the nBranch parameter is out of range.
   \exception CErrnoException - If the one of the required system services
                              failed to return normal.

   */
void
CCamac::BranchRelease(int nBranch)
{
  // Throw if bad branch number:

if((nBranch < 0) || (nBranch >= MAX_BRANCHCOUNT)) {
   throw CRangeError(0, MAX_BRANCHCOUNT-1, nBranch,
		      "Attempting to release a crate");
}

  // If the map has already been made, we need to munmap:
  // Otherwise no action is required.
  if(m_BranchBases[nBranch]) {
   long* pBase = m_BranchBases[nBranch];
   m_BranchBases[nBranch] = (long*)NULL;

   CVMEInterface::Unmap(m_Fds[nBranch], pBase, nBranchSize);
     
  }

}
