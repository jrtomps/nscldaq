/*****************************************************************************
**
**      Filename:   bt_delay.c
**
**      Purpose:    Provide a busy-wait routine for the various drivers.
**
**	Functions:	btk_delay();
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**            Copyright (c) 1997-1999 by SBS Technologies, Inc.
**        Copyright (c) 1990-1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* !LINT */

/*****************************************************************************
**
**      Include files
**
*****************************************************************************/

#include "btdd.h"

#if defined(__linux__)
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/sched.h>

static struct timer_list timeout_data;

#elif   defined(__vxworks)
#include <errno.h>
#include <vxWorks.h>
#include <errnoLib.h>
#include <ioLib.h>
#include <stdLib.h>
#include <taskLib.h>

extern int sbs_get_sys_clk_rate(void); /* proto-type for VxWorks BSP */
                                      /* independent system clock rate call */

#endif	/* defined(__linux__) */

BT_FILE_NUMBER(TRACE_BT_DELAY_C);


/****************************************************************************
**
**      Function:   btk_delay()
**
**      Purpose:    Waits usec microseconds.  Normally a busywait.
**
**      Args:       usec        delay in microseconds
**
**      Returns:    void
**
**      Notes:      Should NEVER call this for a long time. Use a mutex
**		    if the wait is expected to be long.
**
**                  The main purpose of this function is to spin until the
**                  next poll for DMA completion.  Since DMA polling is
**                  meant to be a faster alternative for short transfers
**                  where context switching to catch a DMA interrupt is
**                  slower, it would defeat the purpose of polling to
**                  yield before DMA is done.  If yielding is faster, the
**                  application set the wrong DMA poll threshold.
**
****************************************************************************/

void btk_delay(
    int usec
    )
{
    FUNCTION("btk_delay");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if (usec < 1) {
        usec = 1;
    }

#if defined (_AIX)    /* tv_nsec only is used, limiting total delay to < 1sec */
{
    long start, laps, dns;
    struct timestruc_t snap_m;

    curtime(&snap_m);               /* snapshot start time in ns */
    start = snap_m.tv_nsec;
    dns = usec * 1000;              /* convert requested us delay to ns */

    for (laps = 0; laps < dns; ) {
        curtime(&snap_m);
        laps = snap_m.tv_nsec - start;
        if (laps < 0) {
            laps += 1000000000;
        }
    }
}

#elif defined (__hpux)

    snooze(usec);

#elif defined (__sun)

    DELAY(usec);

#elif defined (__sgi)

    drv_usecwait(usec);

#elif defined (__osf__)

    DELAY(usec);

#elif defined(__linux__)


    udelay(usec);
    
#elif defined(BT_NTDRIVER)

    /*
    ** The minimum time to delay is 1uS, while a maximum of 50us is
    ** recommended in documentation for obvious performance reasons.
    ** The 1uS resolution is very precise, I (Craig Harrison) tested it.
    */
    KeStallExecutionProcessor((ULONG) usec);
    
#elif defined(__vxworks)

{
    ULONG start;
    ULONG curr;
    long laps;
    long dns;

    start = tickGet();              /* current value of tick counter */

                                    /* convert requested delay into ticks */
    /* Use the VxWorks BSP indepented system clack rate call */
    dns = usec * sbs_get_sys_clk_rate() / 1000000; 

    for (laps = 0; laps < dns; ) {
        curr = tickGet();
        laps = curr - start;
        if (laps < 0) {
            laps += 1000000000;
        }
    }
}


#else /* _AIX & etc. */

#error Unknown Operating System

#endif /* _AIX & etc. */

    FEXIT(0);
}


/****************************************************************************
**
**      Function:   btk_sleep()
**
**      Purpose:    Waits usec microseconds.  Do not busywait.
**
**      Args:       usec        delay in microseconds
**
**      Returns:    void
**
**      Notes:      Should NEVER call this for a long time. Use a mutex
**		    if the wait is expected to be long.
**
**                  Be very careful about using this routine in shared code
**                  since most OSs will have restriction on what types
**                  of locks can be held accross this call.
**    
**                  This routine is used to wait in between checking the
**                  DMA semaphore bit.  Don't want to use btk_delay() because
**                  DMAs can take a couple of msec to complete and don't
**                  want to busy wait.  Proper way to do this is for 
**                  remote driver to signal the waiting driver when the
**                  DMA is done and the waiting driver's ISR wakes up
**                  the routine waiting to get the semaphore.  Didn't 
**                  do this for schedule reasons.
**
**                  Linux and Wndows NT still use a busy wait routine
**
****************************************************************************/

bt_error_t btk_sleep(
    int usec)
{
    bt_error_t  retval = BT_SUCCESS;
    
    FUNCTION("btk_sleep");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if (usec < 1) {
        usec = 1;
    }

#if defined (__sgi)
    /*
    ** Both locks and read/write locks can not be held across this
    ** call.  Which for the 965 indicates that the ISR_LOCK and
    ** pio_rwlock can not be held.
    **
    ** No way to catch user signals
    */
    delay(drv_usectohz(usec));

#elif defined(__vxworks)

    {
      STATUS vx_status; 

      /* Use the VxWorks BSP indepented system clack rate call */
      vx_status = taskDelay(usec * sbs_get_sys_clk_rate() / 1000000 );

      if (OK != vx_status) {
          INFO_STR("taskDelay failed."); 
      }
    }
    

#elif defined (__linux__)

    /*
    ** This routine can not time less then 1/HZ seconds
    ** HZ is typically 100 for most Intel32 platforms which
    ** is 10 msec.
    */
    {
    unsigned long usec2;
    unsigned long jiffs;

    usec2 = usec * HZ;
    if (usec2 / 1000000) {
        jiffs = usec2 / 1000000;
    } else {
        jiffs = 1;
    }
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(jiffs);
    }

#elif defined(BT_NTDRIVER)

    {
    LARGE_INTEGER due_time;

    /*
    ** Load relative (i.e. negative) number of 100ns ticks then
    ** nonbusy-wait for the specified interval before returning.
    */
    due_time.QuadPart = -((LONGLONG) (usec) * 10);
    KeDelayExecutionThread(UserMode, FALSE, &due_time);
    }
    
#elif defined(__sun)

    delay(drv_usectohz(usec));


#else /* Unknown OS */

#error Unknown Operating System

#endif /* Unknown OS */

    FEXIT(retval);
    return (retval);
}


/****************************************************************************
**
**      Function:   btk_timeout()
**
**      Purpose:    Schedule a timeout function.
**
**      Args:       f_time_expire() Function to call when time expires
**                  arg_p           Argument pointer to pass to function
**                  usec            Timeout in useconds
**
**      Returns:    btk_timeout_t   A timeout ID which may be needed for 
**                                  btk_untimeout().
**
**      Linux Notes: 
**          Only one timeout may be active at a time.  This caused by 
**          the use of the static struct timer_list variable.
**
****************************************************************************/

btk_timeout_t btk_timeout(
    void (*f_time_expire)(caddr_t), 
    caddr_t arg_p, 
    long usec)
{

    btk_timeout_t retval = (btk_timeout_t) NULL;

    FUNCTION("btk_timeout");
    LOG_UNKNOWN_UNIT;

    FENTRY;

#if defined (_AIX)

#error Must convert tickers to usec for AIX
    timeout(f_time_expire, arg_p, tickers);

#elif defined (__hpux)

#error Must convert tickers to usec for HP-UX
    timeout(f_time_expire, arg_p, tickers, NULL);

#elif defined (__sun)

    retval = timeout((void (*)(void *)) f_time_expire, arg_p, drv_usectohz(usec));

#elif defined (__osf__)

#error Must convert tickers to usec for OSF
    timeout(f_time_expire, arg_p, tickers);

#elif defined (__sgi)

    retval = itimeout(f_time_expire, (void *) arg_p, (long) drv_usectohz((int) usec), plhi);

#elif defined (__vxworks)
  
 {
    STATUS        vx_status;
    btk_timeout_t my_watch_dog_id;    

    my_watch_dog_id = wdCreate();
    if (NULL == my_watch_dog_id) {
        INFO_STR("Memory insufficint.");
        return(NULL);
    }

    /* Use the VxWorks BSP indepented system clack rate call */
    vx_status = wdStart(
                 my_watch_dog_id,         /* watchdog ID delay count, in ticks */
                 (int) (usec * sbs_get_sys_clk_rate() / 1000000), 
                 (FUNCPTR) f_time_expire, /* routine to call on timeout */ 
                 (int) arg_p);            /* parameter to call routine */
    if (OK != vx_status) {
        INFO_STR("Watchdog timer cannot be started.");
        return(NULL);
    }

}

#elif defined (__linux__)

    init_timer(&timeout_data);
    timeout_data.function = (void (*)(unsigned long)) f_time_expire;
    timeout_data.data = (unsigned long) arg_p;
    timeout_data.expires = btk_tck_get() + btk_msec2tck(usec / 1000);
    add_timer(&timeout_data);
    

#elif defined (BT_NTDRIVER)

    /* 
    ** Not implemented
    */

#else  /* _AIX & etc. */

#error Unknown Operating System

#endif /* _AIX & etc. */

    FEXIT(retval);
    return(retval);
}


/****************************************************************************
**
**      Function:   btk_untimeout()
**
**      Purpose:    Unschedule a timeout function.
**
**      Args:       f_time_expire() Function previously scheduled for timeout.
**                  arg_p           Argument pointer to pass to function
**                  timeout_id      timeout_id returned from bt_timeout().
**
**      Returns:    0 on success
**                  BT_ENOT_FOUND on failure
**
****************************************************************************/

/*ARGSUSED0*/
bt_error_t btk_untimeout(
    void (*f_time_expire)(caddr_t), 
    caddr_t arg_p, 
    btk_timeout_t timeout_id)
{
    /* return value- stays zero for most OSs */
    bt_error_t retval = BT_SUCCESS;  

    FUNCTION("btk_untimeout");
    LOG_UNKNOWN_UNIT;

    FENTRY;

#if defined (_AIX)

    untimeout(f_time_expire, arg_p);

#elif defined (__hpux)

    untimeout(f_time_expire, arg_p);

#elif defined(__sun)

    if (untimeout(timeout_id) == -1) {
        retval = BT_EINVAL;
    }

#elif defined (__osf__)

    untimeout(f_time_expire, arg_p);

#elif defined (__sgi)

    untimeout(timeout_id);

#elif defined (__vxworks)

{
    STATUS vx_status;

    /* WARNING: */
    /* This routine can not be call from interrupt level */

    vx_status = wdDelete(timeout_id);
    if (OK != vx_status) {
        INFO_STR("Watchdog timer cannot be de-allocated.");
    }
}

#elif defined (__linux__)

    del_timer(&timeout_data);
    
#elif defined (BT_NTDRIVER)

    /* 
    ** Not implemented
    */

#else /* _AIX & etc. */

#error Unknown Operating System

#endif /* _AIX & etc. */

    FEXIT(retval);
    return(retval);

}
