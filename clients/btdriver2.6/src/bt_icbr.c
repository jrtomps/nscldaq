/*****************************************************************************
**
**      Filename:   bt_icbr.c
**
**      Purpose:    This example program uses the bt_icbr_install() and
**                  bt_icbr_remove() to test the receiving of error
**                  interrupts.
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-1999 by SBS Technologies, Inc.
**        Copyright (c) 1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* LINT */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

#include "btapi.h"

/*****************************************************************************
**
**      Global Data Types
**
*****************************************************************************/
/* This is the data we get from an interrupt */
typedef struct {
    bt_irq_t    m_irq_type;
    bt_error_t  m_status;
    bt_data32_t m_vector;
} icbr_data_t;

/* used to communicate between the ICBR and the main program. */
typedef struct {
    /* m_ is prepended to prevent name conflicts. */
    volatile icbr_data_t   *m_queue_p;
    int                     m_queue_len;
    volatile unsigned int  *m_head_p;
    bt_desc_t               m_btd;
} icbr_state_t;

volatile sig_atomic_t signal_happened = 0;  /* set to 1 in signal handler */

/*****************************************************************************
**
**      Defines
**
*****************************************************************************/
#define QUEUE_LEN 64
#define DEF_UNIT   0   /* default unit number */

/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/
static void usage(const char *prog_p);
static void icbr(void *param_p, bt_irq_t irq_type, bt_data32_t vector);
static void queue_insert(volatile icbr_data_t *queue_p,
                         volatile unsigned int *head_p,
                         int queue_len, icbr_data_t new_data);
static int queue_remove(volatile icbr_data_t *queue_p,
                        volatile unsigned int *head_p,
                        unsigned int *tail_p, int queue_len,
                        icbr_data_t *data_p);
void signal_handler (int signum);


/*****************************************************************************
**
**      Program:    bt_icbr
**
**      Purpose:    This example program uses the bt_icbr_install() and
**                  bt_icbr_remove() to test the receiving of error
**                  interrupts.
**
**      Args:
**          -               Displays the command summary.
**          -u  <unit>      Unit Number to open.  Default is unit 0.
**
*****************************************************************************/
int main (
    int             argc,
    const char    **argv
    )
{
    int             unit        = DEF_UNIT;     /* Unit number to open */
    const char     *name_p      = NULL;         /* Program name */
    const char     *temp_p      = NULL;         /* Detect errors in strtol */
    bt_desc_t       btd;                        /* Device descriptor */
    char            devname[BT_MAX_DEV_NAME];   /* Device name */
    bt_error_t      status;                     /* SBS Bit 3 API return value */
    int             main_ret    = EXIT_SUCCESS; /* Return value for main() */
    bt_irq_t        irq_type    = BT_IRQ_ERROR; /* IRQ type to register for */

    /* Data queue for communication between the ICBR and the main program */
    volatile icbr_data_t queue[QUEUE_LEN];

    volatile unsigned int   head = 0;   /* queue head pointer */
    unsigned int            tail = 0;   /* queue tail pointer */
    icbr_data_t             new_data;   /* data removed from the queue */
    icbr_state_t            icbr_state; /* icbr state */

    /* Original signal handlers */
    void (*old_sigabrt)(int)    = NULL; /* Old SIGABRT handler */
    void (*old_sigint)(int)     = NULL; /* Old SIGINT handler */
    void (*old_sigterm)(int)    = NULL; /* Old SIGTERM handler */

    /*  parse command line arguments  */
    name_p = argv[0];
    argc--;
    argv++;
    while (argc > 0) {

        /* check to see if it's a command line option.  */
        if ((**argv == '-') && (argv[0][1] != '\0') && (argv[0][2] == '\0')) {

            switch(argv[0][1]) {

              case 'u': /* set unit number */
                if ((argc > 1) && (isdigit(argv[1][0]))) {
                    unit = (int) strtoul(argv[1], (char **) &temp_p, 0);
                    if (*temp_p != '\0') {
                        usage(name_p);
                        main_ret = EXIT_FAILURE;
                        goto exit_main;
                    }
                } else {
                    usage(name_p);
                    main_ret = EXIT_FAILURE;
                    goto exit_main;
                }
                argc--;
                argv++;
                break;

              case 'i':
                if ((argc > 1) && (isdigit(argv[1][0]))) {
                    irq_type = (bt_irq_t) strtoul(argv[1], (char **) &temp_p, 0);
                    if (*temp_p != '\0') {
                        usage(name_p);
                        main_ret = EXIT_FAILURE;
                        goto exit_main;
                    }
                } else {
                  usage(name_p);
                  main_ret = EXIT_FAILURE;
                  goto exit_main;
                }
                argc--;
                argv++;
                break;

              default:
                usage(name_p);
                main_ret = EXIT_FAILURE;
                goto exit_main;

            } /* end switch on arguments */

        } else {
            usage(name_p);
            main_ret = EXIT_FAILURE;
            goto exit_main;
        } /* end else bad argument */

        argc--;
        argv++;
    } /* end while more arguments */

    /* Open the logical device */
    status = bt_open(&btd,
        bt_gen_name(unit, BT_DEV_DEFAULT, devname, BT_MAX_DEV_NAME),
        BT_RDWR);
        
    if (BT_SUCCESS != status) {
        /* It is safe to call bt_perror with any status returned from     */
        /* bt_open() or bt_close() with the device descriptor, despite    */
        /* the fact that the descriptor is not valid to do anything else. */
        bt_perror(btd, status, "Could not open the device");
        main_ret = EXIT_FAILURE;
        goto exit_main;
    }

    /* Register the Interrupt Call Back Routine (ICBR). */
    icbr_state.m_btd = btd;
    icbr_state.m_queue_p = queue;
    icbr_state.m_queue_len = QUEUE_LEN;
    icbr_state.m_head_p = &head;
    status = bt_icbr_install(
        btd,                     /* device descriptor */
        irq_type,                /* interrupt type to install */
        icbr,                    /* pointer to interrupt call back routine */
        (void *) &icbr_state,    /* value to pass to icbr */
        BT_VECTOR_ALL);          /* vector to register */
        
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not install interrupt call back routine");
        /* don't exit here- we still need to close the device */
        main_ret = EXIT_FAILURE;
    } else {

#if defined(BT_BROADCAST_MEMORY)

        /* if we are going to handle write interrupts, enable interrupts */
        /* for netram location zero. */
        if(irq_type == BT_IRQ_WRIRQ)
        {
            status = bt_wrirq_set(btd, 0x00000000, TRUE);
            if (BT_SUCCESS != status) {
                bt_perror(btd, status, "Could not enable write interrupts");
		(void)bt_icbr_remove(btd, irq_type, icbr);
                (void)bt_close(btd);
                main_ret = (EXIT_FAILURE);
                goto exit_main;
            }
        }

#endif /* BT_BROADCAST_MEMORY */
    
        /* ICBR was successfully installed-                                 */
        /* We have a limited number of things we are gaurenteed to be       */
        /* able to do in ICBR context (bt_chkerr(), bt_clrerr(), and        */
        /* bt_strerror() are the only functions we can count on having).    */
        /* So instead, the ICBR simply stores the information in a queue    */
        /* which the main program then draws the data out of.  Here in the  */
        /* main program, we can do the full suite of I/O without worrying.  */
        /* We exit upon receiving a signal- generally ^C.                   */
        old_sigabrt = signal(SIGABRT, signal_handler);
        old_sigint = signal(SIGINT, signal_handler);
        old_sigterm = signal(SIGTERM, signal_handler);

        if ((old_sigabrt != SIG_ERR) && (old_sigint != SIG_ERR) &&
            (old_sigterm != SIG_ERR)) {

            puts ("Polling for interrupts- hit ^C (send SIGINT) to exit...");
            fflush(stdout);

            signal_happened = 0;
            while (signal_happened == 0) {

#if defined (BT25801)
                /* handle ^C without signal handlers. */
                if(kbhit()) {
                    if(_getch() == 0x03) {
                        signal_happened = 1;
                    }
                }
#endif
                /* poll for an interrupt occuring. */
                switch (queue_remove(queue, &head, &tail, QUEUE_LEN, &new_data))
                    {

                  case 0: /* nothing in the queue */
                      break;

                  case 1: /* data was successfully removed from the queue */

                      /* what we do depends upon what sort of interrupt it
                         was */
                      switch (new_data.m_irq_type) {

                        case BT_IRQ_ERROR:
                            /* Error interrupt: output some error messages */
                            /* and attempt to clear the error.             */
                            puts ("Error interrupt occurred!");
                            bt_perror(btd, new_data.m_status, name_p);

                            status = bt_clrerr(btd);
                            if (BT_SUCCESS!= status) {
                                /* We could not clear the error */
                                puts ("Could not clear the error!");
                                bt_perror(btd, status,
                                          "Could not clear device error");
#if !defined (BT25801)
                                main_ret = EXIT_FAILURE;
                                signal_happened = 1;
#endif
                            }
                            break;
                        
#if !defined (BT25801)
                        case BT_IRQ_OVERFLOW:
                            /* The interrupt queue between the driver and  */
                            /* the ICBR overflowed - interrupts were lost. */
                            /* Print a message. */
                            puts ("ICBR Queue overflow occurred.");
                            break;
#endif
                        default:
                            /* Some other sort of interrupt occured */
                            /* - print out some information on it.  */
                            printf ("IRQ occurred: irq_type = %lu,"
                                " vector = %lu\n",
                                (unsigned long) new_data.m_irq_type,
                                (unsigned long) new_data.m_vector);
                            break;

                      } /* end switch interrupt type */
                      break;

                  case -1:
                      /* The queue between the ICBR and the main program */
                      /* overflowed                                      */
                      puts ("Local Queue overflow occurred.");
                      break;

                } /* end switch queue remove */

            } /* end while getchar */

            /* reset the signal handlers to the original handlers */
            signal(SIGABRT, old_sigabrt);
            signal(SIGINT, old_sigint);
            signal(SIGTERM, old_sigterm);
        } /* if signal installed */

#if defined(BT_BROADCAST_MEMORY)

        /* turn off write interrupts if required. */
        if(irq_type == BT_IRQ_WRIRQ)
        {
            status = bt_wrirq_set(btd, 0x00000000, FALSE);
            if (BT_SUCCESS != status) {
                bt_perror(btd, status, "Could not disable interrupts");
		(void)bt_icbr_remove(btd, irq_type, icbr);
                (void)bt_close(btd);
                main_ret = (EXIT_FAILURE);
                goto exit_main;
            }
        }

#endif /* BT_BROADCAST_MEMORY */

        /* remove the ICBR */
        status = bt_icbr_remove(btd, irq_type, icbr);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not remove the interrupt call back routine");
            main_ret = EXIT_FAILURE;
        }

    } /* end bt_icbr_install was successful */

    /* Close the logical device. */
    status = bt_close(btd);
    if (BT_SUCCESS != status) {
        /* this is safe to do */
        bt_perror(btd, status, "Could not close the device");
        main_ret = EXIT_FAILURE;
        goto exit_main;
    }

    /* Exit the program  */
  exit_main:
    return(main_ret);

} /* end bt_icbr() */


/*****************************************************************************
**
**      Function:   icbr()
**
**      Purpose:    Interrupt Call Back Routine.  This function is called
**                  once for each error interrupt that occurs.
**
**      Args:       param_p     The pointer passed to bt_icbr_install().
**                              This is a pointer to an icbr_state_t struct.
**                  irq_type    The type of interrupt which caused the
**                              function to be called.  This should either
**                              be BT_IRQ_ERROR or BT_IRQ_OVERFLOW.
**                  vector      This is the value returned from the driver
**                              ISR or the user ISR.  The significance of
**                              this value varies with the irq_type and the
**                              product family.
**
**      Returns:    void
**
*****************************************************************************/
static void icbr(
    void                   *param_p,
    bt_irq_t                irq_type,
    bt_data32_t             vector
    )
{
    volatile icbr_state_t  *icbr_state_p = (volatile icbr_state_t *) param_p;
    icbr_data_t             new_data;

    new_data.m_irq_type = irq_type;
    new_data.m_vector = vector;

    switch (irq_type) {
#if !defined (BT25801)
      case BT_IRQ_OVERFLOW:
        /*
        ** Any ICBR handling should check for this condition.
        **
        ** Usually, this is an error condition requiring special handling
        ** by the application, since it can indicate a loss of data.
        */
        new_data.m_status = BT_SUCCESS;
        break;
#endif
      case BT_IRQ_ERROR:
        new_data.m_status = bt_chkerr(icbr_state_p->m_btd);
        break;

      default:
        /* Some other type of interrupt. */
        new_data.m_status = BT_SUCCESS;
        break;
    }

    queue_insert(icbr_state_p->m_queue_p, icbr_state_p->m_head_p,
                 icbr_state_p->m_queue_len, new_data);

} /* end icbr() */


/*****************************************************************************
**
**      Function:   queue_insert()
**
**      Purpose:    Insert the interrupt data into a FIFO queue.
**                  This is used to transmit data from the ICBR to the
**                  main program (where more things can be done with it).
**
**      Args:       queue_p     The pointer to the queue buffer, an array
**                              of icbr_data_t's queue_len elements long.
**                  head_p      Pointer to the head index.
**                  queue_len   The number of elements in the queue buffer.
**                  new_data    The new data to insert into the queue.
**
**      Returns:    void
**
**      Note:       This routine simply places the data into the queue, and
**                  leaves overflow detection to queue_remove().  queue_insert()
**                  would be the proper place for overflow detection, but we
**                  want to minimize the amount of code executed in icbr().
**                  It is for this same reason that we avoid using a mutex.
**
**                  Since queue_insert() and queue_remove() are NOT protected
**                  by a mutex, only a single thread should be placing
**                  elements into a queue, and queue_remove() may incorrectly
**                  detect overflows when the queue is simply full.
**                  If multiple ICBRs are registered, they should use separate
**                  queues (even if the same ICBR is registered multiple times,
**                  it should use a different queue for each registration).
**
**                  It is suggested that queue length should be an even divisor
**                  of (UINT_MAX + 1).  Otherwise, when the queue indices head
**                  and tail wrap around the UINT_MAX endpoint there will be an
**                  unused gap in the queue and if this gap is large enough,
**                  an unnecessary queue overflow could be generated.  On most
**                  systems, this means QUEUE_LEN should be a power of 2.
**
*****************************************************************************/
static void queue_insert(
    volatile icbr_data_t   *queue_p,
    volatile unsigned int  *head_p,
    int                     queue_len,
    icbr_data_t             new_data
    )
{
    queue_p[*head_p % queue_len] = new_data;
    *head_p += 1;
} /* end queue_insert() */


/*****************************************************************************
**
**      Function:   queue_remove()
**
**      Purpose:    Remove one element of interrupt data from the queue.
**                  This routine is used to receive data from the ICBR
**                  in the main program, where more things can be done
**                  with it.
**
**      Args:       queue_p     The pointer to the queue buffer, an array
**                              of icbr_data_t's queue_len elements long.
**                  head_p      Pointer to the head index.
**                  tail_p      Pointer to the tail index.  Multiple threads
**                              can be removing elements from the same queue
**                              at the same time if they use separate tail
**                              indicies.
**                  queue_len   The number of elements in the queue buffer.
**                              This length needs to be the same as that
**                              passed into queue_insert() for the queue.
**                  data_p      Pointer to the location to store the data
**                              removed from the queue.
**
**      Returns:    int
**                      1 if data was successfully removed from the queue.
**                      0 if no data was in the queue to be removed.
**                     -1 if a queue overflow occured.  The data in *data_p
**                        should not be used in this case, as it may be
**                        garbled (the data was being written to as it was
**                        being read).
**
**      Note:       queue_insert() would be a better place for overflow
**                  detection, but we want to minimize the amount of code
**                  executed in icbr().  It is for this same reason that we
**                  avoid using a mutex.  Avoiding the use of a mutex also
**                  requires that overflow detection be done both before and
**                  after "removal" of the queue tail item. 
**
**                  Since queue_insert() and queue_remove() are NOT protected
**                  by a mutex, only a single thread should be placing
**                  elements into a queue, and queue_remove() may incorrectly
**                  detect overflows when the queue is simply full.
**                  If multiple ICBRs are registered, they should use separate
**                  queues (even if the same ICBR is registered multiple times,
**                  it should use a different queue for each registration).
**
**                  It is suggested that queue length should be an even divisor
**                  of (UINT_MAX + 1).  Otherwise, when the queue indices head
**                  and tail wrap around the UINT_MAX endpoint there will be an
**                  unused gap in the queue and if this gap is large enough,
**                  an unnecessary queue overflow could be generated.  On most
**                  systems, this means QUEUE_LEN should be a power of 2.
**
*****************************************************************************/
static int queue_remove(
    volatile icbr_data_t   *queue_p,
    volatile unsigned int  *head_p,
    unsigned int           *tail_p,
    int                     queue_len,
    icbr_data_t            *data_p
    )
{
    unsigned int            distance;
    int                     retval = 0;

    /* are there any in the queue waiting to be removed? */
    if (*tail_p != *head_p) {

        /* first check for overflows */
        if (*head_p < *tail_p) {
            distance = (UINT_MAX - (*tail_p - *head_p)) + 1;
        } else {
            distance = *head_p - *tail_p;
        }

        if (distance >= QUEUE_LEN) {

            /* We've overflowed- empty the queue and return. */
            *tail_p = *head_p;
            retval = -1;
            goto end_queue_remove;

        } else {

            /* grab the data and put it somewhere safe.  Note that we   */
            /* do not assume this is an atomic operation.               */
            *data_p = queue_p[*tail_p % queue_len];

            /* check for overflows again */
            if (*head_p < *tail_p) {
                distance = (UINT_MAX - (*tail_p - *head_p)) + 1;
            } else {
                distance = *head_p - *tail_p;
            }

            if (distance >= QUEUE_LEN) {

                /* We've overflowed- empty the queue and return. */
                *tail_p = *head_p;
                /* Note: the data we've saved in *data_p should not be */
                /* used- it may have been overwritten in the overflow! */
                retval = -1;
                goto end_queue_remove;
            }

            /* increment the tail pointer */
            *tail_p += 1;

            retval = 1;
        } /* end else distance */
    } /* if data in queue */

  end_queue_remove:
    return(retval);
} /* end queue_remove() */


/*****************************************************************************
**
**      Function:   signal_handler()
**
**      Purpose:    Signal handler which is called upon receiving a SIGABRT,
**                  SIGINT, or SIGTERM signal.  Hopefully, ^C will cause one
**                  of these to occur.
**
**      Args:       sig_num     Signal number that occurred that caused
**                              the handler to be invoked
**
**      Returns:    void
**
*****************************************************************************/
void signal_handler(
    int sig_num
    )
{
    /* just set the flag so that the main program knows that a signal has
       occurred. */
    signal_happened = 1;
}


/*****************************************************************************
**
**      Function:   usage()
**
**      Purpose:    Print a help message for the program.
**
**      Args:       name_p      Name of the program- usually argv[0]
**
**      Returns:    void
**
*****************************************************************************/
static void usage(
    const char *name_p
    )
{
    printf ("%s is an example program demonstrating the use of ICBRs to\n",
            name_p);
    puts   ("receive interrupts.");
    puts   ("For more information, please consult the manual.\n");
    printf ("usage: %s [options]\n", name_p);
    puts   ("Options are:");
    puts   ("\t-\t\tDisplays this message and exits");
    puts   ("\t-u <unit>\tUnit Number to open.");
    printf ("\t\t\tDefault is unit %d.\n", DEF_UNIT);
    puts   ("\t-i <IRQ type>\tInterrupt type to register for.");
    printf ("\t\t\tDefault is %d (BT_IRQ_ERROR).\n", BT_IRQ_ERROR);
} /* end usage() */
