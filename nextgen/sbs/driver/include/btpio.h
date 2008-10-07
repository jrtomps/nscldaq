/*****************************************************************************
**
**      Filename:   btpio.h
**
**      Purpose:    Direct device driver access for PCI driver on Linux
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2002 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BTPIO_H)
#define _BTPIO_H

#include        <linux/ioctl.h>

#include        "btpdef.h"
#include        "btngpci.h"

#define BT_MAX_UNIT     15
#define BT_DRV_NAME     "btp"

#if     !defined(__KERNEL__)
#define BT_DEVNAME      "/dev/" BT_DRV_NAME
#endif  /* defined(__KERNEL__) */

/*****************************************************************************
**
**      ioctl() definitions
**
**  Define the ioctl() commands for this driver
**
**  Names and interface are chosen to maintain some source code compatibility 
**  with raw driver interface available on other versions of UNIX, allowing 
**  code written for other UNIX varients to be ported more easily.
**
*****************************************************************************/

typedef enum {
    BT_MIN_CTRL,                /* Range checking only */

    BT_CTRL_RESET=4,            /* reset local and remote cards */
    BT_CTRL_SETUP,              /* setup adapters for use */

    BT_CTRL_STATUS,             /* return status register */
    BT_CTRL_CLR_STATUS,         /* Clear local node status register */

    BT_CTRL_TAS,                /* Emulate Test and Set instruction */
    BT_CTRL_CAS,                /* Emulate Compare and Swap instruction */

    BT_CTRL_PARAM,              /* Adjust (set) driver parameter */
    BT_CTRL_DEV_ATTRIB,         /* Access current setting of driver parameter */

    BT_CTRL_IOREG,              /* Access and adapter I/O register */
    BT_CTRL_CFG,                /* Access PCI Configuration register */

    BT_CTRL_SND_INTR,           /* Send a programmed interrupt */

    BT_CTRL_SLOCK,              /* Simple device lock/unlock, replaces */
    BT_CTRL_SUNLOCK,            /* locking mechanism previously used */

    BT_CTRL_THREAD_ADD,         /* ICBR thread add */
    BT_CTRL_THREAD_DELETE,      /* ICBR thread delete */
    BT_CTRL_THREAD_REG,         /* ICBR thread register */
    BT_CTRL_THREAD_UNREG,       /* ICBR thread unregister */
    BT_CTRL_THREAD_WAIT,        /* ICBR thread wait */
    BT_CTRL_THREAD_WAKE,        /* ICBR thread wake */
    
    BT_CTRL_HW_READ,            /* Read from bus address */
    BT_CTRL_HW_WRITE,           /* Write to bus address */
    
    BT_CTRL_SEMA_TAKE,          /* Take semaphore */
    BT_CTRL_SEMA_GIVE,          /* Give semaphore */

    BT_CTRL_BIND,               /* Bind/unbind a user buffer for Slave DMA */
    BT_CTRL_UNBIND,             /* Not supported yet... */
    BT_CTRL_HW_BIND,            /* Bind bus address */
    BT_CTRL_HW_UNBIND,          /* Unbind bus address */

    BT_CTRL_LCARD_DIAG,			/* HW diags for local card */
    BT_CTRL_CABLE_DIAG,			/* HW diags for cable */
    BT_CTRL_RCARD_DIAG,			/* HW diags for remote card */
    BT_CTRL_PAIR_DIAG,			/* HW diags for adapter set */
    BT_CTRL_DRIVER_VERSION,			/* Driver version info */

    BT_MAX_CTRL                 /* Range checking only */
} bt_ctrl_t;


/*
** Commands supported in previous UNIX driver raw interface and not supported
** by Linux driver:
** BIOC_SET_HNDSHK, BIOC_CLR_HNDSHK, BIOC_LOG_ERROR, BIOC_NOLOG_ERROR,
** BIOC_REG_INTR, BIOC_AQR_INTR, BIOC_CLEAN, BIOC_LOCK, BIOC_UNLOCK.
*/
#define BT_IO_ERET_FLAG (0x80) /* If first structure element is error value */

#define BIOC_RESET      _IOR('b', BT_CTRL_RESET, bt_status_t)
#define BIOC_SETUP      _IOR('b', BT_CTRL_SETUP, bt_status_t)
#define BIOC_STATUS     _IOR('b', BT_CTRL_STATUS, bt_status_t)
#define BIOC_CLR_STATUS _IOR('b', BT_CTRL_CLR_STATUS, bt_status_t)

#define BIOC_TAS        _IOWR('b', BT_CTRL_TAS|BT_IO_ERET_FLAG, bt_tas_t)
#define BIOC_CAS        _IOWR('b', BT_CTRL_CAS|BT_IO_ERET_FLAG, bt_cas_t)

#define BIOC_PARAM      _IOWR('b', BT_CTRL_PARAM|BT_IO_ERET_FLAG, bt_param_t)
#define BIOC_DEV_ATTRIB _IOWR('b', BT_CTRL_DEV_ATTRIB|BT_IO_ERET_FLAG, bt_param_t)

#define BIOC_IOREG      _IOWR('b', BT_CTRL_IOREG|BT_IO_ERET_FLAG, bt_ioaccess_t)
#define BIOC_CFG        _IOWR('b', BT_CTRL_CFG|BT_IO_ERET_FLAG, bt_ioaccess_t)

#define BIOC_SND_INTR   _IO('b', BT_CTRL_SND_INTR)

#define BIOC_SLOCK      _IOWR('b', BT_CTRL_SLOCK|BT_IO_ERET_FLAG, bt_slock_t)
#define BIOC_SUNLOCK    _IO('b', BT_CTRL_SUNLOCK)

#define BIOC_BIND       _IOWR('b',(BT_CTRL_BIND|BT_IO_ERET_FLAG), bt_bind_t)
#define BIOC_UNBIND     _IOWR('b',(BT_CTRL_UNBIND|BT_IO_ERET_FLAG), bt_bind_t)
#define BIOC_HW_BIND       _IOWR('b', (BT_CTRL_HW_BIND|BT_IO_ERET_FLAG), bt_bind_t)
#define BIOC_HW_UNBIND     _IOWR('b', (BT_CTRL_HW_UNBIND|BT_IO_ERET_FLAG), bt_bind_t)

#define BIOC_HW_READ      _IOWR('b', (BT_CTRL_HW_READ|BT_IO_ERET_FLAG), bt_hw_xfer_t)
#define BIOC_HW_WRITE     _IOWR('b', (BT_CTRL_HW_WRITE|BT_IO_ERET_FLAG), bt_hw_xfer_t)

#define BIOC_THREAD_ADD    _IOWR('b', (BT_CTRL_THREAD_ADD|BT_IO_ERET_FLAG), bt_thread_add_t)
#define BIOC_THREAD_DELETE _IOWR('b', (BT_CTRL_THREAD_DELETE|BT_IO_ERET_FLAG), bt_thread_add_t)
#define BIOC_THREAD_REG    _IOWR('b', (BT_CTRL_THREAD_REG|BT_IO_ERET_FLAG), bt_thread_reg_t)
#define BIOC_THREAD_UNREG  _IOWR('b', (BT_CTRL_THREAD_UNREG|BT_IO_ERET_FLAG), bt_thread_reg_t)
#define BIOC_THREAD_WAIT   _IOWR('b', (BT_CTRL_THREAD_WAIT|BT_IO_ERET_FLAG), bt_thread_wait_t)
#define BIOC_THREAD_WAKE   _IOWR('b', (BT_CTRL_THREAD_WAKE|BT_IO_ERET_FLAG), bt_thread_wait_t)
#define BIOC_SEMA_TAKE     _IOWR('b', (BT_CTRL_SEMA_TAKE|BT_IO_ERET_FLAG), bt_sema_access_t)
#define BIOC_SEMA_GIVE     _IOWR('b', (BT_CTRL_SEMA_GIVE|BT_IO_ERET_FLAG), bt_sema_access_t)

#define BIOC_LCARD_DIAG     _IOWR('b', (BT_CTRL_LCARD_DIAG|BT_IO_ERET_FLAG), bt_hw_diag_t)
#define BIOC_CABLE_DIAG     _IOWR('b', (BT_CTRL_CABLE_DIAG|BT_IO_ERET_FLAG), bt_hw_diag_t)
#define BIOC_RCARD_DIAG     _IOWR('b', (BT_CTRL_RCARD_DIAG|BT_IO_ERET_FLAG), bt_hw_diag_t)
#define BIOC_PAIR_DIAG     _IOWR('b', (BT_CTRL_PAIR_DIAG|BT_IO_ERET_FLAG), bt_hw_diag_t)
#define BIOC_DRIVER_VERSION     _IOWR('b', (BT_CTRL_DRIVER_VERSION|BT_IO_ERET_FLAG), bt_hw_diag_t)

/*******************************************************************************
**      Kernel Level Routines and structures for adding user interrupt service
**  routines.
*******************************************************************************/
#if defined (__KERNEL__) 

#define DMA_PKT_SIZE            256     /* adaptor DMA packet size */

/*
**  Adapter kernel map structure for accessing remote resouces
*/
typedef struct {
    caddr_t     nio_p;          /* Ptr to node registers */
    caddr_t     mreg_p;         /* Pointer to allocated mapping register */
        caddr_t     rmem_p;         /* Pointer to section of remote memory to use */
        bt_data32_t loc_id;         /* Id of local card */
        bt_data32_t rem_id;         /* Id of remote card */
    bt_data32_t nio_len;        /* Lenght of node regs */
    bt_data32_t mreg_start;     /* Starting mapping register allocated */
    bt_data32_t mreg_len;       /* Number of isr mapping regs */
    bt_data32_t rmem_len;       /* Length of isr remote memory */
    bt_swap_t   default_swap;   /* Default swapping for adaptor */
} bt_kmap_t;                    /*   May be wrong if no remote power */
                                /*   during load or if remote */
                                /*   adaptor is changed without */
                                /*   rebooting */

/*
**  User interrupt service routine definition
*/
typedef bt_data32_t bt_uisr_t (u_short unit, caddr_t param_p, u_short irq_type);

/*
**  Structure for registering a remote bus interrupt.
*/
typedef struct {
    u_short     unit;               /* unit number of device */
    u_short     flags;              /* bt_irq_t values - BT_IRQ_OVERFLOW is illegal */
    int         level;              /* cable interrupt number */
    bt_uisr_t   *interrupt_routine; /* User ISR to call */
    caddr_t     param;              /* param passed to user ISR */
} bt_rembus_intr_t;

/*
** Prototypes for kernel functions
*/
extern int bt_kmap(unsigned int unit, bt_kmap_t *kmap_p);
extern int bt_kunmap(unsigned int unit, bt_kmap_t *kmap_p);
extern int bt_rembus_install(unsigned int unit, bt_rembus_intr_t *handler_p);
extern int bt_rembus_remove(unsigned int unit, bt_rembus_intr_t *handler_p);

#endif /* defined(__KERNEL__) */
#endif /* !defined(_BTPIO_H) */
