/******************************************************************************
**
**	Filename:	bt_clre.c
**
**	Purpose:	NanBus UNIX Mirror API bt_clrerr() routine.
**
**	Functions:	bt_clrerr.c
**
**      $Revision: 1.8 $
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
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.8 $" __DATE__;
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
#ifdef FCTACH
    retvalue = BT_SUCCESS;
#else
    bt_status_t param = 0;

    retvalue = bt_ctrl(btd, BIOC_CLR_STATUS, &param);
    if (param & BT_INTR_POWER) {
        retvalue = BT_ENOPWR;    
    } else if (param & ((bt_status_t) LSR_ERROR_MASK << BT_INTR_ERR_SHFT) ){
        retvalue = BT_ESTATUS;
    }
#endif

    return retvalue;
}
