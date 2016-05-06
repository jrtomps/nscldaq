/*****************************************************************************
**      Filename:   bt_trace.h
**
**      Purpose:    SBS driver trace macros and defines.
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_trace.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.18 $
**
*****************************************************************************/
/*****************************************************************************
**      Copyright (c) 1997-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BT_TRACE_H)
#define _BT_TRACE_H

#if     defined(__vxworks)
#include <types/vxTypesOld.h>
#include <logLib.h>   /* for logMsg() calls */
#endif     /* defined(__vxworks) */

#ifdef   __linux__
#include <linux/kernel.h>   /* include kernel for printk if not already included */
#endif  /* __linux__ */

#if defined( __sun )
#include <sys/cmn_err.h>
#endif  /* __sun */
/*****************************************************************************
**
**      Tracing macros
**
*****************************************************************************/

extern bt_data32_t bt_trace_lvl_g;
extern volatile bt_data32_t *bt_trace_mreg_gp32;
extern volatile bt_data64_t *bt_trace_mreg_gp64;
extern volatile bt_data32_t bt_trace_mreg_size;
extern void bt_log(char *str, ...);
extern void bt_log_str (char *str);
extern void bt_log_msg (char *str, ...);

#define BT_FILE_NUMBER(fnumber)  static int trace_file_number_g = fnumber

#ifdef FCTACH
/******************************************************************************
**
**  Trace values.
**
**  Use with TRACE_LVL parameter (BIOC_PARAM & BIOC_DEV_ATTRIB)
**
**  For simple informational tracing use one of the first 4 levels defined
**  below.
**
**  For detailed tracing the trace control value is made up of ORing the 
**  various bits together to enable the trace macros that are of interest 
**  to you.  This type of tracing destroys performance and should only be
**  used by SBS Technical Support.
**
******************************************************************************/
/* 
**  Trace flags controlling error output 
*/
#define BT_TRC_NONE     0x00000000  /* no tracing is enabled whatsoever */
#define BT_TRC_ERROR    0x00000001  /* Fatal system errors, shouldn't happen */
#define BT_TRC_WARN     0x00000002  /* driver errors from external conditions */
#define BT_TRC_INFO     0x00000004  /* errors returned to the user */

/* 
**  Trace flags controlling which major components to trace
*/
#define BT_TRC_CFG      0x00000010  /* driver config and initialization */
#define BT_TRC_RD_WR    0x00000020  /* generic read/write */
#define BT_TRC_MMAP     0x00000040  /* memory mapping and related code */
#define BT_TRC_BIND     0x00000080  /* bt_bind() and related code */


#define BT_TRC_CTRL     0x00000100  /* All ioctl() functions */
#define BT_TRC_OPEN     0x00000200  /* Driver open and close */
#define BT_TRC_TSDK     0x00000400  /* Tachyon Software Development Kit */
#define BT_TRC_TISA     0x00000800  /* Transport Independent Software Arch */


#define BT_TRC_ISR      0x00001000  /* Interrupt Service Routine */
#define BT_TRC_IRQ_DISP 0x00002000  /* Interrupt Request Processing/Dispatch */
#define BT_TRC_ICBR     0x00004000  /* user ICBR and the vector returned by */
                                    /* applicable kernel interrupt handlers */
#define BT_TRC_ISR_ERR  0x00008000  /* ISR errors only */


/* 
**  Trace flags controlling which subsubsystems in the driver 
*/
#define BT_TRC_DMA      0x00010000 /* DMA, each transfer */
#define BT_TRC_PIO      0x00020000 /* PIO, each transfer */
#define BT_TRC_FUNC     0x00040000 /* function entry and exit */
#define BT_TRC_ALLOC    0x00080000 /* Allocation/release of resources */



#define BT_TRC_LOCK     0x00100000 /* all locks (user & driver internal) */
#define BT_TRC_MAPREG   0x00200000 /* details of each map register load */
#define BT_TRC_RAWIO    0x00400000 /* Raw I/O register access */
#define BT_TRC_KLIB     0x00800000 /* Misc. shared kernel routines */


#define BT_TRC_KDDI     0x01000000 /* All 3rd party driver rouitnes KDDI */
#define BT_TRC_LIO      0x02000000 /* Low level I/O register rd/wr */
#define BT_TRC_LOGIN    0x04000000 /* fibre channel login */
#define BT_TRC_CMD      0x08000000 /* fibre channel commands path */


/* 
**  Modifiers on tracing.
**
**  These bits are always used in combination with the other trace bits,
**  to provide more information. Combine them with any of the functional
**  tracing bits (4-27) above.
*/
#define BT_TRC_PROFILE  0x10000000 /* enable H/W timing marks for profiling */
#define BT_TRC_DETAIL   0x20000000 /* Detailed trace of enabled function */
#define BT_TRC_HW_DEBUG 0x40000000 /* enable HW trace instead of console */
#define BT_DO_NOT_USE   0x80000000 /* don't use bit 31, the sign bit, */
                                   /* because it would make some comparison */
                                   /* operations messy */

/* 
**  Default trace level 
*/
#define BT_TRC_DEFAULT  (BT_TRC_ERROR|BT_TRC_WARN)

/*****************************************************************************
**  CCC next section was origionally in btngpci.h
**
**  File numbers for hardware tracing
**
**  Each file should have a unique for number, lower ones for shared, higher
**  ones for private.  These are assigned to trace_file_number_g at the 
**  beginning of each file.
**
*****************************************************************************/

#if defined(__FCTACH)
#define TRACE_BT_BIT_C              0x1
#define TRACE_BT_DELAY_C            0x2
#define TRACE_BT_EVENT_C            0x3
#define TRACE_BT_ICBR_THREAD_C      0x4
#define TRACE_BT_ISR_C              0x5
#define TRACE_BT_LLIST_C            0x6
#define TRACE_BT_MEM_C              0x7
#define TRACE_BT_MUTEX_C            0x8
#define TRACE_BT_NBUS_C             0x9
#define TRACE_BT_PARAM_C            0xa
#define TRACE_BT_RWLCK_C            0xb
#define TRACE_BT_SETUP_C            0xc
#define TRACE_BT_XFER_C             0xd
#define TRACE_BT_DIAG_C             0xe
#define TRACE_BT_BSP_UNIQUE_C       0xf
#define TRACE_BT_TRACE_C            0x10

#define TRACE_FCCHIP_C              0x81
#define TRACE_FCINT_C               0x82
#define TRACE_FCDISC_C              0x83
#define TRACE_FCIMQ_C               0x84
#define TRACE_FCINIT_C              0x85
#define TRACE_FCIO_C                0x86
#define TRACE_FCKNOB_C              0x87
#define TRACE_FCLGN_C               0x88
#define TRACE_FCPORT_C              0x89
#define TRACE_FCPROBE_C             0x8a
#define TRACE_FCXCHG_C              0x8b
#define TRACE_FCXLS_C               0x8c
#define TRACE_LMRAMDSK_C            0x8d
#define TRACE_LMBASE_C              0x8e
#define TRACE_LMTEST_C              0x8f
#define TRACE_SAHDWE_C              0x90

/* TISA Layer files */

/* Common */
#define TRACE_TDCB_C                0x91
#define TRACE_TDINIT_C              0x92
#define TRACE_TDPORT_C              0x93
#define TRACE_TDTIMER_C             0x94
#define TRACE_TDHW_C                0x95
#define TRACE_TDUTIL_C              0x96
#define TRACE_TDESGL_C              0x97
#define TRACE_TDIOCTL_C             0x98
#define TRACE_TDSTATE_C             0x99
#define TRACE_TDINT_C               0x9A

/* Initiator */
#define TRACE_ITDCB_C               0x9B
#define TRACE_ITDDISC_C             0x9C
#define TRACE_ITDINIT_C             0x9D
#define TRACE_ITDLGN_C              0x9E
#define TRACE_ITDTIMER_C            0x9F
#define TRACE_ITDIO_C               0xA0
#define TRACE_ITDIOCTL_C            0xA1
#define TRACE_ITDEVENT_C            0xA2
#define TRACE_ITDRESET_C            0xA3
#define TRACE_ITDERR_C              0xA4

/* Target */
#define TRACE_TTDCB_C               0xA5
#define TRACE_TTDINIT_C             0xA6
#define TRACE_TTDIO_C               0xA7
#define TRACE_TTDLOGIN_C            0xA8
#define TRACE_TTDRESET_C            0xA9
#define TRACE_TTDTIMER_C            0xAA

#define OSTI_VXWORKS_C              0xAB
#define TRACE_FCTACH_CDEV_C         0xAC
#define TRACE_FCTACH_INIT_C         0xAD

#define TRACE_FCTACH_IOCTL_C        0xAE
#define TRACE_FCTACH_RW_C           0xAF
#define TRACE_FCTACH_RAMDISK_C      0xB0

#define TRACE_AGTIAPI_CCB_C         0xB1
#define TRACE_AGTIAPI_CHAR_DRV_C    0x82
#define TRACE_AGTIAPI_CMD_C         0x83
#define TRACE_AGTIAPI_DISCOVERY_C   0x84
#define TRACE_AGTIAPI_LUN_C         0x85
#define TRACE_AGTIAPI_MISC_C        0x86
#define TRACE_AGTIAPI_RESET_C       0x87
#define TRACE_AGTIAPI_SCSI_DRV_C    0x88
#define TRACE_AGTIAPI_TIME_C        0x89
#define TRACE_AGTIAPI_WWN_C         0x8A
#endif  /* AGTIAPI_FC */

#elif defined  BT_NBUS_FAMILY
/*****************************************************************************
**
**  File numbers for hardware tracing
**
**  Each file should have a unique for number, lower ones for shared, higher
**  ones for private.  These are assigned to trace_file_number_g at the 
**  beginning of each file.
**
*****************************************************************************/

#define TRACE_BT_BIT_C              0x1
#define TRACE_BT_DELAY_C            0x2
#define TRACE_BT_EVENT_C            0x3
#define TRACE_BT_ICBR_THREAD_C      0x4
#define TRACE_BT_ISR_C              0x5
#define TRACE_BT_LLIST_C            0x6
#define TRACE_BT_MEM_C              0x7
#define TRACE_BT_MUTEX_C            0x8
#define TRACE_BT_NBUS_C             0x9
#define TRACE_BT_PARAM_C            0xa
#define TRACE_BT_RWLCK_C            0xb
#define TRACE_BT_SETUP_C            0xc
#define TRACE_BT_XFER_C             0xd
#define TRACE_BT_DIAG_C             0xe
#define TRACE_BT_BSP_UNIQUE_C       0xf

#define TRACE_BTP_BIND_C            0x81
#define TRACE_BTP_CDEV_C            0x82
#define TRACE_BTP_CFG_C             0x83
#define TRACE_BTP_CTLS_C            0x84
#define TRACE_BTP_INTR_C            0x85
#define TRACE_BTP_KERN_C            0x86
#define TRACE_BTP_LWIO_C            0x87
#define TRACE_BTP_MMAP_C            0x88
#define TRACE_BTP_RDWR_C            0x89
#define TRACE_BTP_OPEN_C            0x8a

#define TRACE_BTP_IOCTL_C           TRACE_BTP_CTLS_C
#define TRACE_BTP_KERNEL_C          TRACE_BTP_KERN_C
#define TRACE_BTP_LOWIO_C           TRACE_BTP_LWIO_C

/* 
**  Default trace level 
*/
#define BT_TRC_DEFAULT  (BT_TRC_ERROR|BT_TRC_WARN)

#endif  /* BT_NBUS_FAMILY */


/******************************************************************************
**
**  Trace values.
**
**  Use with TRACE_LVL parameter (BIOC_PARAM & BIOC_DEV_ATTRIB)
**
**  For simple informational tracing use one of the first 4 levels defined
**  below.
**
**  For detailed tracing the trace control value is made up of oring the 
**  various bits together to enable the trace macros that are of interest 
**  to you.  This type of tracing destroys performance and should only be
**  used by SBS Technical Support.
**
******************************************************************************/
/* 
**  Trace flags controlling error output 
*/
#define BT_TRC_NONE     0x00000000 /* no tracing is enabled whatsoever */
#define BT_TRC_ERROR    0x00000001 /* Fatal system errors, shouldn't happen */
#define BT_TRC_WARN     0x00000002 /* driver errors from external conditions */
#define BT_TRC_INFO     0x00000004 /* mesg for errors returned to the user */

/* 
**  Trace flags controlling which major components to trace
*/
#define BT_TRC_CFG      0x00000010 /* driver configuration and initialization */
#define BT_TRC_RD_WR    0x00000020 /* generic read/write (outside of DMA&PIO) */
#define BT_TRC_MMAP     0x00000040 /* memory mapping and related code */
#define BT_TRC_BIND     0x00000080 /* bt_bind() and related code */

#define BT_TRC_CTRL     0x00000100 /* All ioctl() functions */
#define BT_TRC_OPEN     0x00000200 /* Driver open and close */

#define BT_TRC_ISR      0x00001000 /* Interrupt Service Routine */
#define BT_TRC_IRQ_DISP 0x00002000 /* Interrupt Request Processing/Dispatch */
#define BT_TRC_ICBR     0x00004000 /* user ICBR and the vector returned by */
                                   /* applicable kernel interrupt handlers */
#define BT_TRC_ISR_ERR  0x00008000 /* ISR errors only */

/* 
**  Trace flags controlling which subsubsystems in the driver 
*/
#define BT_TRC_DMA      0x00010000 /* DMA, each transfer */
#define BT_TRC_PIO      0x00020000 /* PIO, each transfer */
#define BT_TRC_FUNC     0x00040000 /* function entry and exit */
#define BT_TRC_ALLOC    0x00080000 /* Allocation/release of resources */

#define BT_TRC_LOCK     0x00100000 /* all locks (user & driver internal) */
#define BT_TRC_MAPREG   0x00200000 /* details of each map register load */
#define BT_TRC_RAWIO    0x00400000 /* Raw I/O register access */
#define BT_TRC_KLIB     0x00800000 /* Misc. shared kernel routines */

#define BT_TRC_KDDI     0x01000000 /* All 3rd party driver rouitnes KDDI */
#define BT_TRC_LIO      0x02000000 /* Low level I/O register rd/wr */

/* 
**  Modifiers on tracing.
**
**  These bits are always used in combination with the other trace bits,
**  to provide more information. Combine them with any of the functional
**  tracing bits (4-27) above.
*/
#define BT_TRC_PROFILE  0x10000000 /* enable H/W timing marks for profiling */
#define BT_TRC_DETAIL   0x20000000 /* Detailed trace of enabled function */
#define BT_TRC_HW_DEBUG 0x40000000 /* enable HW trace mesg instead of console */
#define BT_TRC_DONT_USE 0x80000000 /* dont use the sign bit */
/* 
**  We don't use bit 31, the sign bit, because it would make some comparison
** operations messy
*/
/*****************************************************************************
**
**  Macros for tracing definitions
**
**  Each function must have FUNCTION() and either LOG_UNKNOWN_UNIT or LOG_UNIT
**  at the beginning.
**
*****************************************************************************/

/*****************************************************************************
** Generic macros to convert a value to a four digit binary coded decimal
** value.      
*****************************************************************************/
#define BT_BCD4(value) ((value) / 1000 % 10 << 12 \
                       | (value) / 100 % 10 << 8 \
                       | (value) / 10 % 10 << 4 \
                       | (value) % 10)

#if !defined(__winxp)
#define FUNCTION(str)        char *t_func_name = (char *)str
#else
#define log_unit_num t_dev_name
#define FUNCTION(func_str)  static char *log_func_name = func_str
#endif


#if !defined(__winxp)
#define LOG_UNKNOWN_UNIT     int t_dev_name = (int)0xffff
#else
#define NO_UNIT             99      /* unit num for non-specific unit */
#define LOG_UNKNOWN_UNIT     unsigned long log_unit_num = NO_UNIT
#endif
 
#define LOG_UNIT(HostID)     int t_dev_name = (int)HostID

#define LOG_DEVID(devid)     unsigned t_dev_name = GET_UNIT_NUM(devid)

#define LOG_UNIT_NUMBER(num) int t_dev_name = (int)num

#if !defined(__winxp)
#define SET_UNIT_NUMBER(num) t_dev_name = (int)num
#else
#define SET_UNIT_NUMBER(unit_number) int t_dev_name = (int)unit_number
#endif

#define LINE_STR             BT_STRIZE(__LINE__)
#define BT_STRIZE(literal)   BT_STR2(literal)
#define BT_STR2(literal)     #literal


/*****************************************************************************
**
**      Tracing macros OS specific
**      Customize these to each implementation
**
*****************************************************************************/

/******************** VXWORKS ***********************************************/
#if     defined(__vxworks)

        /* Formatting prefix for each call */ 
#define LOG_FMT         "%d: %s: " LINE_STR ": "

        /* Arguments to use for each formatting prefix */ 
#define LOG_ARG         (t_dev_name), (t_func_name)


/*      Fatal errors
**
**  A fatal or critical error making further use of the device driver
**  impossible or very severely limited.
*/
#define FATAL_STR(str)    { /* system is unusable */  \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n",  \
                    (int)t_dev_name, (int)t_func_name, \
                    (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                   (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } \
    } \
}

#define FATAL_MSG(str)    { /* system is unusable */  \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } \
    } \
}
    


/*      Warning messages
**
**  An error occurred that the device driver can work around. It may
**  prevent certain functions from operating, however the device driver
**  is otherwise useable. An example would be running out of a system
**  resource.
*/
#define WARN_STR(str)    { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                   (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                   (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } \
    } \
}

#define WARN_MSG(str)    { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } \
    } \
}


/*      Informational messages
**
**  Any time the device driver is about to return an error, there should
**  be a trace message at this level describing the error. This is useful
**  as an aid in debugging user applications.
**
**  NOTE: These are errors caused by an application, not the device driver.
**  By default, this level of tracing is turned off.
*/
#define INFO_STR(str)    { /* Informational messages, why an error return */ \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                   (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                  (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } \
    } \
}

#define INFO_MSG(str)    { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } \
    } \
}


/*      Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY        { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: Entry.\n", (int)t_dev_name, \
                  (int)t_func_name, (int)__LINE__, (int)0, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: Entry.\n", (int)t_dev_name, \
                  (int)t_func_name, (int)__LINE__, (int)0, (int)0, (int)0); \
            } \
        } \
    } \
}

#define FEXIT(exit_val)    { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: Exit = %d 0x%x.\n", \
                (int)t_dev_name, (int)t_func_name, \
                (int)__LINE__, (int)exit_val, (int)exit_val, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: Exit = %d 0x%x.\n", \
                (int)t_dev_name, (int)t_func_name, \
                (int)__LINE__, (int)exit_val, (int)exit_val, (int)0); \
            } \
        } \
    } \
}

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
    if ((trc) == (bt_trace_lvl_g & (trc))) {     /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s: %d: %s\n", (int)t_dev_name, \
                (int)t_func_name, (int)__LINE__, (int)str, (int)0, (int)0); \
            } \
        } \
    } \
}

#define TRC_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) {     /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str; \
            } \
        } \
    } \
}

/*      Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if     defined (DEBUG)
#define DBG_STR(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s\n", (int)__LINE__, (int)str, \
                (int)0, (int)0, (int)0, (int)0); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                logMsg("%d: %s\n", (int)__LINE__, (int)str, \
                (int)0, (int)0, (int)0, (int)0); \
            } \
        } \
    } \
}

#define DBG_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp32 != 0)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str;
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                         (bt_trace_mreg_gp64 != 0)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                bt_log str;
            } \
        } \
    } \
}

#else   /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif  /* DEBUG */

#define BUGGER    { \
        logMsg("%d: %d:\n", (int)__FILE__, (int)__LINE__, \
            (int)0, (int)0, (int)0, (int)0); \
    }

/******************** LINUX *************************************************/
#elif   defined(__linux__)

/* Formatting prefix for each call */
#define LOG_FMT         KERN_NOTICE "dev %d: %s: " __FILE__ "#" LINE_STR ": "

/* Arguments to use for each formatting prefix */
#define LOG_ARG         (t_dev_name), (t_func_name)

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        }  \
    }  \
}

#define FATAL_MSG(str)  { /* system is unusable */      \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str;  \
            } \
        }  \
    }  \
}

/*      Warning messages
**
** An error occurred that the device driver can work around. It may
** prevent certain functions from operating, however the device driver
** is otherwise useable. An example would be running out of a system
** resource.
*/
#define WARN_STR(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        }  \
    }  \
}

#define WARN_MSG(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        }  \
    }  \
}

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        }  \
    }  \
}

#define INFO_MSG(str)   { \
    if (TRC_EVAL(BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        }  \
    }  \
}

/*
**
**      Trace macros controlling which functions are traced
**
**      Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY          { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFB0000 /* Function Beginning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "Entry.\n", LOG_ARG); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFB0000 /* Function Beginning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "Entry.\n", LOG_ARG); \
            } \
        }  \
    }  \
}

#define FEXIT(exit_val) { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFE0000 /* Function Ending */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
                printk(LOG_FMT "Exit = %ld 0x%lx.\n", LOG_ARG, \
                    tmp_exit, tmp_exit); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFE0000 /* Function Ending */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
                printk(LOG_FMT "Exit = %ld 0x%lx.\n", LOG_ARG, \
                    tmp_exit, tmp_exit); \
            } \
        }  \
    }  \
}

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        }  \
    }  \
}

#define TRC_MSG(trc,str) { \
    if (TRC_EVAL(trc)) {        /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        }  \
    }  \
}

/*      Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if defined (DEBUG)

#define DBG_STR(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        }  \
    }  \
}

#define DBG_MSG(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                printk str; \
            } \
        }  \
    }  \
}
#else /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif /* DEBUG */


/******************** LYNX OS ***********************************************/
#elif defined(__lynxos__)

#define LOG_FMT         "%d: %s: " __FILE__ "#" LINE_STR ": "
#define LOG_ARG         (t_dev_name), (t_func_name)


/*      Fatal errors
**
**  A fatal or critical error making further use of the device driver
**  impossible or very severely limited.
*/
#define FATAL_STR(str)    { /* system is unusable */  \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } \
    } \
}

#define FATAL_MSG(str)    { /* system is unusable */    \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } \
    } \
}


/*      Warning messages
**
**  An error occurred that the device driver can work around. It may
**  prevent certain functions from operating, however the device driver
**  is otherwise useable. An example would be running out of a system
**  resource.
*/
#define WARN_STR(str)    { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } \
    } \
}

#define WARN_MSG(str)    { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } \
    } \
}


/*      Informational messages
**
**  Any time the device driver is about to return an error, there should
**  be a trace message at this level describing the error. This is useful
**  as an aid in debugging user applications.
**
**  NOTE: These are errors caused by an application, not the device driver.
**  By default, this level of tracing is turned off.
*/
#define INFO_STR(str)    { /* Informational messages, why an error return */ \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } \
    } \
}


#define INFO_MSG(str)    { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } \
    } \
}

/*      Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY        { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "Entry.\n", LOG_ARG); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFB0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "Entry.\n", LOG_ARG); \
            } \
        } \
    } \
}

#define FEXIT(exit_val)    { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
                ARCH_PRINTF(LOG_FMT "Exit = %ld 0x%lx.\n", \
                        LOG_ARG,tmp_exit,tmp_exit); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
                ARCH_PRINTF(LOG_FMT "Exit = %ld 0x%lx.\n", \
                        LOG_ARG,tmp_exit,tmp_exit); \
            } \
        } \
    } \
}

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
    if ((trc) == (bt_trace_lvl_g & (trc))) {     /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } \
    } \
}
#define TRC_MSG(trc,str) { \
	if ((bt_trace_lvl_g & (trc))) {     /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } \
    } \
}


/*      Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if     defined (DEBUG)
#define DBG_STR(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF(LOG_FMT "%s\n", LOG_ARG, (str)); \
            } \
        } \
    } \
}
#define DBG_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000  \
	            | BT_BCD4(__LINE__); \
            } else { \
                ARCH_PRINTF str;  \
            } \
        } \
    } \
}

#else   /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif  /* DEBUG */


#define BUGGER    { \
	ARCH_PRINTF( __FILE__ ": " LINE_STR "\n"); \
	}

/******************** SUN ***************************************************/
#elif   defined(__sun)

/* Priority setting used by sun kernel print function.  See cmn_err. */
#if !defined (LOGPRI)
#define LOGPRI         bt_logpri_g,
#endif /* LOGPRI */

/* Formatting prefix for each call */
#define LOG_FMT     /* Formatting prefix for each call */ \
    LOGPRI "%d: %s:" LINE_STR ": "

/* Arguments to use for each formatting prefix */
#define LOG_ARG         (t_dev_name), (t_func_name)

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;     /* system is unusable */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));			\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;     /* system is unusable */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));			\
            } \
        }  \
    }  \
}

#define FATAL_MSG(str)  { /* system is unusable */      \
    if (bt_trace_lvl_g) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;     /* system is unusable */	\
	      cmn_err str;						\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xAA0000 /* AA = Fatal */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;     /* system is unusable */	\
	      cmn_err str;						\
            } \
        }  \
    }  \
}

/*      Warning messages
**
** An error occurred that the device driver can work around. It may
** prevent certain functions from operating, however the device driver
** is otherwise useable. An example would be running out of a system
** resource.
*/
#define WARN_STR(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;    /* warning conditions */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));			\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;    /* warning conditions */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));			\
            } \
        }  \
    }  \
}

#define WARN_MSG(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;    /* warning conditions */	\
	      cmn_err str;						\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xBB0000 /* BB = Warning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_WARN;    /* warning conditions */	\
	      cmn_err str;						\
            } \
        }  \
    }  \
}

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;    /* informational */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;    /* informational */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        }  \
    }  \
}

#define INFO_MSG(str)   { \
    if (TRC_EVAL(BT_TRC_INFO)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;    /* informational */	\
	      cmn_err str;					\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xCC0000 /* CC = Info */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;    /* informational */	\
	      cmn_err str;					\
            } \
        }  \
    }  \
}

/*
**
**      Trace macros controlling which functions are traced
**
**      Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY          { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFB0000 /* Function Beginning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "Entry.\n", LOG_ARG);		\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFB0000 /* Function Beginning */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "Entry.\n", LOG_ARG);		\
            } \
        }  \
    }  \
}

#define FEXIT(exit_val) { \
    if (TRC_EVAL(BT_TRC_FUNC)) { \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xFE0000 /* Function Ending */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
		bt_logpri_g = CE_CONT;   /* debug messages */	\
                cmn_err(LOG_FMT "Exit = %ld 0x%lx.\n", LOG_ARG, \
                    tmp_exit, tmp_exit); \
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xFE0000 /* Function Ending */ \
	            | BT_BCD4(__LINE__); \
            } else { \
                long tmp_exit = (long) exit_val; \
		bt_logpri_g = CE_CONT;   /* debug messages */	\
                cmn_err(LOG_FMT "Exit = %ld 0x%lx.\n", LOG_ARG, \
                    tmp_exit, tmp_exit); \
            } \
        }  \
    }  \
}

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
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        }  \
    }  \
}

#define TRC_MSG(trc,str) { \
    if (TRC_EVAL(trc)) {        /* debug messages */ \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err str;					\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xEE0000 /* EE = Standard Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err str;					\
            } \
        }  \
    }  \
}

/*      Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if defined (DEBUG)

#define DBG_STR(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err(LOG_FMT "%s\n", LOG_ARG, (str));		\
            } \
        }  \
    }  \
}

#define DBG_MSG(trc,str) { \
    if (TRC_EVAL(trc))  {  \
        if (bt_trace_mreg_size == 32) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp32 != NULL)) { \
                *bt_trace_mreg_gp32 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err str;					\
            } \
        } else if (bt_trace_mreg_size == 64) { \
            if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
                    (bt_trace_mreg_gp64 != NULL)) { \
                *bt_trace_mreg_gp64 = (trace_file_number_g << 24) \
		        | 0xDD0000 /* DD = Debugging Trace Macro */ \
	            | BT_BCD4(__LINE__); \
            } else { \
	      bt_logpri_g = CE_CONT;   /* debug messages */	\
	      cmn_err str;					\
            } \
        }  \
    }  \
}
#else /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif /* DEBUG */

/******************** WIN XP ************************************************/
#elif   defined(__winxp)

/*****************************************************************************
**
**  Trace macros controlling error output
**
*****************************************************************************/
/*  Fatal errors
**
** A fatal or critical error making further use of the device driver
** impossible or very severely limited.
*/
#if DBG
#define FATAL_STR(str)  { /* system is unusable */  \
    if (bt_trace_lvl_g & BT_TRC_ERROR) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xAA0000 /* AA = Fatal */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_drv_trace(LOG_FMT "ERROR:%s\n", LOG_ARG, (str)); \
            btw_user_trace(LOG_FMT "ERROR:%s\n", LOG_ARG, (str)); \
        } \
    } }

#define FATAL_MSG(str)  { /* system is unusable */  \
    if (bt_trace_lvl_g & BT_TRC_ERROR) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xAA0000 /* AA = Fatal */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str;  \
            btw_drv_trace str;  \
        } \
    } }

#else /* DBG */
#define FATAL_STR(str)  { /* system is unusable */  \
    if (bt_trace_lvl_g & BT_TRC_ERROR) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xAA0000 /* AA = Fatal */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "ERROR:%s\n", LOG_ARG, (str)); \
        } \
    } }

#define FATAL_MSG(str)  { /* system is unusable */  \
    if (bt_trace_lvl_g & BT_TRC_ERROR) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xAA0000 /* AA = Fatal */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str;   \
        } \
    } }
#endif /* DBG */


/*  Warning messages
**
** An error occurred that the device driver can work around. It may
** prevent certain functions from operating, however the device driver
** is otherwise useable. An example would be running out of a system
** resource.
*/

#if DBG
#define WARN_STR(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xBB0000 /* BB = Warning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "WARN:%s\n", LOG_ARG, (str)); \
            btw_drv_trace(LOG_FMT "WARN:%s\n", LOG_ARG, (str)); \
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
            btw_user_trace str; \
            btw_drv_trace str; \
        } \
    } }

#else /* DBG */
#define WARN_STR(str)   { /* Warning messages */ \
    if (bt_trace_lvl_g & (BT_TRC_WARN|BT_TRC_INFO)) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xBB0000 /* BB = Warning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "WARN:%s\n", LOG_ARG, (str)); \
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
            btw_user_trace str; \
        } \
    } }
#endif /* DBG */


/*  Informational messages
**
** Any time the device driver is about to return an error, there should
** be a trace message at this level describing the error. This is useful
** as an aid in debugging user applications.
**
** NOTE: These are errors caused by an application, not the device driver.
** By default, this level of tracing is turned off.
*/

#if DBG
#define INFO_STR(str)   { /* Informational messages, why an error return */ \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "INFO:%s\n", LOG_ARG, (str)); \
            btw_drv_trace(LOG_FMT "INFO:%s\n", LOG_ARG, (str)); \
        } \
    } }

#define INFO_MSG(str)   { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str; \
            btw_drv_trace str; \
        } \
    } }
#else /* DBG */
#define INFO_STR(str)   { /* Informational messages, why an error return */ \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "INFO:%s\n", LOG_ARG, (str)); \
        } \
    } }

#define INFO_MSG(str)   { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xCC0000 /* CC = Info */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str; \
        } \
    } }
#endif /* DBG */


/*****************************************************************************
**
**  Trace macros controlling which functions are traced
**
*****************************************************************************/

/*  Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#if DBG
#define FENTRY      { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xFB0000 /* FB = function Beginning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "Entry.\n", LOG_ARG); \
            btw_drv_trace(LOG_FMT "Entry.\n", LOG_ARG); \
        } \
    } }

#define FEXIT(exit_val) { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xFE0000 /* FE = Function Ending */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "Exit = %d 0x%04x.\n", LOG_ARG, \
                (int) exit_val, (int) exit_val); \
            btw_drv_trace(LOG_FMT "Exit = %d 0x%04x.\n", LOG_ARG, \
                (int) exit_val, (int) exit_val); \
        } \
    } }

#else /* DBG */
#define FENTRY      { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xFB0000 /* FB = function Beginning */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "Entry.\n", LOG_ARG); \
        } \
    } }

#define FEXIT(exit_val) { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xFE0000 /* FE = Function Ending */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "Exit = %d 0x%04x.\n", LOG_ARG, \
                (int) exit_val, (int) exit_val); \
        } \
    } }
#endif /* DBG */

/*  Standard tracing macros
**  
**  Driver tracing macros that are left in the production version of the 
**  driver. These macros can help determine exactly what is happening as
**  the driver is running. They are not normally used by a customer, but
**  left in the driver as an aid for customer support.
**
**  Only specify one of the BT_TRC_ functional bits (4-27) for each
**  macro.
**
**  You can use one of the modifier bit BT_TRC_DETAIL
**  with any of the functional bits. This will cause the
**  trace to occur only if both bits are set.
*/

#if DBG
#define TRC_STR(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) {   /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "%s\n", LOG_ARG, (str)); \
            btw_drv_trace(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define TRC_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) {   /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str; \
            btw_drv_trace str; \
        } \
    } }

#else /* DBG */
#define TRC_STR(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) {   /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define TRC_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) {   /* debug messages */ \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xEE0000 /* EE = Standard Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str; \
        } \
    } }
#endif /* DBG */


/*  Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/

#if DBG
#define DBG_STR(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xDD0000 /* DD = Debug Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace(LOG_FMT "%s\n", LOG_ARG, (str)); \
            btw_drv_trace(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
    } }

#define DBG_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp != NULL)) { \
            *bt_trace_mreg_gp = (trace_file_number_g << 24) \
                | 0xDD0000 /* DD = Debug Trace Macro */ \
                | BT_BCD4(__LINE__); \
        } else { \
            btw_user_trace str; \
            btw_drv_trace str; \
        } \
    } }

#else /* DBG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif /* DBG */

/* 
** Valid trace level mask 
*/
#define BT_TRC_VALID_MASK 0x7fffffff

/* 
** Default tracing level 
*/
#define BT_TRC_USR_DEFAULT  (BT_TRC_ERROR|BT_TRC_WARN|BT_TRC_CFG|BT_TRC_INFO) /* user trace default */

/******************** SGI  ***********************************************/
#elif   defined(__sgi)

extern int bt_logpri_g;

/* Formatting prefix for each call */ 
#define LOG_FMT bt_logpri_g, "%d: %s:" __FILE__ "#" LINE_STR ": "

/* Arguments to use for each formatting prefix */ 
/* #define LOG_ARG bt_name_gp, t_unit, f_name */
#define LOG_ARG         (t_dev_name), (t_func_name)

/*  
**  Fatal errors
**
**  A fatal or critical error making further use of the device driver
**  impossible or very severely limited.
*/
#define FATAL_STR(str) { \
    if (bt_trace_lvl_g)  { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
           (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xAA0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;     /* system is unusable */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
           (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xAA0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;     /* system is unusable */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } \
    } \
  }

/* system is unusable */
#define FATAL_MSG(str) { \
    if (bt_trace_lvl_g ) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xAA0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;     /* system is unusable */  \
          cmn_err str; \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xAA0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;     /* system is unusable */  \
          cmn_err str; \
        } \
      } \
    } \
  }

/*  
**  Warning messages
**
**  An error occurred that the device driver can work around. It may
**  prevent certain functions from operating, however the device driver
**  is otherwise useable. An example would be running out of a system
**  resource.
*/
#define WARN_STR(str) { \
    if (bt_trace_lvl_g & (BT_TRC_WARN | BT_TRC_INFO)) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xBB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;    /* warning conditions */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xBB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;    /* warning conditions */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } \
    } \
  }


#define WARN_MSG(str) { \
    if (bt_trace_lvl_g & (BT_TRC_WARN | BT_TRC_INFO)) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xBB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;    /* warning conditions */  \
          cmn_err str; \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xBB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_WARN;    /* warning conditions */  \
          cmn_err str; \
        } \
      } \
    } \
  }

/*  
**  Informational messages
**
**  Any time the device driver is about to return an error, there should
**  be a trace message at this level describing the error. This is useful
**  as an aid in debugging user applications.
**
**  NOTE: These are errors caused by an application, not the device driver.
**  By default, this level of tracing is turned off.
*/
#define INFO_STR(str) { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xCC0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_CONT;    /* informational */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xCC0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_CONT;    /* informational */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } \
    } \
  }

#define INFO_MSG(str) { \
    if (bt_trace_lvl_g & BT_TRC_INFO) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xCC0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_CONT;    /* informational */  \
          cmn_err str; \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xCC0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_CONT;    /* informational */  \
          cmn_err str; \
        } \
      } \
    } \
  }
 
/*
**  Standard tracing macros
** 
**  Driver tracing macros that are left in the production version of the 
**  driver. These macros can help determine exactly what is happening as
**  the driver is running. They are not normally used by a customer, but
**  left in the driver as an aid for customer support.
**
**  Only specify one of the BT_TRC_ functional bits (4-27) for each
**  macro.
**
**  You can use one of the modifier bits (BT_TRC_PROFILE or BT_TRC_DETAIL)
**  with any of the functional bits. This will cause the
**  trace to occur only if both bits are set.
*/
#define TRC_STR(trc,str) {  \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xDD0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xDD0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } \
    } \
  }

#define TRC_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) { \
      if (bt_trace_mreg_size == 32)  { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xDD0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err str; \
        } \
      } else if (bt_trace_mreg_size == 64)  { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xDD0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err str; \
        } \
      } \
    } \
  }
 
/*  
**  Debugging macros
**
**  These macros are taken out of the production version of the device
**  driver, either because of performance concerns or because they are
**  only of use during actual driver development.
*/
#if defined (DEBUG)
#define DBG_STR(trc,str) {  \
    if ((trc) == (bt_trace_lvl_g & (trc)))  {  \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
           *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xEE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
           *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xEE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */  \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, (str)); \
        } \
      } \
    } \
  }

#define DBG_MSG(trc,str) { \
    if ((trc) == (bt_trace_lvl_g & (trc))) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xEE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err str; \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xEE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err str; \
        } \
      } \
    } \
  }

#else /* DEBUG */
#define DBG_STR(trc,str)
#define DBG_MSG(trc,str)
#endif /* DEBUG */

/*
**  Function entry and exit tracing
**
**  These macros show when each routine is enterred and exitted.
*/
#define FENTRY    { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xFB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, "ENTRY"); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xFB0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err(LOG_FMT "%s\n", LOG_ARG, "ENTRY"); \
        } \
      } \
    } \
  }

/* ed. note: we cannot assume an int is large enough to hold all possible
   return values anymore.  We use a long instead.  */
#define FEXIT(exit_val)     { \
    if (bt_trace_lvl_g & BT_TRC_FUNC) { \
      if (bt_trace_mreg_size == 32) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp32 != NULL)) { \
          *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | 0xFE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err(LOG_FMT "EXIT: %ld\n", LOG_ARG, (long) exit_val); \
        } \
      } else if (bt_trace_mreg_size == 64) { \
        if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
            (bt_trace_mreg_gp64 != NULL)) { \
          *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | 0xFE0000 | BT_BCD4(__LINE__); \
        } else { \
          bt_logpri_g = CE_DEBUG;   /* debug messages */ \
          cmn_err(LOG_FMT "EXIT: %ld\n", LOG_ARG, (long) exit_val); \
        } \
      } \
    }  \
  }

/*  
**  Hardware debugging macros
**
**  These macros are meant to be used with a VMETRO and are never
**  included in a final version of the driver.
**  
**  NOTE:  assumes that unit_p is defined and valid
*/
#if defined (DEBUG)
#define HW_TRC_LINE(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp32 = (func << 24) | BT_BCD4(__LINE__);
#define HW_TRC_POINT(func, point) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp32 = (func << 24) | point;
#define HW_TRC_VALUE(value) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp32 = (bt_data32_t) value;
#define HW_TRC_FOPEN(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp32 = (func << 24) | 0;
#define HW_TRC_FCLOSE(func) if (bt_trace_lvl_g & BT_TRC_PROFILE) *bt_trace_mreg_gp32 = (func << 24) | 0xffu;
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
#define BUGGER    { \
    if (bt_trace_mreg_size == 32) { \
      if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
          (bt_trace_mreg_gp32 != NULL)) { \
        *bt_trace_mreg_gp32 = (trace_file_number_g << 24) | BT_BCD4(__LINE__); \
      } else { \
        bt_logpri_g = CE_CONT;     /* debug messages */ \
        cmn_err(LOG_FMT "\n", LOG_ARG); \
      } \
    } else if (bt_trace_mreg_size == 64) { \
      if ((bt_trace_lvl_g & BT_TRC_HW_DEBUG) && \
          (bt_trace_mreg_gp64 != NULL)) { \
        *bt_trace_mreg_gp64 = (trace_file_number_g << 24) | BT_BCD4(__LINE__); \
      } else { \
        bt_logpri_g = CE_CONT;     /* debug messages */ \
        cmn_err(LOG_FMT "\n", LOG_ARG); \
      } \
    } \
  } 
 

#define HW_BUGGER(func, value)      *bt_trace_mreg_gp32 = (func << 24) | value;


#else   /* !defined(__vxworks | __linux__ | __lynxos__ | __sun | __winxp | __sgi) */
#error  Trace macro not implemented for this operating system
#endif  /* defined(__vxworks | __linux__ | __lynxos__ | __sun | __winxp | __sgi) */


#endif  /* !defined(_BT_TRACE_H) */
