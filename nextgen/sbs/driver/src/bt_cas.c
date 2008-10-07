/******************************************************************************
**
**	Filename:	bt_cas.c
**
**	Purpose:	NanBus UNIX Mirror API bt_cas() routine.
**
**	Functions:	bt_cas.c
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
#include	"btpiflib.h"

/*****************************************************************************
**
**		External routines
**
*****************************************************************************/

/*****************************************************************************
**
**	Name:		bt_cas
**
**	Purpose:	Does a compare and swap atomic transaction of the 
**                      remote bus.
**
**	Args:
**	    btd		Unit descriptor
**	    rem_off     Address of the memory location to check.
**	    cmpval      Compare with memory value, if a match swap_value 
**                      is written to this location.
**	    Swap_val    If cmpval and the memory location match, this value
**                      is written to the location.
**	    prevval_p   Holds the previous value of the memory location.
**          data_size   Size of the memory location to look at.
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

bt_error_t bt_cas(
    bt_desc_t	btd,
    bt_devaddr_t rem_off,
    bt_data32_t cmpval,
    bt_data32_t swapval,
    bt_data32_t *prevval_p,
    size_t  data_size
    )
{
    bt_error_t retvalue;
    bt_cas_t param;

    if (BT_DESC_BAD(btd)) {   /* Bad Descriptor */
       DBG_STR("bt_cas:bad descriptor.");
       retvalue = BT_EDESC;
       return retvalue;
    }

    param.error = BT_SUCCESS;
    param.addr = rem_off;
    param.cmp = cmpval;
    param.swap = swapval;
    param.len = data_size;


    retvalue = bt_ctrl(btd, BIOC_CAS, &param);
    if (BT_SUCCESS == retvalue) {
        DBG_STR("bt_cas:bt_ctrl BIOC_CAS call failed.");
	retvalue = param.error;
    }

    /* return to the user the results */
    *prevval_p = param.result;

    return retvalue;
}
