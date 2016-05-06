/******************************************************************************
**
**      Filename:   bt_info.c
**
**      Purpose:    Next Generation Mirror API semaphore routines.
**
**      Functions:  bt_take_sema(), bt_give_sema()
**
**      $Revision: 1.2 $
**
******************************************************************************/
/******************************************************************************
**
**        Copyright (c) 2000-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
******************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.2 $ "__DATE__;
#endif  /* !LINT */

/*
**  Include files
*/
#include <stddef.h>
#include <unistd.h>

#include "btapi.h"
#include "btpiflib.h"



/******************************************************************************
**
**      Function:       bt_take_sema()
**
**      Purpose:        Attempt to get the givien semaphore.
**
**      Args:
**          btd         Unit descriptor
**          sema        Which semaphore to try and take
**          timeout     Number of millisecond to wait for semaphore.
**                        BT_NO_WAIT is only value guaranteed to be supported
**
**      Modifies:
**
**      Returns:
**          BT_SUCCESS  Successful.
**          BT_EBUSY    Semaphore was allready taken.
**          else        error number.
**
**      Note:
**          Nested calls to bt_take_sema() are not supported.
**
******************************************************************************/

bt_error_t bt_take_sema(
    bt_desc_t btd, 
    bt_reg_t sema, 
    bt_msec_t timeout
    )

{
    bt_sema_access_t    bt_sema;
    bt_error_t          retvalue;

    if (BT_DESC_BAD(btd)) {
        /* Bad Descriptor */
        retvalue = BT_EDESC;
        return retvalue;
    }

    bt_sema.error = (bt_data32_t) BT_SUCCESS;
    bt_sema.sema_reg = (bt_data32_t) sema;                               
    bt_sema.wait_msec = (bt_data32_t) timeout;                               

    retvalue = bt_ctrl(btd, BIOC_SEMA_TAKE, &bt_sema);
    if (BT_SUCCESS == retvalue) {
        retvalue = bt_sema.error;
    } 

    return retvalue;
}

/******************************************************************************
**
**      Function:       bt_give_sema()
**
**      Purpose:        Release the given semaphore.
**
**      Args:
**          btd         Unit descriptor
**          sema        Which semaphore to try and take
**
**      Modifies:
**
**      Returns:
**          BT_SUCCESS  Successful.
**          else        error number.
**
**      Note:
**          Nested calls to bt_give_sema() are not supported.
**          bt_give_sema() should only be called after a successful call to
**          bt_take_sema().
**
******************************************************************************/

bt_error_t bt_give_sema(
    bt_desc_t btd, 
    bt_reg_t sema 
    )

{
    bt_sema_access_t    bt_sema;
    bt_error_t          retvalue;

    if (BT_DESC_BAD(btd)) {
        /* Bad Descriptor */
        retvalue = BT_EDESC;
        return retvalue;
    }

    bt_sema.error = (bt_data32_t) BT_SUCCESS;
    bt_sema.sema_reg = (bt_data32_t) sema;                               

    retvalue = bt_ctrl(btd, BIOC_SEMA_GIVE, &bt_sema);
    if (BT_SUCCESS == retvalue) {
        retvalue = bt_sema.error;
    } 

    return retvalue;
}
