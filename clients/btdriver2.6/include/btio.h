/*****************************************************************************
**
**      Filename:   btio.h
**
**      Purpose:    Bit 3 Adaptor include file used by all projects.
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-2000 by SBS Technologies, Inc.
**        Copyright (c) 1990-1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _BTIO_H
#define _BTIO_H

/*****************************************************************************
**
**  Include common definitions header file
**
*****************************************************************************/

#include "btdef.h"

/*****************************************************************************
**
**  Include the proper platform specific include file here.
**
*****************************************************************************/

/*
**  AIX specific include files
*/

#if defined(_AIX)

#ifdef BT924
#include    <sys/btpio.h>
/* end ifdef  BT924     */

#elif defined(BT923)
#include    <sys/bthio.h>
/* end ifdef  BT923     */

#else /* BT924 || BT923 */
#error Undefined AIX project!
#endif /* BT924 || BT923 */

/* end ifdef _AIX */

/*
**  SunOS specific include files
*/
#elif defined(__sun)

#if defined(BT943) || defined(BT944)
#include    <sys/btsio.h>
/* end ifdef BT943 || BT944 */

#elif defined(BT945)
#include    "btpio.h"
/* end ifdef BT945 */

#elif defined(BT946)
#include    "btpio.h"
/* end ifdef BT946 */

#elif defined(BT999)
#include    "btuio.h"
/* end ifdef BT999 */

#elif defined(BT18902)
#include    "btqio.h"
/* end ifdef  BT18902 */

#elif defined(BT15906)
#include    "btmio.h"
/* end ifdef  BT15906 */

#else /* BT943 etc. */
#error Undefined Sun project!
#endif /* BT943 etc. */

/* end ifdef  sun */

/*
**  HPUX specific include files
*/
#elif defined(__hpux)

#ifdef BT933
#include <sys/bteio.h>
/* end ifdef BT933 */

#elif defined(BT934)
#include <sys/btpio.h>
/* end ifdef BT934 */

#else /* BT933 || BT934 */
#error Undefined HPUX Project!
#endif /* BT933 || BT934 */

/* end ifdef __hpux */

/*
**  SGI specific include files
*/
#elif defined(__sgi)

#if defined(BT965)
#include "btpio.h"
/* end ifdef BT965 */

#elif defined(BT964) 
#include <sys/btgio.h>
/* end ifdef BT964 */

#elif defined(BT963) 
#include <sys/btgio.h>
/* end ifdef BT963 */

#elif defined(BT15904)
#include "btmio.h"
/* end ifdef BT15904 */

#else /* BT965 etc. */
#error Undefined IRIX Project!
#endif /* BT965 etc. */

/* end ifdef __sgi */

/*
** Windows specific include files
*/
#elif defined(BT_NTDRIVER)

#if defined(BT13908)
#include "btfio.h"

#elif (defined(BT983) || defined(BT984))
#include "btwio.h"

#else /* BT13908 */
#error Undefined Windows Project!
#endif /* BT13908 */

/* end ifdef BT_NTDRIVER */

/*
** VxWorks specific include files
*/

#elif     defined(BT18901)
#include "btvio.h"
/* end ifdef  defined(BT18901) */

#elif     defined(BT993)
#include "btpio.h"
/* end ifdef  defined(BT993) */

/*
** NanoBridge specific include files
*/
#elif defined(BT17903)
#include "btuio.h"
/* end ifdef BT17903 */

/*
** Broadcast/Shared Memory specific include files
*/

#elif     defined(BT15901) || defined(BT15991)
#include "btmio.h"
/* end ifdef  BT15901 || BT15991 */

/*
**  uCOS specific include files
*/
#elif defined(BT_uCOS)

#if defined(BT2345)
/* endif BT2345 */

#else /* BT2345 */
#error Undefined uCOS project!
#endif /* BT2345 etc. */

/* end ifdef  uCOS */


/*
** Linux specific include files
*/

#elif defined(__linux__)

#if defined(BT1003)
#include "btpio.h"
#else /* BT1003 */
#error Undefined Linux project!
#endif /* BT1003 */

/*
** Unknown project
*/
#else /* _AIX etc. */
#error Unknown Project!
#endif /* _AIX etc. */

#endif /* !_BTIO_H */
