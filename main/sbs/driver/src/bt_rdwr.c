/******************************************************************************
**
**      Filename:       bt_rdwr.c
**
**      Purpose:        Mirror API implementation of read/write for 
**                      NanoBus/dataBlizzard hardware
**
**      Functions:      bt_read(), bt_write(), bt_hw_read(), bt_hw_write()
**
**      $Revision: 1.12 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.12 $" __DATE__;
#endif /* LINT */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#if defined (__sun)
#include <sys/time_impl.h>
#endif /* defined(__sun) */

#if defined (BT951)
#include <ipc.h>
#include <sem.h>
#else
#include <sys/ipc.h>
#include <sys/sem.h>
#endif /* BT951 */

#if !defined(__linux__)
#include <semaphore.h>
#endif /* __linux__ */

#include "btapi.h"
#include "btpiflib.h"

extern int semaphore_create( key_t key );
extern int semaphore_delete( int sem_id );
extern int semaphore_lock( int sem_id );
extern int semaphore_unlock( int sem_id );

#if defined(BT965)
static size_t bt_bcopy(bt_data8_t *src_p, bt_data8_t *dst_p, size_t length, int data_width, int src_bus);
#endif /* BT965 */

extern pthread_mutex_t bt_lib_mutex_g;
extern int bt_sema_id_gp;

#if defined(__sun)
extern ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
extern ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
#endif /* defined(__sun) */


/*****************************************************************************
**
**      Name:           bt_read
**
**      Purpose:        Transfers data from local device card to user's 
**                      buffer.
**
**      Args:
**          btd         Unit descriptor
**          buffer_p    Address of user data buffer
**          xfer_addr   Address to transfer data to
**          req_len     Amount we were requested to transfer
**          xfer_len_p  Actual amount transferred
**
**      Modifies:
**          xfer_len_p  Actual amount transferred without error
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_read(
    bt_desc_t   btd,
    void *      buffer_p,
    bt_devaddr_t xfer_addr,
    size_t      req_len,
    size_t      *xfer_len_p)
{
    bt_error_t  retvalue = BT_SUCCESS;
    ssize_t     xfer_done;
#if defined(BT1003) || defined(FCTACH) || defined (BT951)
#else
    long        offset = xfer_addr;
    void        *mmap_p = NULL;
#endif /* BT1003 */

    /*
    ** Check for bad descriptors or zero lengths
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        return retvalue;
    }
    if (0 == req_len) {
        retvalue = BT_EINVAL;
        return retvalue;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /* 
    ** Grab the named semaphore
    */
    if (semaphore_lock(bt_sema_id_gp) == -1) {
        perror("Read semaphore lock failed");
        retvalue = BT_EINTR;
        return retvalue;
    }

#if defined(BT965)
    /*
    ** Do pio if misaligned
    */
    if (!IS_D32_ALIGNED(buffer_p) ||
        !IS_D32_ALIGNED(xfer_addr) ||
        !IS_D32_ALIGNED(req_len) ||
        (req_len < 0xa4)) {

        /*
        ** mmap the requested remote address
        */
        retvalue = bt_mmap(btd, &mmap_p, xfer_addr, req_len, BT_RDWR, BT_SWAP_DEFAULT);
        if (retvalue != BT_SUCCESS) {
            goto bt_read_end;
        }

        /*
        ** Do the PIO transfer 
        */
        xfer_done = bt_bcopy((bt_data8_t *) mmap_p, (bt_data8_t *) buffer_p, req_len, BT_WIDTH_ANY, FALSE);
        (void) bt_unmmap(btd, mmap_p, req_len);
        if (xfer_done != req_len) {
            retvalue = BT_EIO;
        }
        if (NULL != xfer_len_p) {
            *xfer_len_p = xfer_done;
        }
        goto bt_read_end;

    /* 
    ** Otherwise use the driver read function for DMA
    */
    } else {
#endif /* BT965 */

        /* 
        ** Use lseek to position to the correct offset
        */
#if defined(BT1003) || defined(FCTACH) || defined (BT951)
        if (lseek(btd->fd, xfer_addr, SEEK_SET) == -1) {
            retvalue = BT_EINVAL;
            goto bt_read_end;
        }

        /*
        ** Do the actual xfer, using pread to combine lseek and read
        */
        xfer_done = read(btd->fd, buffer_p, req_len);
#else /* BT1003 */
        xfer_done = pread(btd->fd, buffer_p, req_len, offset);
#endif /* BT1003 */
        if (xfer_done <= 0) {
            *xfer_len_p = 0;
#if defined(BT965)
            retvalue = (bt_error_t) oserror();
#else /* BT965 */
            retvalue = (bt_error_t) errno;
#endif /* BT965 */
        }
        if (xfer_done > 0) {
            *xfer_len_p = xfer_done;
            if (xfer_done == req_len) {
                retvalue = BT_SUCCESS;
            } else {
#if defined(BT965)
                retvalue = (bt_error_t) oserror();
#else /* BT965 */
                retvalue = (bt_error_t) errno;
#endif /* BT965 */
            }
        }
#if defined(BT965)
    }
#endif /* BT965 */

bt_read_end:
    /* 
    ** Release the named semaphore
    */
    if (semaphore_unlock(bt_sema_id_gp) == -1) {
        perror("Read semaphore unlock failed\n");
        retvalue = BT_EINTR;
    }
    return retvalue;
}

/*****************************************************************************
**
**      Name:           bt_write
**
**      Purpose:        Transfers data to local device card from user's 
**                      buffer.
**
**      Args:
**          btd         Unit descriptor
**          buffer      Address of user data buffer
**          xfer_addr   Address to transfer data to
**          req_len     Amount we were requested to transfer
**          xfer_len_p  Actual amount transferred
**
**      Modifies:
**          xfer_len_p  Actual amount transferred without error
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_write(
    bt_desc_t   btd,
    void *      buffer_p,
    bt_devaddr_t xfer_addr,
    size_t      req_len,
    size_t      *xfer_len_p)
{
    bt_error_t  retvalue = BT_SUCCESS;
    ssize_t     xfer_done;
#if defined(BT1003) || defined(FCTACH) || defined (BT951)
#else
    long        offset = xfer_addr;
    void        *mmap_p = NULL;
#endif /* BT1003 */

    /*
    ** Check for bad descriptor or zero length
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        return retvalue;
    }
    if (0 == req_len) {
        /* Invalid to request zero bytes transferred */
        retvalue = BT_EINVAL;
        return retvalue;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /* 
    ** Grab the named semaphore
    */
    if (semaphore_lock(bt_sema_id_gp) == -1) {
        perror("Write semaphore lock failed");
        retvalue = BT_EINTR;
        return retvalue;
    }

#if defined(BT965)
    /*
    ** Do pio if misaligned
    */
    if (!IS_D32_ALIGNED(buffer_p) ||
        !IS_D32_ALIGNED(xfer_addr) ||
        !IS_D32_ALIGNED(req_len) ||
        (req_len < 0xa4)) {

        /*
        ** mmap the requested remote address
        */
        retvalue = bt_mmap(btd, &mmap_p, xfer_addr, req_len, BT_RDWR, BT_SWAP_DEFAULT);
        if (retvalue != BT_SUCCESS) {
            goto bt_write_end;
        }

        /*
        ** Do the PIO transfer 
        */
        xfer_done = bt_bcopy((bt_data8_t *) buffer_p, (bt_data8_t *) mmap_p, req_len, BT_WIDTH_ANY, FALSE);
        (void) bt_unmmap(btd, mmap_p, req_len);
        if (xfer_done != req_len) {
            retvalue = BT_EIO;
        }
        if (NULL != xfer_len_p) {
            *xfer_len_p = xfer_done;
        }
        goto bt_write_end;

    /* 
    ** Otherwise use the driver write function for DMA
    */
    } else {
#endif /* BT965 */

        /* 
        ** Use lseek to position to the correct offset
        */
#if defined(BT1003) || defined(FCTACH) || defined (BT951)
        if (lseek(btd->fd, xfer_addr, SEEK_SET) == -1) {
            retvalue = BT_EINVAL;
            goto bt_write_end;
        }

        /*
        ** Actually do the xfer, using pwrite to do lseek and write
        */
        xfer_done = write(btd->fd, buffer_p, req_len); 
#else /* BT1003 */
        xfer_done = pwrite(btd->fd, buffer_p, req_len, offset);
#endif /* BT1003 */

        if (xfer_done <= 0) {
            *xfer_len_p = 0;
#if defined(BT965)
            retvalue = (bt_error_t) oserror();
#else /* BT965 */
            retvalue = (bt_error_t) errno;
#endif /* BT965 */
        }
        if (xfer_done > 0) {
            *xfer_len_p = xfer_done;
            if (xfer_done == req_len) {
                retvalue = BT_SUCCESS;
            } else {
#if defined(BT965)
                retvalue = (bt_error_t) oserror();
#else /* BT965 */
                retvalue = (bt_error_t) errno;
#endif /* BT965 */
            }
        }
#if defined(BT965)
    }
#endif /* BT965 */

bt_write_end:
    /* 
    ** Release the named semaphore
    */
    if (semaphore_unlock(bt_sema_id_gp) == -1) {
        perror("Write semaphore unlock failed\n");
        retvalue = BT_EINTR;
    }
    return retvalue;
}

#ifndef FCTACH
/*****************************************************************************
**
**      Name:           bt_hw_read
**
**      Purpose:        Transfers data from logical device to bus memory.
**
**      Args:
**          btd         Unit descriptor
**          bus_addr    Physical bus address
**          xfer_addr   Address to transfer data to
**          req_len     Amount we were requested to transfer
**          xfer_len_p  Actual amount transferred
**
**      Modifies:
**          xfer_len_p  Actual amount transferred without error
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_hw_read(
    bt_desc_t btd,
    bt_devaddr_t bus_addr,
    bt_devaddr_t xfer_addr,
    size_t req_len,
    size_t *xfer_len_p)
{
    bt_error_t      retvalue = BT_SUCCESS;
    bt_hw_xfer_t    hw_xfer;

    /*
    ** Check for bad descriptors or zero lengths
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        goto bt_hw_read_end;
    }
    if (0 == req_len) {
        retvalue = BT_EINVAL;
        goto bt_hw_read_end;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /*
    ** Do the actual xfer, using pread to combine lseek and read
    */
    hw_xfer.local_hw_addr = (bt_data32_t) bus_addr;
    hw_xfer.ldev_addr = (bt_data32_t) xfer_addr;
    hw_xfer.xfer_len = (bt_data32_t) req_len;
    retvalue = bt_ctrl(btd, BIOC_HW_READ, &hw_xfer);
    if (NULL != xfer_len_p) {
        *xfer_len_p = hw_xfer.xfer_len;
    }
      
bt_hw_read_end:
    return retvalue;
}

/*****************************************************************************
**
**      Name:           bt_hw_write
**
**      Purpose:        Transfers data to logical device from bus memory.
**
**      Args:
**          btd         Unit descriptor
**          bus_addr    Physical bus address
**          xfer_addr   Address to transfer data to
**          req_len     Amount we were requested to transfer
**          xfer_len_p  Actual amount transferred
**
**      Modifies:
**          xfer_len_p  Actual amount transferred without error
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_hw_write(
    bt_desc_t btd,
    bt_devaddr_t bus_addr,
    bt_devaddr_t xfer_addr,
    size_t req_len,
    size_t *xfer_len_p)
{
    bt_error_t  retvalue = BT_SUCCESS;
    bt_hw_xfer_t    hw_xfer;

    /*
    ** Check for bad descriptors or zero lengths
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        goto bt_hw_write_end;
    }
    if (0 == req_len) {
        retvalue = BT_EINVAL;
        goto bt_hw_write_end;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /*
    ** Do the actual xfer, using pread to combine lseek and read
    */
    hw_xfer.local_hw_addr = (bt_data32_t) bus_addr;
    hw_xfer.ldev_addr = (bt_data32_t) xfer_addr;
    hw_xfer.xfer_len = (bt_data32_t) req_len;
    retvalue = bt_ctrl(btd, BIOC_HW_WRITE, &hw_xfer);
    if (NULL != xfer_len_p) {
        *xfer_len_p = hw_xfer.xfer_len;
    }
    
bt_hw_write_end:
    return retvalue;
}
#endif  /* FCTACH */

#if defined(BT965)
/******************************************************************************
**
**      Function:       bt_bcopy()
**
**      Purpose:        Replaces system bcopy() routine.  Assumes that
**                      either src_p or dst_p point at Bus memory and
**                      uses the PIO_READ_*() or PIO_WRITE_*() macros
**                      to read or write PCI memory.
**
**      Args:           unit_p          Pointer to device unit structure.
**                      src_p           Address to transfer data from.
**                      dst_p           Address to transfer data to.
**                      length          Amount of data to transfer.
**                      data_width      Data transfer width (i.e. D8/D16/D32).
**                      src_bus         TRUE if src_p points at PCI memory,
**                                      FALSE if dst_p points at PCI memory
**
**      Modifies:       None
**
**      Returns:        Length actually transferred.
**
**      Calls:          None
**
**      Notes:          Assumes either src_p or dst_p, but not both, point
**                      at bus memory.  If neither does, use the old bt_bcopy()
**                      routine.  If both do, write a new routine.
**
**                      Based off the bt_bcopy routine found in bt_datio.c
**                      version 5.15.
** 
**                      Must support a native 64-bit type (bt_data64_t).
**
******************************************************************************/

static size_t bt_bcopy(
    bt_data8_t    *src_p, 
    bt_data8_t    *dst_p, 
    size_t length, 
    int data_width,
    int src_bus)
{
    register volatile bt_data8_t    *sb_p, *db_p;
    register volatile bt_data16_t   *sw_p, *dw_p;
    register volatile bt_data32_t   *sl_p, *dl_p;
    register volatile bt_data64_t   *sll_p, *dll_p;
    register unsigned long          count;
    unsigned long                   length_xferred;
    unsigned long                   tot_length_xferred = 0;


    /* btk_pio_xfer() already set data_width to BT_WIDTH_D64 (BT_NEXT_GEN) or */
    /* BT_WIDTH_D32 (~BT_NEXT_GEN).  Other callers will use BT_WIDTH_D32:     */ 
    if (data_width == BT_WIDTH_ANY) {
	data_width = BT_WIDTH_D32;
    }

    /* While we have not fully completed requested data transfer */
    while (length > 0) {
    
        /* D64 with sufficient length, long aligned source and destination */
        if ((data_width >= BT_WIDTH_D64) && (length >= BT_WIDTH_D64) &&
             IS_D64_ALIGNED((bt_data64_t)src_p | (bt_data64_t)dst_p)) {

            /* Load up long word pointers to source and destination */
            sll_p = (volatile bt_data64_t *) src_p;
            dll_p = (volatile bt_data64_t *) dst_p;

            /*
            ** Calculate the transfer count.
            **
            ** Assume nominal case is where transfer completes.
            */
            count = length / BT_WIDTH_D64;
            length_xferred = count * BT_WIDTH_D64;

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            while (count > 0) {

                /*
                ** NOTE: do not change to D64++ or the powerPC will
                ** core dump.
                */
                *dll_p = *sll_p;
                dll_p += 1;
                sll_p += 1;
                count--;
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D64;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D32 with sufficient length, long aligned source and destination */
        } else if ((data_width >= BT_WIDTH_D32) && (length >= BT_WIDTH_D32) &&
             IS_D32_ALIGNED((bt_data64_t)src_p | (bt_data64_t)dst_p)) {

            /* Load up long word pointers to source and destination */
            sl_p = (volatile bt_data32_t *) src_p;
            dl_p = (volatile bt_data32_t *) dst_p;

            /*
            ** Calculate the transfer count.
            **
            ** Assume nominal case is where transfer completes.
            */

            count = length / BT_WIDTH_D32;
            length_xferred = count * BT_WIDTH_D32;

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            while (count > 0) {

                *dl_p++ = *sl_p;
                sl_p++;
                count--;
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D32;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D16 with sufficient length, word aligned source and destination */
        } else if ((data_width >= BT_WIDTH_D16) && (length >= BT_WIDTH_D16) &&
                   IS_D16_ALIGNED((bt_data64_t)src_p | (bt_data64_t)dst_p)) {

            /*
            ** Differentiate between the requirement that we use D16
            ** versus an alignment issue to get back to using longs.
            **
            ** Assume the nominal case is where transfer completes.
            */
            if (data_width == BT_WIDTH_D16 ||
                (((bt_data64_t) dst_p ^ (bt_data64_t) src_p) & (BT_WIDTH_D32-1)) != 0) {
                count = length / BT_WIDTH_D16;
                length_xferred = count * BT_WIDTH_D16;
            } else {
                count = 1;
                length_xferred = BT_WIDTH_D16;
            }

            /* Load up word pointers to source and destination */
            sw_p = (volatile bt_data16_t *) src_p;
            dw_p = (volatile bt_data16_t *) dst_p;

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            while (count > 0) {

                *dw_p++ = *sw_p;
                sw_p++;
                count--;
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D16;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D8 with sufficient length, byte aligned source and destination */
        } else {

            /*
            ** Differentiate between the requirement that we use D8 vs
            ** an alignment issue to get back to using words or longs.
            ** Assume the nominal case is where the transfer completes.
            */

            if (data_width == BT_WIDTH_D8 ||
                (((bt_data64_t) dst_p ^ (bt_data64_t) src_p) & (BT_WIDTH_D16-1)) != 0) {
                length_xferred = count = length;
            } else {
                length_xferred = count = BT_WIDTH_D8;
            }

            /* Load up byte pointers to source and destination */
            sb_p = (volatile bt_data8_t *) src_p;
            db_p = (volatile bt_data8_t *) dst_p;

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */
            while (count > 0) {

                *db_p++ = *sb_p;
                sb_p++;
                count--;
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count but avoid use of a run-time multiply to get it.
            */
            if (count) {
                length_xferred -= count;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;
        }
    }

    return((long) tot_length_xferred);
}
#endif /* BT965 */
