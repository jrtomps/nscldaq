/******************************************************************************
**
**    Filename:     btp_icbr.c
**
**    Purpose:      IRIX implementation of ICBRs for NanoBus hardware.       
** 
**    Functions:    bt_icbr_install(), bt_icbr_remove(), bt_icbr_thread()
**
**    $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef    LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$" __DATE__;
#endif    /* LINT */

/* #include    <abi_mutex.h> */
#include    <sys/types.h> 
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>      /* added CAR */
#include    <stdlib.h>      /* added CAR */
#include    <fcntl.h>
#include    <sys/mman.h>

#include    "btapi.h"
#include    "btio.h"
#include    "btpiflib.h"

/*
** Local structures
*/

/*
** Global variables
*/

/*
** Static functions
*/
static void bt_thread_cancel(void *param_p);
static void *bt_icbr_thread(void *param_p);
static void bt_icbr_call(bt_desc_t btd, bt_irq_t irq_type, bt_data32_t vector);
static void bt_irq_parse(bt_desc_t btd, bt_irq_q_t *irq_q_p, bt_data64_t *tail_p, bt_irq_t irq_type);


/******************************************************************************
**
** Function: bt_icbr_install()
**       
** Purpose: Install an ICBR (interrupt call-back routine) that will be called
**          when a particular interrupt type occurs.
**
** Args:    bt_desc_t btd: Descriptor returned by the original call to
**          bt_open().
**
**          bt_irq_t irq_type: Type of interrupt this ICBR handles.
**
**          bt_icbr_t *icbr_p: Address of the ICBR (see comments for ICBR
**          prototype).
**
**          void *param_p: A user defined object that is passed unmodified
**          to the ICBR as a parameter.  This is usually a pointer to a data
**          structure.
**
**          bt_data32_t vector: Vector for selective registration (see
**          comments for more information).
**
** Returns: bt_error_t:
**          BT_SUCCESS   Added ICBR to list
**          BT_ENOMEM    Couldn't allocate memory for list element
**
**
** Comments: The prototype for the application's ICBR is:
**           void app_icbr(void *param_p, bt_irq_t irq_type, bt_data32_t vector);
**
**              The ICBR's param_p parameter is same value passed to
**              bt_icbr_install() in it's param_p parameter.
**
**              The ICBR's irq_type parameter is the interrupt type that
**              actually occurred.  If the ICBR thread's interrupt data
**              queue has overflowed, this will be BT_IRQ_OVERFLOW.  Otherwise,
**              it will be the same type as passed to bt_icbr_install() in it's
**              irq_type parameter.
**
**              The value in the ICBR's vector parameter depends on the
**              irq_type parameter, as follows:
**
**              BT_IRQ_ERROR: Vector will be either a value returned
**              by one of the user installed ISR or BT_ESTATUS.
**
**              BT_IRQ_PROG: Vector will either be zero or the 
**              value returned by one of the user installed ISR.
**
**              BT_IRQ_VME: Vector will zero of the value returned
**              by one of the user installed ISR.
**
**
**          ICBRs execute within their own unique threads within an
**          application.  These are referred to as ICBR threads.
**
**          The interrupt call back will be called if:
**          There is an overflow. 
**
**          The type matchs and the registered vector is BT_VECTOR_ALL
**
**          The type matchs and the registered vector matches the actual
**          vector received.
**
******************************************************************************/
bt_error_t bt_icbr_install(
    bt_desc_t   btd,     /* Unit information */
    bt_irq_t    irq,     /* Interrupt type to register for */
    bt_icbr_t  *func_p,  /* Address of ICBR to call */
    void       *param_p, /* Parameter to pass when calling ICBR */
    bt_data32_t vector   /* Vector to match against */
    )
{
    bt_error_t          retvalue = BT_SUCCESS;
    int                 os_ret = 0;
    bt_icbr_list_t      *new_icbr_p = NULL;
    bt_thread_add_t     thread_add_data;                     

    if (BT_DESC_BAD(btd)) {
        DBG_STR("Bad descriptor");
        retvalue = BT_EDESC;
        goto bt_icbr_ins_end;
    }

    /* 
    ** Check range on interrupt type and validate function pointer 
    */
    if ((0 > irq) || 
        (BT_MAX_IRQ < irq)) {
        DBG_STR("Invalid interrupt type");
        retvalue = BT_EINVAL;
        goto bt_icbr_ins_end;
    }
    if (NULL == func_p) {
        DBG_STR("Null ICBR function");
        retvalue = BT_EINVAL;
        goto bt_icbr_ins_end;
    }
    
    /*
    ** Create the new ICBR list item
    */
    new_icbr_p = (bt_icbr_list_t *) malloc(sizeof(bt_icbr_list_t));
    if (NULL == new_icbr_p) {
        retvalue = BT_ENOMEM;
        goto bt_icbr_ins_end;    
    }
    new_icbr_p->type = irq;
    new_icbr_p->vector = vector;
    new_icbr_p->func_p = func_p;
    new_icbr_p->param_p = param_p;

    /* 
    ** Check if the ICBR thread already exists 
    */
    pthread_mutex_lock(&btd->mutex);
    if (btd->icbr_running == FALSE) {

        /*
        ** Init the thread condition variable and start the thread
        */
        os_ret = pthread_cond_init(&btd->icbr_started, NULL);
        assert(0 == os_ret);
        os_ret = pthread_create(&btd->icbr_thread, NULL, bt_icbr_thread, (void *) btd); 
        if (os_ret < 0) {
            pthread_cond_destroy(&btd->icbr_started);
            DBG_STR("Failed creating ICBR thread");
            switch (os_ret) {
              case EAGAIN: /* system limit */
                  retvalue = BT_EBUSY;
                  break;
              case EINVAL:  /* sp was null and len was less than 8192 */
              case ENOMEM:  /* can't allocate stack */
                  retvalue = os_ret;
                  break;
              default:
                 retvalue = BT_EFAIL;
                 break;
            }
            goto bt_icbr_ins_unlock;
        }

        /*
        ** Wait for the ICBR thread to start
        ** Note btd->mutex is reaquired on return from this function
        */
        os_ret = pthread_cond_wait(&btd->icbr_started, &btd->mutex);
        if (os_ret < 0) {
            pthread_cancel(btd->icbr_thread);
            pthread_cond_destroy(&btd->icbr_started);
            btd->icbr_running = FALSE;
            DBG_STR("Failed waiting for ICBR thread to start");
            retvalue = BT_EFAIL;
            goto bt_icbr_ins_unlock;
        }
        
    }

    /*
    ** Check that the ICBR thread is running
    */
    if (!btd->icbr_running) {
        /* 
        ** ICBR list needs to be cleaned up, let bt_close do it 
        */
        DBG_STR("ICBR thread encountered error");
        retvalue = BT_EFAIL;
        goto bt_icbr_ins_unlock;
    }

    /*
    ** Add this interrupt type to the driver so the ICBR
    ** thread gets called on this type of interrupt
    */
    thread_add_data.irq_type = irq;
    thread_add_data.vector = vector;
    thread_add_data.thread_id = btd->thread_id;
    retvalue = bt_ctrl(btd, BIOC_THREAD_ADD, &thread_add_data);
    if (os_ret != BT_SUCCESS) {
        /*
        ** On failure we allow the ICBR thread to continue to run
        ** because it may be processing other ICBRs
        */
        DBG_STR("Add thread ioctl failed");
        goto bt_icbr_ins_unlock;  
    }

    /*
    ** Now add ICBR to list 
    */
    new_icbr_p->next_p = btd->icbr_head_p;
    btd->icbr_head_p = new_icbr_p;
    
bt_icbr_ins_unlock:
    pthread_mutex_unlock(&btd->mutex);
    if (retvalue != BT_SUCCESS) {
        free(new_icbr_p);
    }

bt_icbr_ins_end:
    return retvalue;
}

/*****************************************************************************
**
**  Function:   bt_icbr_remove()
**
**  Purpose:    Remove an ICBR from the list
**
**  Args:
**    btd       Descriptor to call through
**    irq       Type of interrupt to register for
**    func_p    Address of interrupt call back routine to remove
**                      
**  Modifies:
**
**  Returns:    BT_EINVAL if error, otherwise BT_SUCCESS
**
**  Note:
**    Removes all the ICBR register with this type and fucntion from the 
**    driver's descriptor data.
**
**    Eventhough we may remove all of the ICBRs from the list we
**    allow the ICBR thread to continue to execute until the 
**    bt_close() comes.
**
*****************************************************************************/
bt_error_t bt_icbr_remove(
    bt_desc_t    btd,     /* Unit information */
    bt_irq_t     irq,     /* Interrupt type to register for */
    bt_icbr_t    *func_p  /* Address of ICBR to call */
    )
{
    bt_error_t      retvalue = BT_EFAIL;
    bt_icbr_list_t  *curr_icbr_p = NULL;
    bt_icbr_list_t  *prev_icbr_p = NULL;
    bt_thread_add_t thread_sub_data;

    if (BT_DESC_BAD(btd)) {
        DBG_STR("Bad descriptor");
        retvalue = BT_EDESC;
        goto bt_icbr_rem_end;
    }

    /* 
    ** Check range on interrupt type and validate function pointer 
    */
    if ((0 > irq) || 
        (BT_MAX_IRQ < irq)) {
        DBG_STR("Invalid interrupt type");
        retvalue = BT_EINVAL;
        goto bt_icbr_rem_end;
    }
    if (NULL == func_p) {
        DBG_STR("Null ICBR function");
        retvalue = BT_EINVAL;
        goto bt_icbr_rem_end;
    }

    /* 
    ** Search list for given ICBR
    */
    pthread_mutex_lock(&btd->mutex);
    prev_icbr_p = (bt_icbr_list_t *) &btd->icbr_head_p;
    curr_icbr_p = btd->icbr_head_p;
    while (curr_icbr_p != NULL) {
 
        /* 
        ** Check if we found the one we were looking for 
        */
        if ((irq  == curr_icbr_p->type) &&
            (func_p == curr_icbr_p->func_p) ) {

            /* 
            ** Unlink ICBR entry free it and move on in the list
            */
            if (prev_icbr_p == NULL) {
                btd->icbr_head_p = curr_icbr_p->next_p;
                free(curr_icbr_p);
                curr_icbr_p = btd->icbr_head_p;
            } else {
                prev_icbr_p->next_p = curr_icbr_p->next_p;
                free(curr_icbr_p);
                curr_icbr_p = prev_icbr_p->next_p;
            }
            retvalue = BT_SUCCESS;

            /*
            ** If we found the ICBR in the list, tell the driver to 
            ** delete one instance of this irq type
            */
            thread_sub_data.irq_type = irq;
            thread_sub_data.vector = BT_VECTOR_ALL;
            thread_sub_data.thread_id = btd->thread_id;
            if (bt_ctrl(btd, BIOC_THREAD_DELETE, &thread_sub_data) != BT_SUCCESS) {
                /*
                ** Ignore failures, since the routine was removed
                ** from the ICBR list it will not be called and worst
                ** that will happen is that the ICBR thread will still
                ** be called for this irq type.
                */
                DBG_STR("Delete thread ioctl failed");
            }    
            continue;
        }

        /*
        ** Move to the next item on the list
        ** Note we do not stop until we have found all matching
        ** items since we are not matching vectors there may be
        ** multiple entries to remove
        */
        prev_icbr_p = curr_icbr_p;
        curr_icbr_p = prev_icbr_p->next_p;
    }
    pthread_mutex_unlock(&btd->mutex);

    /*
    ** We return BT_SUCCESS as long as we delete at least on ICBR
    */
bt_icbr_rem_end:
    return retvalue;
}


/*****************************************************************************
**
**    Internal functions
**
*****************************************************************************/
/*****************************************************************************
**
**  Function:   bt_thread_cancel()
**
**  Purpose:    Cleans up after the bt_icbr_thread has been canceled
**
**  Args:       param_p     Pointer to the descriptor
**                      
**  Modifies:   Void
**
**  Returns:    Void
**
**  Note:
**
*****************************************************************************/
static void bt_thread_cancel(
    void *param_p)
{
    bt_desc_t       btd = (bt_desc_t) param_p;
    bt_thread_reg_t thread_unreg_data;

    /*
    ** Get the mutex to prevent bt_close from destroying the descriptor
    */
    pthread_mutex_lock(&btd->mutex);
    
    /*
    ** Release mmap, unregister with driver, and clear running flag
    */
    thread_unreg_data.thread_id = btd->thread_id;
    (void) bt_ctrl(btd, BIOC_THREAD_UNREG, &thread_unreg_data);
    (void) munmap((void *) btd->icbr_mmap_p, btd->icbr_mmap_len);
    (void) close(btd->fd_diag);

    /*
    ** Signal bt_close so it can destroy the descriptor
    */
    pthread_cond_signal(&btd->icbr_started);
    btd->icbr_running = FALSE;
    pthread_mutex_unlock(&btd->mutex);

    return;
}

/*****************************************************************************
**
**  Function:   bt_irq_parse()
**
**  Purpose:    Walks through interrupt queue detecting overflows and
**              calling matching ICBRs 
**
**  Args:       btd         Descriptor
**              irq_q_p     Pointer to irq q to look through
**              tail_p      Pointer to local tail of irq q
**              irq_type    Interrupt type
**                      
**  Modifies:   Void
**
**  Returns:    Void
**
**  Note:
**
*****************************************************************************/
static void bt_irq_parse(
    bt_desc_t btd,
    bt_irq_q_t *irq_q_p,
    bt_data64_t *tail_p,
    bt_irq_t irq_type)
{
    bt_data32_t     vector;

    /*
    ** Until head == tail
    */
    while (irq_q_p->head > *tail_p) {
        /* 
        ** The first thing we do is get a copy of the tail irq data
        ** because it is possible for the driver to overwrite this
        ** at any time.  So we get the data then find out where the
        ** head is and check for overflow
        */
        vector = irq_q_p->queue[(bt_data32_t) (*tail_p) % irq_q_p->length];

        /*
        ** Check for overflow, call each ICBR with BT_IRQ_OVERFLOW, and
        ** reset tail to new head.  
        **
        ** Note since we are using 64-bit heads and tails we are 
        ** ignoring the case where the number of irqs has caused
        ** head or tail or both to wrap
        **
        ** Note that head points to the next open spot, also it
        ** is possible to read a non-overflowed vector from the
        ** queue and then detect an overflow and lose this piece of 
        ** data.  This is better than the reverse which is to 
        ** miss an overflow and possible send the wrong vector
        ** to the user.
        */
        if ((irq_q_p->head - *tail_p) > irq_q_p->length) {
            bt_icbr_call(btd, BT_IRQ_OVERFLOW, 0);
            *tail_p = irq_q_p->head;
            
        /*
        ** No overflow, so send interrupt to ICBRs
        ** Loop through all the ICBRs, calling the ones that match
        */
        } else {
            bt_icbr_call(btd, irq_type, vector);
            *tail_p += 1;
        }
    } /* error irq while loop */

    return;
}

/*****************************************************************************
**
**  Function:   bt_icbr_call()
**
**  Purpose:    Walks through the ICBR list and calls all ICBRs
**              that match the given vector and irq type
**
**  Args:       btd         Descriptor
**              irq_type    Interrupt type
**              vector      Vector
**                      
**  Modifies:
**
**  Returns:    Void
**
**  Note:
**
*****************************************************************************/
static void bt_icbr_call(
    bt_desc_t btd,
    bt_irq_t irq_type,
    bt_data32_t vector)
{
    bt_icbr_list_t  *curr_p;

    /*
    ** Grab the mutex and walk the ICBR list
    */
    pthread_mutex_lock(&btd->mutex);
    curr_p = btd->icbr_head_p;
    while (curr_p != NULL) {
        
        /*
        ** Check for matching irq type and vector
        */
        if ((irq_type == curr_p->type) &&
            ((curr_p->vector == BT_VECTOR_ALL) ||
             (curr_p->vector == vector))) {
            (*(curr_p->func_p))(curr_p->param_p, irq_type, vector);
        } else if (irq_type == BT_IRQ_OVERFLOW) {
            (*(curr_p->func_p))(curr_p->param_p, irq_type, vector);
        }
        curr_p = curr_p->next_p;
    }
    pthread_mutex_unlock(&btd->mutex);

    return;
}

/*****************************************************************************
**
**  Function:   bt_icbr_thread()
**
**  Purpose:    This thread waits for an interrupt, then call all the 
**              the registered ICBRs.
**
**  Args:       param_p    Descriptor to call through
**                      
**  Modifies:
**
**  Returns:
**    void
**
**  Note:
**
*****************************************************************************/
static void *bt_icbr_thread(
    void *param_p)
{
    bt_error_t         api_ret;
    bt_desc_t          btd = param_p;
    bt_thread_wait_t   thread_wait_data;
    bt_thread_reg_t    thread_reg_data;
    bt_data8_t         *irq_mmap_p;
    bt_irq_q_t         *error_irq_p;
    bt_irq_q_t         *prog_irq_p;
    bt_irq_q_t         *vme_irq_p;
    bt_data64_t        error_tail, prog_tail, vme_tail;
    bt_devdata_t       q_size;
    char               devname[BT_MAX_DEV_NAME];
    char               *devname_p = &(devname[0]);

    /*
    ** Grab the descriptor mutex
    */
    pthread_mutex_lock(&btd->mutex);

    /*
    ** Register with the driver
    */
    api_ret = bt_ctrl(btd, BIOC_THREAD_REG, &thread_reg_data);
    btd->thread_id = thread_reg_data.thread_id;
    if (api_ret != BT_SUCCESS) {
        DBG_STR("Thread registration failed");
        pthread_cond_signal(&btd->icbr_started);
        pthread_mutex_unlock(&btd->mutex);
        pthread_exit(NULL);
    }

    /*
    ** Get the interrupt queue length
    */
    (void) bt_get_info(btd, BT_INFO_ICBR_Q_SIZE, &q_size); 

    /*
    ** Open the diagnostic device for mmapping the irq queues
    */
    devname_p = bt_gen_name(thread_reg_data.unit, BT_DEV_DIAG, devname_p, BT_MAX_DEV_NAME);
    btd->fd_diag = open((char *) devname_p, O_RDONLY, 0);
    if (btd->fd_diag == -1) { 
        DBG_STR("Open of diag device failed");
        (void) bt_ctrl(btd, BIOC_THREAD_UNREG, &thread_reg_data);
        pthread_cond_signal(&btd->icbr_started);
        pthread_mutex_unlock(&btd->mutex);
        pthread_exit(NULL);
    }
    
    /*
    ** Memory map the interrupt queues
    */
    irq_mmap_p = (bt_data8_t *) mmap(NULL, BTK_Q_NUM * BTK_Q_SIZE(q_size), PROT_READ, MAP_SHARED, btd->fd_diag, BT_DIAG_ISR_Q_OFFSET);
    if (irq_mmap_p == NULL) {
        DBG_STR("Irq queue mmap failed");
        (void) close(btd->fd_diag);
        (void) bt_ctrl(btd, BIOC_THREAD_UNREG, &thread_reg_data);
        pthread_cond_signal(&btd->icbr_started);
        pthread_mutex_unlock(&btd->mutex);
        pthread_exit(NULL);
    }
    btd->icbr_running = TRUE;
    btd->icbr_mmap_p = irq_mmap_p;
    btd->icbr_mmap_len = BTK_Q_NUM * BTK_Q_SIZE(q_size);

    /*
    ** Register a cleanup function to release mmap, unregister with
    ** driver, and clear running flag
    */
    pthread_cleanup_push(bt_thread_cancel, (void *) btd);

    /*
    ** Setup the interrupt queue pointers and set the tails equal to 
    ** the heads
    */
    BTK_Q_ASSIGN(irq_mmap_p, error_irq_p, prog_irq_p, vme_irq_p, q_size);
    error_tail = error_irq_p->head;
    prog_tail = prog_irq_p->head;
    vme_tail = vme_irq_p->head;

    /*
    ** Signal bt_icbr_install() to continue, must do this after we have
    ** sampled the quue heads, otherwise main program may start and begin
    ** receiving interrupts before we have a valid head pointer, thus 
    ** missing the first couple of interrupts
    */
    pthread_cond_signal(&btd->icbr_started);
    pthread_mutex_unlock(&btd->mutex);
    
    /*
    ** Wait for some interrupts to occur
    */
    thread_wait_data.thread_id = btd->thread_id;
    thread_wait_data.wait_msec = BT_FOREVER;
    while (1) {
        api_ret = bt_ctrl(btd, BIOC_THREAD_WAIT, &thread_wait_data);
        pthread_testcancel();
        if (api_ret != BT_SUCCESS) {
            DBG_STR("Thread wait ioctl failed");
            pthread_exit(NULL);
        }

        /*
        ** Parse the error, programmed & vme irq queues
        */
        if (btd->icbr_head_p != NULL) {
            bt_irq_parse(btd, error_irq_p, &error_tail, BT_IRQ_ERROR);
            bt_irq_parse(btd, prog_irq_p, &prog_tail, BT_IRQ_PRG);
            bt_irq_parse(btd, vme_irq_p, &vme_tail, BT_IRQ_IACK);
        } else {
            error_tail = error_irq_p->head;
            prog_tail = prog_irq_p->head;
            vme_tail = vme_irq_p->head;
        }
    }

    /*
    ** Don't know how we would get here, but call the
    ** cleanup routine
    **
    ** Note the pop() routine must exist otherwise it will not compile
    */
    pthread_cleanup_pop(1);
}

