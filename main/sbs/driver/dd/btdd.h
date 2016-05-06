/*****************************************************************************
**
**      Filename:   btdd.h
**
**      Purpose:    Shared driver private header file
**
**      Macros, Prototypes & Functions: 
**      BTK_BZERO, BTK_GET_PROC, BTK_REL_PROC
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997 - 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _BTDD_H
#define _BTDD_H

#define EAS_BIND_CODE   1
#define EAS_A64_CODE    1

/*
** Note:        This file includes the header file (btxdd.h) for the specific
**              driver after defining some data structures and macros shared
**              by several drivers.
*/

/*****************************************************************************
**
**      Header files included
**
*****************************************************************************/

#if	defined(__linux__)

#include    <linux/types.h>
#include	<linux/version.h>


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#include	<asm/semaphore.h>
#else 
#include <linux/semaphore.h>
#endif

#include	<linux/timer.h>

#elif     defined(__unix) || defined(__vxworks) || defined(__lynxos) || \
          defined(__sun)
#include        <sys/types.h>
#endif  /* defined(__unix) || defined(__vxworks) || defined(__lynxos) || */
        /* defined(__sun) */

#if defined (BT_NTDRIVER)

#if defined(BT_WDM_DRIVER)
    /* We're a new WDM driver and not an old NT driver */
#include <wdm.h>
#else /* BT_WDM_DRIVER */
    /* We're an old NT driver */
#include <ntddk.h>
#endif /* BT_WDM_DRIVER */

#elif defined (__sun)
#include <sys/debug.h>  /* For ASSERT macro */

#elif defined (__vxworks)
#include <semLib.h>      /* For mutex and event code */
#include <wdlib.h>        
#include <syslib.h>      /* for sysClkRateGet */
#include <logLib.h>      /* for logMsg() */
#include <stdio.h>       /* needed for fprintf() calls */
#include <string.h>      /* for bzero */


#elif defined (BT_uCOS)
#include <../sys/ucos/includes.h>

#endif  /* OS dependent header file inclusions */

#if   defined(BT984)
#define MIN_SIZE_MAPPING_BAR_0          0x200  // Min Size of BAR0 required mapping
#endif


/*****************************************************************************
**  Generic macros that may be useful to all device driver platforms.
*****************************************************************************/

/* BT_BCD4 converts value to a four digit Binary Coded Decimal value which is */
/* quite useful when invoked as BT_BCD4(__LINE__) in hardware tracing macros. */
#ifndef BT_BCD4
#define BT_BCD4(value)  ((value) / 1000 % 10 << 12 \
                        | (value) / 100 % 10 << 8 \
                        | (value) / 10 % 10 << 4 \
                        | (value) % 10)
#endif

/*****************************************************************************
**
**      Definition of BTK_LOCK_ISR & BTK_UNLOCK_ISR
**
** These macros are meant to provide a means of locking out the driver's 
** interrupt service rouitne.  Currently they provide a system defined return
** type BTK_LOCK_RETURN_T that is required on vxWorks and sgi.  As  
** platforms are implemented, if a return value is not necessary 
** BT_LOCK_RETURN_T should be defined as int and not used during the 
** BTK_LOCK/UNLOCK_ISR macros.  It has not been determined that the 
** BTK_LOCK_RETURN_T will meet the needs of all systems, therefore, this 
** type and these macros may need to be modified in the future, but this 
** is a start for now.
**
*****************************************************************************/
#if defined (_AIX)
#error ISR locking not defined for AIX

#elif defined (__hpux)
#error ISR locking not defined for HP-UX

#elif defined (__sgi)
#define BTK_LOCK_RETURN_T               int
#define BTK_LOCK_ISR(unit_p, curr_pl)   curr_pl = (LOCK(&(unit_p->isr_lock),plhi))
#define BTK_UNLOCK_ISR(unit_p, new_pl)  (UNLOCK(&(unit_p->isr_lock), new_pl))

#elif defined (__sun)
#define BTK_LOCK_RETURN_T               int
#define BTK_LOCK_ISR(unit_p, curr_pl)   mutex_enter(&unit_p->isr_lock);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  mutex_exit(&unit_p->isr_lock);

#elif defined (__vxworks)

/*
** Locking interrupts is a more efficient way of protecting a critical 
** section that task locking since interurpt lockint just involves hardare 
** manipulation and task unlocking requires a pass through the rescheduling 
** code.  But interrupt locking is dangerous. If you make a mistake with 
** interrupt locked, you hang your processor. 
*/
#define BTK_LOCK_RETURN_T                int
#define BTK_LOCK_ISR(unit_p, lock_key)   lock_key = (intLock())
#define BTK_UNLOCK_ISR(unit_p, lock_key) (intUnlock((lock_key)))

#elif defined (BT_uCOS)
#error ISR locking not defined for UCOS

#elif defined (__linux__)
#define BTK_LOCK_RETURN_T               unsigned long
#define BTK_LOCK_ISR(unit_p, curr_pl)   if (in_interrupt()) { \
                                            curr_pl = 0; \
                                            spin_lock(&unit_p->isr_lock); \
                                        } else { \
                                            spin_lock_irqsave(&unit_p->isr_lock, curr_pl); \
                                        }
#define BTK_UNLOCK_ISR(unit_p, new_pl)  if (in_interrupt()) { \
                                            spin_unlock(&unit_p->isr_lock); \
                                        } else { \
                                            spin_unlock_irqrestore(&unit_p->isr_lock, new_pl); \
                                        }

#elif defined(BT_NTDRIVER)
#define BTK_LOCK_RETURN_T               KIRQL
#define BTK_LOCK_ISR(unit_p, curr_pl)   KeAcquireSpinLock(&unit_p->isr_lock, &curr_pl);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  KeReleaseSpinLock(&unit_p->isr_lock, new_pl);

#elif defined(__lynxos)

#define BTK_LOCK_RETURN_T       int
#define BTK_LOCK_ISR(unit_p, curr_pl)   curr_pl = 0; btk_mutex_enter(&unit_p->isr_lock);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  btk_mutex_exit(&unit_p->isr_lock);


#else
#error unknown operating system
#endif /* operating system */


/*
**      Now include the specific driver's definitions
*/

#if     defined(__sun)

/*****************************************************************************
**
**  Data structures: Solaris section
**
*****************************************************************************/

#include <sys/ksynch.h>

#if     defined(BT18sol)

#include "btqdd.h"

#elif   defined(BT945)

#include "btpdd.h"

#elif   defined(BT946)

#include "btpdd.h"

#elif   defined(BT15906)

#include "btmdd.h"

#else

#error  "Didn't define which product."

#endif  /* BT18sol, BT945 */



#elif   defined(__sgi)
/*****************************************************************************
**
**  Data structures: SGI section
**
*****************************************************************************/

#include <sys/types.h>
#include <sys/ksynch.h>
#include <sys/ddi.h>

/* Process identification routines */
typedef u_long bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC (-1)
#define	BTK_GET_PROC(lval) { if (drv_getparm(PPID, &lval)) lval = -1; }
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

#if     defined(BT15904)
#include "btmdd.h"
#elif defined(BT965)
#include "btpdd.h"
#else
#error  "Didn't define which product."
#endif  /* BT15904 */



#elif   defined(__vxworks)

/*****************************************************************************
**
**  Data structures: VxWorks section
**
*****************************************************************************/


/* Process identification routines */
typedef int bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC (-1)
#define	BTK_GET_PROC(lval) { (lval) = taskIdSelf();}
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

#if     defined(BT18901)

#include "btvdd.h"

#elif   defined(BT993)

#include "btpdd.h"

#elif   defined(FCTACH_VXWORKS)
#include "fcvxpdd.h"

#else   /* !defined(BT18901) || !defined(BT993) */

#error  "Didn't define which product."

#endif  /* defined(BT18901), defined(BT993) */



#elif   defined(BT_NTDRIVER)
/*****************************************************************************
**
**  Data structures: NT section
**
*****************************************************************************/

#if     defined(BT15901)

#include "btmdd.h"

#elif   defined(FCTACH_WINXP)

#include "fcwxppdd.h"

#elif   defined(BT13908)

#include "btfdd.h"

#elif   defined(BT13908)

#include "btfdd.h"

#elif   defined(BT_NTDRIVER)

#include "btwdd.h"

#else   /* BT15901 || BT13908 || BT_NTDRIVER */

#error  "Didn't define which product."

#endif  /* BT15901 || BT13908 || BT_NTDRIVER */




#elif   defined(BT_uCOS)
/*****************************************************************************
**
**  Data structures: uC OS section
**
*****************************************************************************/

#if     defined(BT2345)

#include "btfdd.h"

#else

#error  "Didn't define which product."

#endif  /* BT2345 */



/*****************************************************************************
**
**  Data structures: Linux section
**
*****************************************************************************/

#elif   defined(__linux__)

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/spinlock.h>
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
#include "asm/spinlock.h"
#endif /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
           LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

typedef pid_t bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC ((pid_t) -1)

#define	BTK_GET_PROC(lval) { (lval) = current->pid; }
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

#if defined(BT1003) /* Select product model */

#include "btpdd.h"

#elif defined(FCTACH_LINUX) /* Select product model */

#include "fclpdd.h"

#else /* Select product model */

#error  "Didn't define which product."

#endif /* Select product model */


/*****************************************************************************
**
**  Data structures: Lynxos section
**
*****************************************************************************/

#elif defined (__lynxos)

#include "btpdd.h"



/*****************************************************************************
**
**  Data structures: ALL other operating systems
**
*****************************************************************************/

#else   /* Haven't written them yet */

#error  Code not written yet

#endif  /* Define data structures for each operating system */



#endif /* !defined(_BTDD_H) */
