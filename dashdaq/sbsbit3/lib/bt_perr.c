/******************************************************************************
**
**	Filename:	bt_perr.c
**
**	Purpose:	Generic Mirror API bt_perror() routine.
**
**	Functions:	bt_perror
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


#include	<stdio.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>

#include	"btapi.h"


/*****************************************************************************
**
**	Name:		bt_perror
**
**	Purpose:	Prints a descriptive error message to stderr given
**			the Mirror API error number.
**
**	Args:
**	    btd		Unit descriptor
**	    err		Error status value
**	    msg_p	Message header to print out
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    0		No errors
**	    otherwise	
**			Exit value.
**
**	Notes:
**
*****************************************************************************/

#if defined(_WIN32) /* The __stdcall calling convention is used to call Win32 API functions. */
BT_EXTERN bt_error_t _stdcall bt_perror(
#else
bt_error_t bt_perror(
#endif
    bt_desc_t btd,		/* Unit descriptor */
    bt_error_t errval,	/* Error value */
    const char *msg_p	/* Message to print out first */
    )
{
#define	MAX_LINE	200

    char	buffer[MAX_LINE];

    if (NULL != msg_p) {
	fprintf(stderr, "%s: ", msg_p);
    }

    if (NULL == bt_strerror(btd, errval, NULL, &buffer[0], MAX_LINE)) {
        if ('\0' == buffer[0]) {
	    fprintf(stderr, "ERROR: Couldn't get message for error %d.\n",
		(int) errval);
	} else {
	    fprintf(stderr, "ERROR: Partial message for error %d: %s.\n",
		(int) errval, &buffer[0]);
	}
	return BT_ENOMEM;	/* Insufficient resources */
    }

    fprintf(stderr, "%s.\n", &buffer[0]);

    return BT_SUCCESS;
}
