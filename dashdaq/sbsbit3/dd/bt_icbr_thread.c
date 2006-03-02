/*****************************************************************************
**
**      Filename:   bt_icbr_thread.c
**
**      Purpose:    Shared PCI Data Blizzard interrupt queue and ICBR thread 
**                  related routines.
**
**      Functions:   btk_insert_irq(), btk_wakeup_thread(), btk_irq_qs_init()
**                   btk_irq_qs_fini(), btk_thread_add(), btk_thread_delete(),
**                   btk_thread_register(), btk_thread_unregister(),
**                   btk_thread_wait(), btk_thread_wake()
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/
#if !defined (LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* !LINT */

/*
** Include files
*/
#include "btdd.h"
#if defined(__linux__)
#include <linux/vmalloc.h>
#endif /* defined (__linux__) */

/*
** Exported function prototypes
*/
bt_error_t btk_irq_qs_init(bt_unit_t *unit_p, size_t q_size);
void btk_irq_qs_fini(bt_unit_t *unit_p, size_t q_size);
bt_error_t btk_thread_register(bt_unit_t *unit_p, bt_thread_reg_t *reg_p);
bt_error_t btk_thread_unregister(bt_unit_t *unit_p, bt_thread_reg_t *reg_p);
bt_error_t btk_thread_add(bt_unit_t *unit_p, bt_thread_add_t *add_p);
bt_error_t btk_thread_delete(bt_unit_t *unit_p, bt_thread_add_t *add_p);
void btk_insert_irq(bt_unit_t *unit_p, bt_irq_q_t *irq_q_p,bt_data32_t vector);
void btk_wakeup_thread(bt_unit_t *unit_p, int error_irq, int prog_irq, int vme_irq);
bt_error_t btk_thread_wait(bt_unit_t *unit_p, bt_thread_wait_t *wait_p);
bt_error_t btk_thread_wake(bt_unit_t *unit_p, bt_thread_wait_t *wake_p);

/*
** External function prototypes
*/

/*
** Lcoal function prototypes
*/
static int thread_search(void *current_p, void *find_p);

/*
** External variables
*/

/*
** Local variables here
*/
static bt_data64_t  thread_id_g = 1;
static bt_devaddr_t q_offset_g;
BT_FILE_NUMBER(TRACE_BT_ICBR_THREAD_C);


/******************************************************************************
**
**  Function:   btk_irq_qs_init()
**
**  Purpose:    Create the interrupt queues
**
**  Args:       unit_p      Pointer to unit structure
**              q_size      Size in bytes of each irq queue
**
**  Returns:    NA
**
******************************************************************************/

bt_error_t btk_irq_qs_init(
    bt_unit_t *unit_p,
    size_t q_size)

{

    FUNCTION("btk_irq_qs_init");
    LOG_UNIT(unit_p);

    bt_error_t      retval = BT_SUCCESS;
    caddr_t         q_start_p;

    FENTRY;

    /*
    ** Initialize the interrupt queues (circular buffers)
    ** Used to pass interrupt info to library and ICBRs
    **
    ** We do all three at once that way they will be contigous
    ** this way they can be mmaped once and we do not need to worry
    ** about aligning all three to page boundaries.  
    */
#if defined(__linux__)
    /*
    ** Must use vmalloc so memory mapping works correctly, this
    ** will automatically align to a page boundary.
    */
    if ((q_start_p = vmalloc(BTK_Q_NUM * BTK_Q_SIZE(q_size))) == NULL) {
        WARN_STR("Failed to allocate interrupt queues");
        retval = BT_ENOMEM;
    } else {
        q_offset_g = 0;
        
#elif defined(BT_NTDRIVER)  /* defined (__linux__) */
    /*
    ** Must use HalAllocateCommonBuffer so we can mmap it
    ** Comes page aligned
    */
    if ((q_start_p = (caddr_t) HalAllocateCommonBuffer(unit_p->adapter_object, BTK_Q_NUM * BTK_Q_SIZE(q_size), &unit_p->q_paddr, FALSE)) == NULL) {
        WARN_STR("Failed to allocate interrupt queues");
        retval = BT_ENOMEM;
    } else {
        q_offset_g = 0;

#else /* defined (__linux__) */
    if ((q_start_p = btk_mem_alloc(BTK_Q_NUM * BTK_Q_SIZE(q_size) + BT_SYS_PAGE_SIZE, 0)) == NULL) {
        WARN_STR("Failed to allocate interrupt queues");
        retval = BT_ENOMEM;
    } else {
        if ((q_offset_g = (bt_devaddr_t) q_start_p % BT_SYS_PAGE_SIZE) != 0) {
            q_start_p += q_offset_g;
        }
#endif /* defined (__linux__) */
        BTK_Q_ASSIGN(q_start_p, BTK_Q_USE, unit_p->prog_irq_q_p, unit_p->vme_irq_q_p, q_size);
        unit_p->error_irq_q_p->head = 1; 
        unit_p->error_irq_q_p->length = (bt_data32_t) q_size; 
        unit_p->prog_irq_q_p->head = 2; 
        unit_p->prog_irq_q_p->length = (bt_data32_t) q_size; 
        unit_p->vme_irq_q_p->head = 3; 
        unit_p->vme_irq_q_p->length = (bt_data32_t) q_size;
    }

    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_irq_qs_fini()
**
**  Purpose:    Destroy the ICBR thread queues
**
**  Args:       unit_p      Pointer to unit structure
**              q_size      Size in bytes of each irq queue
**
**  Returns:    NA
**
******************************************************************************/

void btk_irq_qs_fini(
    bt_unit_t *unit_p,
    size_t q_size)

{
    caddr_t q_start_p;

    FUNCTION("btk_irq_qs_fini");
    LOG_UNIT(unit_p);

    FENTRY;

    /*
    ** Release memory for the interrupt queues (circular buffers)
    ** We do all three at once because that is the way they were malloced
    */
    q_start_p = (caddr_t) BTK_Q_USE;
#if defined(__linux__)
     vfree((void *) q_start_p);
#elif defined(BT_NTDRIVER)  /* defined (__linux__) */
    HalFreeCommonBuffer(unit_p->adapter_object, BTK_Q_NUM * BTK_Q_SIZE(q_size), unit_p->q_paddr, (void *) q_start_p, FALSE);
#else /* defined (__linux__) */
    if (q_offset_g != 0) {
        q_start_p -= q_offset_g;
    }
    btk_mem_free((void *) q_start_p, BTK_Q_NUM * BTK_Q_SIZE(q_size) + BT_SYS_PAGE_SIZE);
#endif /* defined (__linux__) */

    FEXIT(0);
    return;
}

/******************************************************************************
**
**  Function:   btk_thread_register()
**
**  Purpose:    Registers an ICBR thead with the driver.  This process 
**              creates a unique id for the thread and places the
**              thread on the thread list.
**
**  Args:       unit_p      Pointer to unit structure
**              add_p       Pointer to the ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Approprate error message
**
******************************************************************************/

bt_error_t btk_thread_register(
    bt_unit_t *unit_p,
    bt_thread_reg_t *reg_p)

{

    FUNCTION("btk_thread_register");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Allocate three thread tracking items
    */
    element_p = btk_llist_elem_create(sizeof(bt_ttrack_t), 0);
    if (element_p == (btk_llist_elem_t *) NULL) {
        INFO_STR("Failed allocating thread tracking element");
        retval = BT_ENOMEM;
    } else {

        /*
        ** Initialize the thread event
        */
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
#if defined(__sun)
        btk_event_init(unit_p, &thread_p->thread_event, FALSE, "Thread event", &unit_p->iblock_cookie);
#elif defined(__linux__)
        btk_event_init(unit_p, &thread_p->thread_event, FALSE, unit_p->hirq_cookie);
#else /* other OS */
        btk_event_init(unit_p, &thread_p->thread_event, FALSE);
#endif /* defined(__sun) */

        /*
        ** All counts should be zero
        */
        thread_p->error_count = 0;
        thread_p->prog_count = 0;
        thread_p->vme_count = 0;
    
        /*
        ** Create the thread_id
        */
        BTK_LOCK_ISR(unit_p, isr_pl);
        thread_p->thread_id = thread_id_g;
        reg_p->thread_id = thread_id_g;
        reg_p->unit = unit_p->unit_number;
        
        /*
        ** NOTE: Do not change to D64++ the powerPC does do auto increment
        ** right for D64
        */
        thread_id_g += 1;

        /*
        ** Add thread to list
        */
        btk_llist_insert_last(&unit_p->icbr_thread_list, element_p);
        BTK_UNLOCK_ISR(unit_p, isr_pl);
	TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld registered.\n", 
	        LOG_ARG, (u_long)thread_p->thread_id));
    }
        
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_thread_unregister()
**
**  Purpose:    Removes the given thread from the thread list
**              and destroys any associated resources.
**
**  Args:       unit_p      Pointer to unit structure
**              add_p       Pointer to the ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Appropraite error message
**
******************************************************************************/

bt_error_t btk_thread_unregister(
    bt_unit_t *unit_p,
    bt_thread_reg_t *reg_p)

{

    FUNCTION("btk_thread_unregister");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Look for the given thread in the error thread list
    */
    BTK_LOCK_ISR(unit_p, isr_pl);
    element_p = btk_llist_find_first(&unit_p->icbr_thread_list, thread_search, (void *) &(reg_p->thread_id));
    if (element_p == (btk_llist_elem_t *) NULL) {
        /* Not in list print error */
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        INFO_STR("Interrupt thread not found in thread list");
        retval = BT_EINVAL;
    } else {
        btk_llist_remove(element_p);
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
        btk_event_fini(unit_p, &thread_p->thread_event);
	TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld unregistered.\n", 
	        LOG_ARG, (u_long)thread_p->thread_id));
        btk_llist_elem_destroy(element_p, sizeof(bt_ttrack_t));
    }
            
        
    FEXIT(retval);
    return(retval);
}


/******************************************************************************
**
**  Function:   btk_thread_add()
**
**  Purpose:    Increments the count for the given thread for the
**              given interrupt type.
**
**  Args:       unit_p      Pointer to unit structure
**              add_p       Pointer to the ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Appropraite error message
**
******************************************************************************/

bt_error_t btk_thread_add(
    bt_unit_t *unit_p,
    bt_thread_add_t *add_p)

{

    FUNCTION("btk_thread_add");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Look for the given thread in the requested list
    */
    BTK_LOCK_ISR(unit_p, isr_pl);
    element_p = btk_llist_find_first(&unit_p->icbr_thread_list, thread_search, (void *) &(add_p->thread_id));
    if (element_p == (btk_llist_elem_t *) NULL) {
        /*
        ** Not in list print error
        */
        INFO_STR("Given thread is not registered");
        retval = BT_EINVAL;
            
    /*
    ** Increment count
    */
    } else {
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
        switch (add_p->irq_type) {
          case BT_IRQ_ERROR:
            thread_p->error_count++;
            break;
          case BT_IRQ_PRG:
            thread_p->prog_count++;
            break;
          case BT_IRQ_IACK:
            thread_p->vme_count++;
            break;
          default:
            INFO_STR("Invalid interrupt type");
            retval = BT_EINVAL;
        }
        TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld added to irq type %d; counts %d %d %d.\n", 
	    LOG_ARG, (u_long)thread_p->thread_id, add_p->irq_type,
            thread_p->error_count, thread_p->prog_count, thread_p->vme_count));
    }
    BTK_UNLOCK_ISR(unit_p, isr_pl);
        
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_thread_delete()
**
**  Purpose:    Decrements the count for the given thread for the
**              given interrupt type.
**
**  Args:       unit_p      Pointer to unit structure
**              add_p       Pointer to the ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Appropraite error message
**
******************************************************************************/

bt_error_t btk_thread_delete(
    bt_unit_t *unit_p,
    bt_thread_add_t *add_p)

{

    FUNCTION("btk_thread_delete");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Look for the given thread in the requested list
    */
    BTK_LOCK_ISR(unit_p, isr_pl);
    element_p = btk_llist_find_first(&unit_p->icbr_thread_list, thread_search, (void *) &(add_p->thread_id));
    if (element_p == (btk_llist_elem_t *) NULL) {
        /*
        ** Not in list print error
        */
        INFO_STR("Given thread is not registered");
        retval = BT_EINVAL;
            
    /*
    ** Increment count
    */
    } else {
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
        switch (add_p->irq_type) {
          case BT_IRQ_ERROR:
            thread_p->error_count--;
            break;
          case BT_IRQ_PRG:
            thread_p->prog_count--;
            break;
          case BT_IRQ_IACK:
            thread_p->vme_count--;
            break;
          default:
            INFO_STR("Invalid interrupt type");
            retval = BT_EINVAL;
        }
        TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld deleted from irq type %d; counts %d %d %d.\n", 
	    LOG_ARG, (u_long)thread_p->thread_id, add_p->irq_type,
            thread_p->error_count, thread_p->prog_count, thread_p->vme_count));
    }
    BTK_UNLOCK_ISR(unit_p, isr_pl);
        
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_thread_wait()
**
**  Purpose:    Waits on the thread_event for next interrupt.
**
**  Args:       unit_p      Pointer to unit structure
**              wait_p      Pointer to the wait ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Appropraite error message
**
******************************************************************************/

bt_error_t btk_thread_wait(
    bt_unit_t *unit_p,
    bt_thread_wait_t *wait_p)

{

    FUNCTION("btk_thread_wait");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Only BT_WAIT_FOREVER ok right now
    */
    if (wait_p->wait_msec != BT_FOREVER) {
        INFO_STR("Only BT_FOREVER currently supported");
        retval= BT_ENOSUP;
        goto wait_end;
    }

    /*
    ** Look for the given thread in the requested list
    */
    BTK_LOCK_ISR(unit_p, isr_pl);
    element_p = btk_llist_find_first(&unit_p->icbr_thread_list, thread_search, (void *) &(wait_p->thread_id));
    if (element_p == (btk_llist_elem_t *) NULL) {
        /*
        ** Not in list print error
        */
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        INFO_STR("Given thread is not registered");
        retval = BT_EINVAL;
            
    /*
    ** Wait for interrupt
    */
    } else {
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
	TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld waiting...\n", 
	        LOG_ARG, (u_long)thread_p->thread_id));
        retval = btk_event_wait(unit_p, &thread_p->thread_event, btk_msec2tck(wait_p->wait_msec));
	TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld woke up\n", 
	        LOG_ARG, (u_long)thread_p->thread_id));
    }

wait_end:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_thread_wake()
**
**  Purpose:    Wakes an ICBR thread so it can be canceled.
**
**  Args:       unit_p      Pointer to unit structure
**              wake_p      Pointer to the wait ioctl structure
**
**  Returns:    BT_SUCCESS  O.K.
**              other       Appropraite error message
**
******************************************************************************/

bt_error_t btk_thread_wake(
    bt_unit_t *unit_p,
    bt_thread_wait_t *wake_p)

{

    FUNCTION("btk_thread_wake");
    LOG_UNIT(unit_p);

    bt_error_t              retval = BT_SUCCESS;
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;
    BTK_LOCK_RETURN_T       isr_pl;

    FENTRY;

    /*
    ** Look for the given thread in the requested list
    */
    BTK_LOCK_ISR(unit_p, isr_pl);
    element_p = btk_llist_find_first(&unit_p->icbr_thread_list, thread_search, (void *) &(wake_p->thread_id));
    if (element_p == (btk_llist_elem_t *) NULL) {
        /*
        ** Not in list print error
        */
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        INFO_STR("Given thread is not registered");
        retval = BT_EINVAL;
            
    /*
    ** Signal the ICBR thread (its going to be canceled)
    */
    } else {
        BTK_UNLOCK_ISR(unit_p, isr_pl);
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);
        btk_event_set(unit_p, &thread_p->thread_event);
	TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Thread id %ld woke up\n", 
	        LOG_ARG, (u_long)thread_p->thread_id));
    }

    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**  Function:   btk_insert_irq()
**
**  Purpose:    Inserts an irq into the given circular queue.
**
**  Args:       unit_p      Pointer to unit structure
**              irq_q_p     Pointer to interrupt q structure to insert on
**              vector      Interrupt vector 
**
**  Returns:    NA
**
******************************************************************************/

void btk_insert_irq(
    bt_unit_t *unit_p,
    bt_irq_q_t *irq_q_p,
    bt_data32_t vector)

{

    FUNCTION("btk_insert_irq");
    LOG_UNIT(unit_p);
    
    volatile bt_data64_t    *head_p = &(irq_q_p->head);
    bt_data32_t             *length_p = &(irq_q_p->length);

    FENTRY;
    
    /*
    ** Insert this interrupt into the queue
    **
    ** Please note that the bt_data32_t type cast is required
    ** Visual C++ does not do 64 bit / and % correctly all the time
    */
    irq_q_p->queue[(bt_data32_t) (*head_p) % *length_p] = vector;
        
    /*
    ** NOTE: Do not change to D64++ the powerPC does do auto increment
    ** right for D64
    */
    (*head_p) += 1;

    TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Vector 0x%x added to irq q\n", 
	    LOG_ARG, vector));

    FEXIT(0);
    return;
}

/******************************************************************************
**
**  Function:   btk_wakeup_thread()
**
**  Purpose:    Goes through the ICBR thread list and wakes those
**              sleeping for current interrupt.
**
**  Args:       unit_p      Pointer to unit structure
**              error_irq   Did an error irq occur
**              prog_irq    Did an programmed irq occur
**              vme_irq     Did an VME irq occur
**
**  Returns:    NA
**
**  Notes:      Must be called with BTK_LOCK_ISR in effect
**
******************************************************************************/

void btk_wakeup_thread(
    bt_unit_t *unit_p,
    int error_irq,
    int prog_irq,
    int vme_irq)

{

    FUNCTION("btk_wakeup_thread");
    LOG_UNIT(unit_p);
    
    btk_llist_elem_t        *element_p;
    bt_ttrack_t             *thread_p;

    FENTRY;
    
    /*
    ** Loop, freeing each free thread entry found   
    */
    element_p = btk_llist_first(&unit_p->icbr_thread_list);
    while (element_p != (btk_llist_elem_t *) NULL) {
        thread_p = (bt_ttrack_t *) btk_llist_elem_data(element_p);

        /*
        ** Check for interrupt match
        */
        if ((error_irq && thread_p->error_count) ||
            (prog_irq && thread_p->prog_count) ||
            (vme_irq && thread_p->vme_count)) {

            /*
            ** Signal thread event
            */
            TRC_MSG(BT_TRC_ICBR, (LOG_FMT "Waking thread id %ld\n", 
                    LOG_ARG, (u_long)thread_p->thread_id));
            btk_event_set(unit_p, &thread_p->thread_event);
        }
        element_p = btk_llist_next(element_p);
    }

    FEXIT(0);
    return;
}

/******************************************************************************
**
**  Function:   thread_search()
**
**  Purpose:    Looks for an list element with a matching thread id.
**
**  Args:       current_p   Pointer to current element.
**              find_p      Pointer to element to find.
**
**  Returns:    0           Match.
**              Otherwise   No match.
**
**  Notes:      Should be used with btk_llist_find_?() routines.
**
******************************************************************************/
/*ARGSUSED0*/
static int thread_search(
  void *current_p, 
  void *find_p)
{
  FUNCTION("thread_search");
  LOG_UNKNOWN_UNIT;

  bt_ttrack_t   *c_p = (bt_ttrack_t *) current_p;
  bt_data64_t  *m_p = (bt_data64_t *) find_p;
  int           found = 1;

  FENTRY;

  /*
  ** Search for matching length and offset
  */
  if (c_p->thread_id == *m_p) { 
      found = 0;
  }

  FEXIT(found);
  return(found);
}

