/******************************************************************************
**
**	Filename:	bt_or_io.c
**
**	Purpose:	NanBus UNIX Mirror API bt_or_io() routine.
**
**	Functions:	bt_or_io.c
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
**	Name:		bt_or_io
**
**	Purpose:	One shots a value into a register.   Bitwise ORs the
**                      value with the current value of the register and    
**                      writes that value to the register.
**
**	Args:
**	    device      Logical device handle returned from bt_open()
**	    reg         Register to read.                        
**          value       value to write to the register.         
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

bt_error_t bt_or_io(
    bt_desc_t	device,
    bt_reg_t    reg,
    bt_data32_t value
    )
{
    bt_ioaccess_t bt_reg_io;
    bt_error_t retvalue = BT_SUCCESS;

    if (BT_DESC_BAD(device)) {   /* Bad Descriptor */
       DBG_STR("bt_or_io:bad descriptor.");
       retvalue = BT_EDESC;
       return retvalue;
    }

   bt_reg_io.error = BT_SUCCESS;
   bt_reg_io.offset = reg;
   bt_reg_io.operation = BT_IOSHOT;
   bt_reg_io.value = value;

    retvalue = bt_ctrl(device, BIOC_IOREG, &bt_reg_io);

    return retvalue;
}
