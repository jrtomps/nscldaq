/******************************************************************************
**
**	Filename:	bt_locking.c
**
**	Purpose:	Next Generation API routines for exclusive access.
**
**	Functions:	bt_lock(), bt_unlock()
**
**	$Revision$
**
******************************************************************************/
/*****************************************************************************
**
**		Copyright (c) 2000 by SBS Technologies, Inc.
**		          All Rights Reserved.
**		   License governs use and distribution.
**
*****************************************************************************/

#ifndef	LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$" __DATE__;
#endif	/* LINT */

#include	"btapi.h"
#include	"btpiflib.h"


/*****************************************************************************
**
**	Name:		bt_lock
**
**	Purpose:	Obtains exclusive access to the unit
**
**	Args:
**	    btd		Device Descriptor
**	    wait_len	Time in mSec to wait for the device lock
**	    
**	Returns:
**	    BT_SUCCESS	Always
**
**	Notes:
**	Hardware does not require any locking.
**
*****************************************************************************/

bt_error_t bt_lock(
    bt_desc_t   btd,		/* Device Descriptor */
    bt_msec_t	wait_len	/* time in mSec to wait for lock */
    )
{

    return BT_SUCCESS;
}


/*****************************************************************************
**
**	Name:		bt_unlock
**
**	Purpose:	Releases exclusive access to the unit
**
**	Args:
**	    btd		Device Descriptor
**	    
**	Returns:
**	    BT_SUCCESS	Always
**
**	Notes:
**	Hardware does not require any locking.
**
*****************************************************************************/

bt_error_t bt_unlock(
    bt_desc_t   btd		/* Device Descriptor */
    )
{
    return BT_SUCCESS;
}
