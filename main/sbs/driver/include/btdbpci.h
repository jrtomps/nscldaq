/*****************************************************************************
**
**      Filename:   btdbpci.h
**
**      Purpose:    Data Blizzard Adaptor include 
**                  file used only by Data Blizzard Device Drivers.
**
**      $Revision: 1.9 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2004 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined(_BTDBPCI_H) 
#define _BTDBPCI_H 

#if defined(__vxworks)
#include <stdio.h>
#endif /* defined(__vxworks) */




/*****************************************************************************
**
**  Macro indicating that this product is from the NanoBus family
**
*****************************************************************************/
#ifndef BT_NBUS_FAMILY
#define BT_NBUS_FAMILY
#endif

/* 
** Interrupt queue for storing a list of interrupts that have occured
*/
#define BTK_Q_PAD   2

typedef struct {
    volatile bt_data64_t    head;       /* queue index at which to add next message */
    bt_data32_t             length;     /* number of elements in queue buffer array */
    bt_data32_t             unused1;
    volatile bt_data32_t queue[BTK_Q_PAD];      /* queue buffer, is array of vectors */
} bt_irq_q_t;

/*
** Macro to check for proper size and packing of structures
** that follow
**
** Should be called early during configation
*/
#define BT_CHK_IOCTL_STRUCT                                 \
    if ((sizeof(bt_irq_q_t) != 24) ||                       \
        ((sizeof(bt_irq_q_t) % 8) != 0)) {                  \
        WARN_STR("bt_irq_q_t wrong size");                  \
    } else if ((sizeof(bt_send_vector_t) != 8) ||           \
               ((sizeof(bt_send_vector_t) % 8) != 0)) {     \
        WARN_STR("bt_send_vector_t wrong size");            \
    } else if ((sizeof(bt_param_t) != 16) ||                \
               ((sizeof(bt_param_t) % 8) != 0)) {           \
        WARN_STR("bt_param_t wrong size");                  \
    } else if ((sizeof(bt_tas_t) != 24) ||                  \
               ((sizeof(bt_tas_t) % 8) != 0)) {             \
        WARN_STR("bt_tas_t wrong size");                    \
    } else if ((sizeof(bt_cas_t) != 32) ||                  \
               ((sizeof(bt_cas_t) % 8) != 0)) {             \
        WARN_STR("bt_cas_t wrong size");                    \
    } else if ((sizeof(bt_thread_reg_t) != 16) ||           \
               ((sizeof(bt_thread_reg_t) % 8) != 0)) {      \
        WARN_STR("bt_thread_reg_t wrong size");             \
    } else if ((sizeof(bt_thread_add_t) != 24) ||           \
               ((sizeof(bt_thread_add_t) % 8) != 0)) {      \
        WARN_STR("bt_thread_add_t wrong size");             \
    } else if ((sizeof(bt_thread_wait_t) != 24) ||          \
               ((sizeof(bt_thread_wait_t) % 8) != 0)) {     \
        WARN_STR("bt_thread_wait_t wrong size");            \
    } else if ((sizeof(bt_hw_xfer_t) != 16) ||              \
               ((sizeof(bt_hw_xfer_t) % 8) != 0)) {         \
        WARN_STR("bt_hw_xfer_t wrong size");                \
    } else if ((sizeof(bt_sema_access_t) != 16) ||          \
               ((sizeof(bt_sema_access_t) % 8) != 0)) {     \
        WARN_STR("bt_sema_access_t wrong size");            \
    } else if ((sizeof(bt_hw_diag_t) != (BT_DIAG_MAX_REV_INFO + 20)) ||     \
               ((sizeof(bt_hw_diag_t) % 8) != 0)) {         \
        WARN_STR("bt_hw_diag_t wrong size");                \
    } else if ((sizeof(bt_bind_t) != 48) ||                 \
               ((sizeof(bt_bind_t) % 8) != 0)) {            \
        WARN_STR("bt_bind_t wrong size");                   \
    } else if ((sizeof(bt_ioaccess_t) != 16) ||             \
               ((sizeof(bt_ioaccess_t) % 8) != 0)) {        \
        WARN_STR("bt_ioaccess_t wrong size");               \
    } else if ((sizeof(bt_pcfg_access_t) != 40) ||          \
               ((sizeof(bt_pcfg_access_t) % 8) != 0)) {     \
        WARN_STR("bt_pcfg_access_t wrong size");            \
    } else if (sizeof(bt_pci_reg_t) != 80) {                \
        WARN_STR("bt_pci_reg_t wrong size");                \
    }


/*****************************************************************************
**
**  Logical Devices available. 
**
**  Not all avalible on all adapters.
**
*****************************************************************************/
typedef enum BT_AXSTYPS {
    /*
    ** Original Unix definitions
    */
    BT_AXSIO,                 /* Access I/O registers */
#define BT_MIN_DEV BT_AXSIO   /* Used for parameter checking */
    BT_AXSRDP,                /* Access Remote Dual Port RAM */
#define BT_AXSDP BT_AXSRDP    /* Old DP name references remote DP */
    BT_AXSRI,                 /* Access Remote IO */
    BT_AXSRR,                 /* Access Remote RAM */
    BT_AXSRE,                 /* Access Remote RAM Extended */
    BT_AXS24,                 /* Access Remote RAM 24-bit space */
    BT_AXSLM,                 /* Access Local Memory (remote window) */
    BT_AXSLDP,                /* Access Local Dual Port RAM */
    BT_MAX_AXSTYPS,           /* Maximum number of defined access types */

    /* 
    ** The following definition is used internally by some drivers 
    */
    BT_DEV_RE = BT_AXSRE,     /* Not to be used by application programs! */

    /*
    ** Mirror API definitions
    */
    BT_DEV_DP = BT_AXSRDP,    /* Dual Port RAM */
    BT_DEV_LDP = BT_AXSLDP,   /* Local Dual Port RAM */
    BT_DEV_RDP = BT_AXSRDP,   /* Remote Dual Port RAM */
    BT_DEV_IO = BT_AXSRI,     /* Remote bus A16 (I/O) space */
    BT_DEV_A24 = BT_AXS24,    /* Remote bus A24 (short) space */
    BT_DEV_RR = BT_AXSRR,     /* Remote bus A32 (standard) space */
    BT_DEV_LM = BT_AXSLM,     /* Local Memory device */
    BT_DEV_NIO = BT_AXSIO,    /* Adapter Node I/O registers */
    BT_DEV_DIAG = BT_AXSIO,   /* For Internal Use Only! */
    
    /*
    ** Mirror API aliases
    */
    BT_DEV_A16 = BT_DEV_IO,
    BT_DEV_A32 = BT_DEV_RR,
    BT_DEV_MEM = BT_DEV_RR,
    BT_DEV_DEFAULT = BT_DEV_DP,

    BT_MAX_DEV = BT_MAX_AXSTYPS
    
} bt_dev_t;

#define BT_DEV_SHFT     (5)
#define BT_AXS_SHFT     BT_DEV_SHFT     /* Older name */

/*
** Linux requires that BT_MAX_UNITS be a simple integer with no parathesis
** or operators. It MUST NOT be an expression.
*/
#define BT_MAX_UNITS    31      /* Don't change or you will break Linux */
#if BT_MAX_UNITS != ((1 << BT_DEV_SHFT) - 1)
#error  "Shift and maximum values out of sync."
#endif  /* BT_MAX_UNITS */

/*****************************************************************************
**
**  Structure for parameter set or acquire 
**
**  BIOC_PARAM & BIOC_DEV_ATTRIB
**
*****************************************************************************/
typedef struct {
    bt_data32_t    error;               /* Error code */
    bt_data32_t    vector;              /* Vector to send with irq */
                                        /* Must be below BT_DRV_VECTOR_BASE */
} bt_send_vector_t;

/*****************************************************************************
**
**  Structure for parameter set or acquire 
**
**  BIOC_PARAM & BIOC_DEV_ATTRIB
**
*****************************************************************************/
typedef struct {
    bt_data32_t    error;           /* Error code */
    bt_data32_t    param;           /* Parameter read or write - bt_info_t */
#if !defined (__lynxos)
    bt_data64_t    value;           /* Value to write or value read */
#else
    bt_data32_t    value;           /* Value to write or value read */
    bt_data32_t    unused1;           /* Value to write or value read */
#endif
} bt_param_t;

/*****************************************************************************
**
**  Test and Set IOCTL structure 
**
**  BIOC_TAS
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;              /* Error code */
    bt_data32_t     unused1;
    bt_data64_t     addr;               /* Address of byte to access */
    bt_data8_t      result;             /* Result:     0 = bit was clear */
    bt_data8_t      unused2;            /*             1 = bit was set */
    bt_data16_t     unused3;
    bt_data32_t     unused4;
} bt_tas_t;                             

#define  BT_TAS_MSBIT 0x80

/*****************************************************************************
**
**  Compare and swap IOCTL structure 
**
**  BIOC_CAS
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;          /* Error code */
    bt_data32_t     unused1;
    bt_data64_t     addr;           /* Address to check */
    bt_data32_t     cmp;            /* Compare value */
    bt_data32_t     swap;           /* Swap value */
    bt_data32_t     len;            /* Length of access (sizeof) parameter */
                                    /* Valid values are BT_WIDTH_D?, not 64 */
    bt_data32_t     result;         /* Result of operation */
} bt_cas_t;

/*****************************************************************************
**
**  ICBR thread register/unregister structure 
**
**  BIOC_THREAD_REG & BIOC_THREAD_UNREG
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;              /* Error code */
    
    /* Returned from driver */
    bt_data32_t     unit;               /* Unit number, library requires this */
                                        /*   to do mmap of diag device */
    bt_data64_t     thread_id;          /* Driver level thread id */
    
} bt_thread_reg_t;

/*****************************************************************************
**
**  ICBR thread add/delete structure 
**
**  BIOC_THREAD_ADD & BIOC_THREAD_DELETE
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;          /* Error code */
    bt_data32_t     irq_type;       /* Irq type interrested in - bt_irq_t */
    bt_data32_t     vector;         /* Vector - not currently used by driver */
    bt_data32_t     unused1;        
    bt_data64_t     thread_id;      /* Driver level thread id */
    
} bt_thread_add_t;

/*****************************************************************************
**
**  ICBR thread wait structure 
**
**  BIOC_THREAD_WAIT
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;              /* Error code */
    bt_data32_t     unused1;        
    bt_data64_t     thread_id;          /* Driver level thread id */
    bt_data32_t     wait_msec;          /* Number of mSec to wait for irq */
    bt_data32_t     unused2;            /*   First release only supports BT_NO_WAIT */
    
} bt_thread_wait_t;

/*****************************************************************************
**
**  HW Address Transfer 
**
**  BIOC_HW_READ & BIOC_HW_WRITE
**
*****************************************************************************/
typedef struct {
    bt_data32_t  error;             /* Error code */
    bt_data32_t  local_hw_addr;     /* Local bus address to xfer to/from */
    bt_data32_t  ldev_addr;         /* Logical device address to xfer to/from */
    bt_data32_t  xfer_len;          /* Number of bytes to transfer */
                                    /* On return, number of bytes transferred */
    
} bt_hw_xfer_t;

/*****************************************************************************
**
**  Semaphore Take/Give 
**
**  BIOC_SEMA_TAKE/BIOC_SEMA_GIVE
**
*****************************************************************************/
typedef struct {
    bt_data32_t     error;              /* Error code */
    bt_data32_t     sema_reg;           /* Which semaphore to take/give - bt_reg_t */
    bt_data32_t     wait_msec;          /* Number of mSec to wait for semaphore */
    bt_data32_t     unused1;            /*   First release only supports BT_NO_WAIT */
} bt_sema_access_t;

/*****************************************************************************
**
**  Hardware Diagnostics 
**
**  BIOC_LCARD_DIAG/BIOC_CABLE_DIAG/BIOC_RCARD_DIAG/BIOC_PAIR_DIAG
**
*****************************************************************************/
#define BT_DIAG_MAX_REV_INFO    28
typedef struct {
    bt_data32_t  error;        /* Error code */
    bt_data32_t  line_number;  /* Line number failure was discovered on */
    bt_data32_t  r_ldev;       /* Remote logical device to use during testing */
    bt_data32_t  r_addr;       /* Remote address to use during testing */
    bt_data32_t  r_len;        /* Size in bytes of remote resource to test */
    bt_data8_t   rev_info[BT_DIAG_MAX_REV_INFO];      /* Text string with file and revision number */
    
} bt_hw_diag_t;


/*****************************************************************************
**
**  Status structure 
**
**  BIOC_RESET, BIOC_STATUS, BIOC_CLR_STATUS, BIOC_SETUP
**
*****************************************************************************/
typedef bt_data32_t bt_status_t;
#define BT_STATUS_ERR_SHFT    (24)

    /* interface parity error */
#define BT_STATUS_PARITY      (LSR_PARITY_ERR<<BT_STATUS_ERR_SHFT)   

    /* remote bus error */
#define BT_STATUS_REMBUS      (LSR_REMBUS_ERR<<BT_STATUS_ERR_SHFT)   

    /* interface timeout */
#define BT_STATUS_TIMEOUT     (LSR_TIMEOUT_ERR<<BT_STATUS_ERR_SHFT)

    /* remote bus power or cable missing */
#define BT_STATUS_POWER       (LSR_NO_CONNECT<<BT_STATUS_ERR_SHFT)

#define BT_STATUS_RRESET      (1<<19)   /* remote bus was reset */

#define BT_STATUS_MASK        (BT_STATUS_TIMEOUT|BT_STATUS_POWER|BT_STATUS_PARITY|BT_STATUS_REMBUS)

/* Old names to be obsoleted, use above names for new development */
#define BT_INTR_XMITR      (1<<17)      /* Not used anymore */
#define BT_INTR_OVRRUN     (1<<16)      /* Not used anymore */
#define BT_INTR_RRESET     BT_STATUS_RRESET  
#define BT_INTR_POWER      BT_STATUS_POWER   
#define BT_INTR_TIMEOUT    BT_STATUS_TIMEOUT 
#define BT_INTR_REMBUS     BT_STATUS_REMBUS  
#define BT_INTR_PARITY     BT_STATUS_PARITY  
#define BT_INTR_ERR_SHFT   BT_STATUS_ERR_SHFT

/*****************************************************************************
**
**  Bind structure 
**
**  BIOC_BIND, BIOC_UNBIND
**  BIOC_HW_BIND, BIOC_HW_UNBIND
**
*****************************************************************************/
typedef struct bt_bind_d {
    bt_data32_t     error;          /* Error code  */
    bt_data32_t     unused1;   
    bt_data64_t     host_addr;      /* Virtual address of user buffer to bind */
                                    /*    or physical bus address for HW_BIND */
    bt_data32_t     length;         /* Length of buffer to bind in bytes */
    bt_data32_t     unused2;   
    bt_data8_t      nowait;         /* FALSE: wait if resources unavailable */
    bt_data8_t      unused3;        /*   TRUE: return errno = EBUSY, don't wait */
    bt_data16_t     unused4;
    bt_data32_t     unused5;
    
    /* 
    **  Fields below are written by the driver if binding succeeds.
    */ 
    bt_data32_t     phys_addr;      /* 16 Mbyte offset in VMEbus remote RAM */
                                    /*   window to access to reach buffer */
                                    /*   Should be BT_BIND_NO_CARE if specific */
                                    /*   alignment is not required */
#define BT_BIND_NO_CARE (0xffffffff)
    bt_data32_t     unused6;

    /* 
    ** Following information is for internal device driver use only!
    */ 
    bt_data64_t     sysinfo_p;      /* Pointer to internal system information */
     
} bt_bind_t;

/*****************************************************************************
**
**  Adaptor register access stucture 
**
**  BIOC_IOREG
**
*****************************************************************************/
typedef struct {
    bt_data32_t        error;                /* Error code */
    bt_data32_t        operation;            /* one of enum BT_ACCESS */ 
                                             /* use enum BT_ACCESS values */
    bt_data32_t        offset;               /* Appropriate register offset */
                                             /* use enum BT_REGISTERS values*/
    bt_data32_t        value;                /* Value to write or one-shot */
} bt_ioaccess_t;

/*****************************************************************************
**
**  PCI configuration register access stucture 
**
**  BIOC_PCFG
**
*****************************************************************************/
typedef struct {
    bt_data32_t         error;      /* Error code */
    bt_data32_t         operation;  /* one of enum BT_ACCESS */ 
                                    /* use enum BT_ACCESS values */
    bt_data32_t         bus;        /* Bus number (0 - 255) NOT supported yet */
    bt_data32_t         device;     /* Device number (0 - 15) */
    bt_data32_t         function;   /* Function number (0 - 7) */
    bt_data32_t         size;       /* Access width (1, 2 or 4) */
    bt_data32_t         offset;     /* Configuration register offset */
    bt_data32_t         value;      /* Value to write or value read */
    bt_data32_t         type0;      /* TRUE or FALSE, NOT supported yet */
    bt_data32_t         unused1;
} bt_pcfg_access_t;

/*****************************************************************************
**
**  Adjustable driver parameters.  
**
**  Not all are supported on each platform.
**
**  Use with bt_param_t structure (BIOC_PARAM & BIOC_DEV_ATTRIB)
**
**  #defined names are obsolete and should not be used.  They are for
**  compatability only.  Use BT_INFO_ names for new development.
**
*****************************************************************************/
typedef enum param_valus {
    /*
    ** Mirror API info parameters 
    */
    BT_MIN_INFO,        /* Range checking only */
#define INVALID                 BT_MIN_INFO

    BT_INFO_BLOCK,        /* Can use BLT mode: TRUE/FALSE */
#define DMA_BLOCK               BT_INFO_BLOCK

    BT_INFO_PAUSE,        /* Set pause mode for DMA: TRUE/FALSE */
    /* #define DMA_PAUSE               BT_INFO_PAUSE */ /* Conflicts with 3.x kernel defs */

    BT_INFO_DATAWIDTH,        /* Maximum access width allowed on a transfer */
#define DATA_SIZE               BT_INFO_DATAWIDTH

    BT_INFO_DMA_AMOD,        /* Address Modifier to use for DMA */
#define DMA_ADDR_MOD            BT_INFO_DMA_AMOD

    BT_INFO_PIO_AMOD,        /* Address Modifier to use for PIO */
#define PIO_ADDR_MOD            BT_INFO_PIO_AMOD

    BT_INFO_MMAP_AMOD,        /* Address Modifier to use for MMAP */
#define MMAP_ADDR_MOD           BT_INFO_MMAP_AMOD

    BT_INFO_SWAP,               /* Swapping value to use for logical dev */
#define SWAP_BITS               BT_INFO_SWAP

    BT_INFO_DMA_THRESHOLD,      /* Minimum length before we attempt DMA */
#define THRESHOLD               BT_INFO_DMA_THRESHOLD

    BT_INFO_DMA_POLL_CEILING,   /* Maximum length for polled mode DMA. */
#define DMA_POLL_SIZE           BT_INFO_DMA_POLL_CEILING

    BT_INFO_DMA_WATCHDOG,       /* Watchdog timer for DMA, default 5 seconds */
#define DMA_TIMEOUT             BT_INFO_DMA_WATCHDOG

    BT_INFO_TRACE,            /* Software tracing level */
#define TRACE_LVL               BT_INFO_TRACE

    BT_INFO_RESET_DELAY,    /* Wait time for remote reset to complete */
#define RESET_TIMER         BT_INFO_RESET_DELAY

    BT_INFO_USE_PT,         /* Send PT programmed irqs instead of PR */

    BT_INFO_INC_INHIB,    /* Set increment inhibit for DMA: TRUE/FALSE */
#define DMA_INC_INHIB       BT_INFO_INC_INHIB

    BT_INFO_ICBR_PRIO,     /* Task priority of interrupt dispatch, vxWorks only */
    
    BT_INFO_ICBR_STACK,     /* Task stack size for interrupt dispatch, vxWorks only */

    BT_INFO_GEMS_SWAP,          /* Enable GEMS swapping, GEMS Only */
    
    BT_INFO_LOC_PN,         /* Part number of local card */
#define BT_INFO_GET_BASE    BT_INFO_LOC_PN
#define INF_LDEV_TYPE       BT_INFO_LOC_PN

    BT_INFO_REM_PN,    /* Part number of remote card */
#define INF_RDEV_TYPE       BT_INFO_REM_PN

    BT_INFO_LM_SIZE,    /* Size of local memory device */
#define INF_LM_SIZE         BT_INFO_LM_SIZE

    BT_INFO_ICBR_Q_SIZE,    /* Size of the interrupt dispatch queue */
#define INF_ICBR_Q_SIZE     BT_INFO_ICBR_Q_SIZE

    BT_INFO_BIND_ALIGN,    /* Alignment required for bt_bind() buffers */
#define INF_BIND_ALIGN      BT_INFO_BIND_ALIGN

    BT_INFO_BIND_SIZE,    /* Maximum size of a single bound buffer */
#define INF_BIND_SIZE       BT_INFO_BIND_SIZE

    BT_INFO_BIND_COUNT,    /* Number of concurrent bind requests allowed */
#define INF_BIND_COUNT      BT_INFO_BIND_COUNT

    BT_INFO_TRANSMITTER,    /* TRUE = transmitter mode; FALSE = receiver mode */
#define INF_TRANSMITTER     BT_INFO_TRANSMITTER

    BT_INFO_UNIT_NUM,    /* Unit number (0..15) */
#define INF_UNIT            BT_INFO_UNIT_NUM

    BT_INFO_KMEM_SIZE,      /* Total kernel memory used in bytes */
#define INF_TOTAL_KMEM      BT_INFO_KMEM_SIZE

    BT_INFO_ERROR_COUNT,    /* Number of error interrupts received */
#define BT_INFO_SIG_ERR     BT_INFO_ERROR_COUNT
#define INF_SIG_ERR         BT_INFO_ERROR_COUNT

    BT_INFO_EVENT_COUNT,    /* Number of events (prog irqs) received */
#define BT_INFO_SIG_PRG     BT_INFO_EVENT_COUNT
#define INF_SIG_PRG         BT_INFO_EVENT_COUNT

    BT_INFO_TOTAL_COUNT,    /* Total number of intterunts */
#define BT_INFO_SIG_TOT     BT_INFO_TOTAL_COUNT
#define INF_SIG_TOT         BT_INFO_TOTAL_COUNT

    BT_INFO_IACK_COUNT,     /* Total number of IACK intterunts */
#define BT_INFO_SIG_IACK    BT_INFO_IACK_COUNT
#define INF_SIG_IACK        BT_INFO_IACK_COUNT

    BT_INFO_ICBR_Q_START,   /* starting memory address of icbr_q, vxWorks only */

    INF_LOG_STAT,
#define BT_INFO_LOG_STAT    INF_LOG_STAT

    BT_INFO_BOARD_REV,      /* PCI Rev ID of board, firmware revision */

#if defined (BT1003)
/* EAS A64 */
    BT_INFO_A64_OFFSET,    /* A64 mode bits A32-A63 and mode flag MSB */
/* EAS TMP CODE */
    BT_INFO_KMALLOC_BUF,    /* return kmalloc buffer from driver */ 
    BT_INFO_KFREE_BUF,      /* free kmalloc buffer allocated by driver */
    BT_INFO_KMALLOC_SIZ,    /* size of buffer to kmalloc from driver */
#endif /* BT1003 */

    BT_MAX_INFO        /* Last valid BT_INFO_ value, internal use only */
} bt_info_t;

/* 
** Obsolte parameter values -- to be removed! 
*/

#define INTR_FREEQ_COUNT    BT_MAX_INFO
#define REM_ADDR            BT_MAX_INFO
#define ADDR_MOD            BT_MAX_INFO
#define HNDSHK_DR_COUNT     BT_MAX_INFO
#define HNDSHK_USEC_TIME    BT_MAX_INFO

#define INF_SLOT            BT_MAX_INFO
#define INF_INTR_LVL        BT_MAX_INFO
#define INF_INTR_PRIO       BT_MAX_INFO
#define INF_PT_INTR_LVL     BT_MAX_INFO
#define INF_REMLOC          BT_MAX_INFO
#define INF_REMLEN          BT_MAX_INFO
#define INF_INTR_REGS       BT_MAX_INFO
#define INF_INTR_PICS       BT_MAX_INFO
#define INF_PIO_ERROR       BT_MAX_INFO
#define INF_MEM_PTR         BT_MAX_INFO
#define INF_INTR_QCNT       BT_MAX_INFO

/*****************************************************************************
**
**  Swapping values.
**
**  Not all are supported on each platform.
**
**  Use with SWAP_BITS parameter (BIOC_PARAM & BIOC_DEV_ATTRIB)
**
*****************************************************************************/
typedef enum BT_SWAP_BITS_VALUE {
    BT_SWAP_NONE = 0,   /* No swapping */
    BT_SWAP_BSNBD,      /* Byte swap, non byte data */
    BT_SWAP_WS,         /* Word swap */
    BT_SWAP_WS_BSNBD,   /* Word swap and byte swap, non btye data */
    BT_SWAP_BSBD,       /* Byte swap, btye data */
    BT_SWAP_BSBD_BSNBD, /* Btye swap, byte data and byte swap, non byte data */
    BT_SWAP_BSBD_WS,    /* Byte swap, byte data and word swap */
    BT_SWAP_BSBD_WS_BSNBD,  /* All possible swapping */
    BT_SWAP_DEFAULT,    /* Driver default swapping */
    BT_MAX_SWAP
} bt_swap_t;


/*****************************************************************************
**
**  Register access types
**
**  Use with bt_ioaccess_t & bt_pcfg_access_t structure 
**  (BIOC_IOREG & BIOC_PCFG)
**
*****************************************************************************/
enum BT_ACCESS {
    BT_IOREAD,  /* Register read */
    BT_IOWRITE, /* Register write */
    BT_IOSHOT   /* Register bit set */
};

/*****************************************************************************
**
**  Valid registers
**
**  Use with bt_ioaccess_t structure (BIOC_IOREG)
**
*****************************************************************************/
#define BT_LOC_START    0
#define BT_REM_START    80
#define BT_RPQ_START    192

typedef enum BT_REGISTERS {
    BT_LOC_CMD1 = BT_LOC_START,         /* Local Command register 1 */
    BT_LOC_INT_CTRL,                    /* Local Interrupt Control */
    BT_LOC_INT_STATUS,                  /* Local Interrupt Status */
    BT_LOC_STATUS,                      /* Local Status register */
    BT_LOC_HANDSHAKE,                   /* Local Handshake register */
    BT_LOC_CONFIG_CTRL,                 /* Local Configuration Register */
    BT_LOC_RCMD2,                       /* Local access to remote command 2 */
    BT_LOC_RPAGE,                       /* Local access to remote page reg  */
    BT_LOC_AMOD,                        /* Local address modifier */
    BT_LDMA_CMD,                        /* Local DMA Command register */
    BT_LDMA_RMD_CNT,                    /* Local DMA Remainder Cnt register */
    BT_LDMA_ADDR,                       /* Local DMA Address register */
    BT_LDMA_PKT_CNT,                    /* Local DMA Packet count register */
    BT_LOC_TCSIZE,                      /* TURBOChannel size register */
    BT_LOC_256MB_PAGE,                  /* Local 256MB window remote page */
    BT_LOC_RCMD1,                       /* Local access to remote command 1 */
    BT_LOC_BUS_CTRL,                    /* Local bus control reg (PCI) */
    BT_LOC_LOOPBACK,                    /* Local loopback control (PCI Fiber) */
    BT_LOC_MREG_CTRL,                   /* Local mapping register control */
    BT_LOC_SEMA0,                       /* Local semaphore register 0 */
    BT_LOC_SEMA1,                       /* Local semaphore register 1 */
    BT_LOC_SEMA2,                       /* Local semaphore register 2 */
    BT_LOC_SEMA3,                       /* Local semaphore register 3 */
    BT_LOC_SEMA4,                       /* Local semaphore register 4 */
    BT_LOC_SEMA5,                       /* Local semaphore register 5 */
    BT_LOC_SEMA6,                       /* Local semaphore register 6 */
    BT_LOC_SEMA7,                       /* Local semaphore register 7 */

    BT_REM_CMD1 = BT_REM_START,         /* Remote Command register 1 */
    BT_REM_STATUS,                      /* Remote Status register */
    BT_REM_CMD2,                        /* Remote Command register 2 */
    BT_REM_PAGE,                        /* Remote Page Register */
    BT_REM_AMOD,                        /* Remote Address Modifier */
    BT_REM_IACK,                        /* Remote IACK register */
    BT_REM_CARD_ID,                     /* Remote Card ID Register */
    BT_RDMA_ADDR,                       /* Remote DMA Address */
    BT_RDMA_RMD_CNT,                    /* Remote DMA Remainder Count */
    BT_REM_MREG_CTRL,                   /* Remote mapping register control */
    BT_REM_SLAVE_STATUS,                /* Remote slave status */
    BT_REM_SLAVE_CLRERR,                /* Remote slave clear error */
    BT_REM_SEMA0,                       /* Remote semaphore register 0 */
    BT_REM_SEMA1,                       /* Remote semaphore register 1 */
    BT_REM_SEMA2,                       /* Remote semaphore register 2 */
    BT_REM_SEMA3,                       /* Remote semaphore register 3 */
    BT_REM_SEMA4,                       /* Remote semaphore register 4 */
    BT_REM_SEMA5,                       /* Remote semaphore register 5 */
    BT_REM_SEMA6,                       /* Remote semaphore register 6 */
    BT_REM_SEMA7,                       /* Remote semaphore register 7 */

    /*
    ** Do not access these registers unless you have RPQ #601107 
    ** These registers allow direct access to all registers located on
    ** the remote card
    **
    ** For non RPQ #601107 cards these registers will return garbage
    */
    BT_RPQ_REM_LCMD1 = BT_RPQ_START,
    BT_RPQ_REM_LINT_CTRL,
    BT_RPQ_REM_LSTATUS,
    BT_RPQ_REM_LINT_STATUS,
    BT_RPQ_REM_LBUS_CTRL,
    BT_RPQ_REM_LLOOPBACK,
    
/* EAS A64 */
    /*
    ** Do not access these registers unless you have RPQ #601332
    ** These registers allow direct access to all registers located on
    ** the remote card
    **
    ** For non RPQ #601332 cards these registers will return garbage
    */
    BT_RPQ_REM_A64PIO,              /* Remote card Upper address A64 - PIO */
    BT_RPQ_REM_A64DMA,              /* Remote card Upper address A64 - DMA */
    
    BT_MAX_REGISTER                 /* Maximum number of registers */
} bt_reg_t;

/* The following member names have been obsoleted:
    BT_REM_LSTATUS    replaced by BT_REM_SLAVE_STATUS
    BT_REM_LCLRERR    replaced by BT_REM_SLAVE_CLRERR
    BT_MAX_REGISTERS  replaced by BT_MAX_REGISTER
*/
#define MAX_IOREGS              (0x50)  /* maximum number of I/O registers */ 

/*****************************************************************************
**
**  CSR Layout - PCI to VMEbus or PCI to PCI Products Only
**
**  Register bit definitions
**
**  Use with bt_ioaccess_t structure (BIOC_IOREG)
**
*****************************************************************************/
typedef struct {

/*
** Local Node Command Register (read/write, 8 bits)
*/
        bt_data8_t         loc_cmd1;
#define LOC_CMD1        (0x00)  /* Local Command Register */

#define LC1_CLR_ERROR   (1<<7)  /* w = 1, Clear status error bits */
#define LC1_CLR_PR_INT  (1<<6)  /* Clear the PR interrupt */
#define LC1_SND_PT_INT  (1<<5)  /* Send a PT interrupt */
#define LC1_PT_STATUS   LC1_SND_PT_INT  /* Keep consistent with PR name */
/* Bits 4-0 must always be set to 0. */

/*
** Local Interrupt Control Register (read/write, 8 bits)
*/
        bt_data8_t        loc_int_ctrl;
#define LOC_INT_CTRL            (0x01)  /* local interrupt control register */
#define LOC_INT_CTRL_SIZE       (BT_WIDTH_D8)
#define LIC_INT_PENDING         (1<<7)  /* r  , set when generating irq     */
#define LIC_INT_ENABLE          (1<<6)  /* r/w, non-error interrupt enable  */
#define LIC_ERROR_INT_ENABLE    (1<<5)  /* r/w, error interrupt enable      */
#define LIC_PT_CBL_MASK         (0x7)   /* mask for PT cable select line    */
#define LIC_PT_CBL_NONE         (0x0)   /* send PT interrupt disabled       */
#define LIC_PT_CBL_1            (0x1)   /* PT interrupt sent on cable irq 1 */
#define LIC_PT_CBL_2            (0x2)   /* PT interrupt sent on cable irq 2 */
#define LIC_PT_CBL_3            (0x3)   /* PT interrupt sent on cable irq 3 */
#define LIC_PT_CBL_4            (0x4)   /* PT interrupt sent on cable irq 4 */
#define LIC_PT_CBL_5            (0x5)   /* PT interrupt sent on cable irq 5 */
#define LIC_PT_CBL_6            (0x6)   /* PT interrupt sent on cable irq 6 */
#define LIC_PT_CBL_7            (0x7)   /* PT interrupt sent on cable irq 7 */

#define LIC_EN_INTR     (LIC_INT_ENABLE | LIC_ERROR_INT_ENABLE | LIC_PT_CBL_1)
#define LIC_DIS_INTR    (LIC_PT_CBL_1)

/*
** This bit definition only works with RPQ #601107 
*/
#define LIC_INT_D_SET           (1<<4)  /* r/w, Set/Clear INTD# RPQ #601107 only */

/*
** Local Node Status Register (read only, 8 bits)
*/
        bt_data8_t         loc_status;
#define LOC_STATUS      (0x02)  /* Local Status Register */

#define LSR_PARITY_ERR          (1<<7)
#define LSR_REMBUS_ERR          (1<<6)
#define LSR_PR_STATUS           (1<<5)
#define LSR_TIMEOUT_ERR         (1<<2)
#define LSR_LRC_ERR             (1<<1)  /* DMA only */
#define LSR_NO_CONNECT          (1<<0)

#define LSR_ERROR_MASK  (LSR_PARITY_ERR|LSR_LRC_ERR|LSR_REMBUS_ERR|LSR_TIMEOUT_ERR)
#define LSR_CERROR_MASK (LSR_NO_CONNECT|LSR_ERROR_MASK)

/*
** Local Interrupt Status Register (read only, 8 bits)
*/
        bt_data8_t    loc_int_status;
#define LOC_INT_STATUS          (0x03)  /* interrupt status register */
#define LOC_INT_STATUS_SIZE     (BT_WIDTH_D8)
#define LIS_CINT7               (1<<7)  /* r  , cable interrupt 7 */
#define LIS_CINT6               (1<<6)  /* r  , cable interrupt 6 */
#define LIS_CINT5               (1<<5)  /* r  , cable interrupt 5 */
#define LIS_CINT4               (1<<4)  /* r  , cable interrupt 4 */
#define LIS_CINT3               (1<<3)  /* r  , cable interrupt 3 */
#define LIS_CINT2               (1<<2)  /* r  , cable interrupt 2 */
#define LIS_CINT1               (1<<1)  /* r  , cable interrupt 1 */
#define LIS_CINT_MASK (LIS_CINT1 | LIS_CINT2 | LIS_CINT3 | LIS_CINT4 | LIS_CINT5 | LIS_CINT6 | LIS_CINT7)

/*
** Local Bus Control Register (read/write, 8 bits)
*/
        bt_data8_t    loc_bus_ctrl;
#define LOC_BUS_CTRL            (0x04)  /* bus control register         */
#define LOC_BUS_CTRL_SIZE       (BT_WIDTH_D8)
#define LBC_GEMS_SWAP_ENABLE    (1<<3)  /* enable lword swap for GEMS   */
#define LBC_WS_ENABLE           (1<<2)  /* enable word swap             */
#define LBC_PREEMPT_DISABLE     (1<<1)  /* disable local preempt        */
#define LBC_TRG_ABORT_ENABLE    (1<<0)  /* target abort enable          */

#if defined(BT965)
#define LBC_NORM_OPER           (LBC_WS_ENABLE)
#else
#define LBC_NORM_OPER           (0x0)
#endif /* BT965 */

/*
** Loopback Control Register (read/write, 8 bits)
** Only avaliable on special adapter models, check HW manual
*/
        bt_data8_t    loc_loopback;
#define LOC_LOOPBACK            (0x05)  /* loopback control register    */
#define LOC_LOOPBACK_SIZE       (BT_WIDTH_D8)
#define LLB_LINK_UP             (1<<7)  /* r, link up                   */
#define LLB_REM_LB_ENABLE       (1<<1)  /* enable local loopback */
#define LLB_LOC_LB_ENABLE       (1<<0)  /* enable local loopback */

/*
** Mapping RAM Control Register (read/write, 8 bits)
** Only avaliable on certain adapter models, check HW manual
*/
        bt_data8_t    loc_mreg_ctrl;
#define LOC_MREG_CTRL           (0x06)  /* map reg control register */
#define LOC_MREG_CTRL_SIZE      (BT_WIDTH_D8)
#define LMC_DRIVER_MASTER       (1<<3)  /* local driver master */
#define LMC_DRIVER_ACTIVE       (1<<2)  /* local driver loaded */
#define LMC_DMA_BYPASS_ENABLE   (1<<1)  /* DMA to local bus bypass */
#define LMC_PIO_BYPASS_ENABLE   (1<<0)  /* cable to local bus bypass */

/*
** Undefined area of CSR register space
*/
        bt_data8_t         reserve07;

/*
** Remote Node Command Register 1 (read/write, 8 bits)
*/
        bt_data8_t         rem_cmd1;
#define REM_CMD1        (0x08)
#define RC1_RESET       (1<<7)  /* Reset remote bus */
#define RC1_CLR_PT_INT  (1<<6)  /* Clear PT interrupt */
#define RC1_SND_PR_INT  (1<<5)  /* Send PR interrupt */
#define RC1_LOCK_SEL    (1<<4)  /* Lock Remote bus */
/* Bit 3 must always be set to "0" */
#define RC1_IACK_MASK   (0x7)   /* IACK read level select */

/*
** Remote Node Status Register (read, 8 bits)
** Same ofset as Remote Note command register 1
*/
#define rem_status          rem_cmd1
#define REM_STATUS          (0x08)  /* remote status register */
#define RSR_WAS_RESET       (1<<7)  /* Remote bus was reset */
                                    /* Valid only for A32 VMEbus adaptors */
#define RSR_IACK1           (1<<6)  /* IACK read mode address bit 1 */
#define RSR_PR_STATUS       (1<<5)  /* PR Interrupt is set */
#define RSR_NOT_LOCK_STATUS (1<<4)  /* Remote bus is *NOT* locked */
#define RSR_IACK2           (1<<2)  /* IACK read mode address bit 2 */
#define RSR_PT_STATUS       (1<<1)  /* PT interrupt is set */
#define RSR_IACK0           (1<<0)  /* IACK read mode address bit 0 */


/*
** Remote Node Command Register 2 (read/write, 8 bits)
** Nothing of interest in this register for PCI to PCI applications
*/
        bt_data8_t         rem_cmd2;
#define REM_CMD2        (0x09)

#define RC2_DMA_PAUSE   (1<<7)  /* DMA remote pause after 16 xfers     */
/* Bit 6 must always be set to "0". */
#define RC2_AMOD_SEL    (1<<6)  /* Use Address modifier register       */
#define RC2_DMA_BLK_SEL (1<<5)  /* DMA remote use Block Mode           */
#define RC2_CINT_DISABLE (1<<4) /* Disable passing of remote cable intr */


/*
** Undefined area of CSR register space
** This is normally where the VME page register is.  However,
** this unused for PCI to ? application because of mapping RAM
*/
        bt_data16_t        reserve0a;

/*
** Remote Node Card ID Register (read, 8 bits)
*/
        bt_data8_t         rem_card_id;
#define REM_CARD_ID     (0x0c)

/*
** Remote Node Address Modifier Register (read/write, 8 bits)
** Nothing of interest in this register for PCI to PCI applications
*/
        bt_data8_t         rem_amod;
#define REM_AMOD        (0x0d)

/*
** Remote Node Interrupt Acknowledge Register (read, 16 bits)
*/
        bt_data16_t        rem_iack;
#define REM_IACK        (0x0e)

/*
** Remote Node Mapping RAM Control Register (read/write, 8 bits)
** PCI to PCI only, same offset as the Remote Node Interrupt Acknowledge
**
** Use bit definitions for the Local Node Mapping RAM Control Register
*/
#define rem_mreg_ctrl      rem_iack
#define REM_MREG_CTRL   (0x0e)

/*
** DMA Registers
*/
/*
** Local Node DMA Command Register (read/write, 8 bits)
*/
        bt_data8_t         ldma_cmd;
#define LDMA_CMD        (0x10)

#define LDC_START       (1<<7)          /* Start DMA transfer */
#define LDC_DP_SEL      (1<<6)          /* Transfer to/from dual port RAM */
#define LDC_WRITE_SEL   (1<<5)          /* Transfer from local to remote */
#define LDC_DMA_D32_SEL (1<<4)          /* Transfer as longwords */
#define LDC_DMA_INT_ENABLE (1<<2)       /* Enable DMA DONE interrupt */
#define LDC_DMA_DONE    (1<<1)          /* r=1, DMA is complete */
#define LDC_DMA_BLK_SEL (1<<0)          /* Perform burst DMA transfers */

/*
** Local Node DMA Packet Remainder Count (read/write, 8 bits)
*/
        bt_data8_t         ldma_rmd_cnt;
#define LDMA_RMD_CNT    (0x11)

/*
** Local Node DMA Packet Count (read/write, 16 bits)
*/
        bt_data16_t        ldma_pkt_cnt;
#define LDMA_PKT_CNT    (0x12)

/*
** Local Node DMA Address Register (read/write, 32 bits)
** Certain cards only support 24 bits of local DMA address. See HW manual
** Can only be access by bytes or words
*/
        bt_data16_t        ldma_addr_lo;
        bt_data16_t        ldma_addr_hi;
#define LDMA_ADDR       (0x14)
#define LDMA_ADDR_MASK  (0x00ffffff)

/*
** Remote Node DMA Packet Remainder Count (read/write, 8 bits)
*/
        bt_data8_t         rdma_rmd_cnt;
#define RDMA_RMD_CNT    (0x18)

/*
** Undefined area of CSR register space
*/
        bt_data8_t         reserve19;

/*
** Remote Node DMA Address Register (read/write, 32 bits)
** Certain cards only support 24 bits of remote DMA address. See HW manual
** Can only be access by bytes or words
*/
        bt_data16_t        rdma_addr_hi;
        bt_data16_t        rdma_addr_lo;
#define RDMA_ADDR       (0x1a)

/*
** Remote Node Slave Status Register (read, 8 bits)
*/
        bt_data8_t         rem_slave_status;
#define REM_SLAVE_STATUS    (0x1e)  /* remote node slave status register  */
#define RSS_PARITY_ERR      (1<<7)  /* r=1, interface parity error   */
#define RSS_REMBUS_ERR      (1<<6)  /* r=1, remote bus error           */
#define RSS_PR_STATUS       (1<<5)  /* r=1, pr interrupt (from remote)    */
#define RSS_LRC_ERR         (1<<3)  /* r=1, LRC DMA packet error   */
#define RSS_TIMEOUT_ERR     (1<<2)  /* r=1, interface timeout error       */
#define RSS_PT_STATUS       (1<<1)  /* r=1, pt interrupt (to remote)      */
#define RSS_NO_CONNECT      (1<<0)  /* r=1, no connection to remote       */
#define RSS_ERROR_MASK    (RSS_PARITY_ERR | RSS_REMBUS_ERR \
                | RSS_LRC_ERR | RSS_TIMEOUT_ERR)

#define RSS_CERROR_MASK    (RSS_NO_CONNECT | RSS_ERROR_MASK)

/*
** Remote Node Slave Clear Register (write, 8 bits)
** Same ofset as Remote Node Slave Status Register
*/
#define rem_slave_clrerr    rem_slave_status
#define REM_SLAVE_CLRERR    (0x1e)  /* remote node slave clear error reg. */
#define RSC_CLR_ERROR       (1<<7)  /* w=1, clear status error bits       */

/*
** Undefined area of CSR register space
*/
        bt_data8_t          reserve1f;
/*
** Semaphore Registers
*/
/*
** Local Semaphore Registers (read/write, 8 bits)
** All semaphore registers have the same bit definitions
*/
        bt_data8_t          loc_sema0;
        bt_data8_t          loc_sema1;
        bt_data8_t          loc_sema2;
        bt_data8_t          loc_sema3;
        bt_data8_t          loc_sema4;
        bt_data8_t          loc_sema5;
        bt_data8_t          loc_sema6;
        bt_data8_t          loc_sema7;
        
        bt_data8_t          rem_sema0;
        bt_data8_t          rem_sema1;
        bt_data8_t          rem_sema2;
        bt_data8_t          rem_sema3;
        bt_data8_t          rem_sema4;
        bt_data8_t          rem_sema5;
        bt_data8_t          rem_sema6;
        bt_data8_t          rem_sema7;
#define BT_SEMA_TAKE        (1<<0)
#define BT_SEMA_GIVE        (0)

        bt_data8_t          reserved30;
        bt_data8_t          reserved31;
        bt_data8_t          reserved32;
        bt_data8_t          reserved33;
        bt_data8_t          reserved34;
        bt_data8_t          reserved35;
        bt_data8_t          reserved36;
        bt_data8_t          reserved37;
        
        /*
        ** Do not access these registers unless you have RPQ #601107 
        ** These registers allow direct access to all local registers 
        ** located on the remote card
        **
        ** For non RPQ #601107 cards these registers will return garbage
        **
        ** For bit definitions please use the defines for the 
        ** corresponding local registers.
        */
        bt_data8_t          rpq_rem_lloc_cmd1;
        bt_data8_t          rpq_rem_lint_ctrl;
        bt_data8_t          rpq_rem_lstatus;
        bt_data8_t          rpq_rem_lint_status;
        bt_data8_t          rpq_rem_lbus_ctrl;
        bt_data8_t          rpq_rem_lloopback;
        bt_data8_t          reserved3e;
        bt_data8_t          reserved3f;

/* EAS A64 */
        /*
        ** Do not access these registers unless you have RPQ #601332
        ** These registers allow direct access to all local registers 
        ** located on the remote card
        **
        ** For non RPQ #601332 cards these registers will return garbage
        **
        ** For bit definitions please use the defines for the 
        ** corresponding local registers.
        */
        
        bt_data16_t        reserved40;
        bt_data16_t        reserved42;
        bt_data16_t        reserved44;
        bt_data16_t        reserved46;
        /* Remote Node Upper Address PIO Register */
        /* (read/write, 32 bits) offset 0x48 */
        bt_data16_t        rem_a64pio_addr_lo;
        bt_data16_t        rem_a64pio_addr_hi;
        /* Remote Node Upper Address DMA Register */
        /* (read/write, 32 bits) offset 0x4c */
        bt_data16_t        rem_a64dma_addr_lo;
        bt_data16_t        rem_a64dma_addr_hi;

} bt_pci_reg_t;

/*****************************************************************************
**
**  Define all possible cable interrupts -- not currently useful
**
*****************************************************************************/

enum BT_CABLE_INTR {
    BT_CINT_1 = 1,              /* Cable interrupt one                      */
    BT_CINT_2,                  /* Cable interrupt two                      */
    BT_CINT_3,                  /* Cable interrupt three                    */
    BT_CINT_4,                  /* Cable interrupt four                     */
    BT_CINT_5,                  /* Cable interrupt five                     */
    BT_CINT_6,                  /* Cable interrupt six                      */
    BT_CINT_7,                  /* Cable interrupt seven                    */
    BT_CINT_MAX                 /* Actually more than the maximum           */
};

/*****************************************************************************
**
**  DEFINE GENERIC INTERRUPT HANDLER MACROS:
**
**  Define LOAD_MAP_REG macro here so loading a mapping register is illegal.
**
*****************************************************************************/

#define LOAD_MAP_REG(map_reg_ptr, rem_addr, addr_mod) \
    /* LOAD_MAP_REG(caddr_t map_reg_ptr, u_long rem_addr, u_char addr_mod) */ \
    /* This macro may be used only in a kernel extension interrupt handler */ \
    panic("LOAD_MAP_REG() is illegal - mappable windows are not supported.")


#define BT_FAIL   (-1)
#define BT_SUCC   0


/*****************************************************************************
**
**  List generic macros below
**
*****************************************************************************/

#define  SET_BIT(base, bits)  ((base) |= (bits))
#define  CLR_BIT(base, bits)  ((base) &= ~(bits))
#define  TST_BIT(base, bits)  (((base) & (bits)) != 0)
#define  IS_CLR(base, bits)   (((base) & (bits)) == 0)
#define  IS_SET(base, bits)   TST_BIT(base, bits)

#define IS_D16_ALIGNED(addr)  (((bt_devaddr_t)(addr) & 0x01) == 0)
#define IS_D32_ALIGNED(addr)  (((bt_devaddr_t)(addr) & 0x03) == 0)
#define IS_D64_ALIGNED(addr)  (((bt_devaddr_t)(addr) & 0x07) == 0)


/*****************************************************************************
**
**  List diagnostic and Mirror API macros below
**
**  For Internal Use Only!
**
*****************************************************************************/
#define BT_DIAG_MREG_OFFSET         (0x0)
#define BT_DIAG_NIO_OFFSET          (0x40000000)
#define BT_DIAG_ISR_Q_OFFSET        (0x40100000)

/* 
** These macros make it easier to update the code below for adding 
** interrupt types, keeping track of which irq queue was
** use to do the malloc, and each q size in bytes.  
**
** Both the library and the driver assume that the interrupt queues
** are assigned in the following order from the malloced or mmaped
** buffer: error, programmed, and then VME/IACK.  
**
** BTK_Q_USE and BTK_Q_NUM are used only by the driver, but included
** here to make them easier to find.
**
** BTK_Q_SIZE stands for the size of each queue including the controlling
** bt_irq_q_t structure.  BTK_Q_NUM * BTK_Q_SIZE is the number of bytes 
** malloced and the number of bytes that should be mmap by the library.
**
** BTK_Q_ASSIGN assigns each queue structure to the proper portion of the
** malloced or mmapped region described by q_start_p.
**
** Note:
** To get the q_size (qz) the library should call bt_get_info with
** BT_INFO_ICBR_Q_SIZE
*/
#define BTK_Q_NUM               (BT_MAX_IRQ - 1)    /* # of irq thread queues */
#define BTK_Q_USE               (unit_p->error_irq_q_p) /* Which queue as used for malloc */
#define BTK_Q_BYTES(q_size)     (sizeof(bt_irq_q_t) + sizeof(bt_data32_t) * (q_size - BTK_Q_PAD))
#define BTK_Q_SIZE(qz)          ((BTK_Q_BYTES(qz) % sizeof(bt_data64_t)) ? BTK_Q_BYTES(qz) + sizeof(bt_data32_t) : BTK_Q_BYTES(qz))
#define BTK_Q_ASSIGN(q_start_p, err_q_p, prog_q_p, vme_q_p, q_size)  \
        err_q_p = (bt_irq_q_t *) q_start_p; \
        prog_q_p = (bt_irq_q_t *) (q_start_p + BTK_Q_SIZE(q_size)); \
        vme_q_p = (bt_irq_q_t *) (q_start_p + 2 * BTK_Q_SIZE(q_size)); 

/*
**  List all Bit 3 adaptor IDs registered to date.
**  Used in process of determining remote identification.
*/
#define BT_ID_EISA          0x62            /* EISA ID Value                */
#define BT_ID_EISA2         0x41            /* New EISA ID Value            */
#define BT_ID_MCA3_DMA      0x76            /* MCA type 3 w/slave DMA       */
#define BT_ID_SB_DMA        0xDF            /* SBus ID Value                */
#define BT_ID_TC            0x4D            /* TURBOchannel ID Value        */
#define BT_ID_VME_SDMA      0x80            /* VMEbus w/ Slave DMA          */
#define BT_ID_GIO           0x47            /* GIO ID value - ASCII "G"     */
#define BT_ID_PCI_DMA       0xAB            /* PCI card ID value            */
#define BT_ID_PCI           0xAC            /* PCI wo/DMA card ID value     */
#define BT_ID_VME_NODMA     0x81            /* VME wo/DMA card ID value     */
#define BT_ID_VME_NOINC     0x82            /* VME RPQ w/ DMA Inc Inhibit   */
#define BT_ID_VME_FIBER     0x83            /* VME Fiberoptic card          */
#define BT_ID_VME_FIBER_D64 0x84            /* VME Fiberoptic card w/D64    */
#define BT_ID_VME_DB        0x85            /* VME dataBLIZZARD w/D64       */
#define BT_ID_PCI_FIBER     0xAD            /* PCI Fiberoptic card          */
#define BT_ID_PCI_FIBER_D64 0xAE            /* PCI Fiberoptic card w/D64    */
#define BT_ID_PCI_DBA64_RPQ 0xAF  /* PCI Fiberoptic card w/A64 RPQ 601332   */

/*****************************************************************************
**
**  Include old type and macros names for backwards compatiblity only.
**
**  Do Not Use These Names For New Development.  
**  These Names May Be Obsoleted At Any Time.
**
*****************************************************************************/
/*******************************************************************************
**
**  Lock device structure (for BIOC_LOCK)
**
*******************************************************************************/
typedef struct bt_lock_d {
    bt_data32_t  error;                 /* Error code                        */
    bt_msec_t    timeout;               /* Maximum lock time in milliseconds */
    bt_data32_t  signum;                /* Signal number to send on timeout  */
    bt_data32_t  nowait;                /* FALSE = wait; TRUE = don't wait   */
} bt_lock_t;

#if defined(__osf__)

typedef u_char  data8_t;
typedef u_short data16_t;
typedef uint    data32_t; /* uint = unsigned int = 32 bits */
typedef u_long  data64_t; /* u_long = 64 bits for Alpha */

#define DATA8_SIZ    sizeof(data8_t)
#define DATA16_SIZ   sizeof(data16_t)
#define DATA32_SIZ   sizeof(data32_t)
#define DATA64_SIZ   sizeof(data64_t)
#define DATA_ANY_SIZ 0

#elif defined(_sun) /* _sun */

typedef uint8_t         data8_t;
typedef uint16_t        data16_t;
typedef uint32_t        data32_t;
typedef uint64_t        data64_t;

#define DATA8_SIZ    sizeof(data8_t)
#define DATA16_SIZ   sizeof(data16_t)
#define DATA32_SIZ   sizeof(data32_t)
#define DATA64_SIZ   sizeof(data64_t)
#define DATA_ANY_SIZ 0

#else /* __osf__ */

typedef unsigned char  data8_t;
typedef unsigned short data16_t;
typedef unsigned long  data32_t;

#define DATA8_SIZ    sizeof(data8_t)
#define DATA16_SIZ   sizeof(data16_t)
#define DATA32_SIZ   sizeof(data32_t)
#define DATA_ANY_SIZ 0

#endif /* __osf__ */

#define NIBL_MASK 0xF
#define NIBL_SHFT 4
#define BYTE_MASK 0xFF
#define BYTE_SHFT 8
#define WORD_MASK 0xFFFF
#define WORD_SHFT 16
/* #define LONG_MASK 0xFFFFFFFF  conflict with ntddk.h */

#define WORD_ALIGN(addr)      (((bt_data32_t)(addr) & 0x01) == 0)
#define LONG_ALIGN(addr)      (((bt_data32_t)(addr) & 0x03) == 0)
#define LONGLONG_ALIGN(addr)  (((bt_data32_t)(addr) & 0x07) == 0)

#define ISBTCLR(base, bits)  (((base) & (bits)) == 0)
#define ISBTSET(base, bits)  TST_BIT(base, bits)

#define VME_A32_SB  0x0F    /* A32 Extended Supervisory Block Transfer      */
#define VME_A32_SP  0x0E    /* A32 Extended Supervisory Program Access      */
#define VME_A32_SD  0x0D    /* A32 Extended Supervisory Data Access         */
#define VME_A32_NB  0x0B    /* A32 Extended Non-Privileged Block Transfer   */
#define VME_A32_NP  0x0A    /* A32 Extended Non-Privileged Program Access   */
#define VME_A32_ND  0x09    /* A32 Extended Non-Privileged Data Access      */

#define VME_A32     VME_A32_SD

#define VME_A24_SB  0x3F    /* A24 Standard Supervisory Block Transfer      */
#define VME_A24_SP  0x3E    /* A24 Standard Supervisory Program Access      */
#define VME_A24_SD  0x3D    /* A24 Standard Supervisory Data Access         */
#define VME_A24_NB  0x3B    /* A24 Standard Non-Privileged Block Transfer   */
#define VME_A24_NP  0x3A    /* A24 Standard Non-Privileged Program Access   */
#define VME_A24_ND  0x39    /* A24 Standard Non-Privileged Data Access      */

#define VME_A24     VME_A24_SD
#define VME_A24_MAX 0x1000000

#define VME_A16_SD  0x2D    /* A16 Short Supervisory Data Access            */
#define VME_A16_ND  0x29    /* A16 Short Non-Privileged Data Access         */

#define VME_A16     VME_A16_SD

/*
**  List of any card specific values here.
*/
#define BT_QB_PG_MAX    0x3F                /* Qbus max page register value */

/*****************************************************************************
**
**  Kernel Level Information
**
**  NOT Useful or Visable To Applications
**
**  Do not use because these things change often and are not supporred
**  at the application level.
**
*****************************************************************************/

/******************************************************************************
**
**  PCI configuration register defines
**
******************************************************************************/
#define BT_MAX_CFGREG   64

/*
**  Vendor ID Register
*/
#define CFG_VENDOR_ID           (0x0)           /* vendor ID register */
#define CFG_VENDOR_ID_SIZE      (BT_WIDTH_D16)

#define CVI_BIT3_ID             (0x108A)        /* Bit 3 Computer Corp ID */  
#define BT_MAX_POSREG 7

/*
**  Device ID Register
*/
#define CFG_DEVICE_ID           (0x2)           /* device ID register */
#define CFG_DEVICE_ID_SIZE      (BT_WIDTH_D16)

/*
**  Command Register
*/
#define CFG_COMMAND             (0x4)           /* command register */
#define CFG_COMMAND_SIZE        (BT_WIDTH_D16)

/*
**  Status Register
*/
#define CFG_STATUS              (0x6)           /* status register */
#define CFG_STATUS_SIZE         (BT_WIDTH_D16)

/*
**  Revision ID Register
*/
#define CFG_REVISION_ID         (0x8)           /* revision ID register */
#define CFG_REVISION_ID_SIZE    (BT_WIDTH_D8)

/*
**  Register Level Programming Register (not used)
*/
#define CFG_REGISTER_PROG       (0x9)           /* low byte of class code */
#define CFG_REGISTER_PROG_SIZE  (BT_WIDTH_D8)

/*
**  Class Code Register
*/
#define CFG_CLASS_CODE          (0xa)           /* upper bytes of class code */
#define CFG_CLASS_CODE_SIZE     (BT_WIDTH_D16)

/*
**  Cache Line Register (not used)
*/
#define CFG_CACHE_LINE          (0xc)           /* cache line size register */
#define CFG_CACHE_LINE_SIZE     (BT_WIDTH_D8)

/*
**  Latency Timer Register
*/
#define CFG_LATENCY_TIMER       (0xd)           /* latency timer register */
#define CFG_LATENCY_TIMER_SIZE  (BT_WIDTH_D8)

/*
**  Header Type Register (not used)
*/
#define CFG_HEADER_TYPE         (0xe)           /* header type register */
#define CFG_HEADER_TYPE_SIZE    (BT_WIDTH_D8)

/*
**  Built-In Self Test Register (not used)
*/
#define CFG_SELF_TEST           (0xf)           /* self test register */
#define CFG_SELF_TEST_SIZE      (BT_WIDTH_D8)

/*
**  I/O Mapped Node I/O Base Address Register (not used in AIX)
*/
#define CFG_IO_IO_BASE          (0x10)          /* I/O mapped node I/O */
#define CFG_IO_IO_BASE_SIZE     (BT_WIDTH_D32)

/*
**  Memory Mapped Node I/O Base Address Register
*/
#define CFG_MM_IO_BASE          (0x14)          /* memory mapped node I/O */
#define CFG_MM_IO_BASE_SIZE     (BT_WIDTH_D32)

/*
**  Mapping Register Base Address Register
*/
#define CFG_MAP_REG_BASE        (0x18)          /* mapping registers base addr */
#define CFG_MAP_REG_BASE_SIZE   (BT_WIDTH_D32)


/*
**  Remote Memory Base Address Register
*/
#define CFG_REM_MEM_BASE        (0x1c)          /* remote memory base addr */
#define CFG_REM_MEM_BASE_SIZE   (BT_WIDTH_D32)

/*
**  Interrupt Line Register 
*/
#define CFG_IRQ_LINE            (0x3c)          /* interrupt line register */
#define CFG_IRQ_LINE_SIZE       (BT_WIDTH_D8)

/*
**  Interrupt Pin Register (not used)
*/
#define CFG_IRQ_PIN             (0x3d)          /* interrupt pin register */
#define CFG_IRQ_PIN_SIZE        (BT_WIDTH_D8)

/*
**  Min_Gnt Register (not used)
*/
#define CFG_MIN_GNT             (0x3e)          /* Min_Gnt register */
#define CFG_MIN_GNT_SIZE        (BT_WIDTH_D8)

/*
**  Max_Lat Register (not used)
*/
#define CFG_MAX_LAT             (0x3f)          /* Max_Lat register */
#define CFG_MAX_LAT_SIZE        (BT_WIDTH_D8)

/*
**  Other defines
*/
#define BT_WRITE   (1<<0)
#define BT_READ    (1<<1)

/*
** Size definitions
*/
#define SIZE_1KB            0x00000400  /* 1KByte */
#define SIZE_4KB            0x00001000  /* 4Kbytes */
#define SIZE_8KB            0x00002000  /* 8Kbytes */
#define SIZE_16KB           0x00004000  /* 16Kbytes */
#define SIZE_32KB           0x00008000  /* 32Kbytes */
#define SIZE_64KB           0x00010000  /* 64Kbytes */
#define SIZE_128KB          0x00020000  /* 128Kbytes */
#define SIZE_256KB          0x00040000  /* 256Kbytes */
#define SIZE_512KB          0x00080000  /* 512Kbytes */
#define SIZE_1MB            0x00100000  /* 1Mbytes */
#define SIZE_2MB            0x00200000  /* 1Mbytes */
#define SIZE_4MB            0x00400000  /* 4Mbytes */
#define SIZE_8MB            0x00800000  /* 8Mbytes */
#define SIZE_16MB           0x01000000  /* 16Mbytes */
#define SIZE_32MB           0x02000000  /* 32Mbytes */
#define SIZE_4GB            0x100000000 /* 4Gbytes */

/*
** Structure for VxWork trace message streams 
*/

#if defined(__vxworks)

typedef struct {
    bt_data32_t  error;
    FILE        *stream1;
    FILE        *stream2;
} bt_trace_init_t;

#endif /* defined(__vxworks) */



#endif /* !_BTDBPCI_H */
