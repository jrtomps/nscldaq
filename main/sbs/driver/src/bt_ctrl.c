/******************************************************************************
**
**      Filename:       bt_ctrl.c
**
**      Purpose:        Mirror API implementation of bt_ctrl().       
**
**      Functions:      bt_ctrl()
**
**      $Revision: 1.9 $
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
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.9 $" __DATE__;
#endif /* LINT */

#include <stddef.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>

#if defined(__vxworks)
#include <ioLib.h>
#endif /* defined __vxworks */

#if defined(__sun)
#include <unistd.h>
#include <stropts.h>
#endif /* defined(__sun) */

#include    "btapi.h"
#include    "btpiflib.h"

/*****************************************************************************
**
**              External routines
**
*****************************************************************************/

/*****************************************************************************
**
**      Name:           bt_ctrl
**
**      Purpose:        Access to miscellaneous device functions.
**
**      Args:
**          btd         Unit descriptor
**          ctrl        What control function to access
**          param_p     Function specific parameter
**
**      Modifies:
**          None
**          
**      Returns:
**          BT_SUCCESS  on success
**          BT_ENOSUP   Legacy ioctl()'s not supported
**          otherwise   appropriate error number
**
**      Notes:
**
*****************************************************************************/

bt_error_t bt_ctrl(
    bt_desc_t   btd,
    int ctrl,
    void *param_p)
{
    bt_error_t retvalue = BT_SUCCESS;
    bt_data32_t *status_val = param_p;  /* BIOC_ error value is 32-bit value */

    bool_t error_member_exists = (0 != (_IOC_NR(ctrl) & BT_IO_ERET_FLAG));

    /* 
    ** Check for bad descriptor 
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_ctrl:bad descriptor.");
        retvalue = BT_EDESC;
        goto ctrl_end;
    }

    /*
    ** Initialize return value
    */
    if (error_member_exists) {   /* 1 = first member arg holds error value */
        *status_val = BT_SUCCESS; /* Initialize status member */
    }

    /*
    ** Do the ioctl
    */

#if defined (__vxworks)    
    /* arg 3 of ioctl for vxwork is an int */
    if (-1 == (retvalue = ioctl(btd->fd, ctrl, (int) param_p))) {
#else 	    
    if (-1 == (retvalue = ioctl(btd->fd, ctrl, (void *) param_p))) {
#endif /* defined (__vxworks) */    

        switch (errno) {
          case EBADF:
            retvalue = BT_EDESC;
            break;
          case ENOTTY:
            retvalue = BT_ENOSUP;
            break;
          case BT_EINVAL:
          case BT_ENXIO:
            retvalue = errno;
            break;
          default:
            retvalue = BT_EFAIL;
            break;
        }
    }

    /* 
    ** Replace return value if ioctl() worked and first member 
    ** arg holds error value 
    */
    if ( (error_member_exists) && (BT_SUCCESS == retvalue)) {
        retvalue = *status_val;
    }
         
ctrl_end:
    return retvalue;
}
