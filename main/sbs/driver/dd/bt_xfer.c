/******************************************************************************
**
**      Filename:       bt_xfer.c
**
**      Purpose:        Copies to and from bus memory.
**
**      Functions:      btk_pio_xfer(), btk_dma_xfer(), btk_hw_xfer(),
**                      btk_dma_stop(), btk_dma_pio(), btk_cas(), btk_tas(),
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
** Copyright (c) 2000-2005 by SBS Technologies, Inc.
**
** All Rights Reserved.
** License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ "__DATE__;
#endif  /* LINT */

/*
** Include files
*/
#include "btdd.h"

#if defined(__vxworks)
#include "sbs_bsp_api.h"
#endif /* defined(__vxworks) */

#if defined(__linux__)
#include <asm/io.h>
#endif /* defined(__linux__) */


/*
**  Given local addr & length => number of mreg bits required
*/
#define CALC_LEN_BITS(addr,length)	(((length) / BT_617_PAGE_SIZE) + \
  (((((addr) % BT_617_PAGE_SIZE) + ((length) % BT_617_PAGE_SIZE)) / BT_617_PAGE_SIZE) ? 1 : 0) + \
  (((((addr) % BT_617_PAGE_SIZE) + ((length) % BT_617_PAGE_SIZE)) % BT_617_PAGE_SIZE) ? 1 : 0))



/*****************************************************************************
**
** Unit Structure Assumptions
**   unit_p->csr_p              Pointer to the bt_pci_csr_t CSR registers
**   unit_p->mreg_p             Pointer to the base of mapping registers
**                                structure
**   unit_p->bt_status          Normal unit status info
**     BT_POWER                 Power or cable is off
**     BT_PCI2PCI               Remote card is PCI based
**     BT_NEXT_GEN              Card set is next gen design
**
**
** PIO_READ_64/32/16/8 and PIO_WRITE_64/32/16/8 macros must be defined.
**   These macros allow portability of the data moving routine without
**   calling the btp_get/put_d?() routines.  In most systems these
**   macros will do nothing special (SGI is an exception).
**
*****************************************************************************/
#define BT_LMREG_PCI_2_CABLE_OFFSET    (0x0)
#define BT_LMREG_CABLE_2_PCI_OFFSET    (0x8000)
#define BT_LMREG_DMA_2_PCI_OFFSET      (0xc000)
#define BT_RMREG_PCI_2_CABLE_OFFSET    (0x40000)
#define BT_RMREG_CABLE_2_PCI_OFFSET    (0x48000)
#define BT_RMREG_DMA_2_PCI_OFFSET      (0x4c000)

/*
** Internal functions
*/
static size_t btk_bcopy(bt_unit_t *unit_p, caddr_t src_p, caddr_t dst_p, size_t length, int data_width, int src_bus);
static void btk_dma_poll(bt_unit_t *unit_p, int xfer_length, 
            int data_width, int max_delay);
bt_error_t btk_pio_xfer(bt_unit_t *unit_p, bt_dev_t ldev, caddr_t laddr_p, 
            bt_data32_t ldev_addr, int *xfer_length_p, size_t xfer_dir);
#if defined(__linux__)
bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, 
            bt_data32_t raddr, int *xfer_length, int xfer_dir, int data_width);
#else
bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, 
            bt_data32_t raddr, int xfer_length, int xfer_dir, int data_width);
#endif
bt_error_t btk_hw_xfer(bt_unit_t *unit_p, bt_dev_t ldev, int xfer_dir, 
            bt_hw_xfer_t *hw_xfer_p);
void btk_dma_pio(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr_p, 
            bt_data32_t raddr, int *xfer_length_p, int *dma_flag_p, 
            int *data_width_p, unsigned int *start_p, unsigned int *need_p);
void btk_dma_stop(bt_unit_t *unit_p);
bt_error_t btk_cas(bt_unit_t *unit_p, bt_dev_t axs_type, bt_cas_t *cas_p);
bt_error_t btk_tas(bt_unit_t *unit_p, bt_dev_t axs_type, bt_tas_t *tas_p);
bt_error_t btk_ioreg(bt_unit_t *unit_p, bt_dev_t axs_type, 
            bt_ioaccess_t *ioreg_p);
void bt_print_bit_names( bt_unit_t *unit_p, bt_reg_t adapter_reg, 
            bt_data8_t byte_value);

/*
** External functions
*/
extern bt_error_t btk_pagecheck(bt_unit_t *unit_p, btk_page_t *page_p);
extern bt_error_t btk_setpage(bt_unit_t *unit_p, bt_dev_t ldev, 
            bt_data32_t ldev_addr, btk_page_t *page_p);
extern void btk_dma_watchdog(caddr_t arg_p);
extern void btk_give_drv_sema(bt_unit_t *unit_p);
extern bt_error_t btk_take_drv_sema(bt_unit_t *unit_p);
extern void btp_put_d8(bt_unit_t *unit_p, void *addr_p, bt_data8_t value);
extern void btp_put_d16(bt_unit_t *unit_p, void *addr_p, bt_data16_t value);
extern void btp_put_d32(bt_unit_t *unit_p, void *addr_p, bt_data32_t value);
extern bt_data32_t btk_get_io( bt_unit_t *unit_p, bt_reg_t reg);
extern void btk_put_io( bt_unit_t *unit_p, bt_reg_t reg, bt_data32_t value);
extern void btk_put_mreg_range( bt_unit_t *unit_p, unsigned int mr_start, 
             unsigned int num_mregs, bt_mreg_t mreg_type, bt_data32_t value);
extern void btk_setup_mreg(bt_unit_t *unit_p, bt_dev_t ldev, 
             bt_data32_t *mreg_value_p, bt_operation_t op);
extern bool_t btk_invalid_op(bt_unit_t *unit_p, bt_dev_t type, 
             bt_operation_t op);
extern bool_t btk_invalid_offset(bt_unit_t *unit_p, bt_dev_t type, 
             bt_data64_t offset, bt_data32_t length);
extern void  bt_print_bit_names( bt_unit_t *unit_p, 
             bt_reg_t adapter_reg, bt_data8_t byte_value);

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_XFER_C);


/******************************************************************************
**
**      Function:       btk_pio_xfer()
**
**      Purpose:        Do memory-mapped I/O data transfer for device driver.
**                      The buffer address must already be locked in memory.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      ldev            Logical device number
**                      laddr_p         Local address to transfer to/from.
**                      ldev_addr       Adjusted lseek() offset to transfer to.
**                      xfer_length_p   Pointer to amount of data to transfer.
**                                      Returns actual amount transferred.
**                      xfer_dir        BT_READ  -> xfer from device to buffer.
**                                      BT_WRITE -> xfer from buffer to device.
**
**      Modifies:       *xfer_length_p  Stores actual amount transferred.
**
**      Returns:        BT_SUCCESS      Success.
**                      Otherwise       Error number indicating failure type.
**
**      Notes:
**              The length field is read to see how much should be transferred,
**              and then set to the amount to actually transferred.
**
******************************************************************************/

bt_error_t btk_pio_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    caddr_t laddr_p, 
    bt_data32_t ldev_addr, 
    int *xfer_length_p, 
    size_t xfer_dir)

{
    bt_error_t        retval = BT_SUCCESS;
    size_t            data_left = *xfer_length_p;
    caddr_t           kern_p = laddr_p;
    size_t            data_xferred = 0;
    size_t            data_copied, length;
    int               data_width = (int) unit_p->data_size[ldev];
    caddr_t           bus_p;
    btk_page_t        page_info;
    BTK_LOCK_RETURN_T isr_pl;
    
    FUNCTION("btk_pio_xfer");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;
    
    TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
           (LOG_FMT "offset = 0x%x; dir = %c; length = 0x%x\n", 
           LOG_ARG, ldev_addr, (xfer_dir == BT_READ) ? 'R' : 'W', (int) data_left));

    /*
    ** Define default data width for PIO based on adapter type
    */  
    if (data_width == BT_WIDTH_ANY) {
#if defined(BT1003)
        data_width = BT_WIDTH_D32;
#else /* defined(BT1003) */
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)
	    && (unit_p->rem_id != BT_PN_VME_DB))  /* when talking to a VME board */
                                                  /* NEXT_GEN card, D64 transfers */
                                                  /* aren't split correctly. This */
                                                  /* is a WORKAROUND. */
	{
            data_width = BT_WIDTH_D64;
        } else {
            data_width = BT_WIDTH_D32;
        }
#endif /* defined(BT1003) */    
    }

    /* 
    ** Setup paging info
    */
    retval = btk_setpage(unit_p, ldev, ldev_addr, &page_info);
    if (retval != BT_SUCCESS) {
        INFO_STR("Unable to setup page for PIO read/write");
        goto btk_pio_exit;
    }

#if defined(BT1003)
#if  EAS_A64_CODE
/* EAS A64 */
    /* Setup A64 mode and offset if using BT_PCI_DEVICE_RPQ601332 */
    if (unit_p->rem_id == BT_PN_PCI_DBA64_RPQ) {
        DBG_MSG((BT_TRC_PIO), 
                (LOG_FMT "PIO A64 offset = 0x%x\n", 
                LOG_ARG, unit_p->a64_offset));
        /* The remote board supports A64, program it */
        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, unit_p->a64_offset);
        if ( (btk_get_io(unit_p, BT_RPQ_REM_A64PIO)) != unit_p->a64_offset ) {
            WARN_STR("Verify Write BT_RPQ_REM_A64PIO register failed.\n");
            if (page_info.mreg_need != 0) {
                btk_mutex_enter(&unit_p->mreg_mutex);
                btk_bit_free(unit_p->mmap_aval_p, page_info.mreg_start, page_info.mreg_need);
                btk_mutex_exit(&unit_p->mreg_mutex);
            }
            retval = BT_EIO;
            goto btk_pio_exit;
        }
    }
#endif  /* EAS_A64_CODE */
#endif /* defined(BT1003) */

    /*
    ** Grab the read/write lock to indicate we are doing a PIO
    */
    btk_rwlock_rd_enter(&unit_p->hw_rwlock);

    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count++;
    BTK_UNLOCK_ISR(unit_p, isr_pl);

    /* 
    ** While we still have data left to transfer 
    */
    while (data_left > 0) {

        /* 
        ** Determine length to tranfer during this iteration 
        */
        length = MIN(page_info.page_size - page_info.page_offset, data_left);

        DBG_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
                (LOG_FMT "PIO offset = 0x%x length = 0x%x\n", 
                LOG_ARG, page_info.page_offset, length));


        /* 
        ** Set up pointer to the device 
        */
        bus_p = page_info.bus_base_p + page_info.page_offset;

        /* 
        ** Now it's time to actually move some data
        **
        ** A PIO transfer type
        */
#if !defined(BT993)
        if (xfer_dir == BT_READ) {
            data_copied = btk_bcopy(unit_p, bus_p, kern_p, length, data_width,
                TRUE);
        } else {
            data_copied = btk_bcopy(unit_p, kern_p, bus_p, length, data_width,
                FALSE);
        }
        if (data_copied != length) {
            INFO_STR("PIO error detected during transfer");
            retval = BT_EIO;

            /* update count with partial xfer */
            data_xferred += data_copied;
            break;
        } 

#else /* !defined(BT993) */
        if (xfer_dir == BT_READ) {
            sbs_pci_mem_read_from_device(kern_p, bus_p, (int) length);
        } else {
            sbs_pci_mem_write_to_device(bus_p, kern_p,  (int) length); 
        /*  WARN_MSG((LOG_FMT "kern_p 0x%x bus_p 0x%x\n", 
                      LOG_ARG, kern_p,bus_p));   CCC */
        }
        if (IS_SET(unit_p->bt_status, BT_ERROR)) {
            INFO_STR("PIO error detected during transfer");
            retval = BT_EIO;

            /* someday it would be nice to fix the 993 to work like the rest
               so we can update the count with partial xfer */
            /* data_xferred += ????? EAS - TBD */
            break;
        } else {
            data_copied = length;
        }
#endif /* !defined(BT993) */
        

        /*
        ** Update pointers
        */
        kern_p += data_copied;
        data_left -= data_copied;
        data_xferred += data_copied;
        page_info.page_offset += data_copied;

        /* 
        ** Check if transfer caused us to cross a page boundary 
        */
        if (data_left > 0) {
            retval = btk_pagecheck(unit_p, &page_info);
            if( BT_SUCCESS != retval ) break;
        }

    }


    /*
    ** We want to determine whether this transfer is successful
    ** from a hardware perspective.  So we need to read the
    ** status register since we can not garuentee that ISR
    ** has started, but if we read the status register manually
    ** we will either detect the error or the isr will have fired
    ** and set the BT_ERROR bit
    */
    if( (btk_get_io(unit_p, BT_LOC_STATUS) & LSR_CERROR_MASK)
                     || (IS_SET(unit_p->bt_status, BT_ERROR)) ) {
        INFO_STR("Error detected during PIO transfer");
        retval = BT_ESTATUS;
    }

    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count--;
    BTK_UNLOCK_ISR(unit_p, isr_pl);

    btk_rwlock_rd_exit(&unit_p->hw_rwlock);

    /*
    ** If we had an adapter error we need to clear it
    */
    if (retval == BT_ESTATUS) {
        btk_rwlock_wr_enter(&unit_p->hw_rwlock);
        if (IS_SET(unit_p->bt_status, BT_ERROR)) {
            CLR_BIT(unit_p->bt_status, BT_ERROR);
        }
        btk_rwlock_wr_exit(&unit_p->hw_rwlock);
    }

    /* 
    ** Release map regs allocated in btk_setpage()
    */
    if (page_info.mreg_need != 0) {
        btk_mutex_enter(&unit_p->mreg_mutex);
        btk_bit_free(unit_p->mmap_aval_p, page_info.mreg_start, page_info.mreg_need);
        btk_mutex_exit(&unit_p->mreg_mutex);
    }
    
btk_pio_exit:
#if defined(EASDEBUG)
    if (*xfer_length_p != (int)data_xferred) {
        INFO_MSG((LOG_FMT "Partial PIO transfer; expected 0x%x transferred 0x%x\n",
            LOG_ARG, *xfer_length_p, data_xferred));
    } else {
        INFO_MSG((LOG_FMT "Full PIO transfer; expected 0x%x transferred 0x%x\n",
            LOG_ARG, *xfer_length_p, data_xferred));
    }
#endif
    *xfer_length_p = (int) data_xferred;
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_dma_xfer()
**
**      Purpose:        Do a DMA transfer.
**                      The buffer address must already be locked in memory 
**                      and mapping RAM allocated and programmed
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      ldev            Logical device number
**                      laddr           Local address to transfer to/from.
**                      raddr           Remote address to transfer to/from.
**                      xfer_length     Amount of data to transfer.
**                      xfer_dir        BT_READ  -> xfer from device to buffer.
**                                      BT_WRITE -> xfer from buffer to device.
**
**      Modifies:       None
**
**      Returns:        BT_SUCCESS      Success.
**                      Otherwise       Error number indicating failure type.
**
**      Notes:          Length and addresses are not checked for aligment
**                      requirements and no mapping regs are allocated
**                      or programmed.
**
******************************************************************************/

#if defined(__linux__)
/* EAS - Modified 1003 version (ONLY) to return a closer count of number of bytes read for 2nd level fix.
        Normally calling routine returns number of bytes transferred +/- 64k, customer needed this returned
        closer than this.
        Note hardware will fail reads on packet basis of 256 bytes, so can't get the exact byte/word/dword
        failure.  Note that writes do not fail consistently the failure may occur several packets past.
*/
bt_error_t btk_dma_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    bt_data32_t laddr, 
    bt_data32_t raddr, 
    int *xfer_len,          /* return partial transfer count on reads, zero on writes */
    int xfer_dir,
    int data_width)
{
    bt_data32_t laddr_end; 
    int xfer_length = *xfer_len;
#else
bt_error_t btk_dma_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    bt_data32_t laddr, 
    bt_data32_t raddr, 
    int xfer_len, 
    int xfer_dir,
    int data_width)
{
    int xfer_length = xfer_len;
#endif

    bt_error_t          retval = BT_SUCCESS;
    bt_data8_t          dma_cmd, tmp_reg, amod;
    int                 polled_dma = TRUE;
    int                 timeout_usec;
    static unsigned int fail_count = 0;

    FUNCTION("btk_dma_xfer");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;
    
    TRC_MSG((BT_TRC_DMA | BT_TRC_DETAIL), 
           (LOG_FMT "laddr 0x%x; raddr 0x%x; len 0x%x; dir = %c; size %d\n", 
           LOG_ARG, laddr, raddr, xfer_length, 
           (xfer_dir == BT_READ) ? 'R' : 'W', data_width));

    dma_cmd = btk_get_io(unit_p, BT_LDMA_CMD);
    if ( IS_SET(dma_cmd, LDC_START) ) {
        fail_count++;
        retval = BT_EFAIL;
        goto dma_xfer_exit;
    }
    if ( fail_count ) {
        WARN_MSG((LOG_FMT "Start of DMA transfer was ABORTED %.1u times.\n", LOG_ARG, fail_count));
        fail_count = 0;
    }

    /*
    ** Clear the error bit for DMA and indicate that one is active
    */
    CLR_BIT(unit_p->bt_status, BT_DMA_ERROR);

    /*
    **  Setup the DMA command register
    */
    dma_cmd = LDC_DMA_D32_SEL;
    if ((data_width == BT_WIDTH_D64) || 
        (data_width == BT_WIDTH_D16)) { 
        dma_cmd = 0;
    }
    if (xfer_dir == BT_WRITE) {  
        SET_BIT(dma_cmd, LDC_WRITE_SEL);
    }
    if (ldev == BT_AXSDP) {
        SET_BIT(dma_cmd, LDC_DP_SEL);
    }
    if (xfer_length > unit_p->dma_poll_size) {
        polled_dma = FALSE;
        SET_BIT(dma_cmd, LDC_DMA_INT_ENABLE);
    }
    btk_put_io(unit_p, BT_LDMA_CMD, dma_cmd);
    if ( (btk_get_io(unit_p, BT_LDMA_CMD)) != dma_cmd ) {
        WARN_STR("Verify Write BT_LDMA_CMD register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }

    /*
    **  Setup the remote address reg. & remote partial packet reg.
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, raddr);
    if ( (btk_get_io(unit_p, BT_RDMA_ADDR)) != raddr ) {
        WARN_STR("Verify Write BT_RDMA_ADDR register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }
    btk_put_io(unit_p, BT_RDMA_RMD_CNT, xfer_length & (DMA_PKT_SIZE - 1));
    if ( (btk_get_io(unit_p, BT_RDMA_RMD_CNT)) != (xfer_length & (DMA_PKT_SIZE - 1)) ) {
        WARN_STR("Verify Write BT_RDMA_RMD_CNT register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }
#if defined(BT1003)
#if  EAS_A64_CODE
/* EAS A64 */
    /* Setup A64 mode and offset if using BT_PCI_DEVICE_RPQ601332 */
    if (unit_p->rem_id == BT_PN_PCI_DBA64_RPQ) {
        DBG_MSG((BT_TRC_PIO), 
                (LOG_FMT "DMA A64 offset = 0x%x\n", 
                LOG_ARG, unit_p->a64_offset));
        /* The remote board supports A64, program it */
        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, unit_p->a64_offset);
        if ( (btk_get_io(unit_p, BT_RPQ_REM_A64DMA)) != unit_p->a64_offset ) {
            WARN_STR("Verify Write BT_RPQ_REM_A64DMA register failed.\n");
            retval = BT_EIO;
            goto dma_xfer_exit;
        }
    }
#endif  /* EAS_A64_CODE */
#endif /* defined(BT1003) */

    /*
    **  Setup remote command register 2 and remote amod register
    **  Only on PCI to VME (Multibus and Q-bus are PIO only adapters)
    */
    if (IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
        tmp_reg = RC2_CINT_DISABLE;  /* Disable cable irqs */
        amod = (bt_data8_t)unit_p->dma_addr_mod[ldev];
        if (data_width == BT_WIDTH_D64) {
            SET_BIT(tmp_reg, RC2_DMA_BLK_SEL);
            if (((amod & 0xf) != BT_AMOD_A32_SMBLT) &&
                ((amod & 0xf) != BT_AMOD_A32_NMBLT)) {
                amod = (amod & 0xf0) | BT_AMOD_A32_SMBLT;
            }
        } else if (((amod & 0xf) == BT_AMOD_A32_SB) ||
                   ((amod & 0xf) == BT_AMOD_A32_NB)) {
            SET_BIT(tmp_reg, RC2_DMA_BLK_SEL);
        } else if (IS_SET(unit_p->bt_status, BT_DMA_BLOCK)) {
            if (ldev == BT_DEV_A24) {
                amod = BT_AMOD_A24_SB;
            } else {
                amod = BT_AMOD_A32_SB;
            }
            SET_BIT(tmp_reg, RC2_DMA_BLK_SEL);
        }
        if (IS_SET(unit_p->bt_status, BT_DMA_WAIT)) {    /* Pause mode */
            SET_BIT(tmp_reg, RC2_DMA_PAUSE);
        }
        btk_put_io(unit_p, BT_REM_CMD2, tmp_reg);
        if ( (btk_get_io(unit_p, BT_REM_CMD2)) != tmp_reg ) {
            WARN_STR("Verify Write BT_REM_CMD2 register failed.\n");
            retval = BT_EIO;
            goto dma_xfer_exit;
        }
        btk_put_io(unit_p, BT_REM_AMOD, amod);
        if ( (btk_get_io(unit_p, BT_REM_AMOD)) != amod ) {
            WARN_STR("Verify Write BT_REM_AMOD register failed.\n");
            retval = BT_EIO;
            goto dma_xfer_exit;
        }
        DBG_MSG((BT_TRC_DMA | BT_TRC_DETAIL), 
                (LOG_FMT "DMA: rc2 0x%x; amod 0x%x\n", 
                LOG_ARG, tmp_reg, amod));
    }

    /*
    ** Setup the local DMA address register
    ** Always start from map reg 0, so upper bits of address
    ** are always zero
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, laddr);
    if ( (btk_get_io(unit_p, BT_LDMA_ADDR)) != laddr ) {
        WARN_STR("Verify Write BT_LDMA_ADDR register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }

#if defined(EASDEBUG)
        INFO_MSG((LOG_FMT "Read DMA transfer Start; laddr 0x%08x\n",
                LOG_ARG, laddr));
#endif
    /* 
    ** Setup the local packet and partial packet count
    */
    btk_put_io(unit_p, BT_LDMA_PKT_CNT, (xfer_length >> 8));
    if ( (btk_get_io(unit_p, BT_LDMA_PKT_CNT)) != (xfer_length >> 8) ) {
        WARN_STR("Verify Write BT_LDMA_PKT_CNT register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }
    btk_put_io(unit_p, BT_LDMA_RMD_CNT, xfer_length & (DMA_PKT_SIZE - 1));
    if ( (btk_get_io(unit_p, BT_LDMA_RMD_CNT)) != (xfer_length & (DMA_PKT_SIZE - 1)) ) {
        WARN_STR("Verify Write BT_LDMA_RMD_CNT register failed.\n");
        retval = BT_EIO;
        goto dma_xfer_exit;
    }

    /*
    ** Check for errors before we actually start the DMA
    */
    if (IS_SET(unit_p->bt_status, BT_DMA_ERROR)) {
        btk_put_io(unit_p,BT_LDMA_CMD , 0); /* ??? not sure why this is here */
        btk_put_io(unit_p, BT_REM_CMD2, 0);
		INFO_STR("Error detected during DMA setup.\n");
        retval = BT_ESTATUS;
        goto dma_xfer_exit;
    }
    /* 
    ** Check for a legitimate DMA watchdog timer value.
    ** Otherwise use 5 seconds
    */
    timeout_usec = unit_p->dma_timeout * 10000;
    if (timeout_usec < 1000) {
        INFO_MSG((LOG_FMT "Invalid DMA timeout of %d\n",
                LOG_ARG, timeout_usec));
        timeout_usec = 5000000;
    }

    /*
    ** Wait for interrupt processing to stop
    */
    while (IS_SET(btk_get_io(unit_p, BT_LOC_INT_CTRL), 
                  LIC_INT_PENDING)) {
        ; /* empty while loop */
    }
  
    /* 
    ** Start DMA watchdog timer
    */
    if (!polled_dma) {
#if defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__)
        /*
        ** We do not use watchdog function for NT or VxWorks instead
        ** use timeout feature of btk_event_wait()
        */
        /* Make sure the dma event semaphore is down */
        btk_event_wait(&unit_p->dma_event, BT_NO_WAIT);
#else /* defined(BT_NTDRIVER || __vxworks) || defined(__sun) || defined(__linux__) */
        unit_p->watchdog_id = btk_timeout(btk_dma_watchdog, (void *) unit_p, 
                                         (long) timeout_usec);
#endif /* defined(BT_NTDRIVER || __vxworks) || defined(__sun) || defined(__linux__) */
    }
  
    /* 
    ** Now actually kickoff the DMA master operation 
    */
    SET_BIT(unit_p->bt_status, BT_DMA_ACTIVE);
    btk_put_io(unit_p, BT_LDMA_CMD, dma_cmd | LDC_START);

    /*
    **  Now that the DMA has been started, determine whether
    **  it's cheaper to go to sleep or to spin on completion.
    */
    if (!polled_dma) {

        /* 
        ** It's cheaper to sleep awaiting DMA done interrupt
        ** ISR will cancel the timeout function
        */
#if defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__)
        /*
        ** We do not use watchdog function to set event for NT or VxWorks
        ** instead use timeout feature of btk_event_wait()
        */

        /* Wait for DMA to complete. */
        retval = btk_event_wait(&unit_p->dma_event, btk_msec2tck(timeout_usec/1000));
        if ( BT_SUCCESS != retval ) {
            if ( BT_EABORT == retval ) {
                INFO_STR("DMA transfer was user-interrupted.\n");
            }
            else if ( BT_EBUSY == retval ) {
                INFO_STR("DMA transfer timed out waiting for completion.\n");
            }
            btk_dma_watchdog((caddr_t) unit_p); /* stop the DMA, and set the DMA error flag. */
            /* Try to down the dma event semaphore in case that the ISR upped it since timed out or aborted. */
            btk_event_wait(&unit_p->dma_event, BT_NO_WAIT);
        }
        
#else /* defined(BT_NTDRIVER || __vxworks ) || defined(__sun) || defined(__linux__) */

        /* Wait for DMA to complete. */
        btk_event_wait(&unit_p->dma_event, BT_FOREVER);

#endif /* defined(BT_NTDRIVER || __vxworks ) || defined(__sun) || defined(__linux__) */

    /*
    ** Its cheaper to spin on completion so re-enable interrupts
    ** and loop, periodically checking for the DMA's completion.
    */
    } else {

        /* 
        **  Poll for the DMA's completion based on delay time 
        */
        btk_dma_poll(unit_p, xfer_length, data_width, timeout_usec);

    }

    if ( IS_SET(btk_get_io(unit_p, BT_LDMA_CMD), LDC_START) ) {
        WARN_STR("LDC_START bit in BT_LDMA_CMD register still set when DMA transfer should have stopped!\n");
    }
    else {
        btk_put_io(unit_p,BT_LDMA_CMD , 0);     /* ??? not sure why this is here */
        btk_put_io(unit_p, BT_REM_CMD2, 0);
    }

    /*
    ** We want to determine whether this transfer is successful
    ** from a hardware perspective.  So we need to read the
    ** status register since we can not guarantee that ISR
    ** has started, but if we read the status register manually
    ** we will either detect the error or the isr will have fired
    ** and set the BT_DMA_ERROR bit
    */
    if( (btk_get_io(unit_p, BT_LOC_STATUS) & LSR_CERROR_MASK)
                     || (IS_SET(unit_p->bt_status, BT_DMA_ERROR)) ) {
        INFO_STR("Error detected during DMA transfer");
        retval = BT_ESTATUS;
    }

#if defined(__linux__)
    /* 1003 - 2nd level - return partial transfer count on reads from DMA register */
    if (BT_WRITE == xfer_dir) {
        if (retval == BT_SUCCESS) {
	        xfer_length = 0;
	    }
    }
    else {
        /* Reads - update transfer count from ldma register to indicate number of bytes actually xferred */
        laddr_end = btk_get_io(unit_p, BT_LDMA_ADDR);
#if defined(EASDEBUG)
        INFO_MSG((LOG_FMT "Read DMA transfer Done; laddr_end 0x%08x\n",
                LOG_ARG, laddr_end));
        if ((xfer_length = ((laddr + xfer_length) - laddr_end)) != 0) {
            INFO_MSG((LOG_FMT "Partial Read DMA transfer; expected 0x%x transferred 0x%x\n",
                LOG_ARG, *xfer_len, (*xfer_len - xfer_length)));
        } else {
            INFO_MSG((LOG_FMT "Full Read DMA transfer; expected 0x%x transferred 0x%x\n",
                LOG_ARG, *xfer_len, (*xfer_len - xfer_length)));
        }
#else
        xfer_length = ((laddr + xfer_length) - laddr_end);
#endif
    }

dma_xfer_exit:
        *xfer_len -= xfer_length;
#else

dma_xfer_exit:
#endif

    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_hw_xfer()
**
**      Purpose:        Do a transfer to a PCI bus address.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      ldev            Logical device number
**                      xfer_dir        BT_READ  -> xfer from device to buffer.
**                                      BT_WRITE -> xfer from buffer to device.
**                      hw_xfer_p       Pointer to ioctl structure.
**
**      Modifies:       None
**
**      Returns:        BT_SUCCESS      Success.
**                      Otherwise       Error number indicating failure type.
**
**      Notes:          PIO is not supported and will return error
**
******************************************************************************/

bt_error_t btk_hw_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    int xfer_dir,
    bt_hw_xfer_t *hw_xfer_p)

{
    bt_error_t          retval = BT_SUCCESS;
    bt_data32_t         curr_laddr = hw_xfer_p->local_hw_addr;
    bt_data32_t         curr_raddr = hw_xfer_p->ldev_addr;
    bt_data32_t         dma_addr;
    int                 total_length = hw_xfer_p->xfer_len;
    int                 curr_length = hw_xfer_p->xfer_len;
    int                 dma_flag;
    int                 data_width;
    unsigned int        mreg_need; 
    unsigned int        mreg_start;
    bt_data32_t         mreg_value;

    FUNCTION("btk_hw_xfer");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;
    
    TRC_MSG((BT_TRC_DMA | BT_TRC_DETAIL), 
           (LOG_FMT "HW DMA: laddr 0x%x; raddr 0x%x; len 0x%x; dir = %c\n", 
           LOG_ARG, curr_laddr, curr_raddr, curr_length, 
           (xfer_dir == BT_READ) ? 'R' : 'W'));

    /* 
    ** Adjust for remote extended device
    */
    if (ldev == BT_AXSRE) {
        curr_laddr |= RE_ADJUST;
    }

    /*
    ** Check offset and length
    */
    if (btk_invalid_offset(unit_p, ldev, curr_raddr, total_length)) {
        INFO_STR("Illegal offset and/or length for hardware transfer");
        retval = BT_EACCESS;
        goto hw_xfer_exit;
    }
        
    
    /* 
    **  While there is still data to transfer 
    */
    while (total_length > 0) {

        /*
        ** Reset the local length values
        */
        curr_length = total_length;

        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            btk_dma_pio(unit_p, ldev, curr_laddr, curr_raddr, &curr_length, &dma_flag, &data_width, NULL, &mreg_need);
        } else {
            btk_dma_pio(unit_p, ldev, curr_laddr, curr_raddr, &curr_length, &dma_flag, &data_width, &mreg_start, &mreg_need);
        }

        /*
        ** Fail out if we can not do DMA, PIO not implemented yet
        */
        if (dma_flag != TRUE) {
            retval = BT_ENOSUP;
            goto hw_xfer_exit;
        }

        /*
        ** Now we need to get the DMA semaphore 
        ** Note this routines does nothing in a single driver situtation
        */
        retval = btk_take_drv_sema(unit_p);
        if (retval != BT_SUCCESS) {
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                btk_mutex_enter(&unit_p->mreg_mutex);
                btk_bit_free(unit_p->sdma_aval_p, mreg_start, mreg_need);
                btk_mutex_exit(&unit_p->mreg_mutex);
            }
            btk_mutex_exit(&unit_p->dma_mutex);
            goto hw_xfer_exit;
        }

        /*
        ** Program the mapping RAM if needed, otherwise set the bypass bit
        */
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            mreg_value = curr_laddr & BT_MREG_ADDR_MASK;
            btk_setup_mreg(unit_p, ldev, &mreg_value, BT_OP_DMA);
            {
                unsigned int inx;
                for (inx = mreg_start; inx < (mreg_start + mreg_need); inx++, mreg_value += BT_PAGE_SIZE ) {
                    btk_put_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI, mreg_value);
                    if ( (btk_get_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI)) != mreg_value ) {
                        WARN_MSG((LOG_FMT "Verify Write BT_LMREG_DMA_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                                         LOG_ARG, inx));
                        btk_mutex_enter(&unit_p->mreg_mutex);
                        btk_bit_free(unit_p->sdma_aval_p, mreg_start, mreg_need);
                        btk_mutex_exit(&unit_p->mreg_mutex);
                        btk_mutex_exit(&unit_p->dma_mutex);
                        retval = BT_EIO;
                        goto hw_xfer_exit;
                    }
                }
            }
        } else {
            mreg_value = btk_get_io(unit_p, BT_LOC_MREG_CTRL);
            SET_BIT(mreg_value, LMC_DMA_BYPASS_ENABLE);
            btk_put_io(unit_p, BT_LOC_MREG_CTRL, mreg_value);
            if ( (btk_get_io(unit_p, BT_LOC_MREG_CTRL)) != mreg_value ) {
                CLR_BIT(unit_p->bt_status, BT_DMA_AVAIL);
                WARN_STR("Verify Write BT_LOC_MREG_CTRL reg failed! DMA transfer capability is now disabled.\n");
                btk_mutex_exit(&unit_p->dma_mutex);
                retval = BT_EIO;
                goto hw_xfer_exit;
            }
        }

        /*
        ** If old Nanobus card, we must stop PIO from occuring
        */
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            btk_rwlock_wr_enter(&unit_p->hw_rwlock);
        }
        
        /*
        ** Do the DMA
        */
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            dma_addr = mreg_start * BT_PAGE_SIZE + (curr_laddr & ~BT_MREG_ADDR_MASK);
#if defined(__linux__)
            retval = btk_dma_xfer(unit_p, ldev, dma_addr, curr_raddr, &curr_length, xfer_dir, data_width);
#else
            retval = btk_dma_xfer(unit_p, ldev, dma_addr, curr_raddr, curr_length, xfer_dir, data_width);
#endif
            btk_rwlock_wr_exit(&unit_p->hw_rwlock);
        } else {
#if defined(__linux__)
            retval = btk_dma_xfer(unit_p, ldev, curr_laddr, curr_raddr, &curr_length, xfer_dir, data_width);
#else
            retval = btk_dma_xfer(unit_p, ldev, curr_laddr, curr_raddr, curr_length, xfer_dir, data_width);
#endif
        }

        /*
        ** Release locks and mapping registers
        */
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            btk_mutex_enter(&unit_p->mreg_mutex);
            btk_bit_free(unit_p->sdma_aval_p, mreg_start, mreg_need);
            btk_mutex_exit(&unit_p->mreg_mutex);
        } else {
            mreg_value = btk_get_io(unit_p, BT_LOC_MREG_CTRL);
            CLR_BIT(mreg_value, LMC_DMA_BYPASS_ENABLE);
            btk_put_io(unit_p, BT_LOC_MREG_CTRL, mreg_value);
            if ( (btk_get_io(unit_p, BT_LOC_MREG_CTRL)) != mreg_value ) {
                CLR_BIT(unit_p->bt_status, BT_DMA_AVAIL);
                WARN_STR("Verify Write BT_LOC_MREG_CTRL reg failed! DMA transfer capability is now disabled.\n");
                retval = BT_EIO;
            }
        }
        btk_give_drv_sema(unit_p);
        btk_mutex_exit(&unit_p->dma_mutex);
  
        /*
        ** A DMA operation has been completed, check for errors
        */
        if (retval != BT_SUCCESS) {
            /* INFO message should be printed by failing routine */

#if defined(__linux__)
            /* update for partial transfer */
            total_length    -= curr_length;
#endif
            break;

        /* 
        **  Else transfer ok, update while variables and continue
        */
        } else {
            curr_raddr      += curr_length;
            curr_laddr      += curr_length;
            total_length    -= curr_length;
        }
    } /* while total_length > 0 */

hw_xfer_exit:
    hw_xfer_p->xfer_len -= total_length;
    FEXIT(retval);
    return(retval);
}


/******************************************************************************
**
**      Function:       btk_dma_stop()
**
**      Purpose:        Stops a DMA.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**
**      Modifies:       None
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/

void btk_dma_stop(
    bt_unit_t *unit_p)

{

    bt_data8_t         dma_cmd, tmp_reg;
    int                loop;
    
    FUNCTION("btk_dma_stop");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    TRC_STR((BT_TRC_DMA|BT_TRC_DETAIL), "DMA will be stopped abnormally");

    /*
    ** Set the DMA error flag
    ** Clear out the DMA irq and start bits to shutdown the DMA
    */
    SET_BIT(unit_p->bt_status, BT_DMA_ERROR);
    dma_cmd = btk_get_io(unit_p, BT_LDMA_CMD);
    if ( IS_CLR(dma_cmd, LDC_START) ) {
        WARN_STR("Stopping DMA. Start bit in BT_LDMA_CMD register already cleared!\n");
    }
    CLR_BIT(dma_cmd, LDC_START);
    CLR_BIT(dma_cmd, LDC_DMA_INT_ENABLE);
    
    /*
    **  Wait for the DMA start bit to be cleared
    */
    btk_put_io(unit_p, BT_LDMA_CMD, dma_cmd);
    btk_delay(5);
    for (loop = 0; loop < 1000000; loop++) {
        tmp_reg = btk_get_io(unit_p, BT_LDMA_CMD);
        if ( (IS_CLR( tmp_reg, LDC_START )) && (IS_CLR( tmp_reg, LDC_DMA_INT_ENABLE )) ) {
            break;
        }
        /* It is important to shut off the DMA irq */
        if ( IS_SET( tmp_reg, LDC_DMA_INT_ENABLE ) ) { /* If the write obviously failed, */
            WARN_STR("Failed Write to BT_LDMA_CMD register -- did not clear LDC_DMA_INT_ENABLE bit.\n");
            btk_put_io(unit_p, BT_LDMA_CMD, dma_cmd);
        }
        btk_delay(5);
    }

    /*
    ** Print a warning if the DMA transfer can not be stopped.
    ** Disable DMA transfer capability and print a warning if DMA Done interrupts can not be disabled.
    */
    {
        tmp_reg = btk_get_io(unit_p, BT_LDMA_CMD);
        /*
        ** Only give a warning if DMA is still active.
        ** Checking the LDC_START bit in the BT_LDMA_CMD register at the start of
        ** btk_dma_xfer() prevents starting of the next DMA transfer, if this one is still active.
        */
        if( IS_SET( tmp_reg, LDC_START ) ) {
            WARN_STR("Could not stop DMA transfer!\n");
        }
        /*
        ** Disable DMA and allow PIO transfers only, if disabling of DMA done interrupts failed.
        ** This is BAD -- the ISR will disable ALL interrupts if the DMA done interrupt still occurs.
        ** This will never happen, because up to 200 retries can be attempted (above).
        */
        if( IS_SET( tmp_reg, LDC_DMA_INT_ENABLE ) ) {
            CLR_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            WARN_STR("Could not clear DMA done interrupt enable! DMA transfer capability is now disabled.\n");
        }
    }

    FEXIT(0);
    return;
}

/******************************************************************************
**
**      Function:       btk_dma_pio()
**
**      Purpose:        Determines whether the described transfer can be 
**                      via DMA.  If DMA is possible the dma_flag_p will
**                      be TRUE and the data_width_p, xfer_length_p,
**                      start_p & need_p will be set.  Mapping RAM
**                      are allocated in this routine for DMA but
**                      not for PIO.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      ldev            Logical device number
**                      laddr_p         Pointer to local buffer.
**                      raddr           Remote address..
**                      xfer_length_p   Pointer to xfer length.
**                      dma_flag_p      Pointer whether to do DMA
**                      data_width_p    Pointer to what data with to do
**                      start_p         Starting mapping RAM if DMA
**                                        NULL means don't allocate mreg for 
**                                        Data Blizzard.
**                      need_p          # of mapping RAM used if DMA
**                                        Must not be NULL
**
**      Modifies:       xfer_length_p   What xfer length to do
**                      dma_flag_p      Whether to do DMA
**                      data_width_p    What data with to do
**
**      Returns:        None
**
**      Notes:          No DMA or PIO locks should be held when calling
**                      this routine.  If DMA is allowed and mapping
**                      registers are allocated the the dma_mutex is
**                      obtained before returning from this call and 
**                      must be released in the calling routine's DMA
**                      section.
**
******************************************************************************/

void btk_dma_pio(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    bt_data32_t laddr, 
    bt_data32_t raddr, 
    int *xfer_length_p,
    int *dma_flag_p,
    int *data_width_p,
    unsigned int *start_p,
    unsigned int *need_p)

{
    bt_error_t              retval = BT_SUCCESS;
    int                     orig_length = *xfer_length_p;
    int                     orig_width = (int) unit_p->data_size[ldev];
    register int            data_width = (int) unit_p->data_size[ldev];
    unsigned int            mreg_aval;
    int                     line_number = 0;
    
    
    FUNCTION("btk_dma_pio");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    /*
    ** Seperate by logical device
    */
    switch (ldev) {
      case BT_AXSLM:
      case BT_AXSLDP:
      case BT_AXSIO:
      case BT_AXSRI:
        *dma_flag_p = FALSE;
        line_number = __LINE__;
        goto dma_pio_exit;
      case BT_AXSRDP:
	if (IS_SET(unit_p->bt_status,BT_NEXT_GEN)) {
	    if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
                *dma_flag_p = FALSE;
                line_number = __LINE__;
                goto dma_pio_exit;
	    } else {
/*              WARN_STR("non-PCI2PCI dataBLIZZARD will attempt RDP DMA!\n");
                WARN_MSG((LOG_FMT "o width %d d width %d\n", 
                                   LOG_ARG, orig_width,data_width)); */
	    }
	} else {
	    if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
		WARN_STR("PCI2PCI NanoBus adapter will attempt RDP DMA!\n");
	    } else {
                /* non-PCI2PCI NanoBus should do RDP DMA without comment. :0) */
	    }
        }
        break;

      default:
        break;
    }

    /* 
    ** Check unit paramters 
    */
    if (IS_CLR(unit_p->bt_status, BT_DMA_AVAIL) ||
        (*xfer_length_p < unit_p->dma_threshold)) {
        *data_width_p = orig_width;
        *xfer_length_p = orig_length;
        *dma_flag_p = FALSE;
        line_number = __LINE__;
        goto dma_pio_exit;
    }


    /*
    ** Fix what DMA size we are going to try to do
    */
    if (data_width == BT_WIDTH_ANY) {
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            data_width = BT_WIDTH_D64;
        } else {
            data_width = BT_WIDTH_D32;
        }
    }

    /*
    ** Check logical device parameters
    */
    if (IS_CLR(unit_p->logstat[ldev], STAT_DMA) ||
        (*xfer_length_p < data_width)) {
        *data_width_p = orig_width;
        *xfer_length_p = orig_length;
        *dma_flag_p = FALSE;
        line_number = __LINE__;
        goto dma_pio_exit;
    }

    /*
    ** Break each DMA width case seperately
    */
    switch (data_width) {
      case BT_WIDTH_D64:
        /*
        ** Check for perfect DMA alignment
        */
        if (((laddr % BT_WIDTH_D64) == 0) &&
            ((raddr % BT_WIDTH_D64) == 0)) {
            break;
        /*
        ** Check for same D64 alignment
        ** Do PIO to align to D64, next pass do DMA
        */
        } else if ((laddr % BT_WIDTH_D64) ==
                   (raddr % BT_WIDTH_D64)) {
            *data_width_p = orig_width;
            *xfer_length_p =  (int) (BT_WIDTH_D64 - (laddr % BT_WIDTH_D64));
            *dma_flag_p = FALSE;
            line_number = __LINE__;
            goto dma_pio_exit;
        }
        data_width = BT_WIDTH_D32;
        /*
        ** Intential fall through to lower data width case
        */
        
      case BT_WIDTH_D32:

        /*
        ** Check for perfect DMA alignment
        */
        if (((laddr % BT_WIDTH_D32) == 0) &&
            ((raddr % BT_WIDTH_D32) == 0)) {
            break;
        /*
        ** Check for same D32 alignment
        ** Do PIO to align to D32, next pass do DMA
        */
        } else if ((laddr % BT_WIDTH_D32) ==
                   (raddr % BT_WIDTH_D32)) {
            *data_width_p = orig_width;
            *xfer_length_p = (int) (BT_WIDTH_D32 - (laddr % BT_WIDTH_D32));
            *dma_flag_p = FALSE;
            line_number = __LINE__;
            goto dma_pio_exit;
        }
        data_width = BT_WIDTH_D16;
        /*
        ** Intential fall through to lower data width case
        */
        
      case BT_WIDTH_D16:
        /*
        ** dataBlizzard can only do D64 & D32 DMAs
        */
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            *data_width_p = orig_width;
            *xfer_length_p = orig_length;
            *dma_flag_p = FALSE;
            line_number = __LINE__;
            goto dma_pio_exit;
        }
        
        /*
        ** Check for perfect DMA alignment
        */
        if (((laddr % BT_WIDTH_D16) == 0) &&
            ((raddr % BT_WIDTH_D16) == 0)) {
            break;
        /*
        ** Check for same D16 alignment
        ** Do PIO to align to D16, next pass do DMA
        */
        } else if ((laddr % BT_WIDTH_D16) ==
                   (raddr % BT_WIDTH_D16)) {
            *data_width_p = orig_width;
            *xfer_length_p = (int) (BT_WIDTH_D16 - (laddr % BT_WIDTH_D16));
            *dma_flag_p = FALSE;
            line_number = __LINE__;
            goto dma_pio_exit;
        }
        data_width = BT_WIDTH_D8;
        /*
        ** Intential fall through to lower data width case
        */
        
      default:
      case BT_WIDTH_D8:
        /*
        ** Can't to DMA
        */
        *data_width_p = orig_width;
        *xfer_length_p = orig_length;
        *dma_flag_p = FALSE;
        line_number = __LINE__;
        goto dma_pio_exit;
    }

    /*
    ** DMA O.K., but need to adjust length so it is a mutliple
    ** of data_width 
    */
    *dma_flag_p = TRUE;
    *data_width_p = data_width;
    *xfer_length_p = orig_length & ~(data_width - 1);

    /*
    ** Check for the case of DMA to HW address where we do not
    ** need to allocate mapping RAM
    */
    if (start_p == NULL) {
        *need_p = 0;
        line_number = __LINE__;
        goto dma_pio_exit;
    }

    /*
    ** Here we are able to do DMA but need to get the mapping
    ** RAM
    */
    /*
    ** We want to stop other DMAs at this point because otherwise
    ** they may not be able to get enough mapping RAM eventhough
    ** the mapping RAM will be free up by the other DMA when
    ** it completes
    */
    btk_mutex_enter(&unit_p->dma_mutex);
    btk_mutex_enter(&unit_p->mreg_mutex);
#if defined(BT_NTDRIVER)
    *need_p = (unsigned int) CALC_LEN_BITS((bt_data32_t) laddr, *xfer_length_p);
#else /* BT_NTDRIVER) */
    *need_p = (unsigned int) CALC_LEN_BITS((bt_data64_t) laddr, *xfer_length_p);
#endif /* BT_NTDRIVER) */
    retval = btk_bit_alloc(unit_p->sdma_aval_p, *need_p, 1, start_p);
    if (retval == BT_ENOMEM) {
        /*
        ** Not enough mapping regs to cover entire DMA, find max
        ** Make DMA would be over threshold with the number of regs found
        */
        (void) btk_bit_max(unit_p->sdma_aval_p, 1, &mreg_aval);
        if ((((*need_p - mreg_aval) * BT_PAGE_SIZE) + unit_p->dma_threshold) > (unsigned int) (*xfer_length_p)) {
            TRC_STR(BT_TRC_RD_WR, "DMA mapping RAM too tight to make DMA worth it");
            btk_mutex_exit(&unit_p->mreg_mutex);
            btk_mutex_exit(&unit_p->dma_mutex);
            *data_width_p = orig_width;
            *xfer_length_p = orig_length;
            *dma_flag_p = FALSE;
            line_number = __LINE__;
            goto dma_pio_exit;

        /*
        ** Allocate mapping ram and adjust the length
        */
        } else {
            retval = btk_bit_alloc(unit_p->sdma_aval_p, mreg_aval, 1, start_p);
            btk_mutex_exit(&unit_p->mreg_mutex);
            if (retval != BT_SUCCESS) {
                btk_mutex_exit(&unit_p->dma_mutex);
                INFO_STR("Failed to alloc section of DMA map regs");
                *dma_flag_p = FALSE;
                *data_width_p = orig_width;
                *xfer_length_p = orig_length;
                line_number = __LINE__;
                goto dma_pio_exit;
            } else {
                *xfer_length_p -= (*need_p - mreg_aval) * BT_PAGE_SIZE;
                *need_p = mreg_aval;
            }
        }
    
    /*
    ** Unknown error from btk_bit_alloc() 
    */
    } else if (retval != BT_SUCCESS) {
        btk_mutex_exit(&unit_p->mreg_mutex);
        btk_mutex_exit(&unit_p->dma_mutex);
        INFO_STR("Failed to initial alloc section of DMA map regs");
        *data_width_p = orig_width;
        *xfer_length_p = orig_length;
        *dma_flag_p = FALSE;
        line_number = __LINE__;
        goto dma_pio_exit;

    /*
    ** O.K. we got the mapping RAM
    */
    } else {
        btk_mutex_exit(&unit_p->mreg_mutex);
    }
    line_number = __LINE__;

dma_pio_exit:
    if (*dma_flag_p == FALSE) {
        *need_p = 0;
    }
    TRC_MSG((BT_TRC_RD_WR | BT_TRC_DETAIL), 
            (LOG_FMT "%s; length 0x%x; width %d\n",
            LOG_ARG, *dma_flag_p ? "DMA" : "PIO", *xfer_length_p, 
            *data_width_p));
    TRC_MSG((BT_TRC_RD_WR | BT_TRC_DETAIL), 
            (LOG_FMT "line %d; start %d; need %d\n",
            LOG_ARG, line_number, *start_p, *need_p));
    FEXIT(0);
    return;
}

/******************************************************************************
**
**      Function:       btk_cas()
**
**      Purpose:        Performs Compare and Swap.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      axs_type        Device to perform CAS on
**                      cas_p           Pointer to CAS structure
**
**      Modifies:       Void
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/
bt_error_t btk_cas(
    bt_unit_t *unit_p,
    bt_dev_t axs_type,
    bt_cas_t *cas_p)

{
    bt_error_t            retval = BT_SUCCESS;
    volatile bt_data8_t   *byte_p;
    caddr_t               csr_p;
    bt_data8_t            loc_status;
    btk_page_t            page_info;
    BTK_LOCK_RETURN_T     isr_pl;
    
    FUNCTION("btk_cas");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    /* 
    ** Verify the logical device supports this operation 
    */
    if(btk_invalid_op(unit_p, axs_type, BT_OP_READ) ||
       btk_invalid_op(unit_p, axs_type, BT_OP_WRITE)) {
        INFO_STR("CAS not supported for this logical device.");
        retval = BT_EACCESS;
        goto cas_end;
    }

    /*
    ** Check offset and length
    */
    if (btk_invalid_offset(unit_p, axs_type, cas_p->addr, cas_p->len)) {
        INFO_STR("Illegal offset and/or length for CAS operation");
        retval = BT_EACCESS;
        goto cas_end;
    }

    /* 
    ** Check offset is properly aligned and length is valid 
    */
    switch(cas_p->len) {
      case BT_WIDTH_D8:
        break;
      case BT_WIDTH_D16:
        if (!WORD_ALIGN(cas_p->addr)) {
          INFO_STR("Destination address misaligned");
          retval = BT_EINVAL;
        }
        break;
      case BT_WIDTH_D32:
        if (!LONG_ALIGN(cas_p->addr)) {
          INFO_STR("Destination address misaligned");
          retval = BT_EINVAL;
        }
        break;
      default:
        INFO_STR("Invalid CAS length");
        retval = BT_EINVAL;
        break;
    }
    if (retval != 0) {
        goto cas_end;
    }

    btk_mutex_enter(&(unit_p)->dma_mutex); \
    btk_rwlock_wr_enter(&(unit_p)->hw_rwlock);

    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count++;
    BTK_UNLOCK_ISR(unit_p, isr_pl);
   
    CLR_BIT(unit_p->bt_status, BT_ERROR);

    /*
    ** Use setpage to get a pointer to the remote address
    */
    retval = btk_setpage(
         unit_p, 
         (bt_dev_t) axs_type, 
         (bt_data32_t) cas_p->addr, 
         &page_info);
    if (retval != BT_SUCCESS) {
        btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
        btk_mutex_exit(&(unit_p)->dma_mutex);
        goto cas_end;
    }
    byte_p = (volatile bt_data8_t *) page_info.bus_base_p + page_info.page_offset;

    /*
    ** Get a pointer to the node register, not using low I/O routines
    ** for speed
    */
    csr_p = (caddr_t)unit_p->csr_p;
    if (NULL == csr_p) {
        INFO_STR("Node register address bad");
        btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
        btk_mutex_exit(&(unit_p)->dma_mutex);
        goto cas_end;
    }

    /* 
    ** Disable interrupts when we are screwing around with the remote
    ** command reg 1.  We need to make sure that we do not take an
    ** interrupt during this procedure so we lock them out.
    */
    BTK_LOCK_ISR(unit_p, isr_pl);

    /* 
    ** PCI adaptors require that any Read-Modify-Write transactions
    ** consist of exactly two transactions or the operation will not
    ** work correctly.  
    */
    /*
    ** Do the proper length accesses
    */
    switch (cas_p->len) {
      case BT_WIDTH_D8:
        {
        bt_data8_t compare, swap, result;

        compare = (bt_data8_t) cas_p->cmp;
        swap = (bt_data8_t) cas_p->swap;

        btp_put_d8(unit_p, csr_p + REM_CMD1, RC1_LOCK_SEL);
        result = PIO_READ_8(byte_p);
        if (compare == result) {
          PIO_WRITE_8(swap, byte_p);
        } else {
          PIO_WRITE_8(result, byte_p);
        }
        btp_put_d8(unit_p, csr_p + REM_CMD1, 0);
        cas_p->result = result;
        }
        break;
        
      case BT_WIDTH_D16:
        {
        bt_data16_t compare, swap, result; 
        volatile bt_data16_t *word_p = (volatile bt_data16_t *) byte_p;

        compare = (bt_data16_t) cas_p->cmp;
        swap = (bt_data16_t) cas_p->swap;

        btp_put_d16(unit_p, csr_p + REM_CMD1, RC1_LOCK_SEL);
        result = PIO_READ_16(word_p);
        if (compare == result) {
          PIO_WRITE_16(swap, word_p);
        } else {
          PIO_WRITE_16(result, word_p);
        }
        btp_put_d16(unit_p, csr_p + REM_CMD1, 0);
        cas_p->result = result;
        }
        break;

      case BT_WIDTH_D32:
        {
        bt_data32_t compare, swap, result; 
        volatile bt_data32_t *long_p = (volatile bt_data32_t *) byte_p;

        compare = (bt_data32_t) cas_p->cmp;
        swap = (bt_data32_t) cas_p->swap;

        btp_put_d32(unit_p, csr_p + REM_CMD1, RC1_LOCK_SEL);
        result = PIO_READ_32(long_p);

        if (compare == result) {
          PIO_WRITE_32(swap, long_p);
        } else {
          PIO_WRITE_32(result, long_p);
        }
        btp_put_d32(unit_p, csr_p + REM_CMD1, 0);
        cas_p->result = result;
        }
        break;
    }

    /* 
    ** Reenable interrupts
    */
    BTK_UNLOCK_ISR(unit_p, isr_pl);

    /*
    ** Release mapping reg aquired in btk_setpage
    */
    if (page_info.mreg_need != 0) {
        btk_mutex_enter(&unit_p->mreg_mutex);
        btk_bit_free(unit_p->mmap_aval_p, page_info.mreg_start, page_info.mreg_need);
        btk_mutex_exit(&unit_p->mreg_mutex);
    }

    /*
    ** Check for status errors
    ** Potential here to check for error before our isr
    ** has had a chance to set error flag, so read status manually 
    ** and check it for errors along with the error flag
    */
    loc_status = (bt_data8_t) btk_get_io(unit_p, BT_LOC_STATUS);
    if ((loc_status & LSR_CERROR_MASK) ||
        IS_SET(unit_p->bt_status, BT_ERROR)) {
        CLR_BIT(unit_p->bt_status, BT_ERROR);
        bt_print_bit_names(unit_p, BT_LOC_STATUS, loc_status);
        INFO_STR("I/O error during CAS operation");
        retval = BT_ESTATUS;

    }
    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count--;
    BTK_UNLOCK_ISR(unit_p, isr_pl);
 
    btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
    btk_mutex_exit(&(unit_p)->dma_mutex);


cas_end:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_tas()
**
**      Purpose:        Performs Test and Set.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      axs_type        Device to perform CAS on
**                      tas_p           Pointer to TAS structure
**
**      Modifies:       Void
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/
bt_error_t btk_tas(
    bt_unit_t *unit_p,
    bt_dev_t axs_type,
    bt_tas_t *tas_p)

{
    bt_error_t              retval = BT_SUCCESS;
    volatile bt_data8_t     *byte_p;
    caddr_t                 csr_p;
    bt_data8_t              byte_before, loc_status;
    btk_page_t              page_info;
    BTK_LOCK_RETURN_T       isr_pl;
    
    FUNCTION("btk_tas");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    /* 
    ** Verify the logical device supports this operation 
    */
    if(btk_invalid_op(unit_p, axs_type, BT_OP_READ) ||
       btk_invalid_op(unit_p, axs_type, BT_OP_WRITE)) {
        INFO_STR("TAS not supported for this logical device.");
        retval = BT_EACCESS;
        goto tas_end;
    }

    /*
    ** Check offset and length
    */
    if (btk_invalid_offset(unit_p, axs_type, tas_p->addr, 1)) {
        INFO_STR("Illegal offset and/or length for TAS operation");
        retval = BT_EACCESS;
        goto tas_end;
    }
    
    btk_mutex_enter(&(unit_p)->dma_mutex); \
    btk_rwlock_wr_enter(&(unit_p)->hw_rwlock);

    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count++;
    BTK_UNLOCK_ISR(unit_p, isr_pl);

    CLR_BIT(unit_p->bt_status, BT_ERROR);

    /*
    ** Use setpage to get a pointer to the remote memory location
    */
    retval = btk_setpage(unit_p, (bt_dev_t) axs_type, (bt_data32_t) tas_p->addr, &page_info);
    if (retval != BT_SUCCESS) {
        btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
        btk_mutex_exit(&(unit_p)->dma_mutex);
        goto tas_end;
    }
    byte_p = (volatile bt_data8_t *) page_info.bus_base_p + page_info.page_offset;

    /*
    ** Grab a pointer to the node registers, not using low I/O routines
    ** for speed reasons
    */
    csr_p = (caddr_t)unit_p->csr_p;
    if (NULL == csr_p) {
        INFO_STR("Node register address bad");
        btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
        btk_mutex_exit(&(unit_p)->dma_mutex);
        goto tas_end;
    }

    /* 
    ** Disable interrupts when we are screwing around with the remote
    ** command reg 1.  We need to make sure that we do not take an
    ** interrupt during this procedure so we lock them out.
    */
    BTK_LOCK_ISR(unit_p, isr_pl);

    /* 
    ** PCI adaptors require that any Read-Modify-Write transactions
    ** consist of exactly two transactions or the operation will not
    ** work correctly.  
    */
    btp_put_d8(unit_p, csr_p + REM_CMD1, RC1_LOCK_SEL);
    byte_before = PIO_READ_8(byte_p);

    PIO_WRITE_8((byte_before | BT_TAS_MSBIT), byte_p);

    btp_put_d8(unit_p, csr_p + REM_CMD1, 0);

    /* 
    ** Reenable interrupt and return the result
    */
    BTK_UNLOCK_ISR(unit_p, isr_pl);
    tas_p->result = ((byte_before & BT_TAS_MSBIT) == 0) ? 0 : 1;

    /*
    ** Release mapping reg aquired in btk_setpage
    */
    if (page_info.mreg_need != 0) {
        btk_mutex_enter(&unit_p->mreg_mutex);
        btk_bit_free(unit_p->mmap_aval_p, page_info.mreg_start, page_info.mreg_need);
        btk_mutex_exit(&unit_p->mreg_mutex);
    }

    /*
    ** Check for status errors
    ** Potential here to check for error before our isr
    ** has had a chance to set error flag, so read status manually 
    ** and check it for errors along with the error flag
    */
    loc_status = (bt_data8_t)btk_get_io(unit_p, BT_LOC_STATUS);
    if ((loc_status & LSR_CERROR_MASK) ||
        IS_SET(unit_p->bt_status, BT_ERROR)) {
        bt_print_bit_names(unit_p, BT_LOC_STATUS, loc_status);
        CLR_BIT(unit_p->bt_status, BT_ERROR);
        INFO_STR("I/O error during TAS operation");
        retval = BT_ESTATUS;
    }
 
    BTK_LOCK_ISR(unit_p, isr_pl);
    unit_p->pio_count--;
    BTK_UNLOCK_ISR(unit_p, isr_pl);

    /*
    ** Release the unit lock
    */
    btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
    btk_mutex_exit(&(unit_p)->dma_mutex);

tas_end:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_ioreg()
**
**      Purpose:        Performs register access from ioctl.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      axs_type        Device to perform CAS on
**                      ioreg_p         Pointer to ioreg structure
**
**      Modifies:       Void
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/
bt_error_t btk_ioreg(
    bt_unit_t *unit_p,
    bt_dev_t axs_type,
    bt_ioaccess_t *ioreg_p)

{
    bt_error_t              retval = BT_SUCCESS;
    
    FUNCTION("btk_ioreg");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;
    
    /*
    ** Do range checking and do not allow the remote iack register
    ** to be accessed
    */
    if (ioreg_p->offset >= BT_MAX_REGISTER) {
        INFO_STR("Invalid register number");
        retval = BT_EINVAL;
        goto ioreg_exit;
    } else if (ioreg_p->offset == BT_REM_IACK) {
        INFO_STR("Access to the IACK register not allowed.");
        retval = BT_EINVAL;
        goto ioreg_exit;
    } else if (ioreg_p->offset >= BT_REM_START) {
        btk_rwlock_rd_enter(&unit_p->hw_rwlock);
    }
    

    switch (ioreg_p->operation) {
      case  BT_IOREAD:
        ioreg_p->value = btk_get_io(unit_p, (bt_reg_t) (ioreg_p->offset));
        break;
      case BT_IOWRITE:
      case BT_IOSHOT:
        btk_put_io(unit_p, (bt_reg_t) (ioreg_p->offset), (bt_data32_t) ioreg_p->value);
        break;
      default:
        INFO_STR("Operation requested isn't supported.");
        retval = BT_EINVAL;
        break;
      }

      if (ioreg_p->offset >= BT_REM_START) {
          btk_rwlock_rd_exit(&unit_p->hw_rwlock);
      }

ioreg_exit:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_dma_poll()
**
**      Purpose:        Polls for DMA completion.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      xfer_length     DMA xfer length.
**                      data_size       DMA data width.
**                      max_delay       Maximum time (usec) to wait.
**
**      Modifies:       BT_DMA_ERROR    Set if DMA timed out
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/

static void btk_dma_poll(
    bt_unit_t *unit_p,
    int xfer_length,
    int data_width,
    int max_delay)

{
    bt_data32_t tmp_reg;
    int         initial_delay, delay_per_pkt, total_delay;
    FUNCTION("btk_dma_poll");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

   
    /*
    ** Just guesses right now
    **
    ** D64 Data Blizzard reading
    **  1.8 uSec for 256 bytes across cable (just a guess)
    ** + .4 uSec for each bus retry (just a guess)
    ** + .9 uSec for 32 D64 xfers on bus (30 nSec * 32 xfers)
    ** ----
    **  3.1 uSec per 256 bytes -> 83.2 Mbytes/Sec
    ** 
    ** 1st packet takes an additional 6 uSec.
    */
    if (data_width == BT_WIDTH_D64) {
        /* D64 Data Blizzard reading */
        initial_delay = 9;
        delay_per_pkt = 3;

    /*
    ** The following numbers are just guesses because on the
    ** Octane that I was testing D16 DMA suffered a 900 nSec
    ** penality for each transfer because the host bridge 
    ** disconnected
    **
    ** D16 Old Nanobus reading
    **  4.4 uSec for 256 bytes across cable (just a guess)
    ** + .8 uSec for each bus retry (assume 1 for general case)
    ** +7.6 uSec for 64 D32 xfers on bus (60 nSec * 64 xfers)
    ** ----
    ** 12.8 uSec per 256 bytes -> 20.0 Mbytes/Sec
    ** 
    ** 1st packet takes an additional 15 uSec.
    */
    } else if (data_width == BT_WIDTH_D16) {
        /* D16 old nanobus */
        initial_delay = 29;
        delay_per_pkt = 13;

    /*
    ** Just guesses right now
    **
    ** D32 Data Blizzard reading
    **  1.8 uSec for 256 bytes across cable (just a guess)
    ** + .4 uSec for each bus retry (just a guess)
    ** +1.7 uSec for 32 D64 xfers on bus (30 nSec * 64 xfers)
    ** ----
    **  3.9 uSec per 256 bytes -> 65.6 Mbytes/Sec
    ** 
    ** 1st packet takes an additional 7 uSec.
    */
    } else if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
        /* D32 Data Blizzard reading */
        initial_delay = 11;
        delay_per_pkt = 4;

    /*
    ** The following numbers were measured on a SGI Octane
    ** D32 Old Nanobus reading
    **  2.2 uSec for 256 bytes across cable
    ** + .8 uSec for each bus retry (assume 1 for general case)
    ** +3.8 uSec for 64 D32 xfers on bus (60 nSec * 64 xfers)
    ** ----
    **  6.8 uSec per 256 bytes -> 37.6 Mbytes/Sec
    ** 
    ** 1st packet takes an additional 12 uSec.
    */
    } else {
        /* D32 old nanobus */
        initial_delay = 19;
        delay_per_pkt = 7;
    }

    /*
    ** Initial delay is based on xfer length
    */
    total_delay = initial_delay + (xfer_length / 256) * delay_per_pkt;
    btk_delay(total_delay);
    while (IS_CLR((tmp_reg = btk_get_io(unit_p, BT_LDMA_CMD)), LDC_DMA_DONE) &&
           (total_delay < max_delay)) {
        btk_delay(delay_per_pkt);
        total_delay += delay_per_pkt;
    }
    if (IS_CLR(tmp_reg, LDC_DMA_DONE)) {
        INFO_STR("Polled DMA timed out");
        btk_dma_stop(unit_p);
        SET_BIT(unit_p->bt_status, BT_DMA_ERROR);
    }

    CLR_BIT(unit_p->bt_status, BT_DMA_ACTIVE);

    FEXIT(0);
    return;
}



/******************************************************************************
**
**      Function:       btk_bcopy()
**
**      Purpose:        Replaces system bcopy() routine.  Assumes that
**                      either src_p or dst_p point at Bus memory and
**                      uses the PIO_READ_*() or PIO_WRITE_*() macros
**                      to read or write PCI memory.
**
**      Args:           unit_p          Pointer to device unit structure.
**                      src_p           Address to transfer data from.
**                      dst_p           Address to transfer data to.
**                      length          Amount of data to transfer.
**                      data_width      Data transfer width (i.e. D8/D16/D32).
**                      src_bus         TRUE if src_p points at PCI memory,
**                                      FALSE if dst_p points at PCI memory
**
**      Modifies:       None
**
**      Returns:        Length actually transferred.
**
**      Calls:          None
**
**      Notes:          Assumes either src_p or dst_p, but not both, point
**                      at bus memory.  If neither does, use the old bt_bcopy()
**                      routine.  If both do, write a new routine.
**
**                      Based off the bt_bcopy routine found in bt_datio.c
**                      version 5.15.
** 
**                      Must support a native 64-bit type (bt_data64_t).
**
******************************************************************************/

static size_t btk_bcopy(
    bt_unit_t *unit_p, 
    caddr_t src_p, 
    caddr_t dst_p, 
    size_t length, 
    int data_width,
    int src_bus)
{
    FUNCTION("btk_bcopy");
    LOG_UNIT(unit_p->unit_number);

    register volatile bt_data8_t    *sb_p, *db_p;
    register volatile bt_data16_t   *sw_p, *dw_p;
    register volatile bt_data32_t   *sl_p, *dl_p;
    register volatile bt_data64_t   *sll_p, *dll_p;
    register unsigned long          count;
    unsigned long                   length_xferred;
    unsigned long                   tot_length_xferred = 0;

    FENTRY;

    /* btk_pio_xfer() already set data_width to BT_WIDTH_D64 (BT_NEXT_GEN) or */
    /* BT_WIDTH_D32 (~BT_NEXT_GEN).  Other callers will use BT_WIDTH_D32:     */ 
    if (data_width == BT_WIDTH_ANY) {
	data_width = BT_WIDTH_D32;
    }

    /* While we have not fully completed requested data transfer */
    while (IS_CLR(unit_p->bt_status, BT_ERROR) && (length > 0)) {
    
        /* D64 with sufficient length, long aligned source and destination */
        if ((data_width >= BT_WIDTH_D64) && (length >= BT_WIDTH_D64) &&
             IS_D64_ALIGNED((u_long)src_p | (u_long)dst_p)) {

            /* Load up long word pointers to source and destination */
            sll_p = (volatile bt_data64_t *) src_p;
            dll_p = (volatile bt_data64_t *) dst_p;

            /*
            ** Calculate the transfer count.
            **
            ** Assume nominal case is where transfer completes.
            */
            count = length / BT_WIDTH_D64;
            length_xferred = count * BT_WIDTH_D64;

            TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
                (LOG_FMT "Performing %ld D64 transfers from 0x%lx to 0x%lx\n",
                LOG_ARG, count, (u_long) src_p, (u_long) dst_p));

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            if (src_bus) {
                while (count > 0) {

                    /*
                    ** NOTE: do not change to D64++ or the powerPC will
                    ** core dump.
                    */

                    *dll_p = PIO_READ_64(sll_p);
                    dll_p += 1;
                    sll_p += 1;
    
                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            } else {

                while (count > 0) {

                    PIO_WRITE_64(*sll_p, dll_p);

                    /*
                    ** NOTE: do not change to D64++ or the powerPC will
                    ** core dump.
                    */
                    dll_p += 1;
                    sll_p += 1;

                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D64;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D32 with sufficient length, long aligned source and destination */
        } else if ((data_width >= BT_WIDTH_D32) && (length >= BT_WIDTH_D32) &&
             IS_D32_ALIGNED((u_long)src_p | (u_long)dst_p)) {

            /* Load up long word pointers to source and destination */
            sl_p = (volatile bt_data32_t *) src_p;
            dl_p = (volatile bt_data32_t *) dst_p;

            /*
            ** Calculate the transfer count.
            **
            ** Assume nominal case is where transfer completes.
            */

            count = length / BT_WIDTH_D32;
            length_xferred = count * BT_WIDTH_D32;

            TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
                (LOG_FMT "Performing %ld D32 transfers from 0x%lx to 0x%lx\n",
                LOG_ARG, count, (u_long) src_p, (u_long) dst_p));

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            if (src_bus) {
                while (count > 0) {

                    *dl_p++ = PIO_READ_32(sl_p);
                    sl_p++;
    
                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            } else {

                while (count > 0) {

                    PIO_WRITE_32(*sl_p, dl_p);
                    dl_p++;
                    sl_p++;

                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D32;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D16 with sufficient length, word aligned source and destination */
        } else if ((data_width >= BT_WIDTH_D16) && (length >= BT_WIDTH_D16) &&
                   IS_D16_ALIGNED((u_long)src_p | (u_long)dst_p)) {

            /*
            ** Differentiate between the requirement that we use D16
            ** versus an alignment issue to get back to using longs.
            **
            ** Assume the nominal case is where transfer completes.
            */
            if (data_width == BT_WIDTH_D16 ||
                (((u_long) dst_p ^ (u_long) src_p) & (BT_WIDTH_D32-1)) != 0) {
                count = length / BT_WIDTH_D16;
                length_xferred = count * BT_WIDTH_D16;
            } else {
                count = 1;
                length_xferred = BT_WIDTH_D16;
            }

            /* Load up word pointers to source and destination */
            sw_p = (volatile bt_data16_t *) src_p;
            dw_p = (volatile bt_data16_t *) dst_p;

            TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
                (LOG_FMT "Performing %ld D16 transfers from 0x%lx to 0x%lx\n",
                LOG_ARG, count, (u_long) src_p, (u_long) dst_p));

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */

            if (src_bus) {
                while (count > 0) {

                    *dw_p++ = PIO_READ_16(sw_p);
                    sw_p++;

		    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            } else {
                while (count > 0) {

                    PIO_WRITE_16(*sw_p, dw_p);
                    dw_p++;
                    sw_p++;

		    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count.
            */
            if (count) {
                length_xferred -= count * BT_WIDTH_D16;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;

        /* D8 with sufficient length, byte aligned source and destination */
        } else {

            /*
            ** Differentiate between the requirement that we use D8 vs
            ** an alignment issue to get back to using words or longs.
            ** Assume the nominal case is where the transfer completes.
            */

            if (data_width == BT_WIDTH_D8 ||
                (((u_long) dst_p ^ (u_long) src_p) & (BT_WIDTH_D16-1)) != 0) {
                length_xferred = count = length;
            } else {
                length_xferred = count = BT_WIDTH_D8;
            }

            /* Load up byte pointers to source and destination */
            sb_p = (volatile bt_data8_t *) src_p;
            db_p = (volatile bt_data8_t *) dst_p;

            TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
                (LOG_FMT "Performing %ld D8 transfers from 0x%lx to 0x%lx\n",
                LOG_ARG, count, (u_long) src_p, (u_long) dst_p));

            /*
            ** Here's where the actual data transfer takes place.
            ** This loop must be kept so tight that it squeaks!!!
            */
            if (src_bus) {
                while (count > 0) {

                    *db_p++ = PIO_READ_8(sb_p);
                    sb_p++;

                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            } else {
                while (count > 0) {

                    PIO_WRITE_8(*sb_p, db_p);
                    db_p++;
                    sb_p++;

                    if (IS_SET(unit_p->bt_status, BT_ERROR)) {
                        break;
                    } else {
                        count--;
                    }
                }
            }

            /*
            ** If transfer did not complete recalculate transferred
            ** count but avoid use of a run-time multiply to get it.
            */
            if (count) {
                length_xferred -= count;
            }

            length -= length_xferred;
            src_p += length_xferred;
            dst_p += length_xferred;
            tot_length_xferred += length_xferred;
        }
    }

    

    FEXIT(tot_length_xferred);
    return((long) tot_length_xferred);
}


/******************************************************************************
**
**      Function:       bt_print_bit_names()
**
**      Purpose:        Print out the bit definitions of a adapter register.  
**
**      Args:           unit_p          Pointer to unit structure
**                      adapter_reg     enum of the adapter register
**                      byte_value      8-bit value to decode
**
**      Returns:        none
**
******************************************************************************/
void bt_print_bit_names(
  bt_unit_t *unit_p,
  bt_reg_t adapter_reg,
  bt_data8_t byte_value)
{
#define BUF_LEN 240
  char tmp_buf[BUF_LEN];

  FUNCTION("bt_print_bit_names");
  LOG_UNIT(unit_p->unit_number);

  FENTRY;

  BTK_BZERO(&tmp_buf[0], BUF_LEN);

  switch(adapter_reg) {
    case BT_LOC_STATUS:
      if ((byte_value & LSR_PARITY_ERR) != 0) {
        strcat(&tmp_buf[0], " Fiber-Optic Interface Data Error");
      }
      if ((byte_value & LSR_REMBUS_ERR) != 0) {
        strcat(&tmp_buf[0], " Remote bus Error");
      }
      if ((byte_value & LSR_PR_STATUS) != 0) {
        strcat(&tmp_buf[0], " Receiving PR interrupt");
      }
      if ((byte_value & LSR_TIMEOUT_ERR) != 0) {
        strcat(&tmp_buf[0], " Interface timeout");
      }
      if ((byte_value & LSR_LRC_ERR) != 0) {
        strcat(&tmp_buf[0], " LRC DMA packet error");
      }
      if ((byte_value & LSR_NO_CONNECT) != 0) {
        strcat(&tmp_buf[0], " Remote Buf Power Off");
      }
      break;

    case BT_REM_STATUS:
      if ((byte_value & RSR_WAS_RESET) != 0) {
        strcat(&tmp_buf[0], " Remote bus was reset");
      }
      if ((byte_value & RSR_PR_STATUS) != 0) {
        strcat(&tmp_buf[0], " PR interrupt is set");
      }
      if ((byte_value & RSR_PT_STATUS) != 0) {
        strcat(&tmp_buf[0], " PT interrupt is set");
      }
      break;

    case BT_REM_SLAVE_STATUS:
      if ((byte_value & RSS_PARITY_ERR) != 0) {
        strcat(&tmp_buf[0], " Fiber-Optic Interface Data Error");
      }
      if ((byte_value & RSS_REMBUS_ERR) != 0) {
        strcat(&tmp_buf[0], " Remote bus Error");
      }
      if ((byte_value & RSS_PR_STATUS) != 0) {
        strcat(&tmp_buf[0], " Receiving PR interrupt");
      }
      if ((byte_value & RSS_TIMEOUT_ERR) != 0) {
        strcat(&tmp_buf[0], " Interface timeout");
      }
      if ((byte_value & RSS_LRC_ERR) != 0) {
        strcat(&tmp_buf[0], " LRC DMA packet error");
      }
      if ((byte_value & RSS_TIMEOUT_ERR) != 0) {
        strcat(&tmp_buf[0], " interface timeout error");
      }
      if ((byte_value & RSS_PT_STATUS) != 0) {
        strcat(&tmp_buf[0], " pt interrupt (to remote)");
      }
      if ((byte_value & RSS_NO_CONNECT) != 0) {
        strcat(&tmp_buf[0], " no connection to remote");
      }
      break;

     default:
       INFO_STR("register bits not coded yet.");
       break;
   }


  INFO_STR(&tmp_buf[0]);      

  FEXIT(0);

}
