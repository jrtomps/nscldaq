/******************************************************************************
**
**	Filename:	bt_name.c
**
**	Purpose:	NanoBus Linux Mirror API bt_gen_name() routine.
**
**	Functions:	bt_gen_name()
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

#include	<stddef.h>
#include	<string.h>
#include	<assert.h>

#include	"btapi.h"
#include	"btio.h"


/*****************************************************************************
**
**	Name:		bt_gen_name
**
**	Purpose:	Creates device name from logical device type and
**			unit number.
**
**	Args:
**	    unit	Unit number to open
**	    logdev	Logical device type
**	    name_p	Buffer to stuff name into
**	    max_len	Maximum number of characters in name
**
**	Modifies:
**	    name_p
**	    
**	Returns:
**	    NULL	Error
**	    Otherwise	Pointer to beginning of string name, will be same as
**			value of name_p.
**
**	Notes:
**	Make sure you don't overflow the buffer the user passes in. Right now,
**	the routine is doing redundent checks for this.
**
*****************************************************************************/

char * bt_gen_name(
    int		unit,		/* Which physical unit number */
    bt_dev_t	logdev,		/* Which logical device to access */
    char	*name_p,	/* Pointer to buffer to hold the name */
    size_t	max_len		/* Size of said buffer */
    )
{
    char	*retvalue = NULL;	/* Assume error return */

#define	BUF_LEN	6			/* Number of characters for minor number */
#define	BUF_LIMIT 100000		/* Maximum value +1 that fits in buffer */
    char tmp_buf[BUF_LEN];		/* Space for creating number */

    const char *dev_str_p = "/dev/" BT_DRV_NAME; /* Name of the device */
/*  const char *dev_str_p = "/dev/btq"; CAR Solaris Name of the device */
    const int	dev_len = strlen(dev_str_p);


    memset(name_p, '\0', max_len);

    if ((unit < 0) || (BT_MAX_UNITS < unit)) {
	/* unit is out of range of supported units */
#if	defined(DEBUG)
	fprintf(stderr, "Unit %d is out of range.\n", unit);
#endif	/* defined(DEBUG) */
	goto bt_gen_name_end;
    }

    if ((logdev < BT_MIN_DEV) || (logdev >= BT_MAX_DEV)) {
	/* Invalid logical device */
#if	defined(DEBUG)
	fprintf(stderr, "Logical device %d is out of range.\n", (int) logdev);
#endif	/* defined(DEBUG) */
	goto bt_gen_name_end;
    }

    /* No good way to check that minor number will fit in buffer. */
    assert(((BT_MAX_DEV<<BT_DEV_SHFT)+BT_MAX_UNITS) < BUF_LIMIT);

    /* Create minor number portion of the name */
    sprintf(&tmp_buf[0], "%-1d", (logdev << BT_DEV_SHFT) + unit);

    /* +1 for terminating null char */
    if (max_len < (strlen(&tmp_buf[0]) + dev_len +1)) {
	/* Need a longer buffer than that! */
#if	defined(DEBUG)
	fprintf(stderr, "Device name %s%s longer than %d characters.",
	    dev_str_p, &tmp_buf[0], max_len);
#endif	/* defined(DEBUG) */
	goto bt_gen_name_end;
    }

    (void) strncpy(name_p, dev_str_p, max_len);
    (void) strncat(name_p, &tmp_buf[0], max_len - dev_len);

    retvalue = name_p;

bt_gen_name_end:
     return retvalue;
}
#undef	BUF_LEN
#undef	BUF_LIMIT
