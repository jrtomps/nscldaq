/*****************************************************************************
**      Filename:   bt_rwlck.h
**
**      Purpose:    Generic header for bt_rwlck.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_rwlck.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.7 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_RWLCK_H)
#define _BT_RWLCK_H

/*****************************************************************************
**
**  Include Files 
**
*****************************************************************************/

#if defined (__vxworks)
#include <vxworks.h>
#include <semLib.h>
#endif /* defined (__vxworks) */

#ifdef __linux__
#include <linux/spinlock.h>
#endif /* __linux__) */

/*****************************************************************************
**
**  Data Structures 
**
*****************************************************************************/

#if defined (_AIX)

#elif defined (__winxp)
typedef struct
{
    KSPIN_LOCK spin_lock;   /* protects ref_count */
    FAST_MUTEX fast_mutex;
    KEVENT event;
    long int ref_count;     /* number of readers holding the lock */
} bt_rwlock_t;

#elif defined (__hpux)

#elif defined (__sgi)
typedef rwlock_t bt_rwlock_t;

#elif defined (__sun)
typedef krwlock_t bt_rwlock_t;

#elif defined (__vxworks)
typedef struct
{
    SEM_ID spin_lock;   /* protects ref_count */
    SEM_ID fast_mutex;
    SEM_ID event;
    long int ref_count;     /* number of readers holding the lock */
} bt_rwlock_t;

#elif defined (BT_uCOS)
typedef void bt_rwlock_t;

#elif defined (__linux__)

/* 
** Reader/writer lock routines 
**
** Not properly implemented yet, need to look into support
** for read/write locks in Linux kernel patches.  Current code
** only does a simple lock that is not callable from interrupt
** context.
*/
typedef struct {
    spinlock_t  rw_lock;
    int         lock;
} bt_rwlock_t;

#elif defined (__lynxos)

    /* I believe this type of lock should follow the same conventions as 
     * the Lynxos semaphores, so this type will fall to an int.
     */
typedef int bt_rwlock_t;

#else
#error unknown operating system
#endif /* operating system */


#if defined (__sgi)
    /* left out of sgi header files */
     extern void rw_rdlock(rwlock_t *lockp);  
     extern void rw_wrlock(rwlock_t *lockp); 
#endif /* sgi */

#ifdef BT13908
#pragma alloc_text(PAGE, btk_rwlock_init)
#pragma alloc_text(PAGE, btk_rwlock_fini)
#pragma alloc_text(PAGE, btk_rwlock_wr_enter)
#pragma alloc_text(PAGE, btk_rwlock_rd_enter)
#pragma alloc_text(PAGE, btk_rwlock_wr_exit)
#pragma alloc_text(PAGE, btk_rwlock_rd_exit)
#endif /* BT13908 */

/*****************************************************************************
**
**  Protoypes
**
*****************************************************************************/

#if defined (_AIX)

#elif defined (__winxp)

int btk_rwlock_init(
        bt_rwlock_t *rwlock_p);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_rwlock_init(
        bt_rwlock_t *rwlock_p, 
        char *name_p);

#elif defined (__sun)

#elif defined (__vxworks)
int btk_rwlock_init(
        bt_rwlock_t *rwlock_p);

#elif defined (BT_uCOS)
int btk_rwlock_init(
        bt_rwlock_t *rwlock_p);
   
#elif defined (__linux)

int btk_rwlock_init(
        bt_rwlock_t *rwlock_p);

#elif defined (__lynxos)

int btk_rwlock_init(
        bt_rwlock_t *rwlock_p);

#else
#error unknown operating system
#endif /* operating system */


void btk_rwlock_fini(
        bt_rwlock_t *rwlock_p);

void btk_rwlock_wr_enter(
        bt_rwlock_t *rwlock_p);

void btk_rwlock_rd_enter(
        bt_rwlock_t *rwlock_p);

void btk_rwlock_wr_exit(
        bt_rwlock_t *rwlock_p);

void btk_rwlock_rd_exit(
        bt_rwlock_t *rwlock_p);

/* Macro to get the reader/writer lock reference count.  This is the number
   of readers currently holding the lock.  As written this assumes that all
   implementations of bt_rwlock_t have a ref_count member. */
#define BTK_RWLOCK_REF_COUNT(rwlock_p) (rwlock_p)->ref_count


#endif /* !defined(_BT_RWLCK_H) */
