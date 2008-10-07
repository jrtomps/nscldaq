/******************************************************************************
**
**      Filename:       bt_mmap.c
**
**      Purpose:        IRIX implementation of mmap for NanoBus hardware.       
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $" __DATE__;
#endif  /* LINT */

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
 
#include "btapi.h"
#include "btio.h"
#include "btpiflib.h"



/******************************************************************************
**
**  Function:   bt_mmap()
**
**  Purpose:    Provide Mirror API access to device mmap() routine.
**
**  Args:
**      btd             Device descriptor returned by the call to bt_open
**      mmap_p          Address to hold memory map pointer is returned
**      map_off         Offset from beginning of logical device
**      map_len         Number of bytes to memory map into the application's
**                      memory space.
**      flags           Special flags indicating the read/write permissions
**                      for this memory mapped section.  Flags supported:
**                         BT_RD - allows reads from the mapped location
**                         BT_WR - allows writing from the mapped location
**                         BT_RDWR - allows read/write from mapped location 
**      swapping        Type of swapping to do for memory mapped access
**
**  Returns:
**      BT_SUCCESS      Everything went swell, region mapped successfully.
**      otherwise       Error value, mapping failed.
**
**  Comments:
**
******************************************************************************/

bt_error_t bt_mmap(
    bt_desc_t btd,                 /* descriptor  */
    void **mmap_p,                 /* address to return mmap ptr in */
    bt_devaddr_t map_off,          /* logical device addres to mmap */
    size_t map_len,                /* number of bytes to mmap       */
    bt_accessflag_t flags,         /* permissions for mapped region */
    bt_swap_t swapping)            /* swapping mode */
{
    int prot_flags = 0;             /* protection flags             */
    void *memory_p = NULL;
    int ret_val = BT_SUCCESS;
    size_t size_of_page;
    bt_devaddr_t req_offset;
    bt_devaddr_t req_length;
    bt_devdata_t save_swap;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        ret_val = BT_EDESC;
        goto bt_mmap_end;
    }
    if (BT_PTR_BAD(mmap_p, sizeof(void *))) {
        ret_val = BT_EINVAL;
        goto bt_mmap_end;
    }
        
    /* 
    ** verify R/W permissions against the open 
    */
    if ((( flags & BT_WR) && !(btd->access_flags & BT_WR))  ||
        (( flags & BT_RD) && !(btd->access_flags & BT_RD))) {
        ret_val = BT_EACCESS;
        goto bt_mmap_end;
    }

    /* 
    ** convert the mirror API access flags to LINUX mmap prot flags 
    */
    if( flags & BT_WR) {
        prot_flags |= PROT_WRITE;
    }
    if (flags & BT_WR) {
        prot_flags |= PROT_READ;
    }

    /*
    ** Align the mmap to a page boundary
    */
    size_of_page = getpagesize();
    req_offset = map_off - (map_off % size_of_page);
    req_length = map_len + (map_off % size_of_page);

    /*
    ** Set special swapping if requested
    */
    if (swapping != BT_SWAP_DEFAULT) {
        ret_val = bt_get_info(btd, BT_INFO_SWAP, &save_swap);
        if (ret_val != BT_SUCCESS) {
            goto bt_mmap_end;
        }
        ret_val = bt_set_info(btd, BT_INFO_SWAP, swapping);
        if (ret_val != BT_SUCCESS) {
            goto bt_mmap_end;
        }
    }

    /*
    ** Do the mmap
    */
    memory_p = (void *) mmap( 0,
                     req_length,  /* number of bytes to map           */
                     prot_flags,  /* memory protection                */
                     MAP_SHARED,  /* MAP_SHARED needed for PROT_WRITE */
                     btd->fd,     /* file descrpitor                  */
                     req_offset); /* starting point for map           */

    /*
    ** Restore special swapping if required
    */
    if (swapping != BT_SWAP_DEFAULT) {
        ret_val = bt_set_info(btd, BT_INFO_SWAP, save_swap);
        if (ret_val != BT_SUCCESS) {
            munmap(memory_p, req_length);
            goto bt_mmap_end;
        }
    }

    /*
    ** Adjust the mmap back to the original mmap request 
    */
    if (memory_p == MAP_FAILED) {
#if defined(BT965)
        ret_val = (bt_error_t) oserror();
#else /* BT965 */
        ret_val = (bt_error_t) errno;
#endif /* BT965 */
    } else {
        memory_p = (bt_data8_t *) memory_p + (map_off % size_of_page);
        *mmap_p = (void *) memory_p;
    }


bt_mmap_end:
    return ret_val;
}


/******************************************************************************
**
**  Function:   bt_unmmap()
**
**  Purpose:    Removes memory mapped access
**
**  Args:
**      btd             Device descriptor obtained from bt_open
**      mmap_p          Original memory map pointer returned by bt_mmap()
**      map_len         Number of bytes requested in origional bt_mmap() call
**
**  Returns:
**      BT_SUCCESS      Everything went swell
**      otherwise       Error value
**
**  Comments:
**
******************************************************************************/

bt_error_t bt_unmmap(
    bt_desc_t btd,                  /* descriptor from bt_open     */
    void *mmap_p,                   /* pointer from bt_mmap to unmap */
    size_t map_len                  /* number of bytes requested in map call */
    )
{
    int ret_val = BT_SUCCESS;
    size_t size_of_page;
    bt_devaddr_t req_length;
    bt_data8_t *req_p;
   
    /*
    ** Check for bad descriptors and pointers
    */
    if (BT_DESC_BAD(btd)) {
        ret_val = BT_EDESC;
        goto bt_mmap_end;
    }
    if (BT_PTR_BAD(mmap_p, sizeof(void *))) {
        ret_val = BT_EINVAL;
        goto bt_mmap_end;
    }

    /*
    ** Adjust the length based on whether we needed to
    ** adjust remote offset during the mmap call
    */
    size_of_page = getpagesize();
    req_p = ((bt_data8_t *) mmap_p - ((bt_devaddr_t) mmap_p % size_of_page));
    req_length = map_len + ((bt_devaddr_t) mmap_p % size_of_page);

    if (munmap((void *) req_p,  req_length)) {
#if defined(BT965)
        ret_val = (bt_error_t) oserror();
#else /* BT965 */
        ret_val = (bt_error_t) errno;
#endif /* BT965 */
    }

bt_mmap_end:
    return ret_val;
}
