/******************************************************************************
**
**	Filename:	bt_tas.c
**
**	Purpose:	NanBus UNIX Mirror API bt_tas() routine.
**
**	Functions:	bt_tas.c
**
**      $Revision: 1.7 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef	LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.7 $" __DATE__;
#endif	/* LINT */

#include    "btapi.h"
#include    "btpiflib.h"

/*****************************************************************************
**
**		External routines
**
*****************************************************************************/

/*****************************************************************************
**
**	Name:		bt_tas
**
**	Purpose:	Test and set a bit on the remot bus automatically.
**                      Act on a single byte (bt_data8_t) and only check
**                      the high-order bit of the byte.
**
**	Args:
**	    btd		Unit descriptor
**	    rem_off     Address of byte to test and set.         
**	    prevval_p   Pointer to a buffer which stores the bytes's value
**                      prior to the modification.
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    BT_SUCCESS	No errors
**	    BT_ESTATUS	If there are still status errors.
**	    BT_ENOPWR	Power is off or cable disconnected
**	    BT_IO	Could not query driver
**
**	Notes:
**
*****************************************************************************/

bt_error_t bt_tas(
    bt_desc_t	btd,
    bt_devaddr_t rem_off,
    bt_data8_t *prevval_p 
    )
{
    bt_error_t retvalue;
    bt_tas_t param;

    if (BT_DESC_BAD(btd)) {   /* Bad Descriptor */
       DBG_STR("bt_tas:bad descriptor.");
       retvalue = BT_EDESC;
       return retvalue;
    }

    param.error = BT_SUCCESS;
    param.addr = rem_off;


    retvalue = bt_ctrl(btd, BIOC_TAS, &param);
    if (BT_SUCCESS == retvalue) {
        DBG_STR("bt_tas:bt_ctrl BIOC_TAS failed.");
	retvalue = param.error;
    }


    /* return to the user the results */
    *prevval_p = param.result;

    return retvalue;
}
