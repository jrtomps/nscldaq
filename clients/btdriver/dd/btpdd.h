/*****************************************************************************
**
**      Filename:   btpdd.h
**
**      Purpose:    Header file for Linux/PCI device driver.
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef  _H_BTPDD_H
#define  _H_BTPDD_H


/****************************************************************************
**
**      Include files
**
****************************************************************************/

#if !defined(BTP_CFG_C)
#define __NO_VERSION__
#endif /* defined(BTP_CFG_C */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/version.h>

#include "btio.h"

/******************************************************************************
**
**  General macros
**
******************************************************************************/

/*****************************************************************************
**
** Macros to insert and extract fields
**
** All of the fields are defined with two macros,
**   fieldname_SHFT is the amount to shift the header right by
**   fieldname_MASK is the mask for the size of the field
**
** The macros EXTRACT and INSERT use a shorthand notation, where only
** the field name is specified.  The macro concatenates the _SHFT and _MASK
** portion of the names and then calls the MASK_INSERT or MASK_EXTRACT
** macros to do the actual work.
**
** As long as ALL fields have both the _SHFT and _MASK values defined,
** this should work well.
*****************************************************************************/
#define BT_CONCAT(a,b)     a ## b

#define BT_MASK_EXTRACT(data, shift, mask) \
        (((data) >> (shift)) & (mask))
#define BT_MASK_INSERT(data, shift, mask, value) \
        (((data) & ~((mask)<<(shift))) | ((value) << (shift)))

#define BT_EXTRACT(data, field) \
        BT_MASK_EXTRACT(data, BT_CONCAT(field, _SHFT), BT_CONCAT(field, _MASK))

#define BT_INSERT(data, field, value) \
        BT_MASK_INSERT(data, BT_CONCAT(field, _SHFT), BT_CONCAT(field, _MASK), value)

/******************************************************************************
**
**  Define system maximums and others
**
******************************************************************************/

/*
** Linux doesn't have the normal struct uio / struct buf / strategy()
** interface of most other unix vendors.
*/
#define MAX_DMA_XFER      (0x1000000)   /* 16 Mbytes maximum we can DMA */
#define MIN_DMA_XFER      (4)           /* Four bytes is minimum DMA size */
#define DMA_DELAY_PER_KB  (74)          /* Usecs to delay per Kbyte */
#define DMA_DELAY_POLL    (18)          /* Usecs to delay per 256 */
#define BT_PAGE_SIZE      BT_617_PAGE_SIZE
#define BT_WRITE   (1<<0)
#define BT_READ    (1<<1)
#ifndef MIN
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)   ((a) < (b) ? (b) : (a))
#endif

/*
** Page size for system
*/
#define BT_SYS_PAGE_SIZE    PAGE_SIZE

/*
**  Default values
*/
#define DEFAULT_RESET_TIMER        (200)  /* 200*10 msec = 2 sec Rbus reset */
#define DEFAULT_DMA_THRESHOLD      (0x200) /* DMA enable size */
#define DEFAULT_DMA_TIMEOUT        (500)  /* 500*10 msec = 5 sec DMA watchdog */
#define DEFAULT_DMA_POLL           (0x20)             /* Switch to DMA irq */
#define DEFAULT_LMEM_SIZE          (0x10000)         /* 64 Kbytes of loc mem */
#define DEFAULT_Q_SIZE             (0x400)           /* 1 Kbytes of interrupt q space */

/*
**  Swapping values
*/
#define BT_SWAP_VMEBUS (BT_SWAP_BSBD_BSNBD)  /* Byte swapping for VMEbus */
#define BT_SWAP_MULTIBUS (BT_SWAP_BSBD_WS)   /* Byte swapping for Multibus */
#define BT_SWAP_QBUS (BT_SWAP_BSBD_WS)       /* Byte swapping for Qbus */

/*
**  Unit, logical device, and info macros
*/
#define GET_MINOR_NUM(dev_id) (MINOR((dev_id)->f_dentry->d_inode->i_rdev))
#define GET_UNIT_NUM(dev_id)  (GET_MINOR_NUM(dev_id) & BT_MAX_UNITS)
#define GET_UNIT_PTR(dev_id)  (bt_unit_array_gp[GET_UNIT_NUM(dev_id)])
#define GET_LDEV_TYPE(dev_id) (GET_MINOR_NUM(dev_id) >> BT_AXS_SHFT)

/*****************************************************************************
**
**      Tracing macros
**
*****************************************************************************/
/* 
** Global variables used by tracing system 
*/
extern unsigned long bt_trace_lvl_g;
extern const char *bt_name_gp;
extern volatile bt_data32_t *mreg_gp;
extern volatile bt_data32_t *bt_trace_mreg_gp;

/*
**  Macros for tracing definitions
**
**  Each function must have FUNCTION() and either LOG_UNKNOWN_UNIT,
**  LOG_UNIT or LOG_DEVID at the beginning.
*/
#define FUNCTION(str)  const char *t_func_name = str

#define LOG_UNKNOWN_UNIT        \
        unsigned t_unit = BT_MOCK_UNIT

#define LOG_UNIT(unit_p)        \
        unsigned t_unit = (NULL == (unit_p) ? BT_MOCK_UNIT : (unit_p)->unit_number)

#define LOG_DEVID(devid)        \
        unsigned t_unit = GET_UNIT_NUM(devid)

#define LOG_UNIT_NUMBER(num)  \
  int t_unit = num

#define SET_UNIT_NUMBER(num)  \
  t_unit = num


#define LINE_STR        BT_STRIZE(__LINE__)
#define BT_STRIZE(literal)      BT_STR2(literal)
#define BT_STR2(literal)        #literal

/* Formatting prefix for each call */
#define LOG_FMT         KERN_NOTICE "%s%d: %s: " __FILE__ "#" LINE_STR ": "

/* Arguments to use for each formatting prefix */
#define LOG_ARG         (bt_name_gp), (t_unit), (t_func_name)

/* Convert unit pointer to actual unit number */
#define PTR_TO_UNITN(unit_ptr) \
    (((unit_ptr) == NULL) ? NO_UNIT : (unit_ptr)->unit_num)

/* 
** TRC_EVAL boolean check on trace level
**
** First section to take care of normal trace bits, allow several to be ORed.
** Second section is if BT_TRC_DETAIL is off.
** Third section is if BT_TRC_DETAIL is on.
*/

#define TRC_EVAL(lvl)   \
    ( ( 0 != ( (lvl) & (~BT_TRC_DETAIL) & bt_trace_lvl_g) ) && \
      ( ( 0 == ( (lvl) & BT_TRC_DETAIL) ) || \
        ( 0 != ( bt_trace_lvl_g & BT_TRC_DETAIL) ) ) )


/*      Fatal errors
**
** A fatal or critical error making further use of the device driver
** impossible or very severely limited.
*/
#define FATAL_STR(str)  { /* system is unusable */  \
    if (bt_trace_lvl_g) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xAA0000 /* AA = Fatal */ \
	        | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define FATAL_MSG(str)  { /* system is unusable */      \
    if (bt_trace_lvl_g) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xAA0000 /* AA = Fatal */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk str;  \
	} \
    } }

/*      Warning messages
**
** An error occurred that the device driver can work around. It may
** prevent certain functions from operating, however the device driver
** is otherwise useable. An example would be running out of a system
** resource.
*/
#define WARN_STR(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xBB0000 /* BB = Warning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define WARN_MSG(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xBB0000 /* BB = Warning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk str; \
        } \
    } }

/*      Informational messages
**
** Any time the device driver is about to return an error, there should
** be a trace message at this level describing the error. This is useful
** as an aid in debugging user applications.
**
** NOTE: These are errors caused by an application, not the device driver.
** By default, this level of tracing is turned off.
*/
#define INFO_STR(str)   { /* Informational messages, why an error return */ \
    if (TRC_EVAL(BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define INFO_MSG(str)   { \
    if (TRC_EVAL(BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk str; \
        } \
    } }


/*****************************************************************************
**
**      Trace macros controlling which functions are traced
**
*****************************************************************************/

/*      Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY          { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xFB0000 /* Function Beginning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "Entry.\n", LOG_ARG); \
        } \
    } }

#define FEXIT(exit_val) { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xFE0000 /* Function Ending */ \
                | BT_BCD4(__LINE__); \
        } else { \
            long tmp_exit = (long) exit_val; \
            printk(LOG_FMT "Exit = %ld 0x%lx.\n", LOG_ARG, \
                tmp_exit, tmp_exit); \
        } \
    } }

/*      Standard tracing macros
**      
**  Driver tracing macros that are left in the production version of the 
**  driver. These macros can help determine exactly what is happening as
**  the driver is running. They are not normally used by a customer, but
**  left in the driver as an aid for customer support.
**
**  Only specify one of the BT_TRC_ functional bits (4-27) for each
**  macro.
**
**  You can use one of the modifier bits (BT_TRC_PROFILE, BT_TRC_DETAIL,
**  or BT_TRC_DEBUG) with any of the functional bits. This will cause the
**  trace to occur only if both bits are set.
*/
#define TRC_STR(trc,str) { \
    if (TRC_EVAL(trc)) {        /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define TRC_MSG(trc,str) { \
    if (TRC_EVAL(trc)) {        /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk str; \
        } \
    } }


/*      Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if defined (DEBUG)
#define DBG_STR(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xDD0000 /* DD = Debugging Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define DBG_MSG(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
		| 0xDD0000 /* DD = Debugging Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            printk str; \
        } \
    } }

#else /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif /* DEBUG */


/* 
** Default tracing level 
*/
#undef BT_TRC_DEFAULT

#if     defined(DEBUG)
#define BT_TRC_DEFAULT  \
        (BT_TRC_ERROR|BT_TRC_WARN|BT_TRC_INFO|BT_TRC_CFG)

#else   /* defined(DEBUG) */
#define BT_TRC_DEFAULT  (BT_TRC_ERROR|BT_TRC_WARN)

#endif  /* defined(DEBUG) */

/*  
**  Hardware debugging macros
**
**  These macros are ment to be used with a VMETRO and are never
**  include in a final version of the driver.
**  
**  NOTE:  assumes that unit_p is defined and valid
*/
#if defined (DEBUG)
#define HW_TRC_LINE(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp = (func << 24) | BT_BCD4(__LINE__);
#define HW_TRC_POINT(func, point) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp = (func << 24) | point;
#define HW_TRC_VALUE(value) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp = (bt_data32_t) value;
#define HW_TRC_FOPEN(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp = (func << 24) | 0;
#define HW_TRC_FCLOSE(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp = (func << 24) | 0xffu;
#else /* DEBUG */
#define HW_TRC_LINE(func)
#define HW_TRC_POINT(func, point)
#define HW_TRC_VALUE(value)
#define HW_TRC_FOPEN(func)
#define HW_TRC_FCLOSE(func)
#endif /* DEBUG */


/*
**  Misc. debuging macros
*/
#if     defined(DEBUG)
#define BUGGER    { \
      printk( __FILE__ ": " LINE_STR "\n"); \
    }
#endif  /* defined(DEBUG) */

/*
** Define mmap tracking type
*/
typedef struct {
    unsigned int    start_mreg;
    unsigned int    num_mreg;
    bt_dev_t        axs_type;
    unsigned long   vm_boffset;
    unsigned long   vm_length;
    caddr_t         vm_p;
    
} bt_mtrack_t;

/*
** Define bind tracking type
*/
typedef struct {
    int             phys_off;
    int             phys_len;
    unsigned int    start_mreg;
    unsigned int    num_mreg;
    caddr_t         bind_addr;
    bt_dev_t        axs_type;
} bt_btrack_t;

extern bt_unit_t * bt_unit_array_gp[];

/******************************************************************************
** Bus access macros
**
** Do not use the _64 bit macros, they do not work properly on Linux
** They seem to access the bus correctly but when the data gets back to
** the user it is all scrambled.  Tried a couple of different forms to 
** the macros but the problem seems to be in accessing the user buffer using
** a 64-bit pointer.  Therefore I just made Linux do 32 bit PIO in the
** btk_bcopy() routine.
******************************************************************************/
#define PIO_WRITE_8(val, ptr)  (writeb(val, (bt_data8_t *) ptr))
#define PIO_WRITE_16(val, ptr) (writew(val, (bt_data16_t *) ptr))
#define PIO_WRITE_32(val, ptr) (writel(val, (bt_data32_t *) ptr))
/* PIO_WRITE_64 does not work properly, see note above */
#define PIO_WRITE_64(val, ptr) { bt_data64_t data = val; \
                                 writel(data & 0xffffffff, (bt_data32_t *) ptr); \
                                 writel(data >> 32, (bt_data32_t *) ptr); \
                               }

#define PIO_READ_8(ptr)  (readb((bt_data8_t *) ptr))
#define PIO_READ_16(ptr) (readw((bt_data16_t *) ptr))
#define PIO_READ_32(ptr) (readl((bt_data32_t *) ptr))
/* PIO_READ_64 does not work properly, see note above */
#define PIO_READ_64(ptr) ((bt_data64_t) readl((bt_data32_t *) ptr) | ((bt_data64_t) readl(((bt_data32_t *) ptr) + 1) << 32))


/*******************************************************************************
**      Declare prototypes for external entry points
*******************************************************************************/

/* 
** btp_open.c 
*/
extern int btp_open(struct inode *inode_p, struct file *file_p);
extern int btp_close(struct inode *inode_p, struct file *file_p);

/* 
** btp_rdwr.c 
*/
extern loff_t btp_llseek(struct file *file_p, loff_t offset, int which);
extern ssize_t btp_read(struct file *file_p, char *data_p, size_t length, loff_t * new_fpos_p);
extern ssize_t btp_write(struct file *file_p, const char *data_p, size_t length, loff_t *new_fpos_p);

/* 
** btp_ioctl.c 
*/
extern int btp_ioctl(struct inode *inode_p, struct file *file_p, unsigned int cmd, unsigned long args);

/* 
** btp_mmap.c 
*/
extern int btp_mmap(struct file *file_p, struct vm_area_struct *vm_p);
extern int btp_lm_mmap(struct file *file_p, struct vm_area_struct *vm_p);

/* 
** Access adaptor registers
*/
extern bt_data32_t btk_get_io(bt_unit_t *unit_p, bt_reg_t reg);
extern void btk_put_io(bt_unit_t *unit_p, bt_reg_t reg, bt_data32_t value);

extern void btk_put_mreg_range(bt_unit_t *unit_p, unsigned int mr_start, unsigned int num_mregs, bt_mreg_t mreg_type, bt_data32_t value);
extern void btk_put_mreg(bt_unit_t *unit_p, unsigned int mr_idx, bt_mreg_t mreg_type, bt_data32_t value);
extern bt_data32_t btk_get_mreg(bt_unit_t *unit_p, unsigned int mr_idx, bt_mreg_t mreg_type);
extern void btk_setup_mreg(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t *mreg_value_p, bt_operation_t op);

#endif /* _H_BTPDD_H */
