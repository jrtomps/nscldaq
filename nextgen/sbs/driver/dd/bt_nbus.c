/******************************************************************************
**
**      Filename:       bt_nbus.c
**
**      Purpose:        SBS NanoBus Common Kernel Routines
**                      routines.
**
**      Functions:      btk_get_io(), btk_put_io(), btk_rem_id()
**                      btk_get_mreg(), btk_put_mreg(), btk_put_mreg_range(),
**                      btk_bind_bit_adjust(), btk_setpage(), 
**                      btk_pagecheck(), btk_peer2peer_init()
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $" __DATE__;
#endif  /* LINT */

/*
**  Include files
*/
#include        "btdd.h"


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
**     BT_ERROR                 Set by ISR when error interrupt occurs
**
*****************************************************************************/

/*****************************************************************************
**
**              Local routines
**
*****************************************************************************/
bt_error_t btk_pagecheck(bt_unit_t *unit_p, btk_page_t *page_p);
bt_error_t btk_setpage(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t ldev_addr, btk_page_t *page_p);
void btk_peer2peer_init(bt_unit_t *unit_p);
void btk_setup_mreg(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t *mreg_value_p, bt_operation_t op);
bt_error_t btk_send_irq(bt_unit_t *unit_p, bt_data32_t vector, bool_t driver);


/*****************************************************************************
**
**              External routines
**
*****************************************************************************/
extern bt_data8_t btp_get_d8(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d8(bt_unit_t *unit_p, void *addr_p, bt_data8_t value);
extern bt_data16_t btp_get_d16(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d16(bt_unit_t *unit_p, void *addr_p, bt_data16_t value);
extern bt_data32_t btp_get_d32(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d32(bt_unit_t *unit_p, void *addr_p, bt_data32_t value);


#define LOCK_DMA(u_p)    btk_mutex_enter(&(u_p)->dma_mutex);
#define UNLOCK_DMA(u_p)  btk_mutex_exit(&(u_p)->dma_mutex);

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_NBUS_C);


/*****************************************************************************
**
**      Name:           btk_get_io
**
**      Purpose:        Reads a device register.
**
**      Args:
**          unit_p      Unit pointer
**          reg         Enumeration specifying which register to access.
**          value_p     Pointer to location to store the value read.
**
**      Modifies:
**          value_p->   Set to value read from register.
**          
**      Returns:
**          void
**
**      Notes:
**      The reason that this routine uses a pointer to return the result
**      is so an implementation can more easily replace the routine with 
**      a macro.
**
*****************************************************************************/

bt_data32_t btk_get_io(
    bt_unit_t   *unit_p, 
    bt_reg_t    reg)
{
    size_t      size = BT_WIDTH_D8;
    bt_data32_t mask = BT_D8_MASK;
    bt_data32_t value = 0;
    void        *base_p;

    FUNCTION("btk_get_io");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /*
    ** Check that compiler didn't add holes to structure. If it
    ** did, you can't use offsetof() for all the register offsets.
    */
    BTK_ASSERT(MAX_IOREGS == sizeof(bt_pci_reg_t));

    /*
    ** Check for power
    */
    if ((IS_SET(unit_p->bt_status, BT_POWER)) &&
        (reg >= BT_REM_START)) {
        INFO_STR("Can not read remote register, power or cable off");
        FEXIT(0);
        return(0);
    }

    /* 
    ** Get register address, size and mask 
    */
    switch (reg) {
      case BT_LOC_CMD1:
        base_p = (void *) &(unit_p->csr_p->loc_cmd1);
        break;

      case BT_LOC_INT_CTRL:
        base_p = (void *) &(unit_p->csr_p->loc_int_ctrl);
        break;

      case BT_LOC_INT_STATUS:
        base_p = (void *) &(unit_p->csr_p->loc_int_status);
        break;

      case BT_LOC_STATUS:
        base_p = (void *) &(unit_p->csr_p->loc_status);
        break;

      case BT_LOC_HANDSHAKE:
      case BT_LOC_CONFIG_CTRL:
      case BT_LOC_RCMD2:
      case BT_LOC_RPAGE:
      case BT_LOC_AMOD:
        goto get_invalid_reg;

      case BT_LDMA_CMD:
        base_p = (void *) &(unit_p->csr_p->ldma_cmd);
        break;

      case BT_LDMA_RMD_CNT:
        base_p = (void *) &(unit_p->csr_p->ldma_rmd_cnt);
        break;

      case BT_LDMA_ADDR:
        base_p = (void *) &(unit_p->csr_p->ldma_addr_lo);
        size = BT_WIDTH_D32;
        mask = BT_D32_MASK;
        break;

      case BT_LDMA_PKT_CNT:
        base_p = (void *) &(unit_p->csr_p->ldma_pkt_cnt);
        size = BT_WIDTH_D16;
        mask = BT_D16_MASK;
        break;

      case BT_LOC_TCSIZE:
      case BT_LOC_256MB_PAGE:
      case BT_LOC_RCMD1:
        goto get_invalid_reg;

      case BT_LOC_BUS_CTRL:
        base_p = (void *) &(unit_p->csr_p->loc_bus_ctrl);
        break;

      case BT_LOC_LOOPBACK:
        base_p = (void *) &(unit_p->csr_p->loc_loopback);
        break;

      case BT_LOC_MREG_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_mreg_ctrl);
        break;

      case BT_LOC_SEMA0:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema0);
        break;

      case BT_LOC_SEMA1:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema1);
        break;

      case BT_LOC_SEMA2:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema2);
        break;

      case BT_LOC_SEMA3:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema3);
        break;

      case BT_LOC_SEMA4:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema4);
        break;

      case BT_LOC_SEMA5:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema5);
        break;

      case BT_LOC_SEMA6:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema6);
        break;

      case BT_LOC_SEMA7:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema7);
        break;
        
      /*
      ** Remote Registers
      */
      case BT_REM_CMD1:
        base_p = (void *) &(unit_p->csr_p->rem_cmd1);
        goto end_bt_kget_io;

      case BT_REM_STATUS:
        base_p = (void *) &(unit_p->csr_p->rem_status);
        break;

      case BT_REM_CMD2:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_cmd2);
        break;

      case BT_REM_PAGE:
        goto get_invalid_reg;

      case BT_REM_AMOD:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_amod);
        break;

      case BT_REM_IACK:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_iack);
        size = BT_WIDTH_D16;
        mask = BT_D16_MASK;
        break;

      case BT_REM_CARD_ID:
        base_p = (void *) &(unit_p->csr_p->rem_card_id);
        break;

      case BT_REM_MREG_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_mreg_ctrl);
        break;

      case BT_RDMA_ADDR:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            base_p = (void *) &(unit_p->csr_p->rdma_addr_lo);
        } else {
            base_p = (void *) &(unit_p->csr_p->rdma_addr_hi);
        }
        size = BT_WIDTH_D32;
        mask = BT_D32_MASK;
        break;

      case BT_RDMA_RMD_CNT:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            base_p = (void *) &(unit_p->csr_p->reserve19);
        } else {
            base_p = (void *) &(unit_p->csr_p->rdma_rmd_cnt);
        }
        break;

      case BT_REM_SLAVE_STATUS:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_slave_status);
        break;

      case BT_REM_SLAVE_CLRERR:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_slave_clrerr);
        break;

      case BT_REM_SEMA0:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema0);
        break;

      case BT_REM_SEMA1:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema1);
        break;

      case BT_REM_SEMA2:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema2);
        break;

      case BT_REM_SEMA3:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema3);
        break;

      case BT_REM_SEMA4:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema4);
        break;

      case BT_REM_SEMA5:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema5);
        break;

      case BT_REM_SEMA6:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema6);
        break;

      case BT_REM_SEMA7:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema7);
        break;
        
      case BT_RPQ_REM_LCMD1:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lloc_cmd1);
        break;
        
      case BT_RPQ_REM_LINT_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lint_ctrl);
        break;
        
      case BT_RPQ_REM_LSTATUS:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lstatus);
        break;
        
      case BT_RPQ_REM_LINT_STATUS:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lint_status);
        break;
        
      case BT_RPQ_REM_LBUS_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lbus_ctrl);
        break;
        
      case BT_RPQ_REM_LLOOPBACK:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lloopback);
        break;
        
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_RPQ_REM_A64PIO:
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "Set A64PIO Register %d, value 0x%x, mask = 0x%x.\n", 
            LOG_ARG, reg, value, mask));
        base_p = (void *) &(unit_p->csr_p->rem_a64pio_addr_lo);
        size = BT_WIDTH_D32;
        break;

      case BT_RPQ_REM_A64DMA:
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "Set A64DMA Register %d, value 0x%x, mask = 0x%x.\n", 
            LOG_ARG, reg, value, mask));
        base_p = (void *) &(unit_p->csr_p->rem_a64dma_addr_lo);
        size = BT_WIDTH_D32;
        break;
#endif  /* EAS_A64_CODE */

      default:
        goto get_invalid_reg;
    }

    /* 
    ** Actually do the register access 
    */
    switch (size) {
      case BT_WIDTH_D8:
        value = (bt_data32_t) btp_get_d8(unit_p, base_p);
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "D08 Register %d, value 0x%x, mask = 0x%x.\n", 
            LOG_ARG, reg, value, mask));
        break;

      case BT_WIDTH_D16:
        value = (bt_data32_t) btp_get_d16(unit_p, base_p);
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "D16 Register %d, value 0x%x, mask = 0x%x.\n", 
            LOG_ARG, reg, value, mask));
        break;

      case BT_WIDTH_D32:
        if ((reg == BT_RDMA_ADDR) &&
            (IS_CLR(unit_p->bt_status, BT_PCI2PCI))) {
            value = btp_get_d16(unit_p, base_p)  << BT_D16_SHFT;
            value |= btp_get_d16(unit_p, (void *) ((bt_data8_t *) base_p + 2));
        } else {
            value = btp_get_d16(unit_p, base_p);
            if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
                value |= btp_get_d16(unit_p, (void *) ((bt_data8_t *) base_p + 2)) << BT_D16_SHFT;
            } else {
                value |= btp_get_d8(unit_p, (void *) ((bt_data8_t *) base_p + 2)) << BT_D16_SHFT;
            }
        }
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "D32 Register %d, value 0x%x, mask = 0x%x.\n", 
            LOG_ARG, reg, value, mask));
        break;

      default:
        /* 
        ** This should never happen 
        */
        goto get_invalid_reg;
    }

  end_bt_kget_io:

    value &= mask;
    TRC_MSG(BT_TRC_LIO, (LOG_FMT "Register %d, value 0x%x.\n", 
            LOG_ARG, reg, value));
    FEXIT(value);
    return(value);

    /* 
    ** Attempted to access an invalid register!
    ** No error message because this can happen alot for different
    ** configurations
    */
get_invalid_reg:
    FEXIT(0);
    return(0);
}

/*****************************************************************************
**
**      Name:           btk_put_io
**
**      Purpose:        Writes a device register. Updates the shadow value 
**                      of the device register.
**
**      Args:
**          unit_p      Unit pointer
**          reg         Enumeration specifying which register to access.
**          value       Value to write to register.
**
**      Modifies:
**          void
**          
**      Returns:
**          void
**
**      Notes:
**
*****************************************************************************/

void btk_put_io(
    bt_unit_t * unit_p, 
    bt_reg_t    reg, 
    bt_data32_t value)
{
    size_t      size = BT_WIDTH_D8;
    void        *base_p;

    FUNCTION("btk_put_io");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /*
    ** Check that compiler didn't add holes to structure. If it
    ** did, you can't use offsetof() for all the register offsets.
    */
    BTK_ASSERT(MAX_IOREGS == sizeof(bt_pci_reg_t));

    /*
    ** Check for power
    */
    if ((IS_SET(unit_p->bt_status, BT_POWER)) &&
        (reg >= BT_REM_START)) {
        INFO_STR("Can not write remote register, power or cable off");
        FEXIT(0);
        return;
    }

    /* 
    ** Get register offset and size 
    */
    switch (reg) {

      case BT_LOC_CMD1:
        base_p = (void *) &(unit_p->csr_p->loc_cmd1);
        break;

      case BT_LOC_INT_CTRL:
        base_p = (void *) &(unit_p->csr_p->loc_int_ctrl);
        break;

      case BT_LOC_INT_STATUS:
        base_p = (void *) &(unit_p->csr_p->loc_int_status);
        break;

      case BT_LOC_STATUS:
        base_p = (void *) &(unit_p->csr_p->loc_status);
        break;

      case BT_LOC_HANDSHAKE:
      case BT_LOC_CONFIG_CTRL:
      case BT_LOC_RCMD2:
      case BT_LOC_RPAGE:
      case BT_LOC_AMOD:
        goto put_invalid_reg;

      case BT_LDMA_CMD:
        base_p = (void *) &(unit_p->csr_p->ldma_cmd);
        break;

      case BT_LDMA_RMD_CNT:
        base_p = (void *) &(unit_p->csr_p->ldma_rmd_cnt);
        break;

      case BT_LDMA_ADDR:
        base_p = (void *) &(unit_p->csr_p->ldma_addr_lo);
        size = BT_WIDTH_D32;
        break;

      case BT_LDMA_PKT_CNT:
        base_p = (void *) &(unit_p->csr_p->ldma_pkt_cnt);
        size = BT_WIDTH_D16;
        break;

      case BT_LOC_TCSIZE:
      case BT_LOC_256MB_PAGE:
      case BT_LOC_RCMD1:
        goto put_invalid_reg;

      case BT_LOC_BUS_CTRL:
        base_p = (void *) &(unit_p->csr_p->loc_bus_ctrl);
        break;

      case BT_LOC_LOOPBACK:
        base_p = (void *) &(unit_p->csr_p->loc_loopback);
        break;

      case BT_LOC_MREG_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_mreg_ctrl);
        break;

      case BT_LOC_SEMA0:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema0);
        break;

      case BT_LOC_SEMA1:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema1);
        break;

      case BT_LOC_SEMA2:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema2);
        break;

      case BT_LOC_SEMA3:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema3);
        break;

      case BT_LOC_SEMA4:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema4);
        break;

      case BT_LOC_SEMA5:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema5);
        break;

      case BT_LOC_SEMA6:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema6);
        break;

      case BT_LOC_SEMA7:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->loc_sema7);
        break;

      /*
      ** Remote Registers
      */
      case BT_REM_CMD1:
        base_p = (void *) &(unit_p->csr_p->rem_cmd1);
        break;

      case BT_REM_STATUS:
        TRC_MSG(BT_TRC_LIO, (LOG_FMT "Register %d is read only.\n",
            LOG_ARG, reg));
        goto put_invalid_reg;

      case BT_REM_CMD2:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_cmd2);
        break;

      case BT_REM_PAGE:
        goto put_invalid_reg;

      case BT_REM_AMOD:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_amod);
        break;

      case BT_REM_IACK:
        goto put_invalid_reg;

      case BT_REM_CARD_ID:
        base_p = (void *) &(unit_p->csr_p->rem_card_id);
        break;

      case BT_REM_MREG_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_mreg_ctrl);
        break;

      case BT_RDMA_ADDR:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            base_p = (void *) &(unit_p->csr_p->rdma_addr_lo);
        } else {
            base_p = (void *) &(unit_p->csr_p->rdma_addr_hi);
        }
        size = BT_WIDTH_D32;
        break;

      case BT_RDMA_RMD_CNT:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            base_p = (void *) &(unit_p->csr_p->reserve19);
        } else {
            base_p = (void *) &(unit_p->csr_p->rdma_rmd_cnt);
        }
        break;

      case BT_REM_SLAVE_STATUS:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_slave_status);
        break;

      case BT_REM_SLAVE_CLRERR:
        if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_slave_clrerr);
        break;

      case BT_REM_SEMA0:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema0);
        break;

      case BT_REM_SEMA1:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema1);
        break;

      case BT_REM_SEMA2:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema2);
        break;

      case BT_REM_SEMA3:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema3);
        break;

      case BT_REM_SEMA4:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema4);
        break;

      case BT_REM_SEMA5:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema5);
        break;

      case BT_REM_SEMA6:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema6);
        break;

      case BT_REM_SEMA7:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rem_sema7);
        break;
        
      case BT_RPQ_REM_LCMD1:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lloc_cmd1);
        break;
        
      case BT_RPQ_REM_LINT_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lint_ctrl);
        break;
        
      case BT_RPQ_REM_LSTATUS:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lstatus);
        break;
        
      case BT_RPQ_REM_LINT_STATUS:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lint_status);
        break;
        
      case BT_RPQ_REM_LBUS_CTRL:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lbus_ctrl);
        break;
        
      case BT_RPQ_REM_LLOOPBACK:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_invalid_reg;
        }
        base_p = (void *) &(unit_p->csr_p->rpq_rem_lloopback);
        break;
        
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_RPQ_REM_A64PIO:
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "Get A64PIO Register %d\n", 
            LOG_ARG, reg));
        base_p = (void *) &(unit_p->csr_p->rem_a64pio_addr_lo);
        size = BT_WIDTH_D32;
        break;

      case BT_RPQ_REM_A64DMA:
        TRC_MSG(BT_TRC_LIO|BT_TRC_DETAIL, 
            (LOG_FMT "Get A64DMA Register %d\n", 
            LOG_ARG, reg));
        base_p = (void *) &(unit_p->csr_p->rem_a64dma_addr_lo);
        size = BT_WIDTH_D32;
        break;
#endif  /* EAS_A64_CODE */

      default:
        goto put_invalid_reg;
    }

    /* 
    ** Actually do the register access 
    */
    switch (size) {
      case BT_WIDTH_D8:
        TRC_MSG(BT_TRC_LIO, (LOG_FMT "D08 Register %d, value 0x%x.\n", 
                LOG_ARG, reg, value));
        btp_put_d8(unit_p, base_p, (bt_data8_t) value);
        break;

      case BT_WIDTH_D16:
        TRC_MSG(BT_TRC_LIO, (LOG_FMT "D16 Register %d, value 0x%x.\n", 
                LOG_ARG, reg, value));
        btp_put_d16(unit_p, base_p, (bt_data16_t) value);
        break;

      case BT_WIDTH_D32:
        TRC_MSG(BT_TRC_LIO, (LOG_FMT "D32 Register %d, value 0x%x.\n", 
                LOG_ARG, reg, value));
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            if (reg == BT_RDMA_ADDR) {
               if (IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
                   btp_put_d16(unit_p, base_p, 
                               (bt_data16_t) (value >> BT_D16_SHFT));
                   btp_put_d16(unit_p, (void *) ((bt_data8_t *) base_p + 2), 
                               (bt_data16_t) (value & BT_D16_MASK));  
               } else {
                   btp_put_d16(unit_p, 
                               base_p, 
                               (bt_data16_t) (value & BT_D16_MASK));
                   btp_put_d16(unit_p, 
                               (void *) ((bt_data8_t *) base_p + 2), 
                               (bt_data16_t) (value >> BT_D16_SHFT));
               }

            } else {
               btp_put_d32(unit_p, base_p, (bt_data32_t)(value & BT_D32_MASK));
            }
        } else {
            if ((reg == BT_RDMA_ADDR) &&
                (IS_CLR(unit_p->bt_status, BT_PCI2PCI))) {
                btp_put_d16(unit_p, base_p, 
                           (bt_data16_t) (value >> BT_D16_SHFT));
                btp_put_d16(unit_p, (void *) ((bt_data8_t *) base_p + 2), 
                           (bt_data16_t) (value & BT_D16_MASK));
            } else {
                btp_put_d16(unit_p, base_p, 
                           (bt_data16_t) (value & BT_D16_MASK));
                btp_put_d8(unit_p, (void *) ((bt_data8_t *) base_p + 2), 
                          (bt_data8_t) ((value >> BT_D16_SHFT) & BT_D8_MASK));
            }
        }
        break;

      default:
        /* This should never happen */
        goto put_invalid_reg;
    }

    FEXIT(0);
    return;

    /* 
    ** Attempted to access an invalid register! 
    ** No error message because this can happen alot for different
    ** configurations
    */
put_invalid_reg:
    FEXIT(0);
    return;
}

/*****************************************************************************
**
**      Name:           btk_rem_id
**
**      Purpose:        Determines what adapter card is on the other
**                      side of the cable.
**
**      Args:
**          unit_p      Unit pointer
**
**      Modifies:
**          void
**          
**      Returns:
**          BT_PN_UNKNOWN       Does not recognize the remote card.
**          Otherwise           SBS Part Number of the remote card.
**
**      Notes:
**          Only supports cards that have a remote id register and
**          is specifically written for the PCI adapter card going
**          to either another PCI card or a VMEbus card.
**
*****************************************************************************/

bt_data32_t btk_rem_id(
    bt_unit_t *unit_p)
{
    FUNCTION("bt_rem_id");
    LOG_UNIT(unit_p->unit_number);

    bt_data8_t      rem_id, loc_status, unused;
    bt_data32_t     retvalu = BT_PN_UNKNOWN;

    FENTRY;


    /* The 614 dosn't have a rem-id register */
    /* so just assume the remote card is a also a 614 */
    if (unit_p->loc_id == BT_PN_PCI) {
        rem_id = (bt_data8_t)BT_PN_MB;
        goto bt_rem_id_bypass;
    }

    /*
    **  Clear out unit status information.
    */
    CLR_BIT(unit_p->bt_status, BT_DMA_AVAIL);

    /*
    **  Make sure we have a connection
    */
    loc_status = (bt_data8_t)btk_get_io(unit_p, BT_LOC_STATUS);
    if (IS_SET(loc_status, LSR_NO_CONNECT )) {
        SET_BIT(unit_p->bt_status, BT_POWER);
        goto bt_rem_id_exit;
    }

    /*
    **  Clear flush the cable and clean any errors
    */
    unused = (bt_data8_t)btk_get_io(unit_p, BT_REM_STATUS);
    btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
    CLR_BIT(unit_p->bt_status, BT_ERROR);

    /*
    **  Read the remote  ID register.
    */
    btk_put_io(unit_p, BT_REM_CARD_ID, 0xff);
    rem_id = (bt_data8_t)btk_get_io(unit_p, BT_REM_CARD_ID);

    btk_put_io(unit_p, BT_REM_CARD_ID, 0x0);
    unused = (bt_data8_t)btk_get_io(unit_p, BT_REM_CARD_ID);

    /* 
    ** Remote ID should always return the same value
    ** If it does not, return unknown card number
    */
    if (rem_id != unused) {
        rem_id = 0;
        TRC_STR(BT_TRC_CFG, "Remote ID returned different read values");
    }

    /* 
    ** Check status errors
    ** If there are errors, return unknown card number
    */
    loc_status = (bt_data8_t)btk_get_io(unit_p, BT_LOC_STATUS);
    if ((loc_status & LSR_CERROR_MASK) != 0) {
        rem_id = 0;
        btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
        CLR_BIT(unit_p->bt_status, BT_ERROR);
        TRC_MSG(BT_TRC_CFG, (LOG_FMT "Adapter status error 0x%x\n", 
                LOG_ARG, loc_status));
    }

bt_rem_id_bypass:
    /*
    ** Look through the different IDs
    ** Note BT_NEXT_GEN is set based on loc_id in the driver
    ** specific setup routine
    */
    switch(rem_id) {

        case BT_ID_VME_DB: /* VME side of 617 */
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->bt_status, BT_PCI2PCI);
            retvalu = BT_PN_VME_DB;
            break;

        case BT_ID_VME_SDMA: /* VME side of 617 */
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->bt_status, BT_PCI2PCI);
            retvalu = BT_PN_VME_SDMA;
            break;
        
        case BT_ID_VME_NODMA: /* VME side of 616 */
            CLR_BIT(unit_p->bt_status, BT_PCI2PCI);
            retvalu = BT_PN_VME_NODMA;
            break;
        
        case BT_ID_PCI_DMA: /* PCI side of 617 */
            SET_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->logstat[BT_AXSRDP], STAT_DMA);
            retvalu = BT_PN_PCI_DMA;
            break;
        
        case BT_ID_PCI: /* PCI side of 616 */
            SET_BIT(unit_p->bt_status, BT_PCI2PCI);
            CLR_BIT(unit_p->logstat[BT_AXSRDP], STAT_DMA);
            retvalu = BT_PN_PCI_NODMA;
            break;

        case BT_ID_PCI_FIBER: /* PCI side of 618 */
            SET_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->logstat[BT_AXSRDP], STAT_DMA);
            retvalu = BT_PN_PCI_FIBER;
            break;
        
        case BT_ID_VME_FIBER: /* VME side of 618 */
            CLR_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            retvalu = BT_PN_VME_FIBER;
            break;
            
        case BT_ID_PCI_FIBER_D64: /* PCi side of 704 */
            SET_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->logstat[BT_AXSRDP], STAT_DMA);
            retvalu = BT_PN_PCI_DB;
            break;
 
#if  EAS_A64_CODE
/* EAS A64 */           
        case BT_ID_PCI_DBA64_RPQ: /* PCi side of RPQ 601332 */
            SET_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            CLR_BIT(unit_p->logstat[BT_AXSRDP], STAT_DMA);
            retvalu = BT_PN_PCI_DBA64_RPQ;
            break;
#endif  /* EAS_A64_CODE */
 
         case BT_ID_VME_FIBER_D64: /* VME side of 704 */
            CLR_BIT(unit_p->bt_status, BT_PCI2PCI);
            SET_BIT(unit_p->bt_status, BT_DMA_AVAIL);
            retvalu = BT_PN_VME_DB;
            break;
            
        case 0:
        case BYTE_MASK:
        default:
            break;

    }

bt_rem_id_exit:
    if (IS_CLR(unit_p->bt_status, BT_DMA_LOCAL)) {
        CLR_BIT(unit_p->bt_status, BT_DMA_AVAIL);
    }

    FEXIT(retvalu);
    return(retvalu);
}

/******************************************************************************
**
**      Function:   btk_get_mreg()
**
**      Purpose:    Read one of the mapping registers.
**
**      Args:       unit_p          Pointer to unit dds structure.
**                  mr_idx          Index to mapping register.
**                  mreg_type       Type of mapping register.
**
**      Modifies:
**          None.
**
**      Returns:    Value read from the register
**
**      Notes:
**      Notice that there are no shadow mapping registers.
**
******************************************************************************/

bt_data32_t btk_get_mreg(
    bt_unit_t *unit_p, 
    unsigned int mr_idx,
    bt_mreg_t mreg_type
    )
{
    FUNCTION("btk_get_mreg");
    LOG_UNIT(unit_p->unit_number);

    bt_data8_t   *base_p = (bt_data8_t *) unit_p->mreg_p;
    bt_data32_t  retval = 0;

    /* FENTRY; */

    switch (mreg_type) {
      case BT_LMREG_PCI_2_CABLE:
        base_p += BT_CMR_RMEM_OFFSET;
        break;
      case BT_LMREG_CABLE_2_PCI:
        base_p += BT_CMR_LMEM_OFFSET;
        break;
      case BT_LMREG_DMA_2_PCI:
        base_p += BT_CMR_LDMA_OFFSET;
        break;
      case BT_RMREG_PCI_2_CABLE:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_mreg_end;
        }
        base_p += BT_CMR_RMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_CABLE_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_mreg_end;
        }
        base_p += BT_CMR_LMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_DMA_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto get_mreg_end;
        }
        base_p += BT_CMR_LDMA_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      default:
        break;
    }

    retval= btp_get_d32(unit_p, (void *) (base_p + mr_idx*4));

    TRC_MSG(BT_TRC_MAPREG, (LOG_FMT "mreg offset 0x%x  retval 0x%x\n",
            LOG_ARG, mr_idx * 4, retval));

get_mreg_end:
    /* FEXIT(retval); */
    return(retval);
}

/******************************************************************************
**
**      Function:   btk_put_mreg()
**
**      Purpose:    Write one of the mapping registers.
**
**      Args:       unit_p          Pointer to unit dds structure.
**                  mr_idx          Index to mapping register.
**                  mreg_type       Type of mapping register.
**                  value           Value to write to register.
**
**      Modifies:   NA
**
**      Returns:    void
**
**      Notes:
**
******************************************************************************/
void btk_put_mreg(
    bt_unit_t *unit_p,
    unsigned int mr_idx,
    bt_mreg_t mreg_type,
    bt_data32_t value
    )
{
    FUNCTION("btk_put_mreg");
    LOG_UNIT(unit_p->unit_number);
    
    bt_data8_t   *base_p = (bt_data8_t *) unit_p->mreg_p;

    /* FENTRY; */

    switch (mreg_type) {
      case BT_LMREG_PCI_2_CABLE:
        base_p += BT_CMR_RMEM_OFFSET;
        break;
      case BT_LMREG_CABLE_2_PCI:
        base_p += BT_CMR_LMEM_OFFSET;
        break;
      case BT_LMREG_DMA_2_PCI:
        base_p += BT_CMR_LDMA_OFFSET;
        break;
      case BT_RMREG_PCI_2_CABLE:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_end;
        }
        base_p += BT_CMR_RMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_CABLE_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_end;
        }
        base_p += BT_CMR_LMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_DMA_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_end;
        }
        base_p += BT_CMR_LDMA_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      default:
        break;
    }

    TRC_MSG(BT_TRC_MAPREG,
	    (LOG_FMT "mreg: type %d  base 0x%lx  offset 0x%x  value 0x%x\n",
            LOG_ARG, mreg_type, (u_long)base_p, mr_idx * 4, value));

    btp_put_d32(unit_p, (void *) (base_p + mr_idx*4), value);

put_mreg_end:
    /* FEXIT(0); */
    return;
}



/******************************************************************************
**
**      Function:   btk_put_mreg_range()
**
**      Purpose:    Write a range of mapping registers, incrementing the
**                  page address each time.
**
**      Args:       unit_p          Pointer to unit dds structure.
**                  mr_start        Index to starting mapping register.
**                  mreg_type       Type of mapping register.
**                  num_mregs       Number of consecutive mapping registers
**                                  to program.
**                  value           Value to write to register.
**
**      Returns:    void
**
**      Notes:
**      Notice that there are no shadow mapping registers.
**
******************************************************************************/
void btk_put_mreg_range(
    bt_unit_t *unit_p,
    unsigned int mr_start,
    unsigned int num_mregs,
    bt_mreg_t mreg_type,
    bt_data32_t value
    )
{
    FUNCTION("btk_put_mreg_range");
    LOG_UNIT(unit_p->unit_number);

    unsigned int mr_idx = 0;
    bt_data8_t   *base_p = (bt_data8_t *) unit_p->mreg_p;

    /* FENTRY; */

    switch (mreg_type) {
      case BT_LMREG_PCI_2_CABLE:
        base_p += BT_CMR_RMEM_OFFSET;
        break;
      case BT_LMREG_CABLE_2_PCI:
        base_p += BT_CMR_LMEM_OFFSET;
        break;
      case BT_LMREG_DMA_2_PCI:
        base_p += BT_CMR_LDMA_OFFSET;
        break;
      case BT_RMREG_PCI_2_CABLE:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_range_end;
        }
        base_p += BT_CMR_RMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_CABLE_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_range_end;
        }
        base_p += BT_CMR_LMEM_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      case BT_RMREG_DMA_2_PCI:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) ||
            IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            goto put_mreg_range_end;
        }
        base_p += BT_CMR_LDMA_OFFSET + BT_CMR_REMOTE_OFFSET;
        break;
      default:
        break;
    }

    TRC_MSG(BT_TRC_MAPREG, (LOG_FMT "mreg offset 0x%x to 0x%x  value 0x%x to 0x%x\n",
         LOG_ARG, mr_start * 4, (mr_start + num_mregs - 1) * 4, value, 
         value + BT_PAGE_SIZE*(num_mregs - 1)));

    for (mr_idx = 0; mr_idx < num_mregs * 4; mr_idx += 4, value += BT_PAGE_SIZE) {
        btp_put_d32(unit_p, base_p + mr_idx + mr_start*4, value);
    }

put_mreg_range_end:
    /* FEXIT(0); */
    return;
}

/******************************************************************************
**
**      Function:   btk_bind_bit_adjust()
**
**      Purpose:    Given a Cable to PCI mapping register index this
**                  function returns the actual mapping register
**                  index to program in order to take into account
**                  a VMEbus REM RAM jumper setting that is not
**                  a multiple of 16 Mbytes.
**
**      Args:       unit_p          Pointer to unit structure.
**                  bit             Mapping RAM bit allocated by
**                                    btk_bit_? that you wish to program
**
**      Modifies:   NA
**
**      Returns:    Index of the mapping RAM to program.
**                  Index is from the start of the Cable to PCI 
**                  mapping RAM section.
**
**      Notes:      NO LONGER IMPLEMENTED
**                  
**
******************************************************************************/
unsigned int btk_bind_bit_adjust(
    bt_unit_t *unit_p,
    unsigned int bit
    )
{
    FUNCTION("btk_bind_bit_adjust");
    LOG_UNIT(unit_p->unit_number);

    unsigned    int retval = bit;

    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btk_setpage()
**
**      Purpose:        Determines and loads the proper adaptor page state.
**
**      Args:           unit_p          Pointer to device dds structure.
**                      ldev            Logical device being accessed
**                      ldev_addr       Offset into the logical device.
**                      page_p          Pointer to page information.
**
**      Returns:        BT_SUCCESS      If successful.
**                      Otherwise       BT_ENOMEM or BT_EIO.
**
**      Notes:          The caller must release any mmap mapping ram
**                      allocated by this routine.  mreg_need bit
**                      will be set to != 0 if mapping ram were 
**                      allocated and mreg_start will be the first
**                      one used.
**
******************************************************************************/
bt_error_t btk_setpage(
  register bt_unit_t *unit_p, 
  bt_dev_t ldev, 
  bt_data32_t ldev_addr, 
  register btk_page_t *page_p)
  
{
    FUNCTION("btk_setpage");
    LOG_UNIT(unit_p->unit_number);
    
    bt_error_t  retval = BT_SUCCESS;
    bt_data32_t mreg_value = 0;
    
    FENTRY;
    
    /* 
    **  Circumvent lseek() deficiency of using signed 32bit values
    */
    if (ldev == BT_AXSRE) {
        ldev_addr |= RE_ADJUST;
    }

    /*
    ** Look at kern_size to determine if we use mapping regs or not
    ** a value of zero indicates that we do not use mapping registers
    */
    if (unit_p->kern_length[ldev] == 0) {
        page_p->mreg_need = 1;
        page_p->page_size = BT_PAGE_SIZE;
    } else {
        page_p->mreg_need = 0;
        page_p->page_size = unit_p->kern_length[ldev];
    }
    page_p->bus_base_p = unit_p->kern_addr[ldev];
      
    /*
    ** Reserve mapping RAM for transfer
    */
    if (page_p->mreg_need != 0) {
        btk_mutex_enter(&unit_p->mreg_mutex);
        retval = btk_bit_alloc(unit_p->mmap_aval_p, page_p->mreg_need, 1, &(page_p->mreg_start));
        btk_mutex_exit(&unit_p->mreg_mutex);
        if (retval != BT_SUCCESS) {
            INFO_STR("Unable to allocate mapping reg for PIO read/write");
            retval = BT_ENOMEM;
            goto setpage_end;
        }
    }
    
    /*
    ** Load the paging infomation
    */
    page_p->page_number = (bt_data32_t) (ldev_addr & ~(page_p->page_size - 1));
    page_p->page_offset = ldev_addr & (page_p->page_size - 1);
    if (page_p->mreg_need != 0) {
        page_p->bus_base_p += (page_p->mreg_start * BT_PAGE_SIZE);
    }
    TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
           (LOG_FMT "page size 0x%x; number 0x%x; offset 0x%x\n",
           LOG_ARG, (int) page_p->page_size, page_p->page_number,
           (int) page_p->page_offset));
    TRC_MSG((BT_TRC_PIO | BT_TRC_DETAIL), 
           (LOG_FMT "page base 0x%p; mreg start %d; need %d\n",
           LOG_ARG, page_p->bus_base_p, page_p->mreg_start,
           page_p->mreg_need));
    
    /*
    ** Load the remote address, address modifier, and function
    */
    if (page_p->mreg_need != 0) {
        mreg_value = page_p->page_number & BT_MREG_ADDR_MASK;
        btk_setup_mreg(unit_p, ldev, &mreg_value, BT_OP_PIO);
        btk_put_mreg(unit_p, page_p->mreg_start, BT_LMREG_PCI_2_CABLE, mreg_value);
        if ( (btk_get_mreg(unit_p, page_p->mreg_start, BT_LMREG_PCI_2_CABLE)) != mreg_value ) {
            WARN_MSG((LOG_FMT "Verify Write BT_LMREG_PCI_2_CABLE mapping register mr_idx = 0x%.1X failed.\n",
                                                                         LOG_ARG, page_p->mreg_start));
            btk_mutex_enter(&unit_p->mreg_mutex);
            btk_bit_free(unit_p->mmap_aval_p, page_p->mreg_start, page_p->mreg_need);
            btk_mutex_exit(&unit_p->mreg_mutex);
            retval = BT_EIO;
        }
    }
  
setpage_end:
    FEXIT(retval);
    return(retval);
}
  

/******************************************************************************
**
**      Function:       btk_pagecheck()
**
**      Purpose:        Check if we've crossed a page boundary and update
**                      the paging information as well as the window mapping
**                      registers as necessary.
**
**      Args:           unit_p          Pointer to unit structure.
**                      page_p          Pointer to page control information.
**
**      Returns:        BT_SUCCESS      If successful.
**                      BT_EIO          If mapping register fails to update.
**
**      Notes:          None
**
******************************************************************************/
bt_error_t btk_pagecheck(
    bt_unit_t *unit_p, 
    register btk_page_t *page_p)
{
    bt_data32_t   mreg_value;
    bt_error_t  retval = BT_SUCCESS;

    FUNCTION("btk_pagecheck");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;
  
    /*
    ** If the current page offset shows us as having crossed a page
    ** boundary we'll need to bump the page number, reset the offset
    ** into the new page, and load the adaptor's new remote address.
    */
    if (page_p->page_offset >= page_p->page_size) {

        page_p->page_number += page_p->page_size;
        page_p->page_offset = 0;

        /* 
        ** Read the old mapping register so we can just bump addr part
        */
        if (page_p->mreg_need != 0) {
            mreg_value = btk_get_mreg(unit_p, page_p->mreg_start, BT_LMREG_PCI_2_CABLE);
            mreg_value &= ~ BT_MREG_ADDR_MASK;
            mreg_value |= (page_p->page_number & BT_MREG_ADDR_MASK);
            btk_put_mreg(unit_p, page_p->mreg_start, BT_LMREG_PCI_2_CABLE, mreg_value);
            if ( (btk_get_mreg(unit_p, page_p->mreg_start, BT_LMREG_PCI_2_CABLE)) != mreg_value ) {
                WARN_MSG((LOG_FMT "Verify Write BT_LMREG_PCI_2_CABLE mapping register mr_idx = 0x%.1X failed.\n",
                                                                             LOG_ARG, page_p->mreg_start));
                retval = BT_EIO;
            }
        }
    }
  
    FEXIT(retval);
    return retval;
}

/******************************************************************************
**
**      Function:       btk_peer2peer_init()
**
**      Purpose:        Check if we are in a peer to peer situation
**                      and try to set our master bit.
**
**      Args:           unit_p          Pointer to unit structure.
**
**      Returns:        Void
**
**      Notes:          Assumes this is a data blizzard product.
**                      Assumes the unit is locked down.
**
******************************************************************************/
void btk_peer2peer_init(
    bt_unit_t *unit_p)
{
    bt_data32_t   loc_reg, rem_reg, tmp_reg;
    int           got_local = FALSE, got_both = FALSE, update_peer = TRUE;
    bt_data32_t   count1;
    bt_data32_t   count2;
  
    FUNCTION("btk_peer2peer_init");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /*
    ** Read the local and remote driver control bits
    */
    loc_reg = btk_get_io(unit_p, BT_LOC_MREG_CTRL);
    rem_reg = btk_get_io(unit_p, BT_REM_MREG_CTRL);

    /*
    ** If our active bit is clear, this is first time since load
    */
    if (IS_CLR(loc_reg, LMC_DRIVER_ACTIVE)) {
        goto peer_1st_time;
    /*
    ** In loopback 
    */
    } else if (IS_SET(loc_reg, LMC_DRIVER_MASTER) && 
                IS_SET(rem_reg, LMC_DRIVER_MASTER)) {
        goto peer_1st_time;

    /*
    ** If either the local or remote card is the master then 
    ** nothing to do so exit
    */
    } else if (IS_SET(loc_reg, LMC_DRIVER_MASTER) || 
                IS_SET(rem_reg, LMC_DRIVER_MASTER)) {
        update_peer = FALSE;
        goto peer_send_end;
    } 

peer_1st_time:
    /*
    ** First time through this routine 
    **
    ** Make sure our active bit is set, master bit is clear,
    ** clear the bypass mreg bits 
    */
    SET_BIT(loc_reg, LMC_DRIVER_ACTIVE);
    CLR_BIT(loc_reg, LMC_DRIVER_MASTER);
    CLR_BIT(loc_reg, LMC_PIO_BYPASS_ENABLE);
    CLR_BIT(loc_reg, LMC_DMA_BYPASS_ENABLE);
    btk_put_io(unit_p, BT_LOC_MREG_CTRL, loc_reg);

    /*
    ** Check if remote is master
    */
    loc_reg = btk_get_io(unit_p, BT_LOC_MREG_CTRL);
    rem_reg = btk_get_io(unit_p, BT_REM_MREG_CTRL);
    if (IS_SET(rem_reg, LMC_DRIVER_MASTER)) {
        goto peer_send_end;
    } 

    /*
    ** Here is the difficult case:  No one is master, so we try
    ** to set our master bit by grabing both local and remote
    ** semaphore 7 bits and then setting our master bit if remote
    ** master bit is clear
    */
    /*
    ** Get both semaphores
    */
    got_local = FALSE;
    count2 = 0;
    while (!got_both) {

        /*
        ** Try to get local
        */
        count1 = 0;
        while (!got_local) {
            btk_put_io(unit_p, BT_LOC_SEMA7, BT_SEMA_TAKE);
            tmp_reg = btk_get_io(unit_p, BT_LOC_SEMA7);
            if (IS_SET(tmp_reg, BT_SEMA_TAKE)) {
                got_local = TRUE;
                break;
            } else if  (count1++ > 1000) {
                TRC_STR((BT_TRC_CFG), "Could not get the local semaphore");
                goto peer_send_end;
            } else {
                btk_delay(10);
                continue;
            }
        }

        /*
        ** Got the local, now try to get the remote
        */
        btk_put_io(unit_p, BT_REM_SEMA7, BT_SEMA_TAKE);
        tmp_reg = btk_get_io(unit_p, BT_REM_SEMA7);

        /*
        ** Got the remote
        */
        if (IS_SET(tmp_reg, BT_SEMA_TAKE)) {
            got_both = TRUE;
            break;

        /* 
        ** Unable to get the Remote semaphore
        */
        } else if (count2++ > 200) {
            INFO_STR("Could not get the remote semaphore");
            goto peer_send_end;
        /*
        ** Remote busy, try again
        */
        } else {
            got_local = FALSE;
            btk_put_io(unit_p, BT_LOC_SEMA7, BT_SEMA_GIVE);
            btk_delay(5);
            continue;
        }
    }

    /*
    ** Got local and remote semas; sample remote master bit and
    ** either exit if set or set our local master bit
    */
    rem_reg = btk_get_io(unit_p, BT_REM_MREG_CTRL);
    if (IS_CLR(rem_reg, LMC_DRIVER_MASTER)) {
        SET_BIT(loc_reg, LMC_DRIVER_MASTER);
        btk_put_io(unit_p, BT_LOC_MREG_CTRL, loc_reg);
    }
    btk_put_io(unit_p, BT_LOC_SEMA7, BT_SEMA_GIVE);
    btk_put_io(unit_p, BT_REM_SEMA7, BT_SEMA_GIVE);


peer_send_end:
    /*
    ** Update the BT_MASTER, BT_PEER2PEER & BT_LOOPBACK bits
    */
    loc_reg = btk_get_io(unit_p, BT_LOC_MREG_CTRL);
    rem_reg = btk_get_io(unit_p, BT_REM_MREG_CTRL);
    if (IS_SET(loc_reg, LMC_DRIVER_MASTER)) {
        SET_BIT(unit_p->bt_status, BT_MASTER);
    } else {
        CLR_BIT(unit_p->bt_status, BT_MASTER);
    }
    
    /*
    ** Update the BT_LOOPBACK bit
    */
    if ((IS_SET(loc_reg, LMC_DRIVER_MASTER) && 
         IS_SET(rem_reg, LMC_DRIVER_MASTER)) ||
        (IS_CLR(loc_reg, LMC_DRIVER_MASTER) && 
         IS_CLR(rem_reg, LMC_DRIVER_MASTER))) {
        SET_BIT(unit_p->bt_status, BT_LOOPBACK);
    } else {
        CLR_BIT(unit_p->bt_status, BT_LOOPBACK);
    }

    /*
    ** Update the BT_PEER2PEER bit
    */
    if (IS_CLR(unit_p->bt_status, BT_LOOPBACK) &&
        IS_SET(rem_reg, LMC_DRIVER_ACTIVE)) {
        SET_BIT(unit_p->bt_status, BT_PEER2PEER);
    } else {
        CLR_BIT(unit_p->bt_status, BT_PEER2PEER);
    }

    /*
    ** If we are in a peer to peer setup tell the other
    ** driver we are up;
    */
    if (IS_SET(unit_p->bt_status, BT_PEER2PEER) && update_peer) {
        (void) btk_send_irq(unit_p, BT_DRV_VECTOR_PEER2PEER, TRUE);
    }
    
    TRC_MSG(BT_TRC_OPEN, (LOG_FMT "%s; %s; %s\n", LOG_ARG, 
            IS_SET(unit_p->bt_status, BT_MASTER) ? "Master" : "Slave",
            IS_SET(unit_p->bt_status, BT_PEER2PEER) ? "Peer" : "Single",
            IS_SET(unit_p->bt_status, BT_LOOPBACK) ? "Loopback" : "No loopback"));
    FEXIT(0);
    return;
}

/******************************************************************************
**
**      Function:       btk_take_drv_sema()
**
**      Purpose:        Get the shared semaphore if necessary based on
**                      the master and peer-to-peer bits.
**
**      Args:           unit_p      Pointer to unit structure.
**
**      Returns:        BT_SUCCESS  Semaphore taken
**                      BT_EBUSY    Timeout
**
**      Notes:          btk_sleep() is used so we do not busy wait the 
**                      processor.  Must be very careful as to what locks
**                      are held when ths routine is called.  Refer to 
**                      btk_sleep() docs for OS specific info.
**
******************************************************************************/
bt_error_t btk_take_drv_sema(
    bt_unit_t *unit_p)
{
#if defined( HARDWARE_SEMAPHORE_PROTOCOL_FIXED_ASK_MCMILLAN_AND_RALPH )
    bt_error_t    retval = BT_SUCCESS;
    bt_data32_t   tmp_reg;
    int           timeout = 0, got_sema = FALSE;
    bt_reg_t      sema;
#endif /* HARDWARE_SEMAPHORE_PROTOCOL_FIXED_ASK_MCMILLAN_AND_RALPH */
  
    FUNCTION("btk_take_drv_sema");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;
    
#if !defined( HARDWARE_SEMAPHORE_PROTOCOL_FIXED_ASK_MCMILLAN_AND_RALPH )
    return BT_SUCCESS;
#else
    /*
    ** If were not peer-to-peer, then nothing to do
    */
    if (IS_CLR(unit_p->bt_status, BT_PEER2PEER)) {
        TRC_STR((BT_TRC_KLIB|BT_TRC_DETAIL), 
                 "Sema take single driver situtation");
        goto take_end;
    }

    /*
    ** If we are master get the local if we not get the 
    ** remote sema
    */
    if (IS_SET(unit_p->bt_status, BT_MASTER)) {
        sema = BT_LOC_SEMA7;
    } else {
        sema = BT_REM_SEMA7;
    }

    /*
    ** Get the semaphore, can wait forever if something gets
    ** screwed up and remote driver exits with control of sema
    **
    ** btk_sleep() is used so we do not busy wait the processor
    ** Must be very careful as to what locks are held when
    ** ths routine is called.  Refer to btk_sleep() docs for
    ** OS specific info
    **
    ** Better way is to have remote driver send interrupt and have
    ** our ISR wake up this task when remote releases sema. Schedule
    ** will not permit this.
    **
    ** limiting sleep at 1 second
    */
    while (!got_sema && (timeout < 100)) {
        btk_put_io(unit_p, sema, BT_SEMA_TAKE);
        tmp_reg = btk_get_io(unit_p, sema);
        if (IS_SET(tmp_reg, BT_SEMA_TAKE)) {
            got_sema = TRUE;
            break;
        } else {
            btk_event_wait(&unit_p->dma_event, btk_msec2tck(10));
            timeout++;
            btk_put_io(unit_p, sema, BT_SEMA_TAKE);
            tmp_reg = btk_get_io(unit_p, sema);
            if (IS_SET(tmp_reg, BT_SEMA_TAKE)) {
                got_sema = TRUE;
                break;
            }
        }
    }

    /*
    ** Did we get the semaphore
    */
    if (!got_sema) {
        retval = BT_EBUSY;
        INFO_STR("Timed out waiting for semaphore");
    }

take_end:
    TRC_MSG(BT_TRC_KLIB, (LOG_FMT "Driver sema take %s\n", LOG_ARG, 
            (retval == BT_SUCCESS) ? "Succeeded" : "Failed"))
    FEXIT(retval);
    return (retval);
#endif
}
  

/******************************************************************************
**
**      Function:       btk_give_drv_sema()
**
**      Purpose:        Release the shared semaphore if necessary based on
**                      the master and peer-to-peer bits.
**
**      Args:           unit_p          Pointer to unit structure.
**
**      Returns:        Void
**
**      Notes:          None
**
******************************************************************************/
void btk_give_drv_sema(
    bt_unit_t *unit_p)
{
  
    FUNCTION("btk_give_drv_sema");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

#if !defined( HARDWARE_SEMAPHORE_PROTOCOL_FIXED_ASK_MCMILLAN_AND_RALPH )
    return;
#else
    /*
    ** If were not peer-to-peer, then nothing to do
    */
    if (IS_CLR(unit_p->bt_status, BT_PEER2PEER)) {
        TRC_STR((BT_TRC_KLIB|BT_TRC_DETAIL), 
                 "Sema give single driver situtation");
        goto give_end;
    } else if (IS_SET(unit_p->bt_status, BT_MASTER)) {
        btk_put_io(unit_p, BT_LOC_SEMA7, BT_SEMA_GIVE);
    } else {
        btk_put_io(unit_p, BT_REM_SEMA7, BT_SEMA_GIVE);
    }
    /*
    ** Tell the other driver the semaphore is avaible
    */
    if (IS_SET(unit_p->bt_status, BT_PEER2PEER)) {
        (void) btk_send_irq(unit_p, BT_DRV_VECTOR_SEMA_RELEASE, TRUE);
    }

give_end:
    TRC_STR(BT_TRC_KLIB, "Driver sema given")
    FEXIT(0);
    return;
#endif
}
  

/******************************************************************************
**
**      Function:       btk_take_app_sema()
**
**      Purpose:        Get a semaphore for the application
**
**      Args:           unit_p      Pointer to unit structure.
**                      sema        Which semaphore to take
**                      wait_msec   # of msec to wait for semaphore
**                                    Only BT_NO_WAIT is supported now
**
**      Returns:        BT_SUCCESS  Semaphore taken
**                      BT_EBUSY    Semaphore already taken
**
**      Notes:          Currently this routine does not wait for 
**                      the semaphore to become avaliable and
**                      therefore is a simple register access. Waiting
**                      should be implemented later.
**
******************************************************************************/
bt_error_t btk_take_app_sema(
    bt_unit_t *unit_p,
    bt_reg_t sema,
    bt_msec_t wait_msec)
{
    bt_error_t    retval = BT_SUCCESS;
    bt_data32_t   tmp_reg;
  
    FUNCTION("btk_take_app_sema");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /*
    ** Do error checking 
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        INFO_STR("Semaphore only support on Data Blizzard hardware");
        retval = BT_ENOSUP;
        goto app_take_end;
    }
    if (wait_msec != BT_NO_WAIT) {
        INFO_STR("Waiting for semaphore is not currently supported");
        retval = BT_ENOSUP;
        goto app_take_end;
    }
    switch (sema) {
      case BT_LOC_SEMA0:
      case BT_LOC_SEMA1:
      case BT_LOC_SEMA2:
      case BT_LOC_SEMA3:
      case BT_LOC_SEMA4:
      case BT_LOC_SEMA5:
      case BT_LOC_SEMA6:
      case BT_REM_SEMA0:
      case BT_REM_SEMA1:
      case BT_REM_SEMA2:
      case BT_REM_SEMA3:
      case BT_REM_SEMA4:
      case BT_REM_SEMA5:
      case BT_REM_SEMA6:
        break;
      case BT_LOC_SEMA7:
      case BT_REM_SEMA7:
        INFO_STR("Semaphore 7 reserved for driver use");
        retval = BT_EINVAL;
        break;
      default:
        INFO_STR("Register given is not a semaphore");
        retval = BT_EINVAL;
        break;
    }
        
    /*
    ** Try to get the semaphore
    */
    if (retval == BT_SUCCESS) {
        btk_put_io(unit_p, sema, BT_SEMA_TAKE);
        tmp_reg = btk_get_io(unit_p, sema);
        if (IS_CLR(tmp_reg, BT_SEMA_TAKE)) {
            retval = BT_EBUSY;
        }
    }

app_take_end:
    FEXIT(retval);
    return (retval);
}
  

/******************************************************************************
**
**      Function:       btk_give_app_sema()
**
**      Purpose:        Give a semaphore for the application
**
**      Args:           unit_p      Pointer to unit structure.
**                      sema        Which semaphore to give
**
**      Returns:        BT_SUCCESS  Semaphore taken
**                      other       Appropriate error value
**
**      Notes:          Currently this routine does not do anything
**                      when it gives a semaphore.
**
******************************************************************************/
void btk_give_app_sema(
    bt_unit_t *unit_p,
    bt_reg_t sema)
{
    bt_error_t    retval = BT_SUCCESS;
  
    FUNCTION("btk_give_app_sema");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /*
    ** Do error checking 
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        INFO_STR("Semaphore only support on Data Blizzard hardware");
        retval = BT_ENOSUP;
        goto app_give_end;
    }
    switch (sema) {
      case BT_LOC_SEMA0:
      case BT_LOC_SEMA1:
      case BT_LOC_SEMA2:
      case BT_LOC_SEMA3:
      case BT_LOC_SEMA4:
      case BT_LOC_SEMA5:
      case BT_LOC_SEMA6:
      case BT_REM_SEMA0:
      case BT_REM_SEMA1:
      case BT_REM_SEMA2:
      case BT_REM_SEMA3:
      case BT_REM_SEMA4:
      case BT_REM_SEMA5:
      case BT_REM_SEMA6:
        break;
      case BT_LOC_SEMA7:
      case BT_REM_SEMA7:
        INFO_STR("Semaphore 7 reserved for driver use");
        retval = BT_EINVAL;
        break;
      default:
        INFO_STR("Register given is not a semaphore");
        retval = BT_EINVAL;
        break;
    }
        
    /*
    ** Give the semaphore
    */
    if (retval == BT_SUCCESS) {
        btk_put_io(unit_p, sema, BT_SEMA_GIVE);
    }

app_give_end:
    TRC_STR(BT_TRC_KLIB, "Driver sema given")
    FEXIT(0);
    return;
}

/******************************************************************************
**
**      Function:       btk_setup_mreg()
**
**      Purpose:        Inserts the proper address modifier, swap setting,
**                      and function code in the given mreg_value_p.
**
**      Args:           unit_p          Pointer to device dds structure.
**                      ldev            Logical device being accessed
**                      mreg_value_p    Pointer to map reg value to modify
**                      op              Type of operation, DMA, PIO, bind, etc.
**
**      Returns:        void
**
**      Notes:          Only BT_OP_ values of DMA, PIO, MMAP & BIND should be
**                      used with this function.
**
******************************************************************************/
void btk_setup_mreg(
    bt_unit_t *unit_p, 
    bt_dev_t ldev, 
    bt_data32_t *mreg_value_p, 
    bt_operation_t op)
  
{
    FUNCTION("btk_setup_mreg");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    *mreg_value_p &= BT_MREG_ADDR_MASK;
    
    /*
    **  Setup  address modifier 
    */
    switch (op) {
      case BT_OP_DMA:
        *mreg_value_p |= (unit_p->dma_addr_mod[ldev] << BT_MREG_AMOD_SHIFT);
        break;
      case BT_OP_PIO:
        *mreg_value_p |= (unit_p->pio_addr_mod[ldev] << BT_MREG_AMOD_SHIFT);
        break;
      case BT_OP_BIND:
        break;
      case BT_OP_MMAP:
      default:
        *mreg_value_p |= (unit_p->mmap_addr_mod[ldev] << BT_MREG_AMOD_SHIFT);
        break;
    }
    
    /*
    **  Setup swap bits and function code
    */
    *mreg_value_p |= (unit_p->swap_bits[ldev] << BT_MREG_SWAP_SHIFT);
    if (op != BT_OP_BIND) {
        switch (ldev) {
          case BT_AXSLDP:
          case BT_AXSRDP:
            *mreg_value_p |= (BT_MREG_FUNCT_DPORT << BT_MREG_FUNCT_SHIFT);
            break;
          case BT_AXSRI:
            *mreg_value_p |= (BT_MREG_FUNCT_RIO << BT_MREG_FUNCT_SHIFT);
            break;
          case BT_AXSRR:
          case BT_AXSRE:
          case BT_AXS24:
          default:
            *mreg_value_p |= (BT_MREG_FUNCT_RRAM << BT_MREG_FUNCT_SHIFT);
            break;
        } 
    }
  
    FEXIT(0);
    return;
}

/******************************************************************************
**
**      Function:       btk_send_irq()
**
**      Purpose:        Send an interrupt to the other side with the given
**                      vector
**
**      Args:           unit_p          Pointer to device dds structure.
**                      vector          Vector to send
**                      driver          Is this intra-driver communication
**
**      Returns:        BT_SUCCESS      Successful
**                      other           error
**
**      Notes:          
**
******************************************************************************/
bt_error_t btk_send_irq(
    bt_unit_t *unit_p, 
    bt_data32_t vector, 
    bool_t driver)
  
{
    FUNCTION("btk_send_irq");
    LOG_UNIT(unit_p->unit_number);
    int             pt_irq = FALSE;
    bt_data32_t     tmp_reg;
    bt_error_t      ret_val = BT_SUCCESS;
    BTK_LOCK_RETURN_T isr_pl;
    
    FENTRY;
#define IS_PT_BUSY IS_SET(tmp_reg = btk_get_io(unit_p, BT_LOC_CMD1), LC1_SND_PT_INT)
#define IS_PR_BUSY IS_SET(tmp_reg = btk_get_io(unit_p, BT_REM_STATUS), RSR_PR_STATUS)
      
    /*
    ** Determine if we do a PR to PT
    */
    if (IS_SET(unit_p->bt_status, BT_SEND_PT)) {
        pt_irq = TRUE;
    }
    if (driver) {
        goto send_no_lock_end;
    }

    /*
    ** If not peer to peer don't send intra-driver messages
    */
    if (driver &&
        IS_CLR(unit_p->bt_status, BT_PEER2PEER)) {
        TRC_STR((BT_TRC_LOCK | BT_TRC_DETAIL), "Driver message not sent, not peer-to-peer");
        goto send_no_lock_end;
    }

    /*
    ** Verify that the vector is legal apps < BT_DRV_VECTOR_BASE
    ** drivers > BT_DRV_VECTOR_BASE
    */
    if (driver && 
        (vector <= BT_DRV_VECTOR_BASE)) {
        WARN_STR("Attempt to send illegal driver message");
        ret_val = BT_EFAIL;
        goto send_no_lock_end;
    } else if (!driver && 
        (vector >= BT_DRV_VECTOR_BASE)) {
        WARN_STR("Illegal vector to send to remote");
        ret_val = BT_EINVAL;
        goto send_no_lock_end;
    }

    /*
    ** Grap the prg_irq_mutex to prevent two btk_send_irq from
    ** running concurrently
    */
    btk_mutex_enter(&unit_p->prg_irq_mutex);

    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        /* old cards, remote register access must synchronize lock in case dma active */
        LOCK_DMA(unit_p);
    }
    /*
    ** See if the interrupt is busy
    ** 
    ** For applications, if busy return error
    ** For drivers, if busy wait 25 uSec and try again 
    */
    if (pt_irq) {
        if (IS_PT_BUSY) {
            if (!driver) {
                ret_val = BT_EBUSY;
                goto send_end;
            } else {
                btk_delay(25);
                if (IS_PT_BUSY) {
                    WARN_STR("Unable to send intra-driver message");
                    ret_val = BT_EFAIL;
                    goto send_end;
                }
            }
        }
    } else {
        if (IS_PR_BUSY) {
            if (!driver) {
                ret_val = BT_EBUSY;
                goto send_end;
            } else {
                btk_delay(25);
                if (IS_PR_BUSY) {
                    WARN_STR("Unable to send intra-driver message");
                    ret_val = BT_EFAIL;
                    goto send_end;
                }
            }
        }
    }

    /*
    ** If we are dataBlizzard and PCI to PCI, write the vector
    */
    if (IS_SET(unit_p->bt_status, BT_NEXT_GEN) &&
        IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
        btk_put_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_RMREG_PCI_2_CABLE, vector);
    }

    /*
    ** Send the interrupt, lock out ISR since it might try to set
    ** IACK level bits.
    */
    BTK_LOCK_ISR(unit_p, isr_pl); 
    if (pt_irq) {
        btk_put_io(unit_p, BT_LOC_CMD1, LC1_SND_PT_INT);
    } else {
        btk_put_io(unit_p, BT_REM_CMD1, RC1_SND_PR_INT);
    }
    BTK_UNLOCK_ISR(unit_p, isr_pl); 
    

send_end:
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        UNLOCK_DMA(unit_p);
    }
    btk_mutex_exit(&unit_p->prg_irq_mutex);

send_no_lock_end:
    FEXIT(ret_val);
    return(ret_val);
}

