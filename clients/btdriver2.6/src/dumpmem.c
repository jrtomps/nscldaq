/*****************************************************************************
**
**      Filename:   dumpmem.c
**
**      Purpose:    This example program uses the core SBS Bit 3 Mirror API
**                  routines to access memory through a memory mapped pointer. 
**                  It will display 256 bytes of data.
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997,1998 by SBS Technologies, Inc.
**        Copyright (c) 1996, 1997 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* LINT */


#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <ctype.h>
#include        "btapi.h"

#define NUM_LINES       16   /* number of lines of memory to print to screen */
#define NUM_BYTES       16   /* number of bytes to display on a line of the screen */
#define DEF_UNIT        0    /* default unit number */
#define DEF_ADDR        0x00010000 /* default address */

/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/

static void usage(char *prog_p);


/*****************************************************************************
**
**      Program:    dumpmem
**
**      Purpose:    Prints the first 256 bytes of remote system memory or 
**                  dual port memory as both hexadecimal bytes and ASCII 
**                  characters.
**
**      Args:
**          -               Displays the command summary.
**          -a  <addr>      Address to start at.
**                          Default is address 0.
**          -t  <logdev>    Logical device. (BT_DEV_MEM, BT_DEV_IO, 
**                          BT_DEV_DEFAULT, etc.)
**                          Default is to BT_DEV_DEFAULT
**          -u  <unit>      Unit Number to open.
**                          Default is unit 0.
**
*****************************************************************************/
int main(
    int   argc,
    char  **argv)
{

    int             unit = DEF_UNIT;          /* Which board to use */
    bt_desc_t       btd;                      /* Unit descriptor */
    char            devname[BT_MAX_DEV_NAME]; /* Device name string */
    bt_error_t      status;                   /* Mirror API return value */
    bt_dev_t        type = BT_DEV_DEFAULT;
    char            *prog_p = argv[0];
    unsigned long   remote_address = DEF_ADDR;
    int             length = NUM_LINES * NUM_BYTES;
    int             my_index;
    int             nlines;
    bt_data8_t      data;
    unsigned char   buff[NUM_BYTES + 1];
    void            *remote_p = NULL;   /* holds pointer to a memory region */
    volatile char   *addr_p = NULL;     /* current location being accessed */

    /* Parse the command line */
    argc--;
    while (argv++, argc--) {

        /* Check to see if this is a command line option */
        if (**argv == '-') {

            switch (*(*argv+1)) {

              case 'a':  /* remote address */ 
                argv++;
                argc--;
                remote_address = strtoul(*argv, NULL, 0);
                break;

              case 't':  /* logical device type */
                argv++;
                argc--;

                type = bt_str2dev(*argv);
                if (type >= BT_MAX_DEV) {
                    fprintf(stderr, "Invalid access type: %s\n", *argv);
                    usage(prog_p);
                    return EXIT_FAILURE;
                }

                break;

              case 'u':  /* Unit number */
                argv++;
                argc--;
                unit = strtol(*argv, NULL, 0);
                break;

              default:
                usage(prog_p);
                return EXIT_FAILURE;

            } /* end switch on arguments type */
    
        } else {
            usage(prog_p);
            return EXIT_FAILURE ;
        } /* end else bad argument */

    } /* end if more arguments */

    /* Open the unit */
    status = bt_open(&btd,
        bt_gen_name(unit, type, &devname[0], BT_MAX_DEV_NAME),
        BT_RD | BT_WR);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not open the device");
        return EXIT_FAILURE ;
    }

    /* Clear any outstanding errors */
    status = bt_clrerr(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not clear errors from the device");
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }

    /* Do the actual mmap */
    status = bt_mmap(btd, &remote_p, remote_address, length, 
        BT_RD | BT_WR, BT_SWAP_DEFAULT);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not memory map the device");
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }
    addr_p = (volatile char *) remote_p;

    /* Lock the adaptor */
    status = bt_lock(btd, BT_FOREVER);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not lock the device");
        (void) bt_unmmap(btd, &remote_p, length);
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }

    /* Use the mapped pointer to print the memory contents */
    for (nlines = 0; nlines < NUM_LINES; nlines++) {

        buff[NUM_BYTES] = 0;

        for (my_index = 0; my_index < NUM_BYTES; my_index++) {
            data = *addr_p++;
            buff[my_index] = (isprint(data)) ? data : '.';
            printf("%02x ", data);
        }

        printf(" %s\n", buff);
    } /* end for number of lines of memory to print to screen */


    /* Unlock the adaptor  */
    status = bt_unlock(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not unlock the device");
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }

    /* Release the mmap resources */
    status = bt_unmmap(btd, remote_p, length);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not release the memory map");
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }

    /* Check for Adaptor status errors */
    status = bt_chkerr(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Memory mapped read failed");
        (void) bt_close(btd);
        return EXIT_FAILURE;
    }

    /* Close the device */
    status = bt_close(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not close the device");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS ;
} /* end dumpmem() function */


/*****************************************************************************
**
**      Function:   usage()
**
**      Purpose:    Prints command line arguments.
**
**      Args:       prog_p   Name of the program.  Usually argv[0].
**
**      Returns:    void
**
*****************************************************************************/
static void usage(
    char *prog_p
    )
{
    fprintf(stderr, "usage: %s -[tua]\n", prog_p);
    fprintf(stderr, "         -a <numb>    Address to Access  (default = 0x%08x)\n",
            DEF_ADDR);
    fprintf(stderr, "         -t <logdev>  Logical device     (default = %s)\n",
            bt_dev2str(BT_DEV_DEFAULT));
    fprintf(stderr, "         -u <unit>    Unit Number        (default = %d)\n",
            DEF_UNIT);
} /* end usage() */   
    
