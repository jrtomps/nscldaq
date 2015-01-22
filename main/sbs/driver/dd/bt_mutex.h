/*****************************************************************************
**      Filename:   bt_mutex.h
**
**      Purpose:    Generic header for bt_mutex.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_mutex.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.6 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_MUTEX_H)
#define _BT_MUTEX_H

/*****************************************************************************
**
**  Include Files 
**
*****************************************************************************/

#if	defined(__linux__)
#include	<linux/interrupt.h>
#endif	/* defined(__linux__) */

#if defined (__vxworks)
#if defined (FCTACH)
/*
#include <semLib.h>
*/
#endif /* defined (FCTACH) */
#endif /* defined (__vxworks) */


/*****************************************************************************
**
**  Data Structures 
**
*****************************************************************************/

#if defined (_AIX)

#elif defined (__winxp)

typedef enum {
    BT_SPIN_LOCK,
    BT_FAST_MUTEX
} bt_mutex_type_t;

typedef struct {
    bt_mutex_type_t mutex_type;
    union {
        KSPIN_LOCK spin_lock;
        FAST_MUTEX fast_mutex;
        KMUTEX mutex;
    } mutex_obj;
    KIRQL old_irql;     /* storage for spinlock's old IRQL */
} bt_mutex_t;

#elif defined (__hpux)

#elif defined (__sgi)

typedef mutex_t bt_mutex_t;

#elif defined (__sun)

typedef kmutex_t bt_mutex_t;

#elif defined (__vxworks)

typedef SEM_ID bt_mutex_t;

#elif defined (BT_uCOS)

typedef OS_EVENT * bt_mutex_t;

#elif defined (__linux__)

typedef struct semaphore  bt_mutex_t;

#elif defined (__lynxos)

typedef int bt_mutex_t;

#else
#error unknown operating system
#endif /* operating system */



/*****************************************************************************
**
**  Protoypes
**
*****************************************************************************/

/* Prototype for btk_mutex_init() includes OS dependent parameters */
#if defined (_AIX)

#elif defined (BT_NTDRIVER)
int btk_mutex_init(
        bt_mutex_t *mutex_p, 
        bt_mutex_type_t mutex_type);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_mutex_init(
        bt_mutex_t *mutex_p, 
        char *name_p);

#elif defined (__sun)
int btk_mutex_init(
        bt_mutex_t *mutex_p, 
        char *name_p, 
        ddi_iblock_cookie_t *cookie_p); 

#elif defined (__vxworks)
int btk_mutex_init(
        bt_mutex_t *mutex_p);

#elif defined (BT_uCOS)
int btk_mutex_init(
        bt_mutex_t *mutex_p);

#elif defined (__linux__)

int btk_mutex_init(
        bt_mutex_t *mutex_p, 
        bt_cookie_t irq_cookie);

#elif defined (__lynxos)
int btk_mutex_init(
        bt_mutex_t *mutex_p);

#else
#error unknown operating system
#endif /* operating system */

void btk_mutex_fini(
        bt_mutex_t *mutex_p);

void btk_mutex_enter(
        bt_mutex_t *mutex_p);

void btk_mutex_exit(
        bt_mutex_t *mutex_p);

#endif /* !defined(_BT_MUTEX_H) */
