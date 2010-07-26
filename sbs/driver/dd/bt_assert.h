/*****************************************************************************
**
**      Filename:   bt_assert.h
**
**      Purpose:    Shared driver header file
**
**      Macros, Prototypes & Functions: 
**      BTK_ASSERT
**
**      $Revision: 1.2 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997 - 2004 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _BT_ASSERT_H
#define _BT_ASSERT_H

#if defined (__sun)
#include <sys/debug.h>  /* For ASSERT macro */

#elif defined (__vxworks)
#include <assert.h>

#elif defined (BT_uCOS)
#include <string.h>		/* For BTK_BZERO() macro, memset() */
#include <assert.h>		/* For BTK_ASSERT() macro, assert() */
#include <../sys/ucos/includes.h>

#endif  /* OS dependent header file inclusions */


/*****************************************************************************
**
**      Definition of BTK_ASSERT
**
*****************************************************************************/
/* NT needs NDEBUG define before we can start the assert definitions */
#if defined (BT_NTDRIVER)
#if DBG  
#undef NDEBUG
#else   /* !DBG */
#ifndef NDEBUG
#define NDEBUG
#endif  /* !NDEBUG */
#endif  /* !DBG */
#endif /* end BT_NTDRIVER */


#if     defined(NDEBUG)
#define BTK_ASSERT(X)   (TRUE)

#else   /* NDEBUG */

#if defined (_AIX)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}


#elif defined (BT_NTDRIVER)
#define BTK_ASSERT(EX)  if (!(EX)) { \
    FATAL_STR( #EX ); \
    DbgPrint(" Assertion failed -\n     " __FILE__ " LN " LINE_STR); \
    DbgBreakPoint(); \
}

#elif defined (__hpux)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__sgi)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__sun)

#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); debug_enter("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__vxworks)
#define BTK_ASSERT(EX)  assert((EX))

#elif defined (BT_uCOS)
#define BTK_ASSERT(EX)  assert((EX))

#elif defined (__linux__)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__lynxos)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#else
#error unknown operating system
#endif /* operating system */

#endif /* NDEBUG */

#endif /* !defined(_BT_ASSERT_H) */
