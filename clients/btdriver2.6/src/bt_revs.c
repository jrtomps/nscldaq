/******************************************************************************
**
**      Filename:       bt_revs.c
**
**      Purpose:        This program opens the device driver and prints the 
**                      driver version and the hardware firmware revision.
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2002 by SBS Technologies, Inc.
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

#include        "btapi.h"

/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/
static void usage(char *prg);



/*****************************************************************************
**
**      Program:    bt_revs
**
**      Purpose:    Simply opens a specific device and prints the driver and
**                  firmware revision.
**
**      Args:
**
**      Calls: 
**
*****************************************************************************/
int     main(int argc, char **argv)
{
    int             unit = 0;
    char            devname[BT_MAX_DEV_NAME];
    
    bt_desc_t       btd;
    bt_dev_t        ldev = BT_DEV_MEM;
    bt_error_t      status;

    char            *prog_p = argv[0];
    bt_devdata_t    line_num;
    bt_data8_t      rev_info[BT_DIAG_MAX_REV_INFO];

    /*
    **  Parse the command line
    */
    argc--;
    while (argv++, argc--) {

        if (!strcmp(*argv, "-t")) {
            argv++; argc--;
            ldev = bt_str2dev(*argv);
            if (ldev == BT_MAX_DEV) {
                fprintf(stderr, "Invalid access type: %s\n", *argv);
                return(1);
            }

        } else if (!strcmp("-u", *argv)) {
            argv++; argc--;
            unit = strtol(*argv, NULL, 0);
            
        } else {
            usage(prog_p);
            return(1);
        }
    }

    /*
    ** Open the logical device
    */
    status = bt_open(&btd, bt_gen_name(unit, ldev, devname, BT_MAX_DEV_NAME),
                       BT_RDWR);
    if (status != BT_SUCCESS) {
        bt_perror(btd, status, "bt_open");
        return(1);
    }

    /*
    ** Retrieve the local and remote card part numbers
    */
    if ((status = bt_get_info(btd, BT_INFO_LOC_PN, &line_num)) != BT_SUCCESS) {
        bt_perror(btd, status, "bt_get_info failed");
    } else {
        printf("Local board PN: %d\n", line_num);
    }
    if ((status = bt_get_info(btd, BT_INFO_REM_PN, &line_num)) != BT_SUCCESS) {
        bt_perror(btd, status, "bt_get_info failed");
    } else {
        printf("Remote board PN: %d\n", line_num);
    }

    /*
    ** Retrieve the driver and firmware revision
    */
    if ((status = bt_get_info(btd, BT_INFO_BOARD_REV, &line_num)) != BT_SUCCESS) {
        bt_perror(btd, status, "bt_get_info failed");
    } else {
        printf("Local board's firmware revision: %c, 0x%x\n", line_num, line_num);
    }
    if ((status = bt_driver_version(btd, (char *)&rev_info[0], (int *)&line_num)) != BT_SUCCESS) {
        bt_perror(btd, status, "bt_driver_version failed");
    } else {
        printf("Driver version: %s\n", rev_info);
    }

    /*
    ** Close the adapter
    */
    if ((status = bt_close(btd)) != BT_SUCCESS) {
        bt_perror(btd, status, "bt_close");
        return(1);
    }
    
    return(0);
}


/*****************************************************************************
**
**      Function:   usage()
**
**      Purpose:    Prints command line arguments.
**
**      Args:       
**          prog_p  Pointer to program name.
**
**      Returns:    void
**
**      Calls:      fprintf()
**
*****************************************************************************/

static void    usage(
    char *prog_p)
{
    printf("\nTool to test basic driver open/close/lock.\n");
    printf("USAGE: %s -tuL\n", prog_p);
    printf("  -t <type>     Which access type to test:\n");
    printf("                  a16 - remote I/O (A16); a32 - remote RAM (A32)\n");
    printf("  -u <unit>     Which adaptor to test\n");
    printf("  -l <bytes>    Lenght of remote memory to test\n");
    printf("  -a <addr>     Address of remote memory to test\n");
    printf("  -c <pass>     Pass count\n");
    printf("  -s <test>     Test to run; 0 - all; 1-4 only that test\n");
    printf("          Numeric values use C radix notation\n");
}

