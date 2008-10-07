/******************************************************************************
**
**	Filename:	bt_chke.c
**
**	Purpose:	NanoBus UNIX Mirror API bt_chkerr() routine.
**
**	Functions:	bt_chkerr.c
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
**	Name:		bt_chkerr
**
**	Purpose:	Checks for any errors.
**
**	Args:
**	    btd		Unit descriptor
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    BT_SUCCESS	No errors
**	    BT_ESTATUS	If there are any status errors.
**	    BT_ENOPWR	Power is off or cable disconnected
**	    BT_EPWRCYC	Power has transitioned from off to on
**	    BT_IO	Could not query driver
**
**	Notes:
**
*****************************************************************************/

bt_error_t bt_chkerr(
    bt_desc_t	btd
    )
{
    bt_error_t  retvalue;
    bt_status_t param;

    retvalue = bt_ctrl(btd, BIOC_STATUS, &param);
    if (param & BT_INTR_POWER) {
        retvalue = BT_ENOPWR;    
    } else if (param & ((bt_status_t) LSR_ERROR_MASK << BT_INTR_ERR_SHFT) ){
        retvalue = BT_ESTATUS;
    }
    return retvalue;
}
