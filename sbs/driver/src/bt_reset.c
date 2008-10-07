/******************************************************************************
**
**	Filename:	bt_reset.c
**
**	Purpose:	NanoBus Linux Mirror API bt_reset routine.
**
**	Functions:	bt_reset()
**
**      $Revision: 742 $
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
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $" __DATE__;
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
**	Name:		bt_reset
**
**	Purpose:	Resets the unit.
**
**	Args:
**	    btd		Unit descriptor
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    BT_SUCCESS	on success
**	    otherwise	appropriate error number
**
**	Notes:
**
*****************************************************************************/

bt_error_t bt_reset(
    bt_desc_t	btd
    )
{
    bt_error_t  retvalue;
    bt_status_t param;

    retvalue = bt_ctrl(btd, BIOC_RESET, &param);
    if (param & BT_INTR_POWER) {
        retvalue = BT_ENOPWR;    
    } else if (param & ((bt_status_t) LSR_ERROR_MASK << BT_INTR_ERR_SHFT) ){
        retvalue = BT_ESTATUS;
    }

    return retvalue;
}
