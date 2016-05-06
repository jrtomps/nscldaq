/******************************************************************************
**
**	Filename:	bt_name.c
**
**	Purpose:	NanoBus Linux Mirror API bt_gen_name() routine.
**
**	Functions:	bt_gen_name()
**
**      $Revision: 1.8 $
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
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.8 $" __DATE__;
#endif	/* LINT */

#include	<stddef.h>
#include	<string.h>
#include	<assert.h>

#include "btapi.h"
#include "btpiflib.h"


#ifdef FCTACH
/*****************************************************************************
**
**	Name:		bt_gen_name_from_cardno
**
**	Purpose:	Creates device name from card number
**
**	Args:
**	    cardno	Card number to open
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

char * bt_gen_name_from_cardno(
    int		cardno,		/* Which card number */
    char	*name_p,	/* Pointer to buffer to hold the name */
    size_t	max_len		/* Size of said buffer */
    )
{
    char	*retvalue = NULL;	/* Assume error return */

    char tmp_buf[4];    		/* Space for creating number */

    const char *dev_str_p = "/dev/" BT_DRV_NAME; /* Name of the device */
    /*  "/dev/sbsfctach" Linux FC Tachyon Name of the device */
    const int	dev_len = strlen(dev_str_p);
    int max_cards = (0x100 - (BT_MAX_UNITS << BT_DEV_SHFT));

    memset(name_p, '\0', max_len);

    if ((cardno < 0) || ((max_cards - 1) < cardno)) {
	    /* cardno is out of range of supported cardnos */
    	fprintf(stderr, "Cardno %d is out of range.\n", cardno);
    	goto bt_gen_name_end;
    }

    /* Create minor number portion of the name */
    sprintf(tmp_buf, "%d", cardno);

    /* +1 for terminating null char */
    if (max_len < (strlen(tmp_buf) + dev_len + 1)) {
    	/* Need a longer buffer than that! */
	    fprintf(stderr, "Device name %s%s longer than %d characters.",
	        dev_str_p, tmp_buf, max_len);
    	goto bt_gen_name_end;
    }

    (void) strcpy(name_p, dev_str_p);
    (void) strcat(name_p, tmp_buf);

    retvalue = name_p;

bt_gen_name_end:
     return retvalue;
}


/*****************************************************************************
**
**	Name:		bt_gen_name_from_devno_cardno
**
**	Purpose:	Creates device name from card number and unit number
**
**	Args:
**	    cardno	Card number to open
**	    devno	Device on card to open
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

char * bt_gen_name_from_devno_cardno(
    int		cardno,		/* Which card number */
    int		devno,		/* Which device number */
    char	*name_p,	/* Pointer to buffer to hold the name */
    size_t	max_len		/* Size of said buffer */
    )
{
    char	*retvalue = NULL;	/* Assume error return */

    char tmp_buf[4];    		/* Space for creating number */

    const char *dev_str_p = "/dev/" BT_DRV_NAME; /* Name of the device */
    /*  "/dev/sbsfctach" Linux FC Tachyon Name of the device */
    const int	dev_len = strlen(dev_str_p);
    int max_cards = (0x100 - (BT_MAX_UNITS << BT_DEV_SHFT));

    memset(name_p, '\0', max_len);

    if ((cardno < 0) || ((max_cards - 1) < cardno)) {
	    /* cardno is out of range of supported cardnos */
    	fprintf(stderr, "Cardno %d is out of range.\n", cardno);
    	goto bt_gen_name_end;
    }

    if ((devno <= 0) || (BT_MAX_UNITS < cardno)) {
        if (devno == 0) {
    	    /* devno zero is reserved for card only access */
        	fprintf(stderr, "Device number 0 is reserved for adapter only access.\n");
        } else {
    	    /* devno is out of range of supported cardnos */
        	fprintf(stderr, "Device number %d is out of range.\n", devno);
        }
    	goto bt_gen_name_end;
    }

    /* Create minor number portion of the name */
    sprintf(tmp_buf, "%d", (cardno + (devno << BT_DEV_SHFT)));

    /* +1 for terminating null char */
    if (max_len < (strlen(tmp_buf) + dev_len + 1)) {
    	/* Need a longer buffer than that! */
	    fprintf(stderr, "Device name %s%s longer than %d characters.",
	        dev_str_p, tmp_buf, max_len);
    	goto bt_gen_name_end;
    }

    (void) strcpy(name_p, dev_str_p);
    (void) strcat(name_p, tmp_buf);

    retvalue = name_p;

bt_gen_name_end:
     return retvalue;
}


#else /* FCTACH */
/* DATA BLIZZARD */


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
	    dev_str_p, &tmp_buf[0], (int) max_len);
#endif	/* defined(DEBUG) */
	goto bt_gen_name_end;
    }

    (void) strncpy(name_p, dev_str_p, max_len);
    (void) strncat(name_p, &tmp_buf[0], max_len - dev_len);

    retvalue = name_p;

bt_gen_name_end:
     return retvalue;
}
#endif /* FCTACH */

#undef	BUF_LEN
#undef	BUF_LIMIT
