/*****************************************************************************
**
**      Filename:   bt_reset.c
**
**      Purpose:    This example program uses the Nanobus extension to the
**                  SBS Bit 3 API to do a remote bus reset.
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-1999 by SBS Technologies Inc.
**        Copyright (c) 1996 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* LINT */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "btapi.h"

#if !defined(BT_NBUS_FAMILY) && !defined(BT_NBRIDGE_FAMILY) \
    && !defined(BT_NPORT_FAMILY) && !defined(BT_NANOLION_FAMILY)
#error This example can not be compiled for this family of products.
#endif /* !defined(BT_NBUS_FAMILY) && !defined(BT_NBRIDGE_FAMILY) \
          && !defined(BT_NPORT_FAMILY) */

/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/

static void usage(const char *prog_p);


/*****************************************************************************
**
**      Program:    bt_reset
**
**      Purpose:    This example program uses the Nanobus extension to the
**                  SBS Bit 3 API to do a remote bus reset.
**
**      Args:
**          -               Displays the command summary.
**          -t  <logdev>    Logical device. (BT_DEV_MEM, BT_DEV_IO, 
**                          BT_DEV_DEFAULT, etc.)
**                          Default is to BT_DEV_DEFAULT
**          -u  <unit>      Unit Number to open.  Default is unit 0.
**
*****************************************************************************/
int main (int argc, char ** argv) {
    bt_dev_t     logdev   = BT_DEV_DEFAULT;/* Logical device to open */
    int          unit     = 0;             /* Unit number to open */
    char * name_p         = *argv;         /* Program name */
    char * temp_p;                         /* For detecting errors in
                                              strtol */
    bt_desc_t    btd;                      /* Device descriptor */
    char         devname[BT_MAX_DEV_NAME]; /* Device name */
    bt_error_t   status;                   /* SBS Bit 3 API return value */
    int          main_ret = EXIT_SUCCESS;  /* Return value for main() */

    /* 
    ** parse command line arguments 
    */
    argc--; argv++;
    while (argc > 0) {
        /* check to see if it's a command line option.  */
        if ((**argv == '-') && (argv[0][1] != '\0') &&
            (argv[0][2] == '\0')) {
            switch(argv[0][1]) {
              case 't': /* set logical device */
                if (argc > 1) {
                    logdev = bt_str2dev(argv[1]);
                    if (logdev == BT_MAX_DEV) {
                        usage(name_p);
                        return EXIT_FAILURE;
                    }
                    argc--; argv++;
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
                argc--; argv++;
                break;
              default:
                usage(name_p);
                return EXIT_FAILURE;
            }
        } else {
            usage(name_p);
            return EXIT_FAILURE;
        }
        argc--; argv++;
    }
    /*
    ** Command line parsing completed.
    */

    /*
    ** Open the logical device
    */
    status = bt_open(&btd, bt_gen_name(unit, logdev, devname, BT_MAX_DEV_NAME),
                       BT_RDWR);
    if (status != BT_SUCCESS) {
        /* It is safe to call bt_perror with any status returned from
           bt_open() or bt_close() with the device descriptor, despite
           the fact that the descriptor is not valid to do anything
           else. */
        bt_perror(btd, status, name_p);
        return EXIT_FAILURE;
    }

    /* 
    ** Reset the remote bus.
    */
    status = bt_reset(btd);
    if (status != BT_SUCCESS) {
        bt_perror(btd, status, name_p);
        /* don't exit here- we still need to close the device */
        main_ret = EXIT_FAILURE;
    } else {
        puts ("Success!  Remote bus has been reset.");
    }

    /* 
    ** Close the logical device.
    */
    status = bt_close(btd);
    if (status != BT_SUCCESS) {
        /* this is safe to do */
        bt_perror(btd, status, name_p);
        return EXIT_FAILURE;
    }

    /*
    ** Exit the program 
    */
    return main_ret;

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
static void usage(const char * name_p) {
    printf ("%s is an example program demonstrating the use of the bt_reset()\n",
            name_p);
    puts   ("function of the Nanobus extension to the SBS Bit 3 C API.");
    puts   ("For more information, please consult the manual.\n");
    printf ("usage: %s [options]\n", name_p);
    puts   ("Options are:");
    puts   ("\t-\t\tDisplays this message and exits");
    puts   ("\t-t <logdev>\tLogical device. (BT_DEV_MEM, BT_DEV_IO,");
    puts   ("\t\t\tBT_DEV_DEFAULT, etc.)");
    puts   ("\t\t\tDefault is to BT_DEV_DEFAULT");
    puts   ("\t-u <unit>\tUnit Number to open.  Default is unit 0.");
}
