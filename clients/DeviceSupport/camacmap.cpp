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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
** camacmap.c:
**   Contains daq interfaces to the Bit3/CBD8210 branch highway device.
**   Access to the CAMAC crate is via pointer dereferences into a virtual
**   address space which maps to the CAMAC space as follows:
**     base + |bbb|ccc|nnnnn|aaaa|fffff|w|0
**   (each letter in the set above is a bit) where:
**     bbb   - Is the branch to reference
**     ccc   - Is a crate within the branch (note that the VME controller
**             registers live in crate 0.
**     nnnnn - Is the CAMAC slot.
**     aaaa  - Is the subaddress within the slot.
**     fffff - Is the function code to execute in the selected module.
**     w     - Is 1 for word accesses, and 0 is for longword (24 bit) access.
**
** Author:
**     Ron Fox
**     NSCL
**     Michigan State University
**     East Lansing, MI 48824-1321
**     (c) Copyright NSCL 1999 All rights reserved.
**
** Acknowledgements:
**    Natalie Kruszinska at Nikhef who wrote the Bit 3 device driver,
**    Eric Kasten at NSCL who made it work and gave me code samples on which
**                to base this file.
*/
/*
   $Header$

   Change log:

   $Log$
   Revision 8.3  2006/01/05 17:30:31  ron-fox
   Work around some issues with rethrowing exceptions across constructors
   in shared objects...bouncing back and forth.  Exceptions caught by
   ReadoutStateMachine are now reported at that level before being rethrown.

   Revision 8.2  2005/06/24 11:30:38  ron-fox
   Bring the entire world onto the 8.2 line

   Revision 4.2  2004/11/16 15:24:48  ron-fox
   - Port to the gnu 3.x compiler set.
   - Integrate buid of tests.
   - Integrate build of docos.

   Revision 1.2  2004/11/16 15:23:29  ron-fox
   - Port -> gcc/g++ 3.x
   - Support integrated test building.
   - Support integrated doxygen docu7mentation building.

   Revision 1.1  2003/12/03 19:29:05  ron-fox
    Add generic CAMAC support to the library.

   Revision 3.1  2003/03/22 04:03:46  ron-fox
   Added SBS/Bit3 device driver.

   Revision 2.1  2003/02/11 16:44:58  ron-fox
   Retag to version 2.1 to remove the weird branch I accidently made.

   Revision 1.2  2003/02/05 18:06:17  ron-fox
   Catch up on drift between Readout and the snapshot from which we started
   the port to autotools.

   Revision 2.7  2002/12/30 18:46:57  fox
   Support the WIENER VC/CC32 crate controller hardware thanks to
   Dave Caussyn at FSU (caussyn@nucmar.physics.fsu.edu)

   Revision 2.6  2002/11/20 16:03:09  fox
   Support multiple VME crates and CAMAC Branches spread across the multiple VME
   crates.

   Revision 2.5  2002/10/22 12:37:23  fox
   Straighten out dates in copyright notices in files.

// Revision 2.4  2002/10/10  12:54:25  fox
// Remove multiple copyright strings.
//
// Revision 2.3  2002/10/09  11:27:46  fox
// Add copyright/license stamp.
//
// Revision 2.2  2002/07/02  15:12:23  fox
// Go to 2.xx based releases (recover from client crash too).
//
// Revision 2.1  2002/07/02  15:05:34  fox
// Transition to 2.1 releases
//
// Revision 1.1  2002/07/01  17:37:01  fox
// - Made camacmap.c ->camacmap.cpp
// - The world use CVMEInterface:: rather than direct device access to abstract
//   VME crate access.
//
*/
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <math.h>
#include <assert.h>
#include <CVMEInterface.h>
#include <camac.h>
#include <Iostream.h>

#include <CVMEInterface.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#define VMECRATES      16
#define BRANCHESINCRATE 8 


#define DEF_SIGNAL SIGUSR1
#define DEF_VECTOR -1
#define DEF_IRQ    2


/* This stuff is all pretty obsolete I'm guessing.  - RF */

#define PAGE_SIZE 4096
#define SLOT_SIZE 2048
#define SLOTS_PER_CRATE 25
#define PAGES_PER_CRATE ((SLOTS_PER_CRATE*SLOT_SIZE)/PAGE_SIZE) 
#define BRANCHES        8
#define CRATES_PER_BRANCH 8


static char *pCopyright = "(c) NSCL 1999, All rights reserved\n";

 
/*===================================================================*/
void*
daq_OpenCamacDriver()
{
  /*  Opens the CAMAC device.  The user can use the fd returned by
  **  this call to to call the other functions in this module:
  **  
  ** Returns:
  **     -1            - IF failure, error in errno.
  **    otherwise      - the file id open on the device.
  */

  return CVMEInterface::Open(CVMEInterface::A24);
}


/*===================================================================*/
void*
daq_MapCamacSpace(void* fid)
{
  /*  Maps the entire CAMAC branch highway space and returns a pointer
  **  to it to the caller.  The branch highway space is assumed to consist
  **  of 8 branches with 8 crates all located  contiguously in VME space.
  **  NOTE:  It will be the responsibility of the user of this spaced to
  **  ensure that the branches and crates referenced actually exist.
  **
  ** Formal Parameters:
  **    int fid  [in]:
  **       File id open on the VME A24/D16 device, returned from a call to
  **       daq_OpenCamacDriver().
  ** Returns:
  **   Pointer to the CAMAC highway space Virtual address or NULL if
  **   the map failed.
  */
   void *mbuf;
   unsigned long base;

   base = (1<<23);		/* This is the entire branch base. */

   base = (base&(~(PAGE_SIZE-1))); /* Probably  not needed for full br map */

   mbuf = CVMEInterface::Map(fid, base,
			     (unsigned long)(BRANCHES*CRATES_PER_BRANCH* 
					     PAGE_SIZE*PAGES_PER_CRATE));
   return(mbuf);  
}

/*===================================================================*/
void
daq_UnmapCamacSpace(void* fd, void* pSpace)
{
  /* Unmaps the CAMAC address space mapped with daq_MapCamacSpace
  ** the mapping is assumed to be the entire size of the CAMAC space 
  ** which is BRANCHES*CRATES_PER_BRANCH contiguous crates.
  **
  ** Formal Parameters:
  **
  **   int   fd [in]:
  **      File descriptor open on the vme driver.
  **   void* pSpace [in]:
  **      Pointer returned from daq_MapCamacSpace.
  */

  CVMEInterface::Unmap(fd, pSpace,
			    (unsigned long)(BRANCHES*CRATES_PER_BRANCH* 
					    PAGE_SIZE*PAGES_PER_CRATE));
				    	

}
/*===================================================================*/
void
daq_CloseCamacDriver(void* fid)
{
  /*  Closes the file id open on the CAMAC device.
  **  
  **  int fid [in]:
  **     File id returned from the call to daq_OpenCamacDriver.
  */

  CVMEInterface::Close(fid);
}

/*!
     Access a branch.. This function ensures that a branch of the
     CAMAC system is mapped to process virtual memory.  This function maintains
     set of 16*8 pointers to branch bases in the pBranchBases array.  If the
     pointer is null, a branch is mapped, otherwise, this is a no-op.

     \param pBranch (int [in]):  Branch number to map.

     \note Each vme crate can accept at most 8 branches. Therefore
     the branch number is really a VMEcrate/branch:
     VMEcrate*8+branchincrate

 */


static void*   vmeFds[VMECRATES];
       void*   pBranchBases[VMECRATES*BRANCHESINCRATE];  // static init to NULL
void CESBranchAccess(int nBranch)
{
  int nVme    = nBranch >> 3;	// Extract the VME crate and the
  int nB      = nBranch & 0x7;  // branch inside the crate.

  // Note that failures in the map are probably unrecoverable.  So we
  // allow the exceptions signalled to propagate back up the call stack
  // where hopefully smoeone has installed a last chance handler prior to exit.

  assert((void*)NULL == (void*)0);		// Assumption about init values of ptr array.
  assert(nBranch < VMECRATES*BRANCHESINCRATE);  // in range branch.

  //  If the pointer does not exist, then it needs to be mapped:

  if(pBranchBases[nBranch] == (void*)NULL) {
    if (vmeFds[nVme] == (void*) NULL) {	// Need to open the device:
      vmeFds[nVme] = CVMEInterface::Open(CVMEInterface::Standard, nVme);
    }
    // Map the memory:
    
    try { 

      long nBase = 0x800000 + (nB <<CAMB);
      pBranchBases[nBranch] = CVMEInterface::Map(vmeFds[nVme], nBase,
					       (1L << CAMB));
    }
    catch (...) {
      cerr << "CVMEInterface::Map threw... rethrowing\n";
      throw;
    }
  }
}
/*!
   Map to a WIENER crate controller.  These take up 0x20 << CAMN bytes of
   storage.  We organize the WIENER crates so that a branch is one crate. 
   This allows 8 WIENER crates to be controlled from one vme crate which,
   for now, is probably sufficient.  If we need to do more, we need to
   do something more intelligent with the mapping of 
   crates/branches/vme crates.  So crates 0-7 are in VME crate 0 etc.

*/
void WIENERBranchAccess(int nBranch)
{
  int nVme        =  nBranch >> 3;	// 8 crates == crates / vme crate.
  int nController =  nBranch & 0x7;     // This is the crate #.

  assert ((void*) NULL == (void*)0);    // Assume the pointers init to null.
  assert(nBranch < VMECRATES*BRANCHESINCRATE);

  // Map the crate if the pointer is still NULL:

  if(pBranchBases[nBranch] == (void*)NULL) {
    if(vmeFds[nVme] == (void*)NULL) { // Need to open the device...
      vmeFds[nVme]  = CVMEInterface::Open(CVMEInterface::Standard, nVme);
    }
  
    // Map the memory for a crate...
    // The crates are at : 0x800000 + ((nB *  0x20) << CAMN)
    
    long nBase = 0x800000 + ((nController * 0x20) << CAMN);
    pBranchBases[nBranch] = CVMEInterface::Map(vmeFds[nVme], nBase,
					       0x20 << CAMN);
  }
}

   
