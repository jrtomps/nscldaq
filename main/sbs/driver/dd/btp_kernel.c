/*****************************************************************************
**
**      Filename:   btp_kern.c
**
**      Purpose:    Model 1003 device driver module for installing and
**                  removing kernel user interrupt service routines.
**              
**      $Revision: 2330 $
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999, 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif  /* !LINT */

#include "btdd.h"

/*
** Export kernel routines
*/
EXPORT_SYMBOL(bt_kmap);
EXPORT_SYMBOL(bt_kunmap);
EXPORT_SYMBOL(bt_rembus_install);
EXPORT_SYMBOL(bt_rembus_remove);
EXPORT_SYMBOL(bt_map_vme);
EXPORT_SYMBOL(bt_access_error);
EXPORT_SYMBOL(bt_do_iack);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_KERNEL_C);


/******************************************************************************
**
**      Function:   bt_kmap()
**
**      Purpose:    Return kernel level information to the user that is 
**                  useful in writting a interrupt handler
**
**      Args:       unit        Unit number.
**                  kmap_p      Pointer to bt_kmap_t structure to fill in.
**
**      Returns:    0               Success.
**                  Otherwise       Error number indicating failure type.
**
******************************************************************************/
int bt_kmap(
    unsigned int unit, 
    bt_kmap_t *kmap_p)
{
    FUNCTION("bt_kmap");
    
    bt_unit_t           *unit_p;
    unsigned int        mr_start;
    int                 ret_val = BT_SUCCESS;

    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** Check to see if the unit number and kmap structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -ENXIO;
        goto bt_kmap_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    if (kmap_p == NULL) {
        TRC_STR(BT_TRC_INFO, "Null bt_kmap_t structure passed in.\n");
        ret_val = -EINVAL;
        goto bt_kmap_end;
    }

    /*
    ** Allocate a page of remote memory space and the associated mapping
    ** register for use by the user's interrupt service routine
    */
    btk_mutex_enter(&unit_p->mreg_mutex);
    ret_val = btk_bit_alloc(unit_p->mmap_aval_p, unit_p->mr_page,
                            unit_p->mr_page, &mr_start);
    btk_mutex_exit(&unit_p->mreg_mutex);
    if (BT_SUCCESS != ret_val) {
        TRC_STR(BT_TRC_INFO, "Could not map remote: Not enough mapping registers.\n");
        ret_val = -ENOMEM;
        goto bt_kmap_end;
    }

    /*
    ** Initialize kmap structure for the user 
    */
    kmap_p->nio_p = (caddr_t) unit_p->csr_p;
    kmap_p->mreg_p = unit_p->mreg_p + (mr_start * 4);
    kmap_p->rmem_p = unit_p->rmem_p + (mr_start * BT_PAGE_SIZE);
    kmap_p->loc_id = unit_p->loc_id;
    kmap_p->rem_id = unit_p->rem_id;
    kmap_p->nio_len = MAX_IOREGS;
    kmap_p->mreg_start = mr_start;
    kmap_p->mreg_len = unit_p->mr_page;
    kmap_p->rmem_len = unit_p->mr_page * BT_PAGE_SIZE;
    kmap_p->default_swap = 0&0&0;

bt_kmap_end:
    FEXIT(ret_val);
    return(ret_val);
}


/******************************************************************************
**
**      Function:   bt_kunmap()
**
**      Purpose:    Releases any driver or system resources in the given kmap.
**
**      Args:       unit        Unit number.
**                  kmap_p      Pointer to bt_kmap_t structure.
**
**      Returns:    0               Success
**                  Otherwise       Error number indicating problem.
**
******************************************************************************/
int  bt_kunmap(
    unsigned int unit, 
    bt_kmap_t *kmap_p)
{
    FUNCTION("bt_kunmap"); 

    bt_unit_t       *unit_p;
    int             ret_val = BT_SUCC;

    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** Check to see if the unit number and kmap structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -ENXIO;
        goto bt_unkmap_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    if (kmap_p == NULL) {
        TRC_STR(BT_TRC_INFO, "Null bt_kmap_t structure passed in.\n");
        ret_val = -EINVAL;
        goto bt_unkmap_end;
    }
    
    /*
    ** Allocate a page of remote memory space and the associated mapping
    ** register for use by the user's interrupt service routine
    */
    btk_mutex_enter(&unit_p->mreg_mutex);
    ret_val = btk_bit_free(unit_p->mmap_aval_p, kmap_p->mreg_start, 
                           kmap_p->mreg_len);
    btk_mutex_exit(&unit_p->mreg_mutex);
    if (BT_SUCCESS != ret_val) {
        TRC_STR(BT_TRC_INFO, "Could not release remote mapping.\n");
        ret_val = -EIO;
        goto bt_unkmap_end;
    }

    /*
    ** Zero out kmap structure 
    */
    BTK_BZERO(kmap_p, sizeof(bt_kmap_t));

bt_unkmap_end:
    FEXIT(ret_val);
    return(ret_val);
}


/******************************************************************************
**
**      Function:   bt_rembus_install()
**
**      Purpose:    Support device-specific commands.
**
**      Args:       unit        Unit number.
**                  handler_p   Interrupt handler information.
**
**      Returns:    0               Success.
**                  Otherwise       Error number indicating failure type.
**
**      Notes:      Assumes that interrupt handler is not installed yet (i.e.
**                  does not use clink_lock).
**
******************************************************************************/
int bt_rembus_install(
    unsigned int unit, 
    bt_rembus_intr_t *handler_p)
{
    FUNCTION("bt_rembus_install");
    LOG_UNKNOWN_UNIT;

    btk_llist_elem_t    *llist_element_p;
    btk_llist_t         *llist_p;
    bt_rembus_intr_t    *uisr_p;
    bt_unit_t           *unit_p;
    int                 ret_val = BT_SUCCESS;
    
    FENTRY;

    /*
    ** Check to see if the unit number and rembus structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -ENXIO;
        goto bt_rinstall_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    if ((handler_p == NULL) ||
        (handler_p->interrupt_routine == NULL)) {
        TRC_STR(BT_TRC_INFO, "Invalid bt_rembus_intr_t structure passed in.\n");
        ret_val = -EINVAL;
        goto bt_rinstall_end;
    }

    /*
    ** Verify that the interrupt type is valid
    */
    switch(handler_p->flags) {
      case BT_IRQ_ERROR:
        llist_p = &(unit_p->qh_err_fn);
        SET_BIT(unit_p->bt_status, BT_ER_INTR);
        break;
      case BT_IRQ_PRG:
        llist_p = &(unit_p->qh_prg_fn);
        SET_BIT(unit_p->bt_status, BT_PT_INTR);
        break;
      case BT_IRQ_IACK:
        llist_p = &(unit_p->qh_iack_fn);
        SET_BIT(unit_p->bt_status, BT_RB_INTR);
        break;
      default:
        TRC_STR(BT_TRC_INFO, "Invalid interrupt type.\n");
        ret_val = -EINVAL;
        goto bt_rinstall_end;
    }

    /*
    ** Allocate a new list item to track this uisr
    */
    if ((llist_element_p = btk_llist_elem_create(sizeof(bt_rembus_intr_t), 0)) == NULL) {
        TRC_STR(BT_TRC_INFO, "Could not allocate list element for uisr tracking.\n");
        ret_val = -ENOMEM;
        goto bt_rinstall_end;
    }
    uisr_p = (bt_rembus_intr_t *) btk_llist_elem_data(llist_element_p);
    BTK_BCOPY(handler_p, uisr_p, sizeof(bt_rembus_intr_t));

    /*
    ** Attach the new user ISR on to the end of the list
    */
    btk_llist_insert_last(llist_p, llist_element_p);

bt_rinstall_end:
  FEXIT(ret_val);
  return(ret_val);
}


/******************************************************************************
**
**      Function:   bt_rembus_remove()
**
**      Purpose:    Support device-specific commands.
**
**      Args:       unit        Unit number.
**                  handler_p   Interrupt handler information.
**
**      Returns:    0               Success.
**                  Otherwise       Error number indicating failure type.
**
**      Notes:      Does not currently clear the bits in the unit
**                  'bt_status' flags to indicate when there are no
**                  more interrupt handlers registered.
**
**                  Assumes that interrupt handler is not installed yet (i.e.
**                  does not use clink_lock).
**
******************************************************************************/
int bt_rembus_remove(
    unsigned int unit, 
    bt_rembus_intr_t *handler_p)
{
    FUNCTION("bt_rembus_remove");
    LOG_UNKNOWN_UNIT;

    btk_llist_elem_t    *llist_element_p;
    btk_llist_t         *llist_p;
    bt_rembus_intr_t    *uisr_p;
    bt_unit_t           *unit_p;
    int                 ret_val = BT_SUCCESS;
    
    FENTRY;

    /*
    ** Check to see if the unit number and rembus structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -ENXIO;
        goto bt_rremove_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    if ((handler_p == NULL) ||
        (handler_p->interrupt_routine == NULL)) {
        TRC_STR(BT_TRC_INFO, "Invalid bt_rembus_intr_t structure passed in.\n");
        ret_val = -EINVAL;
        goto bt_rremove_end;
    }

    /*
    ** Verify that the interrupt type is valid
    */
    switch(handler_p->flags) {
      case BT_IRQ_ERROR:
        llist_p = &(unit_p->qh_err_fn);
        break;
      case BT_IRQ_PRG:
        llist_p = &(unit_p->qh_prg_fn);
        break;
      case BT_IRQ_IACK:
        llist_p = &(unit_p->qh_iack_fn);
        break;
      default:
        TRC_STR(BT_TRC_INFO, "Invalid interrupt type.\n");
        ret_val = -EINVAL;
        goto bt_rremove_end;
    }

    /*
    ** Search uisr list for this uisr entry
    */
    llist_element_p = btk_llist_first(llist_p);
    while (llist_element_p != (btk_llist_elem_t *) NULL) {
        uisr_p = (bt_rembus_intr_t *) btk_llist_elem_data(llist_element_p);
        if ((uisr_p != NULL) &&
            (uisr_p->flags == handler_p->flags) &&
            (uisr_p->level == handler_p->level) &&
            (uisr_p->interrupt_routine== handler_p->interrupt_routine) &&
            (uisr_p->param == handler_p->param)) {
            break;
        }
        llist_element_p = btk_llist_next(llist_element_p);
        
    }

    /*
    ** If we found it detach it from the list and destroy it
    */
    if (llist_element_p != NULL) {
        btk_llist_remove(llist_element_p);
        btk_llist_elem_destroy(llist_element_p, sizeof(bt_rembus_intr_t));
    } else {
        TRC_STR(BT_TRC_INFO, "Could find matching uisr to unregister.\n");
        ret_val = -EINVAL;
    }
    
bt_rremove_end:
    FEXIT(ret_val);
    return(ret_val);
}

/******************************************************************************
**
**      Function:   bt_map_vme()
**
**      Purpose:    Return kernel level information to the user that is 
**                  useful in writting a interrupt handler
**
**      Args:       unit        Unit number.
**                  kmap_p      Pointer to bt_kmap_t structure
**                  device      VMEbus device (BT_DEV_A16, A24, A32)
**                  vme_addr    VMEbus address
**                  swap        Swapping bity to use
**                                BT_SWAP_DEFAULT use whatever is in INFO
**                                parameter
**
**      Returns:    Non-NULL        Success.
**                  NULL            Mapping failed.
**
******************************************************************************/
void *bt_map_vme(
    unsigned int unit, 
    bt_kmap_t *kmap_p, 
    bt_dev_t device, 
    bt_data32_t vme_addr, 
    bt_swap_t swap)
{
    FUNCTION("bt_map_vme");
    
    bt_unit_t           *unit_p;
    int                 inx;
    bt_data32_t         mreg_value;
    void                *ret_val_p = NULL;

    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** Check to see if the unit number and kmap structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        goto bt_map_vme_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    if (kmap_p == NULL) {
        TRC_STR(BT_TRC_INFO, "Null bt_kmap_t structure passed in.\n");
        goto bt_map_vme_end;
    }

    /*
    ** Program up the mapping registers
    */
    mreg_value = 0;
    btk_setup_mreg(unit_p, device, &mreg_value, BT_OP_PIO);
    mreg_value &= ~BT_MREG_ADDR_MASK;
    mreg_value |= (vme_addr & BT_MREG_ADDR_MASK);
    if (swap != BT_SWAP_DEFAULT) {
        mreg_value |= (swap << BT_MREG_SWAP_SHIFT);
    }
    for (inx = 0; inx < kmap_p->mreg_len; inx++) {
        btk_put_mreg(unit_p, inx, BT_LMREG_PCI_2_CABLE, mreg_value);
        mreg_value += BT_PAGE_SIZE;
    }

    /*
    **  Return pointer
    */
    ret_val_p = kmap_p->rmem_p;

bt_map_vme_end:
    FEXIT(ret_val_p);
    return(ret_val_p);
}

/******************************************************************************
**
**      Function:   bt_access_error()
**
**      Purpose:    Return kernel level information to the user that is 
**                  useful in writting a interrupt handler
**
**      Args:       unit        Unit number.
**
**      Returns:    0               Success.
**                  Otherwise       Adaptor error found.
**
******************************************************************************/
int bt_access_error(
    unsigned int unit)
{
    FUNCTION("bt_access_error");
    
    bt_unit_t           *unit_p;
    int                 ret_val = BT_SUCCESS;

    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** Check to see if the unit number and kmap structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -ENXIO;
        goto bt_access_error_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);

    /*
    ** Check for errors
    */
    if (btk_get_io(unit_p, BT_LOC_STATUS) & BT_STATUS_MASK) {
        TRC_STR(BT_TRC_ISR_ERR, "Adapter error found durning user ISR.\n");
        ret_val = BT_ESTATUS;
    }

bt_access_error_end:
    FEXIT(ret_val);
    return(ret_val);
}

/******************************************************************************
**
**      Function:   bt_do_iack()
**
**      Purpose:    Do the VMEbus iack and return the vector
**
**      Args:       unit        Unit number.
**                  vme_level   VMEbus level to IACK
**                  vector_p    Vector retrieved from the VME bus
**
**      Returns:    0               Success.
**                  Otherwise       Error number indicating failure type.
**
******************************************************************************/
int bt_do_iack(
    unsigned int unit,
    int vme_level,
    bt_data32_t *vector_p)
{
    FUNCTION("bt_do_iack");
    
    bt_unit_t           *unit_p;
    int                 ret_val = BT_SUCCESS;
    bt_data32_t         tmp_vector;

    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** Check to see if the unit number and kmap structures are valid
    */
    if ((unit_p = bt_unit_array_gp[unit]) == NULL) {
        TRC_STR(BT_TRC_INFO, "Unit number is not currently defined.\n");
        ret_val = -BT_EIO;
        goto bt_do_iack_end;
    }
    if (vector_p == NULL) {
        TRC_STR(BT_TRC_INFO, "Vector pointer NULL.\n");
        ret_val = -BT_EIO;
        goto bt_do_iack_end;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);

    /* 
    ** Do the VMEbus IACK cycle 
    */
    btk_put_io(unit_p, BT_REM_CMD1, vme_level);
    tmp_vector = btk_get_io(unit_p, BT_REM_IACK);
    if (0xff00ul == (0xff00ul & tmp_vector)) {
        /* Adjust for 8-bit vectors */
        tmp_vector &= ~0xff00ul;
    }
    *vector_p = tmp_vector;

bt_do_iack_end:
    FEXIT(ret_val);
    return(ret_val);
}
