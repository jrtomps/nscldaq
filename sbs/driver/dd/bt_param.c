/******************************************************************************
**
**  Filename:   bt_param.c
**
**  Purpose:    Kernel bit map manipulation routines. Used to track allocation
**              and deallocation bit maps.
**
**  Functions:  btk_get_info(), btk_set_info()
**
**  Calls:      
**
**  $Revision: 742 $
**
******************************************************************************/
/******************************************************************************
**
**  Copyright (c) 2000 by SBS Technologies, Inc.
**
**  All Rights Reserved.
**  License governs use and distribution.
**
******************************************************************************/

#if !defined(LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $ "__DATE__;
#endif /* !LINT */

/* define USE_BIGPHYSAREA 1 if you are patching the kernel to add the bigphysarea code */
#define USE_BIGPHYSAREA 0

/*
** Include files
*/
#include "btdd.h"
#if defined (BT1003)
#if EAS_BIND_CODE
#include <asm/io.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#include <linux/wrapper.h>
#else
#include <linux/page-flags.h>
#endif
#if USE_BIGPHYSAREA
#include <linux/bigphysarea.h>
#endif
#endif /* EAS_BIND_CODE */
#endif

/*
** Function prototypes
*/
bt_error_t btk_set_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, 
           bt_devdata_t value);
bt_error_t btk_get_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, 
           bt_devdata_t *value_p);
extern void btk_setup_mreg(bt_unit_t *unit_p, bt_dev_t ldev, 
           bt_data32_t *mreg_value_p, bt_operation_t op);
extern bt_data32_t btk_get_mreg(bt_unit_t *unit_p, unsigned int mr_idx, 
           bt_mreg_t mreg_type);
extern void btk_put_mreg(bt_unit_t *unit_p, unsigned int mr_idx, 
           bt_mreg_t mreg_type, bt_data32_t value);
static void set_lm_swap(bt_unit_t * unit_p);
extern bt_data32_t btk_get_io(bt_unit_t * unit_p, bt_reg_t reg);
extern void btk_put_io(bt_unit_t * unit_p, bt_reg_t reg, bt_data32_t value);

#define LOCK_DMA(u_p)    btk_mutex_enter((u_p), &(u_p)->dma_mutex);
#define UNLOCK_DMA(u_p)  btk_mutex_exit((u_p), &(u_p)->dma_mutex);

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_PARAM_C);


/******************************************************************************
**
**      Function:   btk_set_info()
**
**      Purpose:    Given info parameter and value sets approprate unit
**                  structure member
**
**      Args:       unit_p          Pointer to unit structure.
**                  ldev            Logical device type.
**                  param           Info parameter.
**                  value           Info parameter value to set.
**
**      Returns:    BT_SUCCESS      Successful
**                  other           failure code
**
******************************************************************************/
bt_error_t btk_set_info(
    bt_unit_t *unit_p,
    bt_dev_t ldev,
    bt_info_t param,
    bt_devdata_t value)
{

    bt_error_t   retval = BT_SUCCESS;
    char        *einval_p = "Invalid parameter value";
    int          old_value;
    bt_data32_t  tmp_reg;

    FUNCTION("btk_set_info");
    LOG_UNIT(unit_p);

    FENTRY;
    
    TRC_MSG(BT_TRC_CTRL, 
	    (LOG_FMT "Setting parameter %d for device %d to value 0x%x\n",
	     LOG_ARG, param, ldev, value));

    /* Don't let the array be index outside its definition */
    if (ldev > BT_MAX_AXSTYPS) {
        INFO_STR("Invalid type parameter.");
        retval = BT_EINVAL;
        goto exit_btk_set_info;
    }

    /*
    ** Since the intent of btk_set_info is to *modify* attributes,
    ** only those attributes that are writable should be listed.
    */
    switch (param) {

#if defined (BT1003)
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_INFO_A64_OFFSET:
        /* set new A64 offset and mode bit */
        unit_p->a64_offset = value;
        break;
#endif  /* EAS_A64_CODE */
#if EAS_BIND_CODE
/* EAS TMP CODE */
      case BT_INFO_KMALLOC_SIZ:
        /* set amount of memory to kmallock on get */
        unit_p->bt_kmalloc_size = value;
        break;
      case BT_INFO_KMALLOC_BUF:
        /* set kernel allocated buffer value, allocated by some other means i.e. another driver */
        unit_p->bt_kmalloc_buf = (unsigned int *)value;
        break;
      case BT_INFO_KFREE_BUF:
        if (unit_p->bt_kmalloc_ptr) {
            /* valid, free driver allocated kernel buffer, don't do this if allocated by some other means i.e. bigphysarea patch */
            unsigned long virt_addr;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            unsigned int i;
#endif

            /* unreserve all pages */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            for(i = MAP_NR(unit_p->bt_kmalloc_buf); i <= MAP_NR((void *)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size); i++)
            {
                mem_map_unreserve(i);
            }
#else
            for(virt_addr = (unsigned long)unit_p->bt_kmalloc_buf; virt_addr < (unsigned long)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size;
    	        virt_addr += PAGE_SIZE)
            {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
                mem_map_unreserve(virt_to_page(virt_addr));
#else
                ClearPageReserved(virt_to_page(virt_addr));
#endif
            }
#endif
            kfree(unit_p->bt_kmalloc_ptr);
            unit_p->bt_kmalloc_buf =
            unit_p->bt_kmalloc_ptr = NULL;
        }
#if USE_BIGPHYSAREA
        else if (unit_p->bt_kmalloc_buf) {
            /* allocated kernel buffer via bigphysarea, free it */
            unsigned long virt_addr;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            unsigned int i;
#endif

            /* unreserve all pages */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            for(i = MAP_NR(unit_p->bt_kmalloc_buf); i <= MAP_NR((void *)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size); i++)
            {
                mem_map_unreserve(i);
            }
#else
            for(virt_addr = (unsigned long)unit_p->bt_kmalloc_buf; virt_addr < (unsigned long)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size;
    	        virt_addr += PAGE_SIZE)
            {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
                mem_map_unreserve(virt_to_page(virt_addr));
#else
                ClearPageReserved(virt_to_page(virt_addr));
#endif
            }
#endif
            bigphysarea_free((caddr_t)unit_p->bt_kmalloc_buf, (int)(unit_p->bt_kmalloc_size + (2 * PAGE_SIZE)));
            unit_p->bt_kmalloc_buf =
            unit_p->bt_kmalloc_ptr = NULL;
        }
#endif  /* USE_BIGPHYSAREA */
        break;

#endif /* EAS_BIND_CODE */
#endif

      case BT_INFO_TRACE:
        bt_trace_lvl_g = value;
        break;
        
      case BT_INFO_RESET_DELAY:
        unit_p->reset_timer = (((bt_msec_t) value) != 0) ? 
                               ((bt_msec_t) value) : 
                               ((bt_msec_t) DEFAULT_RESET_TIMER);
        break;
        
      case BT_INFO_PIO_AMOD:
        if (value != 0) {
          unit_p->pio_addr_mod[ldev] = value;
        } else {
          switch (ldev) {
            case BT_AXSRI:
              unit_p->pio_addr_mod[ldev] = BT_AMOD_A16;
              break;
            case BT_AXSRR:
            case BT_AXSRE:
              unit_p->pio_addr_mod[ldev] = BT_AMOD_A32;
              break;
            case BT_AXS24:
              unit_p->pio_addr_mod[ldev] = BT_AMOD_A24;
              break;
            default:
              unit_p->pio_addr_mod[ldev] = 0;
              break;
          }
        }
        break;
        
      case BT_INFO_DMA_AMOD:
        if (value != 0) {
          unit_p->dma_addr_mod[ldev] = value;
        } else {
          switch (ldev) {
            case BT_AXSRI:
              unit_p->dma_addr_mod[ldev] = BT_AMOD_A16;
              break;
            case BT_AXSRR:
            case BT_AXSRE:
              unit_p->dma_addr_mod[ldev] = BT_AMOD_A32;
              break;
            case BT_AXS24:
              unit_p->dma_addr_mod[ldev] = BT_AMOD_A24;
              break;
            default:
              unit_p->dma_addr_mod[ldev] = 0;
              break;
          }
        }
        break;
        
      case BT_INFO_MMAP_AMOD:
        if (value != 0) {
          unit_p->mmap_addr_mod[ldev] = value;
        } else {
          switch (ldev) {
            case BT_AXSRI:
              unit_p->mmap_addr_mod[ldev] = BT_AMOD_A16;
              break;
            case BT_AXSRR:
            case BT_AXSRE:
              unit_p->mmap_addr_mod[ldev] = BT_AMOD_A32;
              break;
            case BT_AXS24:
              unit_p->mmap_addr_mod[ldev] = BT_AMOD_A24;
              break;
            default:
              unit_p->mmap_addr_mod[ldev] = 0;
              break;
          }
        }
        break;
        
      case BT_INFO_DATAWIDTH:
        switch (value) {
          case BT_WIDTH_D64:
          case BT_WIDTH_D32:
          case BT_WIDTH_D16:
          case BT_WIDTH_D8:
          case BT_WIDTH_ANY:
            unit_p->data_size[ldev] = value;
            break;
          default:
            INFO_STR(einval_p);
            retval = BT_EINVAL;
            break;
        }
        break;
        
      case BT_INFO_SWAP:
        old_value = unit_p->swap_bits[ldev];
        if (value >= BT_MAX_SWAP) {
          INFO_STR(einval_p);
          retval = BT_EINVAL;
          break;
        } else if (value == BT_SWAP_DEFAULT) {
          if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            unit_p->swap_bits[ldev] = BT_SWAP_NONE;
          } else {
            unit_p->swap_bits[ldev] = BT_SWAP_VMEBUS;
          }
        } else {
          unit_p->swap_bits[ldev] = value;
        }
        /* The local memory device must be updated with the new swap value */
        /* for BT_AXSLM minor device and all non-PCI2PCI adapters.         */
        if (BT_AXSLM == ldev && 
            IS_CLR(unit_p->bt_status, BT_PCI2PCI) &&
            unit_p->swap_bits[BT_AXSLM] != old_value) {
            set_lm_swap(unit_p);
        }
        break;
        
      case BT_INFO_DMA_THRESHOLD:
        unit_p->dma_threshold = value;
        break;
        
      case BT_INFO_GEMS_SWAP:
        if (value) {
            SET_BIT(unit_p->bt_status, BT_GEMS_SWAP);
            tmp_reg = btk_get_io(unit_p, BT_LOC_BUS_CTRL);
            if (IS_CLR(tmp_reg, LBC_GEMS_SWAP_ENABLE)) {
                SET_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
                btk_put_io(unit_p, BT_LOC_BUS_CTRL, tmp_reg);
            }
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                /* old cards, remote register access must synchronize lock in case dma active */
                LOCK_DMA(unit_p);
            }
            tmp_reg = btk_get_io(unit_p, BT_RPQ_REM_LBUS_CTRL);
            if (IS_CLR(tmp_reg, LBC_GEMS_SWAP_ENABLE)) {
                SET_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
                btk_put_io(unit_p, BT_RPQ_REM_LBUS_CTRL, tmp_reg);
            }
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                UNLOCK_DMA(unit_p);
            }
        } else {
            CLR_BIT(unit_p->bt_status, BT_GEMS_SWAP);
            tmp_reg = btk_get_io(unit_p, BT_LOC_BUS_CTRL);
            if (IS_SET(tmp_reg, LBC_GEMS_SWAP_ENABLE)) {
                CLR_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
                btk_put_io(unit_p, BT_LOC_BUS_CTRL, tmp_reg);
            }
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                /* old cards, remote register access must synchronize lock in case dma active */
                LOCK_DMA(unit_p);
            }
            tmp_reg = btk_get_io(unit_p, BT_RPQ_REM_LBUS_CTRL);
            if (IS_SET(tmp_reg, LBC_GEMS_SWAP_ENABLE)) {
                CLR_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
                btk_put_io(unit_p, BT_RPQ_REM_LBUS_CTRL, tmp_reg);
            }
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                UNLOCK_DMA(unit_p);
            }
        }
        break;
        
      case BT_INFO_PAUSE:
        if (value) {
          SET_BIT(unit_p->bt_status, BT_DMA_WAIT);
        } else {
          CLR_BIT(unit_p->bt_status, BT_DMA_WAIT);
        }
        break;
        
      case BT_INFO_DMA_POLL_CEILING:
        unit_p->dma_poll_size = value;
        break;
        
      case BT_INFO_DMA_WATCHDOG:
        unit_p->dma_timeout = (((bt_msec_t) value) > 0) ?
                               ((bt_msec_t) value) : 
                               ((bt_msec_t) DEFAULT_DMA_TIMEOUT);
        break;
        
      case BT_INFO_INC_INHIB:
        if (value) {
          SET_BIT(unit_p->bt_status, BT_DMA_FIFO);
        } else {
          CLR_BIT(unit_p->bt_status, BT_DMA_FIFO);
        }
        break;
        
      case BT_INFO_BLOCK:
        if (value) {
          SET_BIT(unit_p->bt_status, BT_DMA_BLOCK);
        } else {
          CLR_BIT(unit_p->bt_status, BT_DMA_BLOCK);
        }
        break;
        
      case BT_INFO_USE_PT:
        if (value) {
          SET_BIT(unit_p->bt_status, BT_SEND_PT);
        } else {
          CLR_BIT(unit_p->bt_status, BT_SEND_PT);
        }
        break;
        
      case BT_MIN_INFO:
      default:                /* Default will catch all new ones  */
        INFO_STR("The IOCTL parameter isn't supported.");
        retval = BT_EINVAL;
        break;
    }

exit_btk_set_info:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:   btk_get_info()
**
**      Purpose:    Given info parameter and value sets approprate unit
**                  structure member
**
**      Args:       unit_p          Pointer to unit structure.
**                  ldev            Logical device type.
**                  param           Info parameter.
**                  value_p         Pointer to Info parameter value to set.
**
**      Returns:    BT_SUCCESS      Successful
**                  other           failure code
**
******************************************************************************/
bt_error_t btk_get_info(
    bt_unit_t *unit_p,
    bt_dev_t ldev,
    bt_info_t param,
    bt_devdata_t* value_p)
{
    bt_error_t  retval = BT_SUCCESS;

    FUNCTION("btk_get_info");
    LOG_UNIT(unit_p);

    FENTRY;

    /* Don't let the array be index outside its definition */
    if (ldev > BT_MAX_AXSTYPS) {
        INFO_STR("Invalid type parameter.");
        retval = BT_EINVAL;
        goto exit_btk_set_info;
    }
    
    /*
    ** Since the intent of BIOC_DEV_ATTRIB is to *read* attributes,
    ** all attributes whether readable or writable should be listed.
    */
    switch (param) {
#if defined (BT1003)
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_INFO_A64_OFFSET:
        /* return A64 offset and mode bit */
        *value_p = unit_p->a64_offset;
        break;
#endif  /* EAS_A64_CODE */
#if EAS_BIND_CODE
/* EAS BIND CODE */
      case BT_INFO_KMALLOC_SIZ:
        /* return amount of memory to kmallock on get */
        *value_p = unit_p->bt_kmalloc_size;
        break;
      case BT_INFO_KMALLOC_BUF:
        /* return the physical address to kmalloc buf */
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            int i;
#endif
            unsigned long virt_addr;

            if (unit_p->bt_kmalloc_buf || unit_p->bt_kmalloc_ptr) {
                /* buffer already allocated for this device */
                INFO_STR("Kernel buffer already allocated for this device node\n");
                retval = BT_EINVAL;
                *value_p = 0;
                break;
            }

        
            /* get a memory area with kmalloc and aligned it to a page. This area
               will be physically contigous */
            virt_addr = (unsigned long)(unit_p->bt_kmalloc_ptr = kmalloc((unit_p->bt_kmalloc_size + (2 * PAGE_SIZE)), GFP_KERNEL));
            if (!(unit_p->bt_kmalloc_ptr)) {
#if USE_BIGPHYSAREA
                /* unable to obtain kmalloc allocated buf, try bigphysarea */
                virt_addr = (unsigned long)(bigphysarea_alloc(unit_p->bt_kmalloc_size + (2 * PAGE_SIZE)));
                if (!(virt_addr)) {
#endif  /* USE_BIGPHYSAREA */
                    INFO_STR("Unable to obtain kernel allocated buf\n");
                    retval = BT_EINVAL;
                    *value_p = 0;
                    break;
#if USE_BIGPHYSAREA
                }
#endif  /* USE_BIGPHYSAREA */
            }

            /* physically (page sized) align buffer obtained if it isn't already */
            unit_p->bt_kmalloc_buf = (int *)(((unsigned long)virt_addr + PAGE_SIZE -1) & PAGE_MASK);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
            for(i = MAP_NR(unit_p->bt_kmalloc_buf); i <= MAP_NR((void *)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size); i++) {
                /* reserve all pages to make them remapable */
                mem_map_reserve(i);
            }
#else
            for (virt_addr = (unsigned long)unit_p->bt_kmalloc_buf; 
                 virt_addr < (unsigned long)unit_p->bt_kmalloc_buf + unit_p->bt_kmalloc_size;
                 virt_addr += PAGE_SIZE) {
                 /* reserve all pages to make them remapable */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
                 mem_map_reserve(virt_to_page(virt_addr));
#else
                 SetPageReserved(virt_to_page(virt_addr));
#endif
            }
#endif
            sprintf((char *)unit_p->bt_kmalloc_buf, "Kernel allocated buffer of size 0x%x\n", unit_p->bt_kmalloc_size);

            TRC_MSG((BT_TRC_BIND), 
                    (LOG_FMT "Kernel alloced buf: kernel virt=0x%x phys=0x%x data string=%s\n",
                    LOG_ARG, (unsigned int)unit_p->bt_kmalloc_buf, 
                    (unsigned int)virt_to_phys((volatile void *)unit_p->bt_kmalloc_buf), 
                    (char *)unit_p->bt_kmalloc_buf));

            *value_p = (bt_devdata_t)unit_p->bt_kmalloc_buf;
        }

        break;
#endif /* EAS_BIND_CODE */
#endif

      case BT_INFO_TRACE:
        *value_p = (bt_devdata_t) bt_trace_lvl_g;
        break;
      case BT_INFO_RESET_DELAY:
        *value_p = (bt_devdata_t) unit_p->reset_timer;
        break;
      case BT_INFO_DATAWIDTH:
        *value_p = (bt_devdata_t) unit_p->data_size[ldev];
        break;
      case BT_INFO_DMA_THRESHOLD:
        *value_p = (bt_devdata_t) unit_p->dma_threshold;
        break;
      case BT_INFO_PAUSE:
        *value_p = (bt_devdata_t) (IS_SET(unit_p->bt_status, BT_DMA_WAIT) ? TRUE : FALSE);
        break;
      case BT_INFO_BLOCK:
        *value_p = (bt_devdata_t) (IS_SET(unit_p->bt_status, BT_DMA_BLOCK) ? TRUE : FALSE);
        break;
      case BT_INFO_USE_PT:
        *value_p = (bt_devdata_t) (IS_SET(unit_p->bt_status, BT_SEND_PT) ? TRUE : FALSE);
        break;
      case BT_INFO_INC_INHIB:
        *value_p = (bt_devdata_t) (IS_SET(unit_p->bt_status, BT_DMA_FIFO) ? TRUE : FALSE);
        break;
      case BT_INFO_DMA_POLL_CEILING:
        *value_p = (bt_devdata_t) unit_p->dma_poll_size;
        break;
      case BT_INFO_PIO_AMOD:
        *value_p = (bt_devdata_t) unit_p->pio_addr_mod[ldev];
        break;
      case BT_INFO_DMA_AMOD:
        *value_p = (bt_devdata_t) unit_p->dma_addr_mod[ldev];
        break;
      case BT_INFO_UNIT_NUM:
        *value_p = (bt_devdata_t) unit_p->unit_number;
        break;
      case INF_LOG_STAT:
        *value_p = (bt_devdata_t) unit_p->logstat[ldev];
        break;
      case BT_INFO_MMAP_AMOD:
        *value_p = (bt_devdata_t) unit_p->mmap_addr_mod[ldev];
        break;
      case BT_INFO_LOC_PN:
        *value_p = (bt_devdata_t) unit_p->loc_id;
        break;
      case BT_INFO_ICBR_Q_SIZE:
        *value_p = (bt_devdata_t) unit_p->q_size;
        break;
      case BT_INFO_REM_PN:
        *value_p = (bt_devdata_t) unit_p->rem_id;
        break;
      case BT_INFO_TOTAL_COUNT:
        *value_p = (bt_devdata_t) unit_p->sig_count;
        break;
      case BT_INFO_ERROR_COUNT:
        *value_p = (bt_devdata_t) unit_p->sig_err_cnt;
        break;
      case BT_INFO_EVENT_COUNT:
        *value_p = (bt_devdata_t) unit_p->sig_prg_cnt;
        break;
      case BT_INFO_IACK_COUNT:
        *value_p = (bt_devdata_t) unit_p->sig_other_cnt;
        break;
      case BT_INFO_KMEM_SIZE:
        *value_p = (bt_devdata_t) btk_alloc_total_g;
        break;
      case BT_INFO_GEMS_SWAP:
        if (IS_SET(unit_p->bt_status, BT_GEMS_SWAP)) {
            *value_p = TRUE;
        } else {
            *value_p = FALSE;
        }
        break;
      case BT_INFO_BIND_SIZE:
        if (unit_p->lm_size == 0) {
          *value_p = (bt_devdata_t) BT_MAX_SDMA_LEN;
        } else {
          *value_p = (bt_devdata_t) (BT_MAX_SDMA_LEN - unit_p->lm_size);
        }
        break;
      case BT_INFO_BIND_COUNT:
        if (unit_p->lm_size == 0) {
          *value_p = (bt_devdata_t) (BT_MAX_SDMA_LEN / BT_SYS_PAGE_SIZE);
        } else {
          *value_p = (bt_devdata_t) ((BT_MAX_SDMA_LEN - unit_p->lm_size) / BT_SYS_PAGE_SIZE);
        }
        break;
      case BT_INFO_BIND_ALIGN:
        *value_p = (bt_devdata_t) BT_SYS_PAGE_SIZE;
        break;
      case BT_INFO_LM_SIZE:
        *value_p = (bt_devdata_t) unit_p->lm_size;
        break;
      case BT_INFO_SWAP:
        *value_p = (bt_devdata_t) unit_p->swap_bits[ldev];
        break;
      case BT_INFO_DMA_WATCHDOG:
        *value_p = (bt_devdata_t) unit_p->dma_timeout;
        break;
      case BT_INFO_BOARD_REV:
        *value_p = (bt_devdata_t) unit_p->board_revision;
        break;

/*
** Model 993 for vxWorks specific
*/
#if defined(BT993)
      case BT_INFO_ICBR_Q_START:
        if (NULL != unit_p->error_irq_q_p) {
            *value_p = (bt_devdata_t) unit_p->error_irq_q_p;
        } else {
            *value_p =(bt_devdata_t)  NULL;
        }
        break;

      case BT_INFO_ICBR_PRIO:
        *value_p = (bt_devdata_t) unit_p->disp_prio;
        break;

      case BT_INFO_ICBR_STACK:
        *value_p = (bt_devdata_t) unit_p->disp_stack;
        break;
#endif /* defined(BT993) */

      case BT_MIN_INFO:
      default:
        INFO_STR("The IOCTL parameter isn't supported.");
        retval = BT_EINVAL;
        break;
    }

    if (BT_SUCCESS == retval) {
	TRC_MSG(BT_TRC_CTRL, 
		(LOG_FMT "Value of parameter %d for device %d is 0x%x\n",
		 LOG_ARG, param, ldev, *value_p));
    } else {
	TRC_MSG(BT_TRC_CTRL, 
		(LOG_FMT "Error getting value of parameter %d for device %d\n",
		 LOG_ARG, param, ldev));
    }

exit_btk_set_info:
    FEXIT(retval);
    return(retval);
}


/******************************************************************************
**
**      Function:   set_lm_swap()
**
**      Purpose:    Initializes the local memory buffer mreg swap value
**
**      Args:       unit_p      Pointer to the unit structure being updated
**
**      Returns:    None        void
**
**      Notes:      The swap value of the local memory device must be updated
**                  as soon as unit_p->swap_bits[BT_AXSLM] is changed.
**
******************************************************************************/

static void set_lm_swap(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_lm");
    LOG_UNIT(unit_p);

    unsigned int    need;
    unsigned int    start;
    unsigned int    inx;

    bt_data32_t     mreg_value; /* Value to program mapping register to */
    bt_data32_t     mreg_swap_value = 0;    /* New swap value aligned to mreg. */

    FENTRY;

#if defined (BT_NTDRIVER)
need = unit_p->lm_need;
start = unit_p->lm_start;
#elif defined (BT946)
need = unit_p->lm_need;
start = unit_p->lm_start;
#elif defined (BT1003)
need = unit_p->lm_need;
start = unit_p->lm_start;
#elif defined (BT965)
need = unit_p->lm_need;
start = unit_p->lm_start;
#endif

    /* 
    ** Set swap for all the mapping registers
    */
    if (unit_p->lm_size != 0) {

        /* Set mreg_swap_value to the new swap value. */
        /* mreg_swap_value = (unit_p->swap_bits[ldev] << BT_MREG_SWAP_SHIFT); */
        btk_setup_mreg(unit_p, BT_AXSLM, &mreg_swap_value, BT_OP_BIND);

        for (inx = 0; inx < need; inx++) {
            /*
            ** Handle the case where PAGE_SIZE > BT_PAGE_SIZE (size of each
            ** mapping register).
            */
            mreg_value = btk_get_mreg(unit_p, start + inx, BT_LMREG_CABLE_2_PCI);
	    /* The following statement is correct despite what gcc -Wall says.*/
            mreg_value = (mreg_value & BT_MREG_ADDR_MASK) | mreg_swap_value;
            btk_put_mreg(unit_p, start + inx, BT_LMREG_CABLE_2_PCI, mreg_value);
            btk_put_mreg(unit_p, start + inx, BT_LMREG_DMA_2_PCI, mreg_value);
        }
    }

    FEXIT(0);
    return;
}
