/*****************************************************************************
**
**      Filename:   bt_info.c
**
**      Purpose:    This example program uses the SBS Bit 3 MIRROR API functions
**                  bt_get_info() and bt_set_info() to get or set an
**                  INFO parameter.
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-2000 by SBS Technologies, Inc.
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
#include <assert.h>
#include "btapi.h"

/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/

static bt_info_t str2info(const char *str_p);
static bt_devdata_t getdevdata(const char * str_p);
static void usage(const char *prog_p);

#define DEF_UNIT 0 /* default unit number */

/*****************************************************************************
**
**      Program:    bt_info
**
**      Purpose:    This example program uses the SBS Bit 3 MIRROR API functions
**                  bt_get_info() and bt_set_info() to get or set an
**                  INFO parameter.
**
**      Args:
**          -               Displays the command summary.
**          -p  <param>     Name of the parameter to set.
**          -t  <logdev>    Logical device. (BT_DEV_MEM, BT_DEV_IO,
**                          BT_DEV_DEFAULT, etc.)
**                          Default is to BT_DEV_MEM
**          -u  <unit>      Unit Number to open.  Default is unit 0.
**          -v  <value>     Value to set the parameter to.
**
*****************************************************************************/
int main (
    int argc,
    const char ** argv
    )
{
    bt_dev_t     logdev      = BT_DEV_MEM;  /* Logical device to open */
    int          unit        = DEF_UNIT;    /* Unit number to open */
    const char   *name_p     = *argv;       /* Program name */
    bt_info_t    param       = BT_MAX_INFO; /* parameter to get/set */
    const char   *infoname_p = NULL;        /* name of the info parameter */
    bt_devdata_t value;                     /* parameters' value */
    int          direction   = 0;           /* 0 == get value, 1 == set value */
    bt_desc_t    btd;                       /* Device descriptor */
    char         devname[BT_MAX_DEV_NAME];  /* Device name */
    bt_error_t   status;                    /* SBS Bit 3 API return value */
    int          main_ret    = EXIT_SUCCESS;/* Return value for main() */
    char         *temp_p     = NULL;        /* For detecting errors in strtoul. */

    /*  parse command line arguments  */
    argc--;
    argv++;
    while (argc > 0) {

        /* check to see if it's a command line option.  */
        if ((**argv == '-') && (argv[0][1] != '\0') && (argv[0][2] == '\0')) {

            switch(argv[0][1]) {

             case 'p': /* set parameter name */
                  if (argc > 1) {
                      param = str2info(argv[1]);
                      if (param >= BT_MAX_INFO) {
                          usage(name_p);
                          return EXIT_FAILURE;
                      }
                      infoname_p = argv[1];
                  } else {
                      usage(name_p);
                      return EXIT_FAILURE;
                  }
                  argc--;
                  argv++;
                  break;

             case 'n': /* set parameter number */
                  if (argc > 1) {
                      param = (bt_info_t) getdevdata(argv[1]);
                      if (param >= BT_MAX_INFO) {
                          usage(name_p);
                          return EXIT_FAILURE;
                      }
                      infoname_p = "BT_INFO_?";
                  } else {
                      usage(name_p);
                      return EXIT_FAILURE;
                  }
                  argc--;
                  argv++;
                  break;

                case 't': /* set logical device */
                    if (argc > 1) {
                        logdev = bt_str2dev(argv[1]);
                        if (logdev >= BT_MAX_DEV) {
                            usage(name_p);
                            return EXIT_FAILURE;
                        }
                    argc--;
                    argv++;
                    } else {
                        usage(name_p);
                        return EXIT_FAILURE;
                    }
                    break;

              case 'u': /* set unit number */
                  if ((argc > 1) && (isdigit(argv[1][0]))) {
                      unit = (int) strtoul(argv[1], &temp_p, 0);
                      if (*temp_p != '\0') {
                          usage(name_p);
                          return EXIT_FAILURE;
                      }
                  } else {
                      usage(name_p);
                      return EXIT_FAILURE;
                  }
                 argc--;
                 argv++;
                 break;

              case 'v': /* set parameter value */
                  direction = 1;
                  if (argc > 1) {
                      value = getdevdata(argv[1]);
                  } else {
                      usage(name_p);
                      return EXIT_FAILURE;
                  }
                  argc--;
                  argv++;
                  break;

              default:
                usage(name_p);
                return EXIT_FAILURE;

            } /* end switch on argument type */

        } else {
            usage(name_p);
            return EXIT_FAILURE;
        } /* end else bad argument */

        argc--;
        argv++;
    } /* end while more arguments */

    /* check to make sure param got set. */
    if (param >= BT_MAX_INFO) {
        usage(name_p);
        return EXIT_FAILURE;
    }

    /* Command line parsing completed. */
    /* Open the logical device */
    status = bt_open(&btd, bt_gen_name(unit, logdev, devname,
        BT_MAX_DEV_NAME), BT_RDWR);
    if (BT_SUCCESS != status) {

        /* It is safe to call bt_perror with any status returned from     */
        /* bt_open() or bt_close() with the device descriptor, despite    */
        /* the fact that the descriptor is not valid to do anything else. */
        bt_perror(btd, status, "Could not open the device");
        return EXIT_FAILURE;
    }

    /* Get or set the INFO Parameter */
    if (0 == direction) {

        /* get the info parameter */
        status = bt_get_info(btd, param, &value);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not get info for the device");
            /* don't exit here- we still need to close the device */
            main_ret = EXIT_FAILURE;
        } else {
            printf ("Value of %s is %lu (0x%lX)\n", infoname_p,
            (unsigned long) value, (unsigned long) value);
        }

    } else {
        /* set the info parameter */
        status = bt_set_info(btd, param, value);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not set info for the device");
            /* don't exit here- we still need to close the device */
            main_ret = EXIT_FAILURE;
        } else {
            printf ("%s was set to 0x%lX.\n",  infoname_p, (unsigned long) value);
        }
    } /* end else direction */

    /* Close the logical device. */
    status = bt_close(btd);
    if (BT_SUCCESS != status) {
        /* this is safe to do */
        bt_perror(btd, status, "Could not close the device");
        return EXIT_FAILURE;
    }

    /* Exit the program  */
    return main_ret;

} /* end bt_info() */


/*****************************************************************************
**
**      Function:   str2info()
**
**      Purpose:    Given a string, return the info parameter represented by
**                  by that string.
**
**      Args:       str_p   String to examine
**
**      Returns:    bt_info_t
**                  INFO parameter to get or set.
**                  BT_MAX_INFO if the info parameter is not recognized.
**
*****************************************************************************/
static bt_info_t str2info(
    const char *str_p
    )
{

#define TEST(x) if (strcmp(#x, str_p) == 0) return (bt_info_t) x
#define TEST2(x,y) if ((strcmp (#x #y, str_p) == 0) || (strcmp(#y, str_p) == 0)) \
                       return (bt_info_t) x##y

#if defined(BT25801)

    /* 25-801 Broadcast Memory common device configuration parameters. */
    TEST2(BT_INFO_,ICBR_Q_SIZE);
    TEST2(BT_INFO_,LOC_PN);
    TEST2(BT_INFO_,SWAP);
    TEST2(BT_INFO_,TRACE);

    /* Specific 25-801 Broadcast Memory device configuration parameters. */
    TEST2(BT_INFO_,HUB_NUM);
    TEST2(BT_INFO_,HUB_SLOT_NUM);
    TEST2(BT_INFO_,MAX_NODES);
    TEST2(BT_INFO_,NETRAM_SIZE);
    TEST2(BT_INFO_,NODE_ID);
    TEST2(BT_INFO_,NODE_ID_AUTOCFG);
    TEST2(BT_INFO_,NODE_MON_ADDR);
    TEST2(BT_INFO_,NODE_MON_INTERVAL);
    TEST2(BT_INFO_,NODE_MON_SIZE);
    TEST2(BT_INFO_,NODE_MON_SUPPORT);
    TEST2(BT_INFO_,PORT_NUM);
    TEST2(BT_INFO_,RECOV_RXERR);
    TEST2(BT_INFO_,RECOV_TXERR);
    TEST2(BT_INFO_,UNIT_NUM);
    
    /* Implementation specific broadcast memory device    */
    /* configuration parameters.                        */
    TEST2(BT_INFO_,BUS_NUM);
    TEST2(BT_INFO_,INTR_LVL);
    TEST2(BT_INFO_,SLOT_NUM);
    TEST2(BT_INFO_,MAX_ICBRQS);

    TEST2(BT_INFO_,BLOCK);
    TEST2(BT_INFO_,DATAWIDTH);
    TEST2(BT_INFO_,DMA_AMOD);
    TEST2(BT_INFO_,DMA_POLL_CEILING);
    TEST2(BT_INFO_,DMA_THRESHOLD);
    TEST2(BT_INFO_,PAUSE);

#else

    /* These are the INFO parameters supported on all SBS Bit 3 Products */
    TEST2(BT_INFO_,BLOCK);
    TEST2(BT_INFO_,DATAWIDTH);
    TEST2(BT_INFO_,DMA_AMOD);
    TEST2(BT_INFO_,DMA_POLL_CEILING);
    TEST2(BT_INFO_,DMA_THRESHOLD);
    TEST2(BT_INFO_,DMA_WATCHDOG);
    TEST2(BT_INFO_,ICBR_Q_SIZE);
    TEST2(BT_INFO_,LOC_PN);
    TEST2(BT_INFO_,MMAP_AMOD);
    TEST2(BT_INFO_,PAUSE);
    TEST2(BT_INFO_,PIO_AMOD);
    TEST2(BT_INFO_,REM_PN);
    TEST2(BT_INFO_,SWAP);
    TEST2(BT_INFO_,TRACE);
    TEST2(BT_INFO_,BOARD_REV);

#endif

#if defined(BT1003)
    TEST2(BT_INFO_,A64_OFFSET);
    TEST2(BT_INFO_,KMALLOC_SIZ);
    TEST2(BT_INFO_,KMALLOC_BUF);
    TEST2(BT_INFO_,KFREE_BUF);
#endif

#ifdef BT_NBUS_FAMILY

    /* INFO parameters supported on all SBS Bit 3 Nanobus Products */
    TEST2(BT_INFO_,BIND_ALIGN);
    TEST2(BT_INFO_,BIND_COUNT);
    TEST2(BT_INFO_,BIND_SIZE);
    TEST2(BT_INFO_,TRANSMITTER);
    TEST2(BT_INFO_,RESET_DELAY);
    TEST2(BT_INFO_,LM_SIZE);
    TEST2(BT_INFO_,INC_INHIB);  /* Data Blizzard may support this. */
    TEST2(BT_INFO_,USE_PT);
    TEST2(BT_INFO_,UNIT_NUM);
    TEST2(BT_INFO_,LOG_STAT);
    TEST2(BT_INFO_,TOTAL_COUNT);
    TEST2(BT_INFO_,ERROR_COUNT);
    TEST2(BT_INFO_,EVENT_COUNT);
    TEST2(BT_INFO_,IACK_COUNT);
    TEST2(BT_INFO_,KMEM_SIZE);
    TEST2(BT_INFO_,USE_PT);
    TEST2(BT_INFO_,GEMS_SWAP);

#if defined(BT973) || defined(BT983) || defined(BT984)

    /* 973 and 983/984 specific INFO parameters */
    TEST2(BT_INFO_,SIG_ERR);
    TEST2(BT_INFO_,SIG_IACK);
    TEST2(BT_INFO_,SIG_PRG);
    TEST2(BT_INFO_,SIG_TOT);

#endif /* 973 || 983 || 984  */

#endif /* BT_NBUS_FAMILY */

#if defined(BT_NBRIDGE_FAMILY)

    /* Info parameters for NanoBridge products */
    TEST2(BT_INFO_,RESET_DELAY);
    TEST2(BT_INFO_,LM_SIZE);
    TEST2(BT_INFO_,LOG_STAT);
    TEST2(BT_INFO_,BUS_NUM);
    TEST2(BT_INFO_,SLOT_NUM);
    TEST2(BT_INFO_,UNIT_NUM);
    TEST2(BT_INFO_,TOTAL_COUNT);
    TEST2(BT_INFO_,EVENT_COUNT);
    TEST2(BT_INFO_,ERROR_COUNT);
    TEST2(BT_INFO_,IACK_COUNT);
    TEST2(BT_INFO_,MMAP_SWAP);
    TEST2(BT_INFO_,SDMA_SWAP);
    TEST2(BT_INFO_,LMON_ADDR);
    TEST2(BT_INFO_,LMON_AMOD);

#endif /* BT_NBRIDGE_FAMILY */

#ifdef BT_NPORT_FAMILY

    /* INFO parameters supported on all SBS Bit 3 NanoPort Products */

    TEST2(BT_INFO_,RESET_DELAY);
    TEST2(BT_INFO_,DEADLOCK_COUNT);
    TEST2(BT_INFO_,DMA_AW);
    TEST2(BT_INFO_,ERROR_COUNT);
    TEST2(BT_INFO_,EVENT_COUNT);
    TEST2(BT_INFO_,INC_INHIB);
    TEST2(BT_INFO_,LM_SIZE);
    TEST2(BT_INFO_,MMAP_AW);
    TEST2(BT_INFO_,UNIT_NUM);

#endif /* BT_NPORT_FAMILY */

#if defined(BT13908)

    /* Info parameters for 1394 products */
    TEST2(BT_INFO_,RESET_DELAY);
    TEST2(BT_INFO_,LM_SIZE);
    TEST2(BT_INFO_,TOTAL_COUNT);
    TEST2(BT_INFO_,EVENT_COUNT);
    TEST2(BT_INFO_,ERROR_COUNT);
    TEST2(BT_INFO_,IACK_COUNT);
    TEST2(BT_INFO_,WIN_SIZE);
    TEST2(BT_INFO_,ROM_VERSION);
    TEST2(BT_INFO_,ARB_LEVEL);
    TEST2(BT_INFO_,ARB_TYPE);
    TEST2(BT_INFO_,BIND_ALIGN);
    TEST2(BT_INFO_,BIND_COUNT);
    TEST2(BT_INFO_,BIND_SIZE);
    TEST2(BT_INFO_,NUM_PACKETS);
    TEST2(BT_INFO_,NUM_MESSAGES);
    TEST2(BT_INFO_,MESSAGE_TIMEOUT);
    TEST2(BT_INFO_,REM_TRACE);
    TEST2(BT_INFO_,DIAG_1);
    TEST2(BT_INFO_,DIAG_2);
    TEST2(BT_INFO_,DIAG_3);
    TEST2(BT_INFO_,DIAG_4);
    TEST2(BT_INFO_,MAX_PACKETSIZE);
    TEST2(BT_INFO_,LOC_TRACE);

#endif /* defined(BT13908) */

#if     defined(BT18901) || defined(BT993)

    /* INFO parameters specific to the VxWorks Device Drivers. */

    TEST2(BT_INFO_,ICBR_PRIO);
    TEST2(BT_INFO_,ICBR_STACK);

#endif  /* defined(BT18901) || defined(BT993) */

#if defined(BT15901) || defined(BT15991) || defined(BT15904) || defined(BT15906)

    /* INFO parameters specific to the 15-901 Windows NT Broadcast Memory
       Driver */

    TEST2(BT_INFO_,BUS_NUM);
    TEST2(BT_INFO_,HUB_NUM);
    TEST2(BT_INFO_,HUB_SLOT_NUM);
    TEST2(BT_INFO_,INTR_LVL);
    TEST2(BT_INFO_,MAX_NODES);
    TEST2(BT_INFO_,MUTEX_SIZE);
    TEST2(BT_INFO_,NODE_ID);
    TEST2(BT_INFO_,NODE_ID_AUTOCFG);
    TEST2(BT_INFO_,NODE_MON_ADDR);
    TEST2(BT_INFO_,NODE_MON_INTERVAL);
    TEST2(BT_INFO_,NODE_MON_SIZE);
    TEST2(BT_INFO_,NODE_MON_SUPPORT);
    TEST2(BT_INFO_,NETRAM_SIZE);
    TEST2(BT_INFO_,PORT_NUM);
    TEST2(BT_INFO_,RECOV_RXERR);
    TEST2(BT_INFO_,RECOV_TXERR);
    TEST2(BT_INFO_,SLOT_NUM);
    TEST2(BT_INFO_,UNIT_NUM);
    TEST2(BT_INFO_,MAX_ICBRQS);

#endif /* defined(BT15901) || defined(BT15991) || defined(BT15904) \
       || defined(BT15906) */

    return BT_MAX_INFO;
}  /* end str2info() */




/*****************************************************************************
**
**      Function:   getdevdata()
**
**      Purpose:    Given a string and an INFO parameter, return the
**                  correct bt_devdata_t for that string.
**
**      Args:       ptr_p   String to examine
**
**      Returns:    bt_devdata_t
**                      Correct devdata to use.
**                      assert(0) is called on error.
**
*****************************************************************************/
static bt_devdata_t getdevdata(
    const char * str_p
    )
{

    if (isdigit(*str_p)) {
        /* always accept a numeric representation */
        return (bt_devdata_t) strtoul (str_p, NULL, 0);
    } else if (((*str_p == '-') || (*str_p == '+')) && isdigit(str_p[1])) {
        return (bt_devdata_t) strtol (str_p, NULL, 0);
    }

    puts ("Unrecognized INFO parameter!");
    assert(0);
    return (bt_devdata_t) 0;

} /* end getdevdata() */




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
    const char * name_p
    )
{
    printf ("%s is an example program demonstrating how to get and set\n",
            name_p);
    puts   ("INFO parameters with the SBS Bit 3 MIRROR API.");
    puts   ("For more information, please consult the manual.\n");
    printf ("usage: %s [options]\n", name_p);
    puts   ("Options are:");
    puts   ("\t-\t\tDisplays this message and exits");
    puts   ("\t-p <param>\tINFO parameter text name to get or set.");
    puts   ("\t-n <param>\tINFO parameter enum number to get or set.");

#if defined(BT25801)

    /* 25-801 Broadcast Memory common device configuration parameters. */
    puts   ("\t\t\tBT_INFO_ICBR_Q_SIZE  BT_INFO_LOC_PN");
    puts   ("\t\t\tBT_INFO_SWAP  BT_INFO_TRACE");
    /* Specific 25-801 Broadcast Memory device configuration parameters. */
    puts   ("\t\t\tBT_INFO_HUB_NUM  BT_INFO_HUB_SLOT_NUM");
    puts   ("\t\t\tBT_INFO_MAX_NODES  BT_INFO_NETRAM_SIZE");
    puts   ("\t\t\tBT_INFO_NODE_ID  BT_INFO_NODE_ID_AUTOCFG");
    puts   ("\t\t\tBT_INFO_NODE_MOD_ADDR  BT_INFO_NODE_MON_INTERVAL");
    puts   ("\t\t\tBT_INFO_NODE_MON_SIZE  BT_INFO_NODE_MON_SUPPORT");
    puts   ("\t\t\tBT_INFO_PORT_NUM  BT_INFO_RECOV_RCERR");
    puts   ("\t\t\tBT_INFO_RECOV_TXERR  BT_INFO_UNIT_NUM");
    
    /* Implementation specific broadcast memory device    */
    /* configuration parameters.                        */
    puts   ("\t\t\tBT_INFO_BUS_NUM  BT_INFO_INTR_LVL");
    puts   ("\t\t\tBT_INFO_SLOT_NUM  BT_INFO_MAX_ICBRQS");
    puts   ("\t\t\tBT_INFO_BLOCK  BT_INFO_DATAWIDTH");
    puts   ("\t\t\tBT_INFO_DMA_AMOD  BT_INFO_DMA_POLL_CEILING");
    puts   ("\t\t\tBT_INFO_DMA_THRESHOLD  BT_INFO_PAUSE");

#else

    /* These are the INFO parameters supported on all SBS Bit 3 Products */
    puts   ("\t\t\tBT_INFO_BLOCK  BT_INFO_DATAWIDTH");
    puts   ("\t\t\tBT_INFO_DMA_AMOD  BT_INFO_DMA_POLL_CEILING");
    puts   ("\t\t\tBT_INFO_DMA_THRESHOLD  BT_INFO_DMA_WATCHDOG");
    puts   ("\t\t\tBT_INFO_ICBR_Q_SIZE  BT_INFO_LOC_PN");
    puts   ("\t\t\tBT_INFO_MMAP_AMOD  BT_INFO_PAUSE");
    puts   ("\t\t\tBT_INFO_PIO_AMOD  BT_INFO_REM_PN");
    puts   ("\t\t\tBT_INFO_SWAP  BT_INFO_TRACE");

#endif

#ifdef BT_NBUS_FAMILY

    /* INFO parameters supported on all SBS Bit 3 Nanobus Products */
    puts   ("\t\t\tBT_INFO_BIND_ALIGN  BT_INFO_BIND_COUNT");
    puts   ("\t\t\tBT_INFO_BIND_SIZE  BT_INFO_TRANSMITTER"); 
    puts   ("\t\t\tBT_INFO_RESET_DELAY  BT_INFO_LM_SIZE");
    puts   ("\t\t\tBT_INFO_INC_INHIB  BT_INFO_USE_PT");
    puts   ("\t\t\tBT_INFO_UNIT_NUM  BT_INFO_LOG_STAT");
    puts   ("\t\t\tBT_INFO_TOTAL_COUNT  BT_INFO_ERROR_COUNT");
    puts   ("\t\t\tBT_INFO_EVENT_COUNT  BT_INFO_IACK_COUNT");
    puts   ("\t\t\tBT_INFO_KMEM_SIZE");

#if defined(BT973) || defined(BT983) || defined(BT984)

    /* 973 and 983/984 specific INFO parameters */
    puts   ("\t\t\tBT_INFO_SIG_ERR  BT_INFO_SIG_IACK");
    puts   ("\t\t\tBT_INFO_SIG_PRG  BT_INFO_SIG_TOT");

#endif /* 973 || 983 || 984 */

#endif /* BT_NBUS_FAMILY */

#if defined(BT_NBRIDGE_FAMILY)

    /* Info parameters for NanoBridge products */
    puts    ("\t\t\tBT_INFO_RESET_DELAY  BT_INFO_LM_SIZE");
    puts   ("\t\t\tBT_INFO_LOG_STAT  BT_INFO_BUS_NUM");
    puts    ("\t\t\tBT_INFO_SLOT_NUM  BT_INFO_UNIT_NUM");
    puts   ("\t\t\tBT_INFO_TOTAL_COUNT  BT_INFO_EVENT_COUNT");
    puts    ("\t\t\tBT_INFO_ERROR_COUNT  BT_INFO_IACK_COUNT");
    puts   ("\t\t\tBT_INFO_MMAP_SWAP  BT_INFO_SDMA_SWAP");
    puts    ("\t\t\tBT_INFO_LMON_ADDR  BT_INFO_LMON_AMOD");
    puts    ("\t\t\tBT_INFO_USE_PT BT_INFO_GEMS_SWAP");

#endif /* BT_NBRIDGE_FAMILY */

#ifdef BT_NPORT_FAMILY

    /* INFO parameters supported on all SBS Bit 3 NanoPort Products */

    puts    ("\t\t\tBT_INFO_RESET_DELAY  BT_INFO_DEADLOCK_COUNT");
    puts   ("\t\t\tBT_INFO_DMA_AW  BT_INFO_ERROR_COUNT");
    puts    ("\t\t\tBT_INFO_EVENT_COUNT  BT_INFO_INC_INHIB");
    puts   ("\t\t\tBT_INFO_LM_SIZE  BT_INFO_MMAP_AW");
    puts    ("\t\t\tBT_INFO_UNIT_NUM");

#endif /* BT_NPORT_FAMILY */

#if defined(BT13908)

    /* Info parameters for 1394 products */
    puts    ("\t\t\tBT_INFO_RESET_DELAY  BT_INFO_LM_SIZE");
    puts   ("\t\t\tBT_INFO_TOTAL_COUNT  BT_INFO_EVENT_COUNT");
    puts    ("\t\t\tBT_INFO_ERROR_COUNT  BT_INFO_IACK_COUNT");
    puts   ("\t\t\tBT_INFO_WIN_SIZE  BT_ROM_VERSION");
    puts    ("\t\t\tBT_INFO_ARB_LEVEL  BT_INFO_ARB_TYPE");
    puts   ("\t\t\tBT_INFO_BIND_ALIGN  BT_INFO_BIND_COUNT");
    puts    ("\t\t\tBT_INFO_BIND_SIZE  BT_INFO_NUM_PACKETS");
    puts   ("\t\t\tBT_INFO_NUM_MESSAGES  BT_INFO_MESSAGE_TIMEOUT");
    puts    ("\t\t\tBT_INFO_REM_TRACE  BT_INFO_DIAG_1");
    puts   ("\t\t\tBT_INFO_DIAG_2  BT_INFO_DIAG_3");
    puts    ("\t\t\tBT_INFO_DIAG_4  BT_INFO_MAX_PACKETSIZE");
    puts   ("\t\t\tBT_INFO_LOC_TRACE");

#endif /* defined(BT13908) */

#if     defined(BT18901) || defined(BT993)

    /* INFO parameters specific to the VxWorks Device Drivers. */

    puts    ("\t\t\tBT_INFO_ICBR_PRIO  BT_INFO_ICBR_STACK.");

#endif  /* defined(BT18901) || defined(BT993) */

#if defined(BT1003)
/* EAS A64 */
    puts    ("\t\t\tBT_INFO_A64_OFFSET");
/* EAS TMP CODE */
    puts    ("\t\t\tBT_INFO_KMALLOC_SIZ");
    puts    ("\t\t\tBT_INFO_KMALLOC_BUF  BT_INFO_KFREE_BUF.");
#endif  /* defined(BT1003) */

#if defined(BT15901) || defined(BT15991) || defined(BT15904) || defined(BT15906)

    /* INFO parameters specific to the 15-901 Windows NT Broadcast Memory
       Driver */

    puts    ("\tBT_INFO_BUS_NUM  BT_INFO_HUB_NUM  BT_INFO_HUB_SLOT_NUM  BT_INFO_INT_LVL.");
    puts    ("\tBT_INFO_MAX_NODES  BT_INFO_MUTES_SIZE  BT_INFO_NODE_ID  BT_INFO_NODE_ID_AUTOCFG.");
    puts    ("\tBT_INFO_NODE_MON_ADDR  BT_INFO_NODE_MON_INTERVAL  BT_INFO_NODE_MON_SIZE  BT_INFO_NODE_MON_SUPPORT.");
    puts    ("\tBT_INFO_NETRAM_SIZE  BT_INFO_PORT_NUM  BT_INFO_RECOV_RXERR  BT_INFO_RECOV_TXERR.");
    puts    ("\tBT_INFO_SLOT_NUM  BT_INFO_UNIT_NUM  BT_INFO_MAX_ICBRQS.");

#endif /* defined(BT15901) || defined(BT15991) || defined(BT15904) \
       || defined(BT15906) */

    puts   ("\t-t <logdev>\tLogical device. (BT_DEV_MEM, BT_DEV_IO,");
    puts   ("\t\t\tBT_DEV_DEFAULT, etc.)");
    puts   ("\t\t\tDefault is to BT_DEV_MEM.");
    puts   ("\t-u <unit>\tUnit Number to open.");
    printf ("\t\t\tDefault is unit %d. \n", DEF_UNIT);
    puts   ("\t-v <value>\tValue to set the INFO parameter to.  If this");
    puts   ("\t\t\tparameter is not given, the INFO parameter is written");
    puts   ("\t\t\tto the standard out instead.");


} /* end usage() */
