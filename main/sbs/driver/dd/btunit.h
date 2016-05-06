/*****************************************************************************
**
**      Filename:   btunit.h
**
**      Purpose:    SBS Data Blizzard/Adaptor include 
**                  file used only by Data Blizzard Device Drivers.
**
**      $Revision: 1.11 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined(_BTUNIT_H) 
#define _BTUNIT_H 


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

/*
**  Logical devices and unit macros
*/
#define BT_MOCK_UNIT   99               /* When real physical unit is unknown */

#ifdef __sgi
extern minor_t btp_get_minor_no (dev_t dev);
#define BT_UNIT(dev)   ((int)(((long) btp_get_minor_no(dev)) & BT_MAX_UNITS))
#define BT_AXSTYP(dev) ((int)(((long) btp_get_minor_no(dev)) >> BT_DEV_SHFT))

#elif defined(__sun)
#define BT_UNIT(dev)   (int)(getminor(dev) & BT_MAX_UNITS)
#define BT_AXSTYP(dev) (int)(getminor(dev) >> BT_DEV_SHFT)

#elif defined(__linux__)
#define BT_UNIT(dev)   (int)(MINOR(dev) & BT_MAX_UNITS)
#define BT_AXSTYP(dev) (int)(MINOR(dev) >> BT_DEV_SHFT)

#else
#define BT_UNIT(dev)   (int)(minor(dev) & BT_MAX_UNITS)
#define BT_AXSTYP(dev) (int)(minor(dev) >> BT_DEV_SHFT)

#endif

/* 
** Used to circumvent lseek() deficiency of using signed 32bit values 
*/
#define RE_ADJUST (0x80000000UL)

/*
** Vector values used for intra-driver communications
**
** All values above BT_DRV_VECTOR_BASE are reserved
** for SBS use only.
*/
#define BT_DRV_VECTOR_BASE              0xff000000
#define BT_DRV_VECTOR_PEER2PEER         BT_DRV_VECTOR_BASE + 1
#define BT_DRV_VECTOR_SEMA_RELEASE      BT_DRV_VECTOR_BASE + 2

/*
** Paging structure for the driver
*/
typedef struct {
    size_t          page_size;
    size_t          page_offset;
    bt_data32_t     page_number;
    unsigned int    mreg_start;
    unsigned int    mreg_need;
    caddr_t         bus_base_p;
} btk_page_t;

/*
** ICBR thread tracking data
*/
typedef struct {
    bt_event_t      thread_event;
    bt_data64_t     thread_id;
    int             error_count;
    int             prog_count;
    int             vme_count;
} bt_ttrack_t;

/*
** Various types of device operations
*/
typedef enum {
    BT_OP_OPEN,     /* Only used with btk_invalid_op() */
    BT_OP_CLOSE,    /* Only used with btk_invalid_op() */
    BT_OP_READ,     /* Only used with btk_invalid_op() */
    BT_OP_WRITE,    /* Only used with btk_invalid_op() */
    BT_OP_MMAP,
    BT_OP_BIND,
    BT_OP_DMA,      /* Only used with btk_setup_mreg() */
    BT_OP_PIO,      /* Only used with btk_setup_mreg() */
    BT_MAX_OP
} bt_operation_t;

/*
** Types of mapping registers
*/
typedef enum {
    BT_LMREG_PCI_2_CABLE,
    BT_LMREG_CABLE_2_PCI,
    BT_LMREG_DMA_2_PCI,
    BT_RMREG_PCI_2_CABLE,
    BT_RMREG_CABLE_2_PCI,
    BT_RMREG_DMA_2_PCI,
    BT_MREG_PRG_VECTOR = 0x1ffe,
    BT_MREG_HW_TRACE = 0x1fff
} bt_mreg_t;

/*
** Mapping register constants
*/
#define BT_MREG_SWAP_SHIFT      (1)             /* Swap shift */
#define BT_MREG_SWAP_MASK       (0xe)           /* Swap mask */
#define BT_MREG_FUNCT_SHIFT     (4)             /* Function shift */
#define BT_MREG_FUNCT_MASK      (0x30)          /* Function mask */
#define BT_MREG_AMOD_SHIFT      (6)             /* Amod shift */
#define BT_MREG_AMOD_MASK       (0xfc0)         /* Amod mask */
#define BT_MREG_ADDR_SHIFT      (12)            /* Address shift */
#define BT_MREG_ADDR_MASK       (0xfffff000)    /* Address mask */

#define BT_MREG_INVALID         (1<<0)  /* Invalidate mapping register */
#define BT_MREG_FUNCT_PCFG      (0)     /* Remote configurationfunction code */
#define BT_MREG_FUNCT_RIO       (1)     /* Remote I/O function code */
#define BT_MREG_FUNCT_RRAM      (2)     /* Remote RAM  function code */
#define BT_MREG_FUNCT_DPORT     (3)     /* Dual Port RAM  function code */

/*
** PCI card constants
*/
#define BT_CCR_DISABLE_DEV      (0x0)           /* disables the 617 */
#define BT_CCR_NORM_OPER        (0x46)          /* normal operation mode */

#define BT_CSR_RESET_ERR        (0xffff)        /* reset any errors */

#define BT_CLT_MAX_VALUE        (0xe0)          /* Max. latency timer */

/*
** PCI BAR 2, mapping registers, sizes
** _P2P indicates sizes or offsets for PCI to PCI applications
** _NGEN indicates sizes for next generation adapters
*/
#define BT_CMR_SPACE_SIZE       (0x10000)       /* 64k of mapping regs */
#define BT_CMR_SPACE_SIZE_NGEN  (0x80000)       /* 512k of mapping regs NEXT_GEN*/
#define BT_CMR_REG_SIZE         (BT_WIDTH_D32)  /* size of a mapping reg */
#define BT_617_PAGE_SIZE        (0x1000)    /* 4K page size */

/*
** Offsets and sizes in to the mapping register windows for the various
** adapter windows PCI to cable, local dual port, etc.
*/
/*
** Local PCI to cable mapping register window constants (Remote memory)
** _P2P indicates sizes or offsets for PCI to PCI applications
** _NGEN indicates sizes for next generation adapters
*/
#define BT_CMR_RMEM_OFFSET      (0x0) 
#define BT_CMR_RMEM_SIZE        (0x8000)
#define BT_CMR_RMEM_SIZE_P2P    (0x4000) 

/*
** Local Cable to PCI mapping register window constants (Local memory)
** _P2P indicates sizes or offsets for PCI to PCI applications
** _NGEN indicates sizes for next generation adapters
*/
#define BT_CMR_LMEM_OFFSET      (BT_CMR_RMEM_SIZE)
#define BT_CMR_LMEM_SIZE        (0x4000)
#define BT_MAX_SDMA_LEN         (BT_CMR_LMEM_SIZE*BT_617_PAGE_SIZE/BT_CMR_REG_SIZE)

/*
** Local DMA to PCI mapping register window constants (DMA memory)
** _P2P indicates sizes or offsets for PCI to PCI applications
** _NGEN indicates sizes for next generation adapters
*/
#define BT_CMR_LDMA_OFFSET      (BT_CMR_LMEM_OFFSET + BT_CMR_LMEM_SIZE)
#define BT_CMR_LDMA_SIZE        (0x4000) 
#define BT_MAX_DMA_LEN          BT_MAX_SDMA_LEN

/*
** Local Dual Port memory portion of mapping registers (LDP memory)
** _P2P indicates sizes or offsets for PCI to PCI applications
** This only exists on next generation products so _NGEN has been dropped
*/
#define BT_CMR_DP_OFFSET        (BT_CMR_LDMA_OFFSET + BT_CMR_LDMA_SIZE)
#define BT_CMR_DP_SIZE          ((BT_CMR_SPACE_SIZE_NGEN/2)-BT_CMR_DP_OFFSET)

/*
** Remote mapping registers are identical to local ones just add
** half the total mapping registers
*/
#define BT_CMR_REMOTE_OFFSET    (BT_CMR_SPACE_SIZE_NGEN/2)

/*
** PCI Bar 3, remote memory window sizes
*/
#define BT_CRM_SPACE_SIZE       (0x2000000)
#define BT_CRM_SPACE_SIZE_P2P   (0x1000000)

#define BT_MAX_MMAP_BIT  (BT_CMR_RMEM_SIZE/BT_CMR_REG_SIZE) /* Max bit in mmap bit map */
#define BT_MAX_SDMA_BIT  (BT_CMR_LMEM_SIZE/BT_CMR_REG_SIZE) /* Max bit in sdma bit map */

#define DMA_PKT_SIZE  256

#include "btuisr.h"

#if (defined(BT983) || defined(BT984))
    /* additions for XP */
#define PCI_NUM_BARS                    7      // Total number of PCI BAR registers
#define MAX_NAME_LENGTH                 0x20   // Max length of registered device name

// Device Location Structure
typedef struct _DEVICE_LOCATION
{
    u_long DeviceId;
    u_long VendorId;
    u_long BusNumber;
    u_long SlotNumber;
    UCHAR SerialNumber[16];
} DEVICE_LOCATION;

// PCI BAR Space information 
typedef struct _PCI_BAR_INFO
{
    u_long             *pVa;                    // BAR Kernel Addresses
    PHYSICAL_ADDRESS  Physical;                // BAR Physical Addresses
    u_long             Size;                    // BAR size
    BOOLEAN           IsIoMapped;              // Memory or I/O mapped?
    BOOLEAN           IsUserMappable;          // Can this BAR be mapped?
    PMDL              pMdl;                    // MDL for the BAR space
} PCI_BAR_INFO;
#endif /* (defined(BT983) || defined(BT984)) */


/******************************************************************************
**
** Information held by driver on each unit
**
** Unit Structure
**
******************************************************************************/
typedef struct bt_unit_d {

/*****
****** START OF OS UNIQUE UNIT STUCTURE INFORMATION 
*****/

    /*
    ** SGI 965 specific infomation
    */
#if defined(BT965)
    vertex_hdl_t dev_vertex;            /* Vertex to the device */
    pciio_piomap_t cfg_map;             /* Mappings for various windows */
    pciio_piomap_t nio_map;
    pciio_piomap_t mreg_map;
    pciio_piomap_t rmem_map;
    struct buf      *dmabuf_p;          /* Current buf structure pointer */
    pciio_dmamap_t  dma_map;            /* DMA map pointer */
    alenlist_t      p_list;             /* Address list- PCI physical */
    alenlist_t      v_list;             /* Address list- kernel virtual */
    struct bt_kmap_s *window_p;         /* Adaptor window information */
    pciio_intr_t  intr_hdl;             /* Handle for isr attach routine */
    device_desc_t device_desc;          /* device descriptor */
    lock_t  isr_lock;                   /* Lock for interrupt service rouitne */
    struct buf  *lm_buf_p;              /* Pointer to local memory device buf */
    caddr_t     lm_raw_p;               /* Original local memory pointer to free */
    int         lm_raw_len;             /* Original local memory allocated */
    pciio_dmamap_t lm_map_p;            /* DMA map used for local memory */
    vertex_hdl_t logdev_node[BT_MAX_AXSTYPS];   /* The verticies created for the
                                                   logical devices. */
    int         first_dma;
    unsigned int        lm_start;       /* Starting mreg of local memory */
    unsigned int        lm_need;        /* Number of mregs need for local memory */

    /*
    ** VxWorks stuff
    */
#elif defined(BT993)

    /*
    ** The VxWorks device header must be the first thing in its unit 
    ** sturcture! DO NOT MOVE vxw_hdr. 
    */
    DEV_HDR       vxw_hdr;         /* VxWorks device header */

    /*
    **    pci device information 
    */
    int           pci_bus;         /* PCI Bus number */
    int           pci_device;      /* PCI device number */
    int           pci_func;        /* PCI function number */
    int           pci_bus_type;    /* PCI bus type (internal, pri, sec) */
    char         *name_p;          /* Base name of the device */

    /*
    **    Mapping register information 
    */
    char         *cfg_map;         /* Physical PCI addr of config registers  */
    char         *nio_map;         /* Physical PCI addr of CSR registers  */
    char         *mreg_map;        /* Physical PCI addr of map registers  */
    char         *rmem_map;        /* Physical PCI addr of remote windows */


    /*
    **  Interrupt related variables
    */
    bt_kmap_t     window;          /* Adapter window information */
    int           disp_stack;      /* Stack size of dispatch task */
    int           disp_prio;       /* Priority of dispatch task */

    /*
    **  needed for VMEbus to PCI slave DMA accesses
    */
    unsigned int  mr_page;         /* Mapping regs per page */


    /*
    ** Keep track of open()s, close()s, and logical devices.
    */
    unsigned long ref_count;                /* Current total number of open()s */
    long          open_total;               /* Opens across all logical devs */

    
    /*
    **  Local memory information
    */
    int           lm_raw_len;       /* Original local memory allocated */
    caddr_t       lm_raw_p;         /* Original local memory allocated */


    /*
    ** Local Memory (BT_DEV_LM) device information 
    */
    char           *lm_phys_addr;   /* VMEbus A24/A32 address of BT_DEV_LM */
    bt_addr_width_t lm_space;	    /* Address Width: BT_AW_A32 or BT_AW_A24 */
    char           *lm_base_p;      /* Local bus address of BT_DEV_LM */
    unsigned int    lm_b_resid;     /* need to release resources for lm device */
    unsigned int    lm_b_flags;
    
    /*
    ** Windows NT 983 984 specific infomation
    */
#elif (defined(BT983) || defined(BT984))
    bt_pci_config_t     pci_config;     /* PCI configuration from HAL */
    PADAPTER_OBJECT     adapter_object; /* DMA adapter object from HAL */
    PHYSICAL_ADDRESS    q_paddr;        /* Physical bus address of interrupt queue */
    bt_data32_t         open_cnt;       /* Number of opens */
    unsigned int        uisr_mreg;      /* Which mapping reg is for user ISRs */
    KSPIN_LOCK          isr_lock;       /* Lock for interrupt service rouitne */
    PHYSICAL_ADDRESS    lm_paddr;       /* Physical bus address of local memory */
    unsigned int        lm_start;       /* Starting mreg of local memory */
    unsigned int        lm_need;        /* Number of mregs need for local memory */
    UNICODE_STRING      params_path;    /* registry path for unit parameters */

    /* additions for XP */
    int                 dpc_busy_flag;  /* set to TRUE if the DPC is running */
    KDPC                irq_dpc;
    KSPIN_LOCK          irq_sl;         /* Lock for interrupt service rouitne */
    KEVENT              evRemove;
    bt_data32_t         usage;
    PDEVICE_OBJECT      pPhysicalDeviceObject;
    PDEVICE_OBJECT      pDeviceObject;            // Device object this extension belongs to
    PDEVICE_OBJECT      pLowerDeviceObject;
    DEVICE_LOCATION     Device;
    WCHAR               DeviceLinkNameBuffer[MAX_NAME_LENGTH];
    PCI_BAR_INFO        PciBar[PCI_NUM_BARS];
    // Hardware locking parameters
    KSPIN_LOCK          HwAccessLock;
    LIST_ENTRY          InterruptEventList;
    KSPIN_LOCK          InterruptEventListLock;
    LIST_ENTRY          UserAddressMappingList;
    KSPIN_LOCK          UserAddressMappingListLock;
    LIST_ENTRY          UserCommonBufferMappingList;
    KSPIN_LOCK          UserCommonBufferMappingListLock;
    DEVICE_POWER_STATE  Power;
    bt_data32_t *       PowerIdleCounter;
    // Interrupt handling variables
    PKINTERRUPT         pInterruptObject;
    BOOLEAN             removing;

    /*
    ** Solaris Model 946 specific infomation
    */
#elif defined(BT946)
    dev_info_t  *devinfo_p;             /* Device information */
    
    ddi_acc_handle_t  nio_regs_hdl;     /* Node register handle */
    ddi_acc_handle_t  mregs_hdl;        /* mapping register handle */
    ddi_acc_handle_t  rmem_hdl;         /* Remote memory window handle  */

    ddi_dma_handle_t    dma_handle;     /* Put here for btp_intr().    */
    ddi_iblock_cookie_t iblock_cookie;  /* Returned by ddi_add_intr(). */
    
    struct buf  *lm_buf_p;              /* Pointer to buf structure for local memory */
    int         lm_start;               /* Starting mapping reg for local memory */
    int         lm_need;                /* Number of mapping regs needed for local memory */
    bt_data32_t lm_offset;              /* Offset to align local memory to 4k page */
    kmutex_t    isr_lock;               /* Lock for interrupt service rouitne */
    kmutex_t    kddi_mutex;             /* Lock for interrupt service rouitne */
    
    bt_data32_t buf_offset;             /* uio_offset from read/write for DMA */
    kcondvar_t  buf_condvar;            /* Buf condvar per unit.       */

    bt_data32_t rem_ram_addr;           /* Starting address of REM RAM window on VME */

    /*
    ** Linux Stuff
    */
#elif defined(BT1003)

    struct pci_dev *dev_p;		/* Location of device in PCI space */

    /* 
    ** PCI configuration information  -- NOT FROM PCI CONFIG REGISTERS 
    */
    unsigned long csr_phys_addr;    /* physical address of Node I/O */
    unsigned long rr_phys_addr;     /* physical address of remote ram */
    unsigned long mr_phys_addr;     /* physical address of mapping ram */
    unsigned int irq;               /* System IRQ level */

    bt_cookie_t hirq_cookie;	    /* Hardware IRQ cookie */
    bt_cookie_t sirq_cookie;	    /* Software IRQ cookie */
    bt_cookie_t task_cookie;	    /* Task level cookie, no special context */

    unsigned int    mr_page;               /* Mapping regs per page */

    bt_mutex_t  llist_mutex;        /* Protect non-isr linked lists */
    /* Related shared members are: llist_bind_mutex and llist_map_mutex. */

    /*  [Used only by btk_mutex_init() and btk_mutex_fini() in btp_attach().]
    **  Locking variables that do inhibit the interrupt service routine
    */
    spinlock_t      isr_lock;       /* Lock for interrupt service rouitne */

    /*
    **  DMA related parameters
    */
    dma_addr_t      lm_dma_handle; 

    /* 
    **  Logical device variables
    */
    long        open_cnt[BT_MAX_AXSTYPS];       /* Opens on this logical dev */


    /*
    **  Local memory information
    */
    caddr_t         lm_kaddr;       /* Kernel address of local memory */
    unsigned int    lm_start;       /* Starting mapping reg */
    unsigned int    lm_need;        /* Number mapping regs needed */

    /* 
    **  Misc variables
    */
    long            open_total;     /* Opens across all logical devs */
    struct pm_dev   *device_pm;     /* Device Power management */

#elif defined (__lynxos)

    /* 
    ** PCI configuration information  -- NOT FROM PCI CONFIG REGISTERS 
    */
    unsigned long csr_phys_addr;    /* physical address of Node I/O */
    unsigned long rr_phys_addr;     /* physical address of remote ram */
    unsigned long mr_phys_addr;     /* physical address of mapping ram */

    unsigned int irq;               /* System IRQ level */

    /*
    **  Local memory information
    */
    unsigned int    lm_start;       /* Starting mreg of local memory */
    unsigned int    lm_need;        /* Number of mregs need for local memory */
    caddr_t         lm_kaddr;       /* Kernel address of local memory */

    bt_mutex_t      isr_lock;       /* Lock for interrupt service rouitne */
#if 0
    bt_mutex_t      dma_mutex;      /* Prevents two DMAs from occurring */
#endif

    /* XXX remove these variables when possible, Lynxos will NOT use 
     * double buffering. Nor will we need the cookie variables.
     */
    void *          dma_buf_p;      /* Kernel intermediate data buffer */
    size_t          dma_buf_size;   /* Size of buffer */

    /* Bind buffer management */
    void *          kmi_btrackp;

    /* Used to keep track of a prcess ID, when locking and unlocking memory */
    int             pid;

    bt_cookie_t hirq_cookie;	    /* Hardware IRQ cookie */
    bt_cookie_t sirq_cookie;	    /* Software IRQ cookie */
    bt_cookie_t task_cookie;	    /* Task level cookie, no special context */

    struct dbstatics *dbs;  /* Per device resource, contains DRM handle */

    int             sgl_len; /* List length */

    struct dmachain *sgl;  /* scatter gather list of user's buffer */
    /*
    **  needed for VMEbus to PCI slave DMA accesses
    */
    unsigned int  mr_page;         /* Mapping regs per page */

    bt_mutex_t  llist_mutex;        /* Protect non-isr linked lists */

    /* 
    **  Misc variables
    */
    long        open_total;     /* Opens across all logical devs */

    /* 
    **  Logical device variables
    */
    long        open_cnt[BT_MAX_AXSTYPS];       /* Opens on this logical dev */

#else /* By Product */
#error Unknown product
#endif /* By Product */


/*****
****** START OF COMMON UNIT STUCTURE INFORMATION
*****/

    volatile bt_data32_t   bt_status;  /* Status word for this unit */
#define BT_OPEN_INHIBIT     (1<< 0)     /* Inhibit device open */
#define BT_IS_OPEN          (1<< 1)     /* At least one unit is open */
#define BT_ONLINE           (1<< 2)     /* Set if Device is online */
#define BT_PRIV             (1<< 3)     /* Set if in privileged mode */
#define BT_LOGERR           (1<< 4)     /* Set if status interrupt errlog EN */
#define BT_LM_EN            (1<< 5)     /* Set if Local Memory is ENabled */
#define BT_INT              (1<< 6)     /* Set if interrupt handler registrd */
#define BT_DMA_FIFO         (1<< 7)     /* Set if DMA to FIFO */
#define BT_RESET            (1<< 8)     /* Set if a reset is pending */
#define BT_DMA_ERROR        (1<< 9)     /* Set if error during DMA */
#define BT_POWER            (1<<10)     /* Set if NO remote connection */
#define BT_ERROR            (1<<11)     /* Set if an Error has occurred */
#define BT_DMA_LOCAL        (1<<12)     /* Set if local adaptor does DMA */
#define BT_DMA_AVAIL        (1<<13)     /* Set if adaptor pair can does DMA */
#define BT_DMA_BLOCK        (1<<14)     /* Set if use block mode for DMAs */
#define BT_DMA_WAIT         (1<<15)     /* Set if DMA is in PAUSE mode */
#define BT_PCI2PCI          (1<<16)     /* Set if remote card is PCI based */
#define BT_NEXT_GEN         (1<<17)     /* Set if adapteris next gen design */
/*                          (1<<18)        Not Used */
#define BT_DMA_ACTIVE       (1<<19)     /* A DMA is in progress */
#define BT_RB_INTR          (1<<20)     /* Rembus intr handler is registered */
#define BT_PT_INTR          (1<<21)     /* Prog intr handler is registered */
#define BT_ER_INTR          (1<<22)     /* Error intr handler is registered */
#define BT_SEND_PT          (1<<23)     /* When set, use PT when sending irq */
#define BT_PEER2PEER        (1<<24)     /* Set if remote driver is installed */
#define BT_MASTER           (1<<25)     /* Set if use local sem7 to control */
                                        /* DMA otherwise use remote sema7 */ 
#define BT_LOOPBACK         (1<<26)     /* Set if loopback connector is installed */
#define BT_GEMS_SWAP        (1<<27)     /* Set if GEMS swapping is in effect */
/*                          (1<<28)        Not Used */
#define BT_BUF_WAIT         (1<<29)     /* Another is waiting to enter strategy, Solaris only */
#define BT_BUF_LOCK         (1<<30)     /* Process currently entering stratedy, Solaris only */
/*                          (1<<31)        Not Used */

/* 
**  Format string for %b printf() of bt_status flags 
*/
#define BT_STATUS_FORMAT "\20\40NOTUSED\37BUFLOCK\36BUFWAIT\35NOTUSED\34NOTUSED\33NOTUSED\32MASTER\31PEER2PEER\30NOTUSED\27ER_INTR\26PT_INTR\25RB_INTR\24DMA_ACTIVE\23NOTUSED\22NEXT_GEN\21PCI2PCI\20DMA_WAIT\17NOTUSED\16DMA_AVAIL\15DMA_LOCAL\14ERROR\13POWER\12TRANSMIT\11RESET\10HANDSHAKE\7INT\6DPLOC\5LOGERR\4PRIV\3ONLINE\2IS_OPEN\1OPEN_INHIBIT"

    /*
    **  Physical card information
    */
    bt_data8_t  unit_number;            /* Physical unit number */
    bt_data32_t loc_id;                 /* Part number of local card */
    bt_data32_t rem_id;                 /* Part number of remote card */
    bt_data8_t  board_revision;         /* PCI config rev id register value */
    bt_data8_t  driver_version[BT_DIAG_MAX_REV_INFO];   /* Driver version string */

    /*
    **  Mapping register information
    */
    caddr_t cfg_p;                        /* Pointers for various windows */
    caddr_t nio_p;
    caddr_t mreg_p;
    caddr_t rmem_p;
    bt_pci_reg_t *csr_p;                /* Pointer to CSRs */

    /*
    **  DMA related parameters
    */
    int             dma_threshold;      /* DMA transfer threshold value */
    int             dma_poll_size;      /* Size at which < to use polled DMA */
    bt_msec_t       dma_timeout;        /* DMA watchdog timer value (1 msec) */
    bt_event_t      dma_event;          /* Synchronization for DMA completion */
    bt_mutex_t      dma_mutex;          /* Prevents two DMAs from occurring */
    btk_timeout_t   watchdog_id;        /* DMA watchdog timeout id */

    /* 
    **  queue heads for various stages of interrupt processing 
    */
    btk_llist_t qh_err_fn;             /* Error interrupt handlers */
    btk_llist_t qh_prg_fn;             /* Programmed interrupt handlers */
    btk_llist_t qh_iack_fn;            /* Remote bus interrupt handlers */
    btk_llist_t icbr_thread_list;      /* List of ICBR threads to wake */
    bt_irq_q_t  *error_irq_q_p;        /* Error interrupt queue */
    bt_irq_q_t  *prog_irq_q_p;         /* Programmed  interrupt queue */
    bt_irq_q_t  *vme_irq_q_p;          /* VME interrupt queue */
    bt_data32_t q_size;                /* Interrupt q size */

    /* 
    **  Interrupt servicing and status information
    */
    volatile int   sig_count;          /* Number of signals sent to processes */
    volatile int   sig_prg_cnt;        /* ... from programmed interrupts. */
    volatile int   sig_err_cnt;        /* ... from error interrupts. */
    volatile int   sig_other_cnt;      /* ... for any other reason. */

    /*
    **  Interrupt related variables
    */
    btk_timeout_t wakeup_id;            /* Remote bus reset timeout id */

    /* 
    **  Logical device variables
    */
    caddr_t     kern_addr[BT_MAX_AXSTYPS];      /* Kernel addr of windows */
    bt_data32_t kern_length[BT_MAX_AXSTYPS];    /* Kernel size of windows */
    int         pio_addr_mod[BT_MAX_AXSTYPS];   /* Addr modifiers used for PIO */
    int         dma_addr_mod[BT_MAX_AXSTYPS];   /* Addr modifiers used for DMA */
    int         mmap_addr_mod[BT_MAX_AXSTYPS];  /* Addr modifiers used for MMAP */
    int         swap_bits[BT_MAX_AXSTYPS];      /* Swap bit each logical dev */
    int         data_size[BT_MAX_AXSTYPS];      /* Data transfer size */
    int         open[BT_MAX_AXSTYPS];           /* Device currently open */
    bt_data32_t logstat[BT_MAX_AXSTYPS];        /* Device status */
#define  STAT_ONLINE     (1<<0)         /* Set if logical device is online */
#define  STAT_READ       (1<<2)         /* Set if can read logical device */
#define  STAT_WRITE      (1<<3)         /* Set if can write logical device */
#define  STAT_BIND       (1<<4)         /* Set if can bind logical device */
#define  STAT_DMA        (1<<5)         /* Set if can use DMA to go to dev */
#define  STAT_LOCAL      (1<<6)         /* set if this device is local */
#define  STAT_MMAP       (1<<7)         /* Can logical device use mmap */

/* 
**  Format string for %b printf() of logstat flags 
*/
#define BT_LOGSTAT_FORMAT "\20\10MMAP\7LOCAL\6DMA\5NOTUSED\4WRITE\3READ\2OPEN\1ONLINE"


    /* 
    **  Resources necessary for tracking mmap and slave DMA requests.
    **  Pointers are to bit maps of available mapping registers, 
    **  One bit map is needed for PCI to VMEbus mmap requests and the second is
    **  needed for VMEbus to PCI slave DMA accesses.
    */
    void        *mmap_aval_p;
    void        *sdma_aval_p;
    btk_llist_t qh_mmap_requests;      /* Used to track the mmap requests */
    btk_llist_t qh_bind_requests;      /* Used to track the bind requests */

    /*
    **  Locking variables that do not effect the interrupt service routine
    */
    bt_mutex_t  unit_lock;          /* General unit locking */

    bt_mutex_t  open_mutex;         /* Prevents open and close from clashing */

    bt_mutex_t  mreg_mutex;         /* Protects the mreg bit maps while alloc or free */
    bt_mutex_t  llist_bind_mutex;   /* Protect bind linked lists */
    bt_mutex_t  llist_map_mutex;    /* Protect map linked lists */
    bt_mutex_t  prg_irq_mutex;      /* Protect against sending two prog interrupt at same time */


    bt_rwlock_t hw_rwlock;          /* Prevents TAS/CAS & PIO from clashing */

    /*
    **  Local memory information
    */
    int         lm_size;                /* User requested local memory size */

    /* 
    **  Misc variables
    */
    bt_msec_t           reset_timer;    /* Time in 10ms to wait for reset */
    volatile bt_data8_t last_error;     /* ORed value of last error(s) */

    int                 pio_count;      /* track active pio's             */
/* EAS A64 */
    unsigned int        a64_offset;     /* RPQ 601332 A32-A63 and mode bit */
/* EAS BIND CODE */
    unsigned int    bt_kmalloc_size;    /* size of kernel buffer to kmalloc for bind */
    unsigned int    *bt_kmalloc_buf;    /* aligned kernel buffer kmalloced for bind */
    unsigned int    *bt_kmalloc_ptr;    /* kernel buffer kmalloced for bind (save for free) */

} bt_unit_t;


#if defined(__vxworks)

/* The lseek_off must be tracked with each open call */
typedef struct {
    /* Stuff to keep track of for each file descriptor openned */
    bt_unit_t    *unit_p;
    int           unit;
    bt_dev_t      logdev;
    long          lseek_off;
} bt_fd_t;

typedef bt_fd_t *bt_dev_id_t;

#endif /* defined __vxworks */

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


#endif /* _BTUNIT_H */

