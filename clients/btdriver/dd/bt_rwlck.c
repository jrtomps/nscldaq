/*****************************************************************************
**
**  Filename:   bt_rwlck.c
**
**  Purpose:    Kernel reader/writer lock routines
**
**  Functions:  btk_rwlock_init(), btk_rwlock_fini(),
**          btk_rwlock_rd_enter(), btk_rwlock_wr_enter(),
**          btk_rwlock_rd_exit(), btk_rwlock_wr_exit()
**
**  $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**            Copyright (c) 1997, 1998, 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
#ifdef  __EPI__
__asm(".rdata");
__asm(".ascii\"@(#)"__FILE__"  $Revision$ "__DATE__"\\0\"");
#else
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* __EPI__*/
#endif  /* !LINT */


/*****************************************************************************
**
**      Include files
**
*****************************************************************************/

#include "btdd.h"

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_RWLCK_C);

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

/******************************************************************************
**
**  Function:   btk_rwlock_init()
**
**  Purpose:    Initializes a kernel reader/writer lock object.
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be intialized.
**
**      Solaris specific parameters:
**
**      name_p      String name for mutex
**      iblock_p    Interrupt cookie
**
**  Returns:
**      0:          Success.
**      Otherwise:  Error value.
**
**  Notes:
**
**
**  NT Notes:
**      The caller must be running at IRQL PASSIVE_LEVEL.
**
******************************************************************************/

int btk_rwlock_init(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
#if     defined(__sun)
    ,
    char        *name_p,
    ddi_iblock_cookie_t *rw_cookie_p
#elif   defined(__sgi)
    ,
    char        *name_p
#endif  /* defined(__sun) */
    )
{
    FUNCTION("btk_rwlock_init");
    LOG_UNIT(unit_p);
    int retvalue = 0;   /* assume success */

    FENTRY;


#if     defined(_NTDDK_)

    KeInitializeSpinLock(&rwlock_p->spin_lock);
    ExInitializeFastMutex(&rwlock_p->fast_mutex);
    rwlock_p->ref_count = 0;

    /* Use a NotificationEvent to permit it to be explicitly cleared by
       btk_rwlock_rd_enter(). The event will be in the signaled state
       unless the reader lock has been acquired. */
    KeInitializeEvent(&rwlock_p->event, NotificationEvent, TRUE);

#elif   defined(__sun)

    rw_init(rwlock_p, name_p, RW_DRIVER, rw_cookie_p);

#elif   defined(__sgi)

    RW_INIT(rwlock_p, name_p);

#elif   defined(BT_uCOS)
    /* not implemented */
    retvalue = BT_ENOSUP;

#elif   defined(__vxworks)

    rwlock_p->spin_lock = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
    if (NULL == rwlock_p->spin_lock) {
        INFO_STR("Couldn't create read/write lock to protect the ref_count.");
        retvalue = BT_ENOMEM;
    }

    rwlock_p->fast_mutex = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
    if (NULL == rwlock_p->fast_mutex) {
        INFO_STR("Couldn't create read/write lock.");
        retvalue = BT_ENOMEM;
    }

    rwlock_p->ref_count = 0;

    rwlock_p->event = semCCreate(SEM_Q_PRIORITY, 0);
    if (NULL == rwlock_p->event) {
        INFO_STR("Couldn't create read/write event.");
        retvalue = BT_ENOMEM;
    }

#elif   defined(__linux__)
    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    */
    spin_lock_init(&rwlock_p->rw_lock);

#else

#error  Code not written yet

#endif  /* _NTDDK_, __sun */


    FEXIT(retvalue);
    return retvalue;
}

/******************************************************************************
**
**  Function:   btk_rwlock_fini()
**
**  Purpose:    Releases any resources allocated by btk_rwlock_init().
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be intialized.
**
**  Returns:    void
**
**  Notes:
**
******************************************************************************/

void btk_rwlock_fini(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
    )
{
    FUNCTION("btk_rwlock_fini");
    LOG_UNIT(unit_p);

#if   defined(__vxworks)
    SEM_ID    dead_rwlock;
    STATUS    vx_ret;
#endif  /* vxworks */

    FENTRY;

#if     defined(_NTDDK_)

    ;

#elif   defined(__sun)
    rw_destroy(rwlock_p);

#elif   defined(__sgi)
    RW_DESTROY(rwlock_p);

#elif   defined(BT_uCOS)
    /* not implemented */

#elif   defined(__vxworks)
    
    dead_rwlock = rwlock_p->event;
    rwlock_p->event = NULL;
    vx_ret = semMGiveForce(dead_rwlock);
    if (OK != vx_ret) {
        INFO_STR("Semaphore ID is invalid.");
    } else {
        semDelete(dead_rwlock);
    }

    dead_rwlock = rwlock_p->fast_mutex;
    rwlock_p->fast_mutex = NULL;
    vx_ret = semMGiveForce(dead_rwlock);
    if (OK != vx_ret) {
        INFO_STR("Semaphore ID is invalid.");
    } else {
        semDelete(dead_rwlock);
    }

    dead_rwlock = rwlock_p->spin_lock;
    rwlock_p->spin_lock = NULL;
    vx_ret = semMGiveForce(dead_rwlock);
    if (OK != vx_ret) {
        INFO_STR("Semaphore ID is invalid.");
    } else {
        semDelete(dead_rwlock);
    }

    rwlock_p->ref_count = 0;

#elif   defined(__linux__)

    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    **
    ** Currently, no action required
    */
    ;
    
#else

#error  Code not written yet

#endif  /* _NTDDK_ */

    FEXIT(0);
}

/******************************************************************************
**
**  Function:   btk_rwlock_wr_enter()
**
**  Purpose:    Acquire the reader/writer lock for writer (exclusive) access.
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be acquired.
**
**  Returns:    void
**
**  Notes:
**      Preference is given to write requests.  If the lock is held by one
**      or more readers when btk_rwlock_wr_enter() is called, all subsequent
**      calls to btk_rwlock_rd_enter() will block until the pending write
**      request has been acquired and released.  This is done to avoid starving
**      the write requests.
**
**      Reader/write lock operations are not recursive.  If the same thread
**      calls either btk_rwlock_wr_enter() or btk_rwlock_rd_enter() after
**      acquiring either the reader or writer lock, a deadlock will result.
**
**  NT Notes:
**      The caller must be running at IRQL PASSIVE_LEVEL.
**
******************************************************************************/

void btk_rwlock_wr_enter(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
    )
{
    FUNCTION("btk_rwlock_wr_enter");
    LOG_UNIT(unit_p);

#if   defined(__vxworks)
    STATUS vx_ret;
#endif /* vxworks */

    FENTRY;

#if     defined(_NTDDK_)

    ExAcquireFastMutex(&rwlock_p->fast_mutex);
    KeWaitForSingleObject(&rwlock_p->event, UserRequest, UserMode, FALSE, NULL);

#elif   defined(__sun)

    rw_enter(rwlock_p, RW_WRITER);

#elif   defined(__sgi)
    RW_WRLOCK(rwlock_p);

#elif   defined(BT_uCOS)
    /* not implemented */

#elif   defined(__vxworks)

    vx_ret = semTake(rwlock_p->fast_mutex, WAIT_FOREVER);
    if (OK != vx_ret) {
        INFO_STR("Invalid semaphore ID or task timed out.")
    }
    
#elif defined(__linux__)
    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    */
    spin_lock(&rwlock_p->rw_lock);

#else

#error  Code not written yet

#endif  /* _NTDDK_ */

    FEXIT(0);
}

/******************************************************************************
**
**  Function:   btk_rwlock_rd_enter()
**
**  Purpose:    Acquire the reader/writer lock for reader (shared) access.
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be acquired.
**
**  Returns:    void
**
**  Notes:
**      An arbitrary number of threads may hold the reader lock simultaneously.
**
**      Preference is given to write requests.  If the lock is held by one
**      or more readers when btk_rwlock_wr_enter() is called, all subsequent
**      calls to btk_rwlock_rd_enter() will block until the pending write
**      request has been acquired and released.  This is done to avoid starving
**      the write requests.
**
**      Reader/write lock operations are not recursive.  If the same thread
**      calls either btk_rwlock_wr_enter() or btk_rwlock_rd_enter() after
**      acquiring either the reader or writer lock, a deadlock will result.
**
**  NT Notes:
**      The caller must be running at IRQL PASSIVE_LEVEL.
**
******************************************************************************/

void btk_rwlock_rd_enter(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
    )
{
    FUNCTION("btk_rwlock_rd_enter");
    LOG_UNIT(unit_p);

#if   defined(__vxworks)
    STATUS vx_ret;
#endif /* vxworks */


    FENTRY;

#if     defined(_NTDDK_)
    {
    KIRQL old_irql;

    ExAcquireFastMutex(&rwlock_p->fast_mutex);
    KeAcquireSpinLock(&rwlock_p->spin_lock, &old_irql);
    if(rwlock_p->ref_count++ == 0) {
        KeClearEvent(&rwlock_p->event);
    }
    KeReleaseSpinLock(&rwlock_p->spin_lock, old_irql);
    ExReleaseFastMutex(&rwlock_p->fast_mutex);

    }

#elif   defined(__sun)

    rw_enter(rwlock_p, RW_READER);

#elif   defined(__sgi)
    RW_RDLOCK(rwlock_p);

#elif   defined(BT_uCOS)
    /* not implemented */

#elif   defined(__vxworks)

    vx_ret = semTake(rwlock_p->fast_mutex, WAIT_FOREVER);
    if (OK != vx_ret) {
        INFO_STR("Invalid semaphore ID or task timed out.");
        goto exit_btk_rwlock_rd_enter;
    } 

    vx_ret = semTake(rwlock_p->spin_lock, WAIT_FOREVER);
    if (OK != vx_ret) {
        INFO_STR("Invalid semaphore ID or task timed out.");
        goto exit_btk_rwlock_rd_enter;
    } 

    rwlock_p->ref_count++;

    vx_ret = semGive(rwlock_p->spin_lock);
    if (OK != vx_ret) {
        /*
        ** VxWorks screws up on semGive now and then so 
        ** force the semaphore give. 
        */
        vx_ret = semMGiveForce(rwlock_p->spin_lock);
        if (OK != vx_ret) {
            INFO_STR("Invalid semaphore ID.");
            goto exit_btk_rwlock_rd_enter;
        } 
    }

    vx_ret = semGive(rwlock_p->fast_mutex);
    if (OK != vx_ret) {
        /*
        ** VxWorks screws up on semGive now and then so 
        ** force the semaphore give. 
        */
        vx_ret = semMGiveForce(rwlock_p->fast_mutex);
        if (OK != vx_ret) {
            INFO_STR("Invalid semaphore ID.");
            goto exit_btk_rwlock_rd_enter;
        } 
    }
    

#elif defined(__linux__)
    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    */
    spin_lock(&rwlock_p->rw_lock);

#else

#error  Code not written yet

#endif  /* _NTDDK_ */

exit_btk_rwlock_rd_enter:
    FEXIT(0);
}

/******************************************************************************
**
**  Function:   btk_rwlock_wr_exit()
**
**  Purpose:    Releases a kernel writer lock.
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be released.
**
**  Returns:    void
**
**  Notes:
**      The caller must be running in the context of the same thread that
**      acquired the lock.
**
**  NT Notes:
**      The caller must be running at IRQL PASSIVE_LEVEL.
**
******************************************************************************/

void btk_rwlock_wr_exit(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
    )
{
    FUNCTION("btk_rwlock_wr_exit");
    LOG_UNIT(unit_p);

#if   defined(__vxworks)
    STATUS vx_ret;
#endif /* vxworks */

    FENTRY;

#if     defined(_NTDDK_)

    ExReleaseFastMutex(&rwlock_p->fast_mutex);

#elif   defined(__sun)

    rw_exit(rwlock_p);

#elif   defined(__sgi)
    RW_UNLOCK(rwlock_p);

#elif   defined(BT_uCOS)
    /* not implemented */

#elif   defined(__vxworks)

    vx_ret = semGive(rwlock_p->fast_mutex);
    if (OK != vx_ret) {
        /*
        ** VxWorks screws up on semGive now and then so 
        ** force the semaphore give. 
        */
        vx_ret = semMGiveForce(rwlock_p->fast_mutex);
        if (OK != vx_ret) {
            INFO_STR("Invalid semaphore ID.");
        } 
    }

#elif defined(__linux__)
    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    */
    spin_unlock(&rwlock_p->rw_lock);

#else

#error  Code not written yet

#endif  /* _NTDDK_ */

    FEXIT(0);
}

/******************************************************************************
**
**  Function:   btk_rwlock_rd_exit()
**
**  Purpose:    Releases a kernel reader lock.
**
**  Args:
**      unit_p      Pointer to unit structure.
**      rwlock_p    Pointer to bt_rwlock_t to be released.
**
**  Returns:    void
**
**  Notes:
**      The caller must be running in the context of the same thread that
**      acquired the lock.
**
**  NT Notes:
**      The caller must be running at IRQL PASSIVE_LEVEL.
**
******************************************************************************/

void btk_rwlock_rd_exit(
    bt_unit_t   *unit_p,
    bt_rwlock_t *rwlock_p
    )
{
    FUNCTION("btk_rwlock_rd_exit");
    LOG_UNIT(unit_p);

#if   defined(__vxworks)
    STATUS vx_ret;
#endif  /* vxworks */

    FENTRY;

#if     defined(_NTDDK_)
    {
    KIRQL old_irql;

    KeAcquireSpinLock(&rwlock_p->spin_lock, &old_irql);
    if(--rwlock_p->ref_count <= 0) {
        rwlock_p->ref_count = 0;    /* just in case its ever < 0 */
        KeSetEvent(&rwlock_p->event, EVENT_INCREMENT, FALSE);
    }
    KeReleaseSpinLock(&rwlock_p->spin_lock, old_irql);

    }

#elif   defined(__sun)

    rw_exit(rwlock_p);

#elif   defined(__sgi)
    RW_UNLOCK(rwlock_p);

#elif   defined(BT_uCOS)
    /* not implemented */

#elif   defined(__vxworks)

    vx_ret = semTake(rwlock_p->spin_lock, WAIT_FOREVER);
    if (OK != vx_ret) {
       INFO_STR("Invalid semaphore ID or task timed out.");
       goto exit_btk_rwlock_rd_exit;
    }

    rwlock_p->ref_count--;

    vx_ret = semGive(rwlock_p->spin_lock);
    if (OK != vx_ret) {
        /*
        ** VxWorks screws up on semGive now and then so 
        ** force the semaphore give.
        */
        vx_ret = semMGiveForce(rwlock_p->fast_mutex);
        if (OK != vx_ret) {
            INFO_STR("Invalid semaphore ID.");
            goto exit_btk_rwlock_rd_exit;
        } 
    }
    
#elif defined(__linux__)
    /* 
    ** Not properly implemented yet, need to look into support
    ** for read/write locks in Linux kernel patches.  Current code
    ** only does a simple lock that is not callable from interrupt
    ** context.
    */
    spin_unlock(&rwlock_p->rw_lock);

#else

#error  Code not written yet

#endif  /* _NTDDK_ */

exit_btk_rwlock_rd_exit:
    FEXIT(0);
}

