/******************************************************************************
**
**	Filename:	bt_stat.c
**
**	Purpose:	Bit 3 NanoPort bt_stat routine.
**
**	Functions:	bt_status()
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
**	Name:		bt_status
**
**	Purpose:	gets the shadow value of the STATUS register
**
**	Args:
**	    btd		Unit descriptor
**          status_p	Pointer to internal driver status  
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

bt_error_t bt_status(
    bt_desc_t	btd,
    bt_data32_t *status_p
    )
{
    bt_status_t bt_stat = 0;

    bt_error_t retvalue;

    retvalue = bt_ctrl(btd, BIOC_STATUS, &bt_stat);
    if (bt_stat & BT_INTR_POWER) {
        retvalue = BT_ENOPWR;    
    } else if (bt_stat & ((bt_status_t) LSR_ERROR_MASK << BT_INTR_ERR_SHFT) ){
        retvalue = BT_ESTATUS;
    }
    
    /* return the status back to the caller */
    *status_p = (bt_data32_t) bt_stat;

    return retvalue;
}
