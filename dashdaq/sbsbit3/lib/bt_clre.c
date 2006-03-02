/******************************************************************************
**
**	Filename:	bt_clre.c
**
**	Purpose:	NanBus UNIX Mirror API bt_clrerr() routine.
**
**	Functions:	bt_clrerr.c
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef	LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$" __DATE__;
#endif	/* LINT */

#include	"btio.h"
#include	"btapi.h"

/*****************************************************************************
**
**		External routines
**
*****************************************************************************/

/*****************************************************************************
**
**	Name:		bt_clrerr
**
**	Purpose:	Clears any error status in the device.
**
**	Args:
**	    btd		Unit descriptor
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

bt_error_t bt_clrerr(
    bt_desc_t	btd
    )
{
    bt_error_t retvalue;
    bt_status_t param;

    retvalue = bt_ctrl(btd, BIOC_CLR_STATUS, &param);
    if (param & BT_INTR_POWER) {
        retvalue = BT_ENOPWR;    
    } else if (param & ((bt_status_t) LSR_ERROR_MASK << BT_INTR_ERR_SHFT) ){
        retvalue = BT_ESTATUS;
    }

    return retvalue;
}
