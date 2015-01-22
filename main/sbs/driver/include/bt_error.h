/*****************************************************************************
**
**      Filename:   bt_error.h
**
**      Purpose:    include file for mirror API error definitions.        
**
**      $Revision: 1.4 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000-04 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
******************************************************************************/
#if !defined (_BT_ERROR_H)
#define _BT_ERROR_H

#if defined(__vxworks)
#include <stdio.h>
#endif /* defined(__vxworks) */

/* Begin btnbus.h diff */

#define	BT_M_btp	(0x7fff<<16)	/* Assumed module number    */
#if	!defined(M_btp)                 /* from 993 btpdef.h  CCC   */
#define	M_btp	BT_M_btp
#endif	/* !defined(M_btp) */

/******************************************************************************
**
**	Define error numbers    (CCC from 993 btngpci.h)
**
** In the cases where there is an errno defined by Standard C or POSIX that
** is the equivilent to our error return, we would like to use it.
******************************************************************************/

#if	defined(__vxworks)
#define	POSIX_EQUIV(BT_ENO, ENO)	BT_ENO = ENO
#define	BT_ESTART	M_btp

#elif   defined(__winxp__)
#define POSIX_EQUIV(BT_ENO, ENO)        BT_ENO

#elif   defined(__sun)
#define	POSIX_EQUIV(BT_ENO, ENO)	BT_ENO
#define	BT_ESTART	0x1000

#elif   defined(__sgi)
#define	POSIX_EQUIV(BT_ENO, ENO)	BT_ENO
#define	BT_ESTART	0x1000

#elif   defined(__linux__)
#define	POSIX_EQUIV(BT_ENO, ENO)	BT_ENO
#define	BT_ESTART	0x1000

#elif   defined(__lynxos)
#define	POSIX_EQUIV(BT_ENO, ENO)	BT_ENO
#define	BT_ESTART	0x1000

#else	/* unknown operating system */
#error Unknown Operating System!
#endif	/* defined(__vxworks) */


typedef enum {   /* from 993 btngpci.h  */
    BT_SUCCESS = 0,			/* Everything is just swell */
    POSIX_EQUIV(BT_EIO, EIO),		/* Input or output error */
    POSIX_EQUIV(BT_ENXIO, ENXIO),	/* Non-existent device */
    POSIX_EQUIV(BT_ENOMEM, ENOMEM),	/* Out of memory or other resource */
    POSIX_EQUIV(BT_EINVAL, EINVAL),	/* Invalid parameter */
    POSIX_EQUIV(BT_EACCESS, EACCES),	/* Access failed: permission denied */
    POSIX_EQUIV(BT_EDESC, EBADF),	/* Invalid descriptor */
    POSIX_EQUIV(BT_ENOSUP, ENOSYS),	/* Option not supported on this */
    					/* particular implementation. */

    POSIX_EQUIV(BT_EABORT, EINTR),	/* Interrupted a system call */

#define BT_EINTR BT_EABORT              /* Obsolete form */

    POSIX_EQUIV(BT_ESYSMAX, BT_ESTART), /* past last POSIX defined error */

    BT_EFAIL,		/* Something went wrong. Generic error. */
    BT_ENORD,           /* Device does not support reads */
    BT_ENOWR,           /* Device does not support writes */
    BT_ESTATUS,		/* Status register error value */
    BT_ENOPWR,          /* Power is off or cable is disconnected */
    BT_EPWRCYC,         /* Power was cycled */
    BT_EBUSY,		/* Timer expired before a resource became available. */
    BT_ELCARD,          /* Local card failed diagnostics */
    BT_ECABLE,          /* Interconnection cable failed diagnostics */
    BT_ERCARD,          /* Remote card failed diagnostics */
    BT_EPAIR,           /* The pair of adapter cards failed diagnostics */
    BT_EEXCEPT,         /* OS exception error */
    BT_EEVT_NOT_REGISTERED, /* enviorment not registered */
    BT_ENOT_FOUND,      /* Attempt to acquire or remove an unknown event */
    BT_ENOT_LOCKED,     /* attempt to unlock a unlocked unit */ 
    BT_ENOT_TRANSMITTER, /* Remote access requested on a unit that is not a transmitter */
    BT_EPCI_CONFIG,     /* Error reading or writting PCI configuration registers */
    BT_ELOCAL,          /* Attempt to perform TAS or CAS on local device */
    BT_EUNKNOWN_REMID,  /* Remote adaptor not supported */
    BT_ENO_UNIT,        /* Unit does not exist */
    BT_ENO_MMAP,        /* Logical device does not support bt_mmap() */
    BT_EHANDLE,         /* Attempt to get unique signal handle failed */

    BT_MAX_ERR		/* Last Error number */
} bt_error_t;


#endif  /* _BT_ERROR_H */
