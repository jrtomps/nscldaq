/******************************************************************************
**
**      Filename:       btuisr.h
**
**      Purpose:        SBS Technologies User ISR Device Driver Interface 
**                      header file.
**
**      $Revision: 1.1 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _H_BT_UISR
#define _H_BT_UISR

/*
**  OS specific unit structure map structure for accessing remote resouces
*/
typedef struct { 
    
    caddr_t     nio_p;         /* Ptr to node registers */
    caddr_t     mreg_p;        /* ptr to allocated mapping register */
    caddr_t     rmem_p;        /* ptr to remote memory to use */
    bt_data32_t loc_id;        /* Id of local card */
    bt_data32_t rem_id;        /* Id of remote card */
    bt_data32_t nio_len;       /* Length of node regs */
    bt_data32_t mreg_start;    /* Starting mapping register allocated */
    bt_data32_t mreg_len;      /* Number of isr mapping regs */
    bt_data32_t rmem_len;      /* Length of isr remote memory */
    bt_swap_t   default_swap;  /* Default swapping for adaptor */
} bt_kmap_t;                   /*   May be wrong if no remote power */
                               /*   during load or if remote */
                               /*   adaptor is changed without */
                               /*   rebooting */

/*
**  User interrupt service routine definition
*/
typedef bt_data32_t bt_uisr_t (u_short unit, caddr_t param_p, bt_irq_t irq_type);

/*
**  Structure for registering a remote bus interrupt.
*/
typedef struct {
    u_short     unit;               /* unit number of device */
    bt_irq_t    flags;              /* bt_irq_t values - BT_IRQ_OVERFLOW is illegal */
    int         level;              /* cable interrupt number */
#if !defined(BT946)
    bt_uisr_t  *interrupt_routine;  /* User ISR to call */
#else
    uint_t  (*interrupt_routine)(caddr_t);  /* User ISR to call */
#endif
    caddr_t     param;              /* param passed to user ISR */
#if defined(BT946)
    bt_data8_t  vector;
#endif

} bt_rembus_intr_t;

/*
** Prototypes for kernel functions
*/
extern int bt_kmap(unsigned int unit, bt_kmap_t *kmap_p);
extern int bt_kunmap(unsigned int unit, bt_kmap_t *kmap_p);
extern int bt_rembus_install(unsigned int unit, bt_rembus_intr_t *handler_p);
extern int bt_rembus_remove(unsigned int unit, bt_rembus_intr_t *handler_p);
extern void *bt_map_vme(unsigned int unit, bt_kmap_t *kmap_p, bt_dev_t device, bt_data32_t vme_addr, bt_swap_t swap);
extern int bt_access_error(unsigned int unit);
extern int bt_do_iack(unsigned int unit, int vme_level, bt_data32_t *vector_p);

#endif /* _H_BT_UISR */

