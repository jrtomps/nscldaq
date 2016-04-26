/****************************************************************************
**
**      Filename:       bt_setup.c
**
**      Purpose:        PCI Adaptor device driver module.
**                      Initializes adapter.
**
**      $Revision: 2330 $
**
****************************************************************************/
/****************************************************************************
**
**        Copyright (c) 2000-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
****************************************************************************/

#if !defined(LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif	/* !LINT */

/*
**  Include files
*/
#include "btdd.h"

/*
** Routines within this file
*/
void btk_setup(bt_unit_t *unit_p);
bool_t btk_invalid_op(bt_unit_t *unit_p, bt_dev_t type, bt_operation_t op);
bool_t btk_invalid_offset(bt_unit_t *unit_p, bt_dev_t type, bt_data64_t offset, bt_data32_t length);

/*
** External routines
*/
extern bt_error_t btk_setpage(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t ldev_addr, btk_page_t *page_p);
extern bt_data32_t btk_rem_id(bt_unit_t *unit_p);
extern bt_data32_t btp_get_d32(bt_unit_t *unit_p, void *addr_p);
extern void btk_peer2peer_init(bt_unit_t *unit_p);
bt_data32_t btk_get_io(bt_unit_t *unit_p, bt_reg_t reg); 
void btk_put_io(bt_unit_t *unit_p, bt_reg_t reg, bt_data32_t value); 

/*
** Local defines
*/

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_SETUP_C);

  
/******************************************************************************
**
**      Function:       btk_setup()
**
**      Purpose:        Identifies remote card and configure adapter set.
**
**      Args:           unit_p      Unit pointer
**          
**
**      Returns:        void
**
**      Notes:  Must be called with the unit completely locked.
**                Must hold dma_mutex
**                          hw_rwlock
**                          mreg_mutex
**                          BTK_ISR_LOCK
**
******************************************************************************/
void btk_setup( 
  bt_unit_t *unit_p)
{
    FUNCTION("btk_setup");
    LOG_UNIT(unit_p->unit_number);
    
    int                 bad_combo;
    bt_data32_t         tmp_reg;
    
    FENTRY;
    
    /*
    **  Disable interrupts and make sure DMA is off
    */
    btk_put_io(unit_p, BT_LOC_INT_CTRL, 0);
    btk_put_io(unit_p, BT_LDMA_CMD, 0);

    /* 
    **  Clear the unit status bits
    */
    unit_p->bt_status &= ~(BT_DMA_AVAIL | BT_ONLINE | BT_ERROR | BT_POWER);
    unit_p->last_error = 0;
    unit_p->rem_id = BT_PN_UNKNOWN;

    /*
    ** If we do not have a connection no need to go any further
    */
    if (IS_SET(btk_get_io(unit_p, BT_LOC_STATUS), LSR_NO_CONNECT)) {
        SET_BIT(unit_p->bt_status, BT_POWER);
        INFO_STR("Cable disconnected or remote power off");
    } else {

        
        if (unit_p->loc_id == BT_PN_PCI) {
            /* the 614 dosen't have a rem-id register */
            /* so just assume the remote card is also */
            /* a 614. */
            unit_p->rem_id = BT_PN_MB;
        } else { 
            /*
            ** ID the remote card
            ** Note btk_rem_id sets BT_DMA_AVAIL & BT_PCI2PCI
            */
            unit_p->rem_id = (bt_data32_t) btk_rem_id(unit_p);
        }

        /*
        **  Have to check that we didn't mix different designs of Nanobus
        **  DMA interface. Also determine if next generation
        */
        switch (unit_p->loc_id) {
          case BT_PN_PCI_DMA:
            CLR_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_VME_SDMA) &&
                       (unit_p->rem_id != BT_PN_PCI_DMA));
            break;
          case BT_PN_PCI_NODMA:
            CLR_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_PCI_NODMA) &&
                       (unit_p->rem_id != BT_PN_VME_NODMA));
            break;
          case BT_PN_PCI:
            CLR_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_MB) && 
                       (unit_p->rem_id != BT_PN_QBUS));
            break;
          case BT_PN_PCI_FIBER:
            CLR_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_VME_FIBER) &&
                       (unit_p->rem_id != BT_PN_PCI_FIBER));
            break; 
          case BT_PN_PCI_DB:
            SET_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_VME_DB) &&
                           (unit_p->rem_id != BT_PN_PCI_DB));
            break; 
#if  EAS_A64_CODE
/* EAS A64 */
          case BT_PN_PCI_DBA64_RPQ:
            SET_BIT(unit_p->bt_status, BT_NEXT_GEN);
            bad_combo = ((unit_p->rem_id != BT_PN_PCI_DBA64_RPQ) &&
                           (unit_p->rem_id != BT_PN_PCI_DBA64_RPQ));
            break; 
#endif  /* EAS_A64_CODE */
          case BT_PN_UNKNOWN:
          default:
            /* Hmm.  How did we get here? */
            INFO_STR("Unknown card");
            bad_combo = 1;
            break;
        }
        if (bad_combo && 
            (unit_p->rem_id != BT_PN_UNKNOWN)) {
            WARN_MSG((LOG_FMT "Remote card %d not supported for use with PCI card %d\n",
                      LOG_ARG, unit_p->rem_id, unit_p->loc_id));
            unit_p->rem_id = BT_PN_UNKNOWN;
            CLR_BIT(unit_p->bt_status, BT_ONLINE);
        }
    }

    /*
    **  Perform specific initialization based on the remote card's type.
    */
    switch( unit_p->rem_id ) {

      case BT_PN_VME_SDMA:        /* PCI to VMEbus w/slave DMA */
      case BT_PN_VME_FIBER:
      case BT_PN_VME_DB:
        break;

      case BT_PN_PCI_DMA:         /* PCI to PCI mode */
      case BT_PN_PCI_FIBER:
      case BT_PN_PCI_DB:
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_PN_PCI_DBA64_RPQ:
#endif  /* EAS_A64_CODE */
        break;

      case BT_PN_VME_NODMA:       /* PCI to VME no DMA */ 
        break;

      case BT_PN_PCI_NODMA:       /* PCI to VME no DMA */ 
        break;

      case BT_PN_QBUS:            /* Qbus */
      case BT_PN_MB:              /* Multibus I */
        break;

      case BT_PN_PCI:             /* 614/615 */
        break;
    
      case BT_PN_UNKNOWN:         /* Remote not recognized */
      default:
        unit_p->rem_id = BT_PN_UNKNOWN;
        break;
    }

    /*
    ** Setup logical device information
    **
    ** Logical device info that is constant is done in btp_cfg.c
    **
    ** Note: things like swap_bits, pio_amod must be done only
    ** once in btp_cfg.c otherwise user values will be overwritten
    ** everytime the remote is powered off or cable disconnected.
    **
    ** Device's logstat are placed in one of three states
    **   0 -> We know the device does not exist
    **   STAT_ONLINE -> remote id unknown
    **   Approprate flags -> Device exists with these flags
    ** Also note
    **   STAT_READ -> bt_read is allowed
    **   STAT_WRITE -> bt_write is allowed
    **   STAT_MMAP -> bt_mmap is allowed
    **   STAT_DMA -> DMA can be used
    **   STAT_BIND -> bt_bind is allowed
    */
    if (unit_p->rem_id == BT_PN_UNKNOWN) {
        /* Unknown remote device */
#if  EAS_A64_CODE
/* EAS A64 */
        if ((unit_p->loc_id == BT_PN_PCI_DB) ||
            (unit_p->loc_id == BT_PN_PCI_DBA64_RPQ))
#else
        if (unit_p->loc_id == BT_PN_PCI_DB)
#endif  /* EAS_A64_CODE */
        /* Moved the open bracket outside of the the #if statement to improve
         * readability of the the opening and closing braces.
         */
        {

            unit_p->kern_addr[BT_AXSLDP] = unit_p->mreg_p + BT_CMR_DP_OFFSET;
            unit_p->kern_length[BT_AXSLDP] = BT_CMR_DP_SIZE;
            unit_p->logstat[BT_AXSLDP] |= STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_MMAP | STAT_BIND;
        } else {
            unit_p->kern_addr[BT_AXSLDP] = NULL;
            unit_p->logstat[BT_AXSLDP] = 0;
        }
        unit_p->kern_addr[BT_AXSRDP] = NULL;
        unit_p->kern_length[BT_AXSRDP] = 0;
        unit_p->logstat[BT_AXSRDP] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXSRI] = NULL;
        unit_p->kern_length[BT_AXSRI] = 0;
        unit_p->logstat[BT_AXSRI] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXS24] = NULL;
        unit_p->kern_length[BT_AXS24] = 0;
        unit_p->logstat[BT_AXS24] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXSRR] = NULL;
        unit_p->kern_length[BT_AXSRR] = 0;
        unit_p->logstat[BT_AXSRR] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXSRE] = NULL;
        unit_p->kern_length[BT_AXSRE] = 0;
        unit_p->logstat[BT_AXSRE] = STAT_ONLINE;
        
    } else if (IS_SET(unit_p->bt_status, BT_PCI2PCI) &&
               IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
        /* PCI to PCI and next gen product */
        unit_p->kern_addr[BT_AXSLDP] = unit_p->mreg_p + BT_CMR_DP_OFFSET;
        unit_p->kern_length[BT_AXSLDP] = BT_CMR_DP_SIZE;
        unit_p->logstat[BT_AXSLDP] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP | STAT_LOCAL;
        unit_p->kern_addr[BT_AXSRDP] = unit_p->mreg_p + BT_CMR_DP_OFFSET + BT_CMR_REMOTE_OFFSET;
        unit_p->kern_length[BT_AXSRDP] = BT_CMR_DP_SIZE;
        unit_p->logstat[BT_AXSRDP] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP;

        unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRI] = 0;
        unit_p->logstat[BT_AXSRI] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXS24] = NULL;
        unit_p->kern_length[BT_AXS24] = 0;
        unit_p->logstat[BT_AXS24] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRR] = 0;
        unit_p->logstat[BT_AXSRR] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRE] = 0;
        unit_p->logstat[BT_AXSRE] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;

        btk_map_half(unit_p->mmap_aval_p);
      
    } else if (IS_SET(unit_p->bt_status, BT_PCI2PCI) &&
               IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        /* PCI to PCI, but old Nanobus product */
        unit_p->kern_addr[BT_AXSLDP] = NULL;
        unit_p->logstat[BT_AXSLDP] = 0;
        unit_p->kern_addr[BT_AXSRDP] = NULL;
        unit_p->logstat[BT_AXSRDP] = 0;
        unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRI] = 0;
        unit_p->logstat[BT_AXSRI] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXS24] = NULL;
        unit_p->kern_length[BT_AXS24] = 0;
        unit_p->logstat[BT_AXS24] = STAT_ONLINE;
        unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRR] = 0;
        unit_p->logstat[BT_AXSRR] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRE] = 0;
        unit_p->logstat[BT_AXSRE] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        btk_map_half(unit_p->mmap_aval_p);
        
    } else if (IS_CLR(unit_p->bt_status, BT_PCI2PCI) &&
               IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
        /* PCI to VME and next gen product */
        unit_p->kern_addr[BT_AXSLDP] = unit_p->mreg_p + BT_CMR_DP_OFFSET;
        unit_p->kern_length[BT_AXSLDP] = BT_CMR_DP_SIZE;
        unit_p->logstat[BT_AXSLDP] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP | STAT_LOCAL;
        unit_p->kern_addr[BT_AXSRDP] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRDP] = 0;
        unit_p->logstat[BT_AXSRDP] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRI] = 0;
        unit_p->logstat[BT_AXSRI] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXS24] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXS24] = 0;
        unit_p->logstat[BT_AXS24] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRR] = 0;
        unit_p->logstat[BT_AXSRR] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRE] = 0;
        unit_p->logstat[BT_AXSRE] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        btk_map_restore(unit_p->mmap_aval_p);
        
    } else {
        /* PCI to VME, but old Nanobus product */
        unit_p->kern_addr[BT_AXSLDP] = NULL;
        unit_p->logstat[BT_AXSLDP] = 0;
        unit_p->kern_addr[BT_AXSRDP] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRDP] = 0;
        unit_p->logstat[BT_AXSRDP] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRI] = 0;
        unit_p->logstat[BT_AXSRI] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXS24] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXS24] = 0;
        unit_p->logstat[BT_AXS24] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRR] = 0;
        unit_p->logstat[BT_AXSRR] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
        unit_p->kern_length[BT_AXSRE] = 0;
        unit_p->logstat[BT_AXSRE] = STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_DMA | STAT_BIND | STAT_MMAP;
        btk_map_restore(unit_p->mmap_aval_p);
    }

    /*
    ** Setup logical device info for the diagnostic device
    */
    unit_p->kern_addr[BT_DEV_DIAG] = NULL;
    unit_p->kern_length[BT_DEV_DIAG] = 0;
    unit_p->logstat[BT_DEV_DIAG] = STAT_ONLINE | STAT_BIND | STAT_MMAP;

    /*
    **  Initialize any local register values
    */
    btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
    tmp_reg = LBC_NORM_OPER;
    if (IS_SET(unit_p->bt_status, BT_GEMS_SWAP)) {
        SET_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
    }
    btk_put_io(unit_p, BT_LOC_BUS_CTRL, tmp_reg);

    /*
    **  Initialize any remote register values
    */
    if (unit_p->rem_id != BT_PN_UNKNOWN) {
        btk_put_io(unit_p, BT_REM_CMD1, 0);
        btk_put_io(unit_p, BT_REM_CMD2, 0);
        if (IS_SET(unit_p->bt_status, BT_GEMS_SWAP)) {
            tmp_reg = btk_get_io(unit_p, BT_RPQ_REM_LBUS_CTRL);
            if (IS_CLR(tmp_reg, LBC_GEMS_SWAP_ENABLE)) {
                SET_BIT(tmp_reg, LBC_GEMS_SWAP_ENABLE);
                btk_put_io(unit_p, BT_RPQ_REM_LBUS_CTRL, tmp_reg);
            }
        }
    }
    
    /*
    **  Check for status errors
    */
    if (btk_get_io(unit_p, BT_LOC_STATUS) & LSR_ERROR_MASK) {

      /*
      ** Clear the error but kept device off line
      */
        btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
        INFO_STR("Status error found during setup, device kept off-line");
      
    /* 
    **  If remote card is unknown and cable is on, leave device
    **  off-line
    */
    } else if (unit_p->rem_id == BT_PN_UNKNOWN) {
        INFO_STR("remote card unknown, device kept off-line");

    /*
    **  Everything appears ok, put the device on-line
    */
    } else {
        unit_p->bt_status |= BT_ONLINE;

        /*
        ** Handle the case of Peer-to-Peer drivers
        */
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            btk_peer2peer_init(unit_p);
        }

#if defined(__sgi)
        /*
        ** SGI only!
        **
        ** For PCI to VME applications, check to see if remote Dual Port is
        ** installed.  Currently, doing a byte-aligned write to DP causes
        ** the system to panic.  D32 reads work OK.  So during setup we do
        ** a D32 read to try and detect Dual Port.  If we encounter an error,
        ** we assume DualPort doesn't exist and disable it.
        */
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
            btk_page_t            save_page;

            /* 
            ** Clear errors 
            */
            btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
            if (btk_setpage(unit_p, BT_AXSDP, 0, &save_page) == BT_SUCCESS) {

                /*
                ** Do the access
                */
                (void) btp_get_d32(unit_p, save_page.bus_base_p);

                /* 
                ** Flush the interface and check for errors
                ** Disable DP if error
                */
                (void) btk_get_io(unit_p, BT_REM_STATUS);
                if ((btk_get_io(unit_p, BT_LOC_STATUS) & LSR_ERROR_MASK) != 0) {
                  CLR_BIT(unit_p->logstat[BT_AXSDP], STAT_ONLINE);
                  INFO_STR("Failed to detect Dual Port (not installed?) - BT_AXSDP disabled");
                  btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
                }         else {
                  SET_BIT(unit_p->logstat[BT_AXSDP], STAT_ONLINE);
                }

                /* 
                ** Release map regs allocated in btk_setpage()
                */
                if (save_page.mreg_need != 0) {
                    btk_bit_free(unit_p->mmap_aval_p, save_page.mreg_start, save_page.mreg_need);
                }
            } else {
                CLR_BIT(unit_p->logstat[BT_AXSDP], STAT_ONLINE);
                INFO_STR("Attempt to set page register failed- Dual port kept off-line");
            }
        }
#endif /* __sgi */
    }

    /*
    ** Enable interrupts this card
    */
    btk_put_io(unit_p, BT_LOC_INT_CTRL, LIC_EN_INTR);

    FEXIT(0);
    return;
}
  
/******************************************************************************
**
**      Function:       btk_invalid_op()
**
**      Purpose:        Determines whether the given operation is valid
**                      for the given logical device.
**
**      Args:           unit_p      Unit pointer
**                      type        Logical device type
**                      op          Operation (open, close, etc)
**
**      Returns:        TRUE        Invalid operation
**                      FALSE       Valid operation
**
**      Notes:
**
******************************************************************************/
bool_t btk_invalid_op( 
    bt_unit_t *unit_p,
    bt_dev_t type,
    bt_operation_t op)
{
    FUNCTION("btk_invalid_op");
    LOG_UNIT(unit_p->unit_number);

    bool_t              invalid = FALSE;
    BTK_LOCK_RETURN_T   isr_pl;
    
    FENTRY;
    BTK_ASSERT(unit_p != NULL);
    BTK_ASSERT(type < BT_MAX_AXSTYPS);
    BTK_ASSERT(op < BT_MAX_OP);
    
    /*
    ** If type is open or close let it go through
    */
    if ((op == BT_OP_OPEN) || 
        (op == BT_OP_CLOSE)) {
        goto op_end;
    }

    /*
    ** If the remote card is unknown or power is off
    ** Try to clear the problem by setting up the card again
    */
    if ((unit_p->rem_id == BT_PN_UNKNOWN) || 
        IS_SET(unit_p->bt_status, BT_POWER)) {
        
        /*
        ** Prevent PIO and DMA from occuring
        */
        btk_mutex_enter(&(unit_p)->dma_mutex); \
        btk_rwlock_wr_enter(&(unit_p)->hw_rwlock);
        btk_mutex_enter(&(unit_p->mreg_mutex));

        /*
        ** Again lock out the interrupt handler while we do a setup
        */
        BTK_LOCK_ISR(unit_p, isr_pl);
        btk_setup(unit_p);
        BTK_UNLOCK_ISR(unit_p, isr_pl);

        /*
        ** Release the unit lock
        */
        btk_mutex_exit(&(unit_p->mreg_mutex));
        btk_rwlock_wr_exit(&(unit_p)->hw_rwlock); \
        btk_mutex_exit(&(unit_p)->dma_mutex);
    }

    /*
    ** If the remote card is unknown or power is off
    */
    if (((unit_p->rem_id == BT_PN_UNKNOWN) || IS_SET(unit_p->bt_status, BT_POWER)) &&
        (IS_CLR(unit_p->logstat[type], STAT_LOCAL))) {
        INFO_STR("Operation not allowed - Remote card unknown or power is off");
        invalid = TRUE;
        goto op_end;

    /*
    ** If unit is off-line, reject
    */
    } else if (IS_CLR(unit_p->bt_status, BT_ONLINE)) {
        INFO_STR("Operation not allowed - Unit off-line");
        invalid = TRUE;
        goto op_end;

    /*
    ** If logical device is off-line, reject
    */
    } else if (IS_CLR(unit_p->logstat[type], STAT_ONLINE)) {
        INFO_STR("Operation not allowed - Logical device off-line");
        invalid = TRUE;
        goto op_end;

    /*
    ** If reads are suported
    */
    } else if (IS_CLR(unit_p->logstat[type], STAT_READ) &&
               (op == BT_OP_READ)) {
        INFO_STR("Operation not allowed - device doesn't support reads");
        invalid = TRUE;
        goto op_end;

    /*
    ** If writes are suported
    */
    } else if (IS_CLR(unit_p->logstat[type], STAT_WRITE) &&
               (op == BT_OP_WRITE)) {
        INFO_STR("Operation not allowed - device doesn't support writes");
        invalid = TRUE;
        goto op_end;

    /*
    ** If mmapping are suported
    */
    } else if (IS_CLR(unit_p->logstat[type], STAT_MMAP) &&
               (op == BT_OP_MMAP)) {
        INFO_STR("Operation not allowed - device doesn't support mmapping");
        invalid = TRUE;
        goto op_end;

    /*
    ** If bind are suported
    */
    } else if (IS_CLR(unit_p->logstat[type], STAT_BIND) &&
               (op == BT_OP_BIND)) {
        INFO_STR("Operation not allowed - device doesn't support bind");
        invalid = TRUE;
        goto op_end;
    }
    
op_end:
    FEXIT(invalid);
    return invalid;
}
  
/******************************************************************************
**
**      Function:       btk_invalid_offset()
**
**      Purpose:        Determines whether the given offset and length are
**                      valid for the given logical device.
**
**      Args:           unit_p      Unit pointer
**                      type        Logical device type
**                      offset      Offset in logical device
**                      length      Number of bytes to check
**
**      Returns:        TRUE        Invalid offset/length
**                      FALSE       Valid offset/length
**
**      Notes:
**
******************************************************************************/
bool_t btk_invalid_offset( 
    bt_unit_t *unit_p,
    bt_dev_t type,
    bt_data64_t offset,
    bt_data32_t length)
{
    FUNCTION("btk_invalid_offset");
    LOG_UNIT(unit_p->unit_number);

    bool_t          invalid = FALSE;
    bt_data64_t     max_off;
    
    FENTRY;
    BTK_ASSERT(unit_p != NULL);
    BTK_ASSERT(type < BT_MAX_AXSTYPS);
    
    /*
    ** Determine max offset for each possible logical device
    */
    switch (type) {
      case BT_DEV_LDP:
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            max_off = BT_CMR_DP_SIZE;
        } else {
            max_off = 0;
        }
        break;
      case BT_DEV_RDP:
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
                max_off = BT_CMR_DP_SIZE;
            } else {
                max_off = BT_AMOD_DP_MAX;
            }
        } else {
            if (IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
                max_off = 0;
            } else {
                max_off = BT_AMOD_DP_MAX;
            }
        }
        break;
      case BT_DEV_LM:
        max_off = unit_p->lm_size;
        break;
      case BT_DEV_A16:
        max_off = BT_AMOD_A16_MAX;
        break;
      case BT_DEV_A24:
        max_off = BT_AMOD_A24_MAX;
        break;
      case BT_DEV_A32:
      case BT_AXSRE:
#if defined(__vxworks)
        max_off = 0x100000000ULL;
#else /* __vxworks */
#if defined(BT1003)
        max_off = 0xffffffff;
#else
        max_off = 0x100000000;
#endif
#endif /* __vxworks */
        break;
      case BT_DEV_DIAG:
#if defined(__vxworks)
        max_off = 0x100000000ULL;
#else /* __vxworks */
#if defined(BT1003)
        max_off = 0xffffffff;
#else
        max_off = 0x100000000;
#endif
#endif /* __vxworks */
        break;
      default:
        INFO_STR("Invalid logical device");
        invalid = TRUE;
        break;
    }

    /*
    ** Check whether offset is too big
    */
#if defined(BT1003)
    if (max_off == 0xffffffff) {
	    if (offset > (max_off + 1)) {
	        INFO_MSG((LOG_FMT "Logical device offset too large, max 0x%lx\n",
	                  LOG_ARG, (u_long)max_off));
	        invalid = TRUE;
	    } else if (offset > (max_off - length + 1)) {
	        INFO_MSG((LOG_FMT "Requested offset/length would extend past end of device, max 0x%lx\n",
	                  LOG_ARG, (u_long)max_off));
	        invalid = TRUE;
	    }
    } else {
	    if (offset > max_off) {
	        INFO_MSG((LOG_FMT "Logical device offset too large, max 0x%lx\n",
	                  LOG_ARG, (u_long)max_off));
	        invalid = TRUE;
	    } else if (offset > (max_off - length)) {
	        INFO_MSG((LOG_FMT "Requested offset/length would extend past end of device, max 0x%lx\n",
	                  LOG_ARG, (u_long)max_off));
	        invalid = TRUE;
	    }
    }
#else
    if (offset > max_off) {
        INFO_MSG((LOG_FMT "Logical device offset too large, max 0x%lx\n",
                  LOG_ARG, (u_long)max_off));
        invalid = TRUE;
    } else if (offset > (max_off - length)) {
        INFO_MSG((LOG_FMT "Requested offset/length would extend past end of device, max 0x%lx\n",
                  LOG_ARG, (u_long)max_off));
        invalid = TRUE;
    }
#endif
    
    FEXIT(invalid);
    return invalid;
}

/******************************************************************************
**
**      Function:       btk_init_swapping()
**
**      Purpose:        Initializes default swapping modes.
**
**      Args:           unit_p      Unit pointer
**
**      Returns:        TRUE        Success
**                      FALSE       
**
**      Notes:
**
******************************************************************************/

int btk_init_swapping (bt_unit_t * unit_p)
{
    FUNCTION("btk_init_swapping");
    LOG_UNIT(unit_p->unit_number);
    
    int     inx = 0;
    int     ret_val = 0;

    FENTRY;
    
    /*
    **  Setup swapping bits for logical unit_p->dev_vertexs depending on what 
    **  type of Adaptor we have
    */
    for (inx = 0; inx < BT_MAX_AXSTYPS; inx++) 
    {
        switch (unit_p->rem_id) 
        {
            /*
             * The remote side is a PCI bus based card.
             */
            case BT_PN_PCI_DMA:
            case BT_PN_PCI_NODMA:
            case BT_PN_PCI_FIBER:
            case BT_PN_PCI_DB:

#if  EAS_A64_CODE /* EAS A64 */
            case BT_PN_PCI_DBA64_RPQ:
#endif  /* EAS_A64_CODE */

                unit_p->swap_bits[inx] = BT_SWAP_VMEBUS;

#if defined (BT965) 
                unit_p->window.default_swap = BT_SWAP_VMEBUS;
#elif defined (BT993)
                unit_p->window.default_swap = BT_SWAP_VMEBUS;
#if (CPU_FAMILY == PPC) 
                unit_p->swap_bits[inx] = BT_SWAP_BSBD;
                unit_p->window.default_swap = BT_SWAP_BSBD;
#endif /* (CPU_FAMILY == PPC) */
#endif /* defined(BT965) */

                break;

            /*
             * The remote side is a VMEbus based card.
             */
            case BT_PN_VME_NOINC:
            case BT_PN_VME_NODMA:
            case BT_PN_VME_SDMA:
            /* case BT_PN_VME_SDMA_64:BT_PN_VME_SDMA_64 == BT_PN_VME_FIBER_D64 */
            case BT_PN_VME2_DMA:
            case BT_PN_VME_DMA:
            case BT_PN_VME:
            /* case BT_PN_VME2:  BT_PN_VME2 == BT_PN_VME_NODMA above.     */
            case BT_PN_VME_FIBER:
            case BT_PN_VME_A24:
            case BT_PN_VME_PCI:	/* EXPN may not be supported by this driver. */
            case BT_PN_VME64:	/* 2866 may not be supported by this driver. */
            case BT_PN_VME_NBDG:/* NBDG may not be supported by this driver. */
            case BT_PN_VME_DB:
                unit_p->swap_bits[inx] = BT_SWAP_VMEBUS;

#if defined (BT965) 
                unit_p->window.default_swap = BT_SWAP_VMEBUS;
#elif defined (BT993)
                unit_p->window.default_swap = BT_SWAP_VMEBUS;
#endif

                break;

            /*
             * The remote side is a Qbus based card.
             */
            case BT_PN_QBUS:
                unit_p->swap_bits[inx] = BT_SWAP_QBUS;

#if defined (BT965) 
                unit_p->window.default_swap = BT_SWAP_QBUS;
#elif defined (BT993)
                unit_p->window.default_swap = BT_SWAP_QBUS;
#endif

                break;

            /*
             * The remote side is a Multibus based card.
             */
            case BT_PN_MB:
                unit_p->swap_bits[inx] = BT_SWAP_MULTIBUS;

#if defined (BT965)
                unit_p->window.default_swap = BT_SWAP_MULTIBUS;
#elif defined (BT993)
                unit_p->window.default_swap = BT_SWAP_MULTIBUS;
#endif

                break;

            /*
             * The remote side is an unknown bus based card.
             */
            case BT_PN_UNKNOWN:
            default:
            /*
            ** For next gen assume PCI to PCI
            */

#if  EAS_A64_CODE /* EAS A64 */

                if ((unit_p->loc_id == BT_PN_PCI_DB) ||
                    (unit_p->loc_id == BT_PN_PCI_DBA64_RPQ))

#else

                if (unit_p->loc_id == BT_PN_PCI_DB)

#endif  /* EAS_A64_CODE */

                {
                    unit_p->swap_bits[inx] = BT_SWAP_NONE;

#if defined (BT965) 
                    unit_p->window.default_swap = BT_SWAP_NONE;
#elif defined (BT993)
                    unit_p->window.default_swap = BT_SWAP_NONE;
#endif

                } else {
                    unit_p->swap_bits[inx] = BT_SWAP_VMEBUS;

#if defined (BT965) 
                    unit_p->window.default_swap = BT_SWAP_VMEBUS;
#elif defined (BT993)
                    unit_p->window.default_swap = BT_SWAP_VMEBUS;
#if (CPU_FAMILY == PPC) 
                    unit_p->swap_bits[inx] = BT_SWAP_BSBD;
                    unit_p->window.default_swap = BT_SWAP_BSBD;
#endif /* (CPU_FAMILY == PPC) */
#endif /* end defined(BT965) */

                }

                break;

        } /* End switch statement */
    } /* End for loop */

    /* Take care of the local memory device */
    unit_p->swap_bits[BT_DEV_LM] = BT_SWAP_NONE;

    FEXIT(ret_val);
    return ret_val;
}
