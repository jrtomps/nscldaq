/******************************************************************************
**
**	Filename:	bt_send_irq.c
**
**	Purpose:	NanBus UNIX Mirror API bt_send_irq() routine.
**
**	Functions:	bt_send_irq.c
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
**	Name:		bt_send_irq
**
**	Purpose:	Sends a programmed interrupt to the remote bus.   
**
**	Args:
**	    device         logical devie handle returned from bt_ipen().
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

bt_error_t bt_send_irq(
    bt_desc_t	device    
    )
{
    bt_error_t retvalue = BT_SUCCESS;
    bt_error_t errorval = BT_SUCCESS;

    if (BT_DESC_BAD(device)) {   /* Bad Descriptor */
       DBG_STR("bt_send_irq:bad descriptor.");
       retvalue = BT_EDESC;
       return retvalue;
    }

    retvalue = bt_ctrl(device, BIOC_SND_INTR,NULL);
    if (BT_SUCCESS != retvalue) {
        DBG_STR("bt_send_irq:bt_ctrl BIOC_SND_INTR call failed.");
	retvalue = errorval;
    }

    return retvalue;
}
