/******************************************************************************
**
**	Filename:	bt_ior.c
**
**	Purpose:	NanoBus Linux Mirror API bt_get_io() routine.
**
**	Functions:	bt_get_io()
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

#include	<stddef.h>
#include	<stdlib.h>

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
**	Name:		bt_get_io
**
**	Purpose:	Read a device register.
**
**	Args:
**	    btd		Unit descriptor
**	    reg		Enumeration specifying which register to access.
**	    value_p	Pointer to location to store the value read.
**
**	Modifies:
**	    value_p->	Set to value read from register.
**
**	Returns:
**	    0		Success
**	    Otherwise	bt_error_t value
**
**	Notes:
**
*****************************************************************************/

bt_error_t bt_get_io(
    bt_desc_t	btd, 
    bt_reg_t	reg, 
    bt_data32_t  *value_p
    )
{
    bt_ioaccess_t bt_reg_io;

    bt_error_t retvalue = BT_SUCCESS;

    if (BT_DESC_BAD(btd)) {
	/* Bad Descriptor */
        DBG_STR("bt_get_io:bad descriptor.")
 	retvalue = BT_EDESC;
	return retvalue;
    }

    bt_reg_io.error = BT_SUCCESS;
    bt_reg_io.offset = reg;
    bt_reg_io.operation = BT_IOREAD;

    retvalue = bt_ctrl(btd, BIOC_IOREG, &bt_reg_io);
    if (BT_SUCCESS == retvalue) {
        *value_p = bt_reg_io.value;
    }

    return retvalue;
}
