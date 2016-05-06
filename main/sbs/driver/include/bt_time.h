/*****************************************************************************
**
**      Filename:   bt_time.h
**
**      Purpose:    Shared driver header file
**
**      Macros, Defines, Prototypes & Functions: 
**      BT_NO_WAIT & BT_FOREVER
**
**      $Revision: 1.2 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-2005 by SBS Technologies, Inc.
**        Copyright (c) 1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BT_TIME_H)
#define _BT_TIME_H

/*
**  Define Mirror API constants that vary based on OS
*/
#define BT_FOREVER  (0xffffffff)
#define BT_NO_WAIT  (0)

typedef bt_data32_t bt_msec_t;

#endif /* !defined(_BT_TIME_H) */
