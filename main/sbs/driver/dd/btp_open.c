/******************************************************************************
**
**      Filename:       btp_open.c
**
**      Purpose:        Device open and close entry points for Linux driver.
**
**      Functions:      btp_open(), btp_close()
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $" __DATE__;
#endif  /* LINT */

#include "btdd.h"
#include <linux/module.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0))
#include <linux/smp_lock.h>
#endif

/*
**  Function prototypes of external functions used within
*/
extern void btk_setup(bt_unit_t *unit_p);
void bt_unbind(bt_unit_t *unit_p, bt_dev_t axs_type);

/*
**  Function prototypes of static functions used within
*/
static void queue_clean(bt_unit_t *unit_p, btk_llist_t *qhead_p);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_OPEN_C);


/*****************************************************************************
**
**      Name:           btp_open()
**
**      Purpose:        Open the device driver for access.
**
**      Args:
**          inode_p     Pointer to inode structure
**          file_p      Pointer to file structure
**          
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          <0          Error number
**
**      Notes:
**      The sys_open() routine protects the driver from any race conditions
**      between the module unloading and the open operation.
**
*****************************************************************************/

int btp_open(
    struct inode *inode_p,
    struct file *file_p
    )
{
    int                 ret_val = 0;
    bt_unit_t           *unit_p; 
    bt_dev_t            type;
    bool_t              new_open = FALSE;
    BTK_LOCK_RETURN_T   saved_spl;

    FUNCTION("btp_open");
    LOG_DEVID(file_p);

    FENTRY;

    unit_p = GET_UNIT_PTR(file_p);

    /*
    ** Check for valid unit and on-line status
    */
    if (NULL == unit_p) {
        ret_val = -ENXIO;
        goto open_end;
    }
    type = GET_LDEV_TYPE(file_p);
    TRC_MSG(BT_TRC_OPEN, (LOG_FMT "access type %d.\n", LOG_ARG, type) );

    /*
    ** Check that device is on-line
    */
    if (IS_CLR(unit_p->logstat[type], STAT_ONLINE)) {
        INFO_STR("Logical device is not online.");
        ret_val = -ENXIO;
        goto open_end;
    }

    /* 
     **  Get the open mutex to prevent a race condition from
     **  occurring with the close routine trying to determine if
     **  all devices are close
     */
    btk_mutex_enter(&(unit_p->open_mutex));
    
    /*
    ** Check for first unit open
    */
    if (0 == (BT_IS_OPEN & unit_p->bt_status)) {
        unit_p->bt_status = (unit_p->bt_status & (~BT_ERROR)) | BT_IS_OPEN;
        unit_p->last_error = 0;
        new_open = TRUE;
    }

    /* 
     **  Mark the associated logical unit as in use 
     */
    unit_p->open[type] = TRUE;
    (void)try_module_get(THIS_MODULE);

    /*
    ** Increment open counts
    */
    unit_p->open_cnt[type] += 1;
    unit_p->open_total += 1;

    /*
    ** If first open, initialize device
    */
    if (new_open ||
        IS_CLR(unit_p->bt_status, BT_ONLINE) || 
        IS_SET(unit_p->bt_status, BT_POWER) ) {
        btk_mutex_enter(&unit_p->dma_mutex);
        btk_rwlock_wr_enter(&unit_p->hw_rwlock);
        btk_mutex_enter(&(unit_p->mreg_mutex));
        BTK_LOCK_ISR(unit_p, saved_spl);
        btk_setup(unit_p);
        BTK_UNLOCK_ISR(unit_p, saved_spl);
        btk_mutex_exit(&(unit_p->mreg_mutex));
        btk_rwlock_wr_exit(&unit_p->hw_rwlock);
        btk_mutex_exit(&unit_p->dma_mutex);
    }

    /* 
    **  Print out the bt_status value in detail 
    */
    TRC_MSG((BT_TRC_OPEN | BT_TRC_DETAIL), 
        (LOG_FMT "bt_status = 0x%x\n",
         LOG_ARG, unit_p->bt_status));

    /*
    **  Release the open mutex to allow further open and closes
    */
    btk_mutex_exit(&(unit_p->open_mutex));

open_end:
    FEXIT(ret_val);
    return ret_val;
}


/*****************************************************************************
**
**      Name:           btp_close()
**
**      Purpose:        Close the device driver and do any cleanup.
**
**      Args:
**          inode_p     Pointer to inode structure
**          file_p      Pointer to file structure
**          
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          <0          Error number
**
**      Notes:
**
*****************************************************************************/

int btp_close(
    struct inode *inode_p,
    struct file *file_p
    )
{
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);
    int ret_val = 0;

    bt_dev_t    type;   /* Logical device type */

    FUNCTION("btp_close");
    LOG_DEVID(file_p);

    FENTRY;


    type = GET_LDEV_TYPE(file_p);

    /*
    **  Aquire the open mutex, which prevents further opens or
    **  closes.  This prevents a race condition in that close
    **  is trying to determine whether any devices are open and
    **  open is currently executing.
    */
    btk_mutex_enter(&(unit_p->open_mutex));

    /*
    **  Close this logical device 
    */
    unit_p->open[type] = FALSE;
    TRC_MSG(BT_TRC_OPEN, 
        (LOG_FMT "CLOSE: axs %d; logstat = 0x%x\n",
         LOG_ARG, type, unit_p->logstat[type]));

    unit_p->open_cnt[type] -= 1;
    unit_p->open_total -= 1;

#if defined(DEBUG)
    if (unit_p->open_cnt[type] < 0) {
        WARN_MSG((LOG_FMT "Open count for device %d is %ld.\n", LOG_ARG,
            type, unit_p->open_cnt[type]));
    }
    if (unit_p->open_total < 0) {
        WARN_MSG((LOG_FMT "Open total count is %ld.\n", LOG_ARG,
            unit_p->open_total));
    }
#endif /* defined(DEBUG) */

    /*
    ** See if this is the last open for this logical device
    */
    if (0 == unit_p->open_cnt[type]) {
        bt_unbind(unit_p, type);
    }

    /*
    ** See if this is the last open for this unit
    */
    if (0 == unit_p->open_total) {
        /* 
        **  As no logical devices remain open, close physical device 
        */
        TRC_STR(BT_TRC_OPEN, "No logical devices open, closing unit");
        CLR_BIT(unit_p->bt_status, BT_IS_OPEN);
        
        /* 
        **  Clear out all of the interrupt queues 
        **  Locking out isr is done in queue_clean
        */
        queue_clean(unit_p, &unit_p->icbr_thread_list);
    }
    module_put(THIS_MODULE);

    /* 
    **  Release the open mutex, to allow the unit to be opened again 
    */
    btk_mutex_exit(&(unit_p->open_mutex));

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**  Function:  queue_clean()
**
**  Purpose:   Clear the queue of registered ICBR threads.
**
**  Args:      unit_p      unit pointer
**             qhead_p     Pointer to head of queue.
**
**  Returns:   Void
**
**  Notes:     Meant to be called during unit close
**
******************************************************************************/
static void queue_clean(
  bt_unit_t *unit_p,
  btk_llist_t *qhead_p)
{
  FUNCTION("queue_clean");
  LOG_UNIT(unit_p->unit_number);

  btk_llist_elem_t  *element_p;
  bt_ttrack_t       *thread_p;
  BTK_LOCK_RETURN_T isr_pl;

  FENTRY;

  /*
  ** Loop, freeing each free thread entry found   
  */
  BTK_LOCK_ISR(unit_p, isr_pl);
  while ((element_p = btk_llist_first(qhead_p)) != (btk_llist_elem_t *) NULL) {
    btk_llist_remove(element_p);
    thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);

    TRC_MSG(BT_TRC_OPEN, 
            (LOG_FMT "found dead ICBR thread 0x%x\n",
            LOG_ARG, (int) thread_p->thread_id));

    /*
    ** Destroy the thread event and release the element memory
    */
    btk_event_fini(&thread_p->thread_event);
    btk_llist_elem_destroy(element_p, sizeof(bt_ttrack_t));
  }
  BTK_UNLOCK_ISR(unit_p, isr_pl);

  FEXIT(0);
  return;
}

