/*****************************************************************************
**
**      Filename:   bt_mutex.c
**
**      Purpose:    Kernel mutual exclusion routines
**
**      Functions:      btk_mutex_init(), btk_mutex_fini(),
**                      btk_mutex_enter(), btk_mutex_exit()
**
**      $Revision: 2330 $
**
*****************************************************************************/
/*****************************************************************************
**
**            Copyright (c) 1997-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
#ifdef  __EPI__
__asm(".rdata");
__asm(".ascii\"@(#)"__FILE__"  $Revision: 2330 $ \\0\"");
__asm(".align 3; .text");
#else
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif  /* __EPI__*/
#endif  /* !LINT */

/*****************************************************************************
**
**      Include files
**
*****************************************************************************/

#include "btdd.h"



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

BT_FILE_NUMBER(TRACE_BT_MUTEX_C);


/******************************************************************************
**
**      Function:       btk_mutex_init()
**
**      Purpose:        Initializes a kernel mutual exclusion object.
**
**      Args:
**              mutex_p         Pointer to bt_mutex_t to be intialized
**
**              Solaris Specific parameters:
**
**              name_p          String name for mutex
**              iblock_p        Interrupt cookie
**
**              VxWorks specific parameters:
**              None
**
**              SGI Specific parameters:
**              name_p          String name for mutex, may be NULL
**
**              NT Specific parameters:
**              mutex_type	Specifies type as BT_SPIN_LOCK or BT_FAST_MUTEX
**
**              Linux Specific parameters:
**              irq_excl        == 0, never used from IRQ level
**                              Otherwise it may be at IRQ level so we use
**                              a spinlock.
**
**      Returns:
**              0               Success
**              Otherwise       error value.
**
**      Notes:
**
**  NT Notes:
**      Caller must be running at IRQL <= DISPATCH_LEVEL.
**
******************************************************************************/
int btk_mutex_init(
    bt_mutex_t  *mutex_p   /* pointer to bt_mutex_t */

#if     defined(__sun)
        ,
    char        *name_p,
    ddi_iblock_cookie_t *mutex_cookie_p

#elif defined(_NTDDK_)
        ,
    bt_mutex_type_t mutex_type

#elif defined(__sgi)
        ,
    char        *name_p
#elif defined(__linux__)
        ,
    bt_cookie_t irq_cookie
#endif
    )
{
    int         retvalue = BT_SUCCESS;   /* Assume success */

    FUNCTION("btk_mutex_init");
    LOG_UNKNOWN_UNIT;   
    FENTRY;

#if     defined(__sun)
    mutex_init(mutex_p, name_p, MUTEX_DRIVER, mutex_cookie_p);

#elif   defined (__sgi)

    MUTEX_INIT(mutex_p, MUTEX_DEFAULT, name_p);


#elif   defined(__vxworks)
    *mutex_p = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
    if (NULL == *mutex_p) {
        INFO_STR("Couldn't create semaphore.");
        retvalue = BT_ENOMEM;
    }

#elif   defined(_NTDDK_)
    switch(mutex_type) {
      case BT_SPIN_LOCK:
        KeInitializeSpinLock(&mutex_p->mutex_obj.spin_lock);
        break;
      case BT_FAST_MUTEX:
        ExInitializeFastMutex(&mutex_p->mutex_obj.fast_mutex);
        break;
      default:
        FATAL_STR("Invalid mutex type, defaulting to BT_FAST_MUTEX\n");
        mutex_type = BT_FAST_MUTEX;
        BTK_ASSERT(FALSE);
        retvalue = BT_EINVAL;
        break;
    }
    mutex_p->mutex_type = mutex_type;   

#elif   defined(BT_uCOS)

    /* create the semaphore and initilize the event to full so a call to */
    /* OSSemPend() will decrement the semaphore count and lock out */
    /* any other calling task. */
    *mutex_p = OSSemCreate(0); 
    if (NULL == *mutex_p) {
        INFO_STR("No event control block available.\n");
        retvalue = ENOMEM;
    }
    

#elif	defined(__linux__)

    sema_init(mutex_p, 1);

#elif defined(__lynxos)

    /* Allow anyone to acquire the mutex */
    *mutex_p = 1;

#else
#error  Code not written yet
#endif  /* __sun, __vxworks, _NTDDK_ */

    FEXIT(retvalue); 
    return retvalue;
} /* end btk_mutex_init() */


/******************************************************************************
**
**      Function:       btk_mutex_fini()
**
**      Purpose:        Releases any resources allocated by btk_mutex_init().
**
**      Args:
**              mutex_p         Pointer to bt_mutex_t to be intialized
**          
**
**      Returns:        Void
**
**      Notes:
**
******************************************************************************/
void btk_mutex_fini(
    bt_mutex_t  *mutex_p   /* pointer to bt_mutex_t */
    )
{
#if     defined(__vxworks)
    SEM_ID      dead_mutex;
    STATUS      vx_ret;
#endif  /* defined(__vxworks) */

    FUNCTION("btk_mutex_fini");
    LOG_UNKNOWN_UNIT;   
    FENTRY;


#if     defined(__sun)
    mutex_destroy(mutex_p);

#elif   defined(__sgi)
    MUTEX_DESTROY(mutex_p);

#elif   defined(__vxworks)
    dead_mutex = *mutex_p;
    *mutex_p = NULL;
        
    vx_ret = semMGiveForce(dead_mutex);
    if (OK != vx_ret) {
        INFO_STR("Semaphore ID is invalid.");
    } else {
        semDelete(dead_mutex);
    }

#elif   defined(_NTDDK_)
    switch(mutex_p->mutex_type) {
      case BT_SPIN_LOCK:
        break;
      case BT_FAST_MUTEX:
        break;
      default:
        BTK_ASSERT(FALSE);
        break;
    }

#elif   defined(BT_uCOS)

    /* no action required */
    /* Once an uCOS semaphore is created it can not be deleted */
    ;

#elif   defined(__linux__)

    /* no action required */
    ;

#elif defined (__lynxos)

    /* No action is required */
    ;

#else
#error  Code not written yet
#endif  /* defined(__sun, _NTDDK_) */

    FEXIT(0);

    return;
} /* end btk_mutex_fini() */



/******************************************************************************
**
**  Function:   btk_mutex_enter()
**
**  Purpose:    Start section protected by mutex.
**
**  Args:
**  mutex_p             Pointer to bt_mutex_t to be entered.
**
**  Returns:    Void
**
**  Notes:
**      Calls to btk_mutex_enter() cannot be nested.
**
**  NT Notes:
**      If mutex_p->mutex_type == BT_SPIN_LOCK:
**      1. Callers must be running at IRQL <= DISPATCH_LEVEL.
**      2. The code guarded by btk_mutex_enter() must neither be pageable nor
**         make any references to pageable data. 
**      3. The code guarded by btk_mutex_enter() can neither call any external
**         function that might access pageable data or raise an exception, nor
**         can it generate any exceptions. 
**      4. The caller should release the spin lock with btk_mutex_exit() as
**         quickly as possible. 
**      5. Busy waits the CPU if the spin lock cannot be acquired immediately.
**
**      If mutex_p->mutex_type == BT_FAST_MUTEX:
**      1. Callers must be running at IRQL < DISPATCH_LEVEL.
**      2. Puts the caller into a wait state and switches to another thread
**         of execution if the fast mutex cannot be acquired immediately.
**
******************************************************************************/
void btk_mutex_enter(
    bt_mutex_t  *mutex_p    /* pointer to bt_mutex_t */
    )
{
    FUNCTION("btk_mutex_enter");
    LOG_UNKNOWN_UNIT;   
    FENTRY;

#if     defined(__sun)
    mutex_enter(mutex_p);

#elif   defined(__sgi)

    MUTEX_LOCK(mutex_p, -1);    /* -1 is unused param in IRIX 6.5.4 */

#elif   defined(__vxworks)
    {
        STATUS  vx_ret;

        vx_ret = semTake(*mutex_p, WAIT_FOREVER); 
        if (OK != vx_ret) {
            INFO_STR("Invalid semaphore ID or task timed out.");
        }

    }

#elif   defined(_NTDDK_)
    {
        KIRQL local_old_irql;

        switch(mutex_p->mutex_type) {
          case BT_SPIN_LOCK:
            KeAcquireSpinLock(&mutex_p->mutex_obj.spin_lock, &local_old_irql);
            mutex_p->old_irql = local_old_irql;
            break;
          case BT_FAST_MUTEX:
            ExAcquireFastMutex(&mutex_p->mutex_obj.fast_mutex);
            break;
          default:
            BTK_ASSERT(FALSE);
            break;
        }

    }

#elif   defined(BT_uCOS)

    {
        UBYTE err;

        err = OSSemPost(*mutex_p);

        if (OS_NO_ERR != err) {
            TRC_STR(BT_TRC_KLIB, "Semaphore count overflow.\n");
        }
    }
            
#elif	defined(__linux__)

    (void) down(mutex_p);

#elif defined (__lynxos)

    /* Acquire the lock */
    swait(mutex_p, SEM_SIGIGNORE);

#else
#error  Code not written yet
#endif  /* __sun, __vxworks, _NTDDK_ */

    FEXIT(0);

    return;
} /* end btk_mutex_enter() */



/******************************************************************************
**
**  Function:   btk_mutex_exit()
**
**  Purpose:    End section protected by mutex.
**
**  Args:
**      mutex_p         Pointer to bt_mutex_t to be exited.
**
**  Returns:    Void
**
**  NT Notes:
**      If mutex_p->mutex_type == BT_SPIN_LOCK caller is running
**      IRQL DISPATCH_LEVEL.  On return, IRQL is restored to the level at
**      which btk_mutex_enter() was called.
**
**      If mutex_p->mutex_type == BT_FAST_MUTEX callers must be running at
**      IRQL < DISPATCH_LEVEL.
**
******************************************************************************/
void btk_mutex_exit(
    bt_mutex_t  *mutex_p  /* pointer to bt_mutex_t */
    )
{
    FUNCTION("btk_mutex_exit");
    LOG_UNKNOWN_UNIT;   
    FENTRY;

#if     defined (__sun)
    mutex_exit(mutex_p);

#elif   defined(__sgi)
    MUTEX_UNLOCK(mutex_p);

#elif   defined(__vxworks)

    {
        STATUS  vx_ret;

        vx_ret = semGive(*mutex_p); 

        if (OK != vx_ret) {
            /* VxWorks misses up on semGive now and then so */
            /* force the semaphore give.  */          
            vx_ret = semMGiveForce(*mutex_p);
            if (OK != vx_ret) {
                INFO_STR("Semaphore ID is invalid.")
            }
        }

    }

#elif   defined(_NTDDK_)
    {
        KIRQL new_irql;

        switch(mutex_p->mutex_type) {
          case BT_SPIN_LOCK:
            new_irql = mutex_p->old_irql;
            KeReleaseSpinLock(&mutex_p->mutex_obj.spin_lock, new_irql);
            break;
          case BT_FAST_MUTEX:
            ExReleaseFastMutex(&mutex_p->mutex_obj.fast_mutex);
            break;
          default:
            BTK_ASSERT(FALSE);
            break;
        }

    }


#elif defined(BT_uCOS)
    {
        UBYTE err;

        OSSemPend(*mutex_p, 0, &err);

        if (OS_NO_ERR != err) {
            TRC_STR(BT_TRC_KLIB, "Semaphore timeout occured.\n");
        }
    }

#elif	defined(__linux__)

    (void) up(mutex_p);


#elif defined (__lynxos)

    /* Release the lock */
    ssignal(mutex_p);


#else
#error  Code not written yet
#endif  /* __sun, __vxworks, _NTDDK_ */

    FEXIT(0);
    return;
} /* end btk_mutex_exit() */

