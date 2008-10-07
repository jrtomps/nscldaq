/*****************************************************************************
**
**      Filename:   btpdef.h
**
**      Purpose:    Bit 3 616, 617 & 618 adapter specific header file
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-1999,2001 by SBS Technologies, Inc.
**        Copyright (c) 1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BTPDEF_H)
#define _BTPDEF_H

#include <linux/errno.h>        /* Needed for system error numbers */


/*
** Various types used by the Mirror API
*/
typedef bt_data32_t bt_msec_t;
typedef bt_data32_t bt_devdata_t;
typedef bt_data32_t bt_devaddr_t;
typedef bt_data8_t  bt_addr_mod_t;

/*
**  Define Mirror API constants that vary based on OS
*/
#define BT_FOREVER  (0xffffffff)
#define BT_NO_WAIT  (0)

/*
** Define the various flags for memory mapping
*/
#define BT_MAP_WRITE            BT_WR
#define BT_MAP_READ             BT_RD
#define BT_MAP_SHARE            (1<<2)

#define BT_MAP_RW       (BT_MAP_READ | BT_MAP_WRITE)

/*
** Define interrupt handling information
*/
typedef enum {
   BT_IRQ_OVERFLOW,     /* Overflowed ability to deliver IRQ notifications */
   BT_IRQ_ERROR,        /* Error interrupt */
   BT_IRQ_PRG,
   BT_IRQ_IACK,
   BT_MAX_IRQ
} bt_irq_t;

#define BT_VECTOR_ALL   ((bt_data32_t) 0)

typedef  void bt_icbr_t(void * param_p, bt_irq_t irq_type, 
        bt_data32_t vector);

/*
** Macro to help align pointers
*/
#define BT_ALIGN_PTR(buf_p, align) ((((unsigned long)(buf_p))&((align) - 1))?\
                                    ((ptrdiff_t)((align)-(((unsigned long) \
                                     (buf_p))&((align)-1)))):((ptrdiff_t)0))
#endif /* !defined(_BTPDEF_H) */


