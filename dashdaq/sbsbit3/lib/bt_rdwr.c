/******************************************************************************
**
**      Filename:       bt_rdwr.c
**
**      Purpose:        Mirror API implementation of read/write for 
**                      NanoBus/dataBlizzard hardware
**
**      Functions:      bt_read(), bt_write(), bt_hw_read(), bt_hw_write()
**
**      $Revision$
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
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$" __DATE__;
#endif /* LINT */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "btio.h"
#include "btapi.h"
#include "btpiflib.h"


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
    long        offset = xfer_addr;

    /*
    ** Check for bad descriptors or zero lengths
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        goto bt_read_end;
    }
    if (0 == req_len) {
        retvalue = BT_EINVAL;
        goto bt_read_end;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /* 
    ** Use lseek to position to the correct offset
    */
#if defined(BT1003)
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

bt_read_end:
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
    long        offset = xfer_addr;

    /*
    ** Check for bad descriptor or zero length
    */
    if (BT_DESC_BAD(btd)) {
        /* Bad descriptor */
        retvalue = BT_EDESC;
        goto bt_write_end;
    }
    if (0 == req_len) {
        /* Invalid to request zero bytes transferred */
        retvalue = BT_EINVAL;
        goto bt_write_end;
    }

    /*
    ** So far we have done zero bytes
    */
    if (NULL != xfer_len_p) {
        *xfer_len_p = 0;
    }

    /* 
    ** Use lseek to position to the correct offset
    */
#if defined(BT1003)
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

bt_write_end:
    return retvalue;
}

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

