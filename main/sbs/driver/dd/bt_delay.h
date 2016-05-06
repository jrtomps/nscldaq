/*****************************************************************************
**      Filename:   bt_delay.h
**
**      Purpose:    Bit 3 Generic header for bt_delay.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_delay.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.10 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_DELAY_H)
#define _BT_DELAY_H

#if defined(__linux__)
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/sched.h>
//#include <linux/sched.h>

#elif   defined(__vxworks)
#include <taskLib.h>  /* taskDelay() call */
#include <wdLib.h> /* wdCreate(), wdStart, ect calls */
#include <tickLib.h>  /* tickGet() call */
extern int sbs_get_sys_clk_rate(void); /* proto-type for VxWorks BSP */
                                      /* independent system clock rate call */

#endif	/* defined(__linux__) */

/*****************************************************************************
**
**  Data Structures 
**
*****************************************************************************/

#if defined (_AIX)

#elif defined (__winxp)
typedef PKTIMER btk_timeout_t;

typedef LONGLONG bt_tck_t;

#elif defined (__hpux)

#elif defined (__sgi)
typedef toid_t btk_timeout_t;

typedef int bt_tck_t;

#elif defined (__sun)
typedef clock_t bt_tck_t;       /* Type for driver internal time */

#if defined(SUNOS5_5) || defined (SUNOS5_6)
typedef int btk_timeout_t;
#else /* defined(SUNOS5_5) || defined (SUNOS5_6) */
typedef timeout_id_t btk_timeout_t;
#endif /* defined(SUNOS5_5) || defined (SUNOS5_6) */

#elif defined (__vxworks)
typedef WDOG_ID btk_timeout_t;

typedef int bt_tck_t;

#elif defined (BT_uCOS)

#elif defined (__linux__)
typedef int btk_timeout_t;

typedef unsigned long bt_tck_t;

#elif defined (__lynxos)

typedef int btk_timeout_t;

typedef unsigned long bt_tck_t;

#else
#error unknown operating system
#endif /* operating system */


/*****************************************************************************
**
**  Protoypes 
**
*****************************************************************************/

extern void btk_delay(int usec);

extern bt_error_t btk_sleep(int usec);

extern btk_timeout_t btk_timeout(
        void (*f_time_expire)(caddr_t), 
        caddr_t arg_p, 
        long usec);

extern bt_error_t btk_untimeout(
	void (*f_time_expire)(caddr_t), 
	caddr_t arg_p, 
        btk_timeout_t timeout_id);

#if     defined(__sun)

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_usectohz(time * 1000));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    return (BT_FOREVER == time) ? (BT_FOREVER) :
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_hztousec(time) / 1000);
}

static bt_tck_t btk_tck_get(void)
{
    unsigned long current_time = 0;
    
    if (0 != drv_getparm(LBOLT, &current_time)) {
        current_time = 0;
    }
    return (bt_tck_t) current_time;
}

#elif   defined(__sgi)

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert microsecods to clock ticks */
    /* time = number of microseconds to convert to clock ticks */
    /* No error value is returned.  If the number of ticks equivalent */
    /* to the number of microseconds is too large to represent, then */
    /* the max value is returned. */
    return (bt_tck_t) ((BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_usectohz((clock_t) time * 1000)));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to microseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the microsecond time is too large to   */
    /* bt represented, then the max clock_t value is returned. */
    return (bt_msec_t) ((BT_FOREVER == time) ? (BT_FOREVER) :
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_hztousec((clock_t) time) / 1000));
}

static bt_tck_t btk_tck_get(void)
{
    unsigned long current_ticks = 0;
    int ret_val;

    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  
    ret_val = drv_getparm(LBOLT, &current_ticks);

    /* if couldn't read ticks return zero to caller */
    if (ret_val == -1) {
        current_ticks = 0;
    }
    
    return((bt_tck_t)current_ticks);
}

#elif   defined(__vxworks)

/*
**      Time conversion routines
**
**      btk_msec2tck(), btk_tck2mesc() and btk_tck_get() are in
**      bt_delay.c for vxworks.
*/

bt_tck_t btk_msec2tck(bt_msec_t time); 

bt_msec_t btk_tck2msec(bt_tck_t time);

bt_tck_t btk_tck_get(void);


#elif   defined(BT_NTDRIVER)
/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

#if defined(BT13908)
static bt_tck_t btk_msec2tck(bt_msec_t time);
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_msec2tck)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    return (BT_FOREVER == time) ? ((bt_tck_t) MAXLONGLONG) : 
        ((BT_NO_WAIT == time) ? ((bt_tck_t) 0) : (((bt_tck_t) time) * 10000));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    return (bt_msec_t)((MAXLONGLONG == time) ? (BT_FOREVER) :
        ((0 == time) ? BT_NO_WAIT : time / 10000));
}

static bt_tck_t btk_tck_get(void)
{
    LARGE_INTEGER current_time;

    KeQuerySystemTime(&current_time);
    return current_time.QuadPart;
}

#elif   defined(BT_uCOS)

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert microsecods to clock ticks */
    /* time = number of microseconds to convert to clock ticks */

    /* No error value is returned.  */
    /* Assume TICK_HZ <= 1000, so no chance of overflow */

    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : (time / (1000/ TICK_HZ) ));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to microseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the microsecond time is too large to   */
    /* be represented, then the max clock_t value is returned. */

    bt_msec_t msec;

    if ((BT_FOREVER == time) || (BT_NO_WAIT == time)) {
        msec = time;
    } else if (time < (BT_D32_MASK-1) / (1000 / TICK_HZ)) {
        /* If no overflow.  Above check assumes that TICK_HZ <= 1000 */
        msec = time * 1000 / TICK_HZ  ;

    } else {
        /* If it would overflow */
        msec = BT_D32_MASK - 1;
    } 

    return (msec);
}

static bt_tck_t btk_tck_get(void)
{
    ULONG	current_ticks;

    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  
    current_ticks = OSTimeGet();

    return((bt_tck_t)current_ticks);
}

#elif   defined(__linux__)

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter multiple times.
*/

static __inline__ bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert millisecods to clock ticks */
    /* time = number of milliseconds to convert to clock ticks */

    /* No error value is returned.  */

    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ( (BT_NO_WAIT == time) ? BT_NO_WAIT : 
	  (time > BT_D32_MASK/1000 -1) ? BT_D32_MASK-1 : (time * HZ / 1000) );
}

static __inline__ bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to milliseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the millisecond time is too large to   */
    /* be represented, then the max value is returned. */

    bt_msec_t msec;

    if ((BT_FOREVER == time) || (BT_NO_WAIT == time)) {
        msec = time;
    } else if (time < (BT_D32_MASK-1) * HZ / 1000) {
        msec = time * 1000 / HZ  ;

    } else {
        /* If it would overflow */
        msec = BT_D32_MASK - 1;
    } 

    return (msec);
}

static __inline__ bt_tck_t btk_tck_get(void)
{
    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  

    return((bt_tck_t) jiffies);
}

#elif defined (__lynxos)

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter multiple times.
*/

static __inline__ bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert millisecods to clock ticks */
    /* time = number of milliseconds to convert to clock ticks */

    /* No error value is returned.  */

    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ( (BT_NO_WAIT == time) ? BT_NO_WAIT : 
	  (time > ULONG_MAX/1000 -1) ? ULONG_MAX-1 : (time * LHZ / 1000) );
}

static __inline__ bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to milliseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the millisecond time is too large to   */
    /* be represented, then the max value is returned. */

    bt_msec_t msec;

    if ((BT_FOREVER == time) || (BT_NO_WAIT == time)) {
        msec = time;
    } else if (time < (ULONG_MAX-1) / (1000 / LHZ)) {
        msec = time * 1000 / LHZ  ;

    } else {
        /* If it would overflow */
        msec = ULONG_MAX - 1;
    } 

    return (msec);
}

static __inline__ bt_tck_t btk_tck_get(void)
{
    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  

    return((bt_tck_t) 0);
}

#else   /* Haven't written them yet */

#error  Code not written yet

#endif  /* Define data structures for each operating system */

#endif /* !defined(_BT_DELAY_H) */
