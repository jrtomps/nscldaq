/*****************************************************************************
**
**        Filename:    bt_cat.c
**
**        Purpose:     Either reads from stdin and writes to the addr
**                     or reads from the addr and writes to stdout.
**
**        $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**          Copyright (c) 1997,1998 by SBS Technologies, Inc.      
**          Copyright (c) 1996,1997 by Bit 3 Computer Corporation.
**                             All Rights Reserved.
**                  License governs use and distribution.
**
*****************************************************************************/
	
#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* LINT */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "btapi.h"

#define DEF_ADDR 0x00010000    /* default address */
#define DEF_UNIT 0             /* default unit */
#define DEF_BUF_LEN 4096       /* default buffer length */

/*****************************************************************************
**
**        Function prototypes
**
*****************************************************************************/

static void usage(const char *prog_p);


/*****************************************************************************
**
**        Program:     bt_cat
**
**        Purpose:     Either reads from stdin and writes to the addr
**                     or reads from the addr and writes to stdout.
**
**        Args:
**             -                Displays the command summary.
**             -a  <addr>       Address to read from or write to.
**                              Default is offset 0.
**             -b  <buflen>     Sets the buffer size.  bt_cat attempts to read
**                              and write multiples of this value.
**                              Default is 4096.
**             -l  <trans>      If this argument is given, bt_cat reads trans
**                              bytes from the addr and writes them to
**                              stdout.
**                              If this argument is not given, bt_cat reads
**                              from stdin until it reaches the end of file,
**                              and writes the data to the addr.
**             -t  <logdev>     Logical device. (BT_DEV_MEM, BT_DEV_IO, 
**                              BT_DEV_DEFAULT, etc.)
**                              Default is to BT_DEV_DEFAULT
**             -u  <unit>       Unit Number to open.  Default is unit 0.
**
*****************************************************************************/
int main (
     int argc,
    char ** argv
    ) 
{     
    bt_dev_t     logdev    = BT_DEV_DEFAULT;/* Logical device to open */
    int          unit      = DEF_UNIT;      /* Unit number to open */
    bt_devaddr_t remaddr   = DEF_ADDR;      /* Address to read from or write to */
    const char   *name_p   = *argv;         /* Program name */
    size_t       buf_len   = DEF_BUF_LEN;   /* buffer size */
    char         *temp_p   = NULL;          /* For detecting errors in strtol */
    bt_desc_t    btd;                       /* Device descriptor */
    char         devname[BT_MAX_DEV_NAME];  /* Device name */
    bt_error_t   status;                    /* SBS Bit 3 API return value */
    int          main_ret  = EXIT_SUCCESS;  /* Return value for main() */
  
    /* read/write buffer -  this gets allocated on the heap. */
    char         *buf_p    = NULL;
     
    /* number of bytes to transfer. Only used if reading from the remote bus. */
    size_t       trans_len = 0;                

    /* 0 == read from stdin, 1 == read from addr */
    int          direction = 0; 

    size_t       xfer_rem;    /* number of bytes left to transfer */
    size_t       xfer_done;   /* number of bytes actually transferred. */
    size_t       num_wrote;   /* number of bytes returned by fwrite()/fread() */



     /*  parse command line arguments */
     argc--;
     argv++;
     while (argc > 0) {

         /* check to see if it's a command line option.  */
         if ((**argv == '-') && (argv[0][1] != '\0') && (argv[0][2] == '\0')) {

             switch(argv[0][1]) {

                 case 'a': /* set address */
                     if ((argc > 1) && (isdigit(argv[1][0]))) {
                         remaddr = (bt_devaddr_t) strtoul(argv[1], &temp_p, 0);
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

                case 'b': /* set buffer length */
                     if ((argc > 1) && (isdigit(argv[1][0]))) {
                         buf_len = (size_t) strtoul(argv[1], &temp_p, 0);
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

                 case 'l': /* set transfer length */
                     if ((argc > 1) && (isdigit(argv[1][0]))) {
                         direction = 1; /* we are reading from the remote bus. */
                         trans_len = (size_t) strtoul(argv[1], &temp_p, 0);
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

                default:
                     usage(name_p);
                     return EXIT_FAILURE;
             } /* end switch statment */

         } else {
             usage(name_p);
             return EXIT_FAILURE;
         } /* end else bad argument */
          
         argc--;
         argv++;
     } /* end while more arguments */

     /* Command line parsing completed. */

     /* Allocate the buffer */
     buf_p = malloc(buf_len);
     if (NULL == buf_p) {
         fprintf(stderr, "%s: ERROR- could not allocate buffer.\n",name_p);
         return EXIT_FAILURE;
     }
     memset(buf_p, 0, buf_len);

     /* Open the logical device */
     status = bt_open(&btd, bt_gen_name(unit, logdev, devname, 
         BT_MAX_DEV_NAME),BT_RDWR);
     if (BT_SUCCESS != status) {
         /* It is safe to call bt_perror with any status returned from  */
         /* bt_open() or bt_close() with the device descriptor, despite */
         /* the fact that the descriptor is not valid to do anything    */
         /* else.                                                       */
         bt_perror(btd, status, "Could not open the device");
         free(buf_p);
         return EXIT_FAILURE;
     }

     /* Transfer the data */
     if (1 == direction) {

         /* we are reading from the addr and writing to stdout. */
         /* trans_len has been set to the length to transfer.    */
         xfer_rem = trans_len;
         while (xfer_rem > 0) {

             /* Read the data from the addr */
             xfer_done = 0;
             status = bt_read(btd, buf_p, remaddr, 
                 ((xfer_rem > buf_len)?buf_len:xfer_rem), &xfer_done);
             if (BT_SUCCESS != status) {
                 bt_perror(btd, status, "Could not read from the device");
                 fprintf(stderr, "Only read %lu bytes of %lu bytes.\n",
                     (unsigned long) (trans_len - xfer_rem),
                     (unsigned long) trans_len);
                  main_ret = EXIT_FAILURE;
                  break; /* while loop */
             }

             /* write the data to stdout */
             num_wrote = fwrite(buf_p, sizeof(char), xfer_done, stdout);
             if (num_wrote != xfer_done) {
                 fprintf(stderr, "ERROR: only wrote %lu bytes of %lu to"
                     " standard out!\n", (unsigned long) num_wrote,
                     (unsigned long) xfer_done);
                 perror("fwrite failed. ");
                 main_ret = EXIT_FAILURE;
                 break; /* while loop */
             }

             xfer_rem -= xfer_done;
             remaddr += xfer_done;
         } /* end while transfer remaining loop */

     } else {

         /* we are reading from stdin and writing to the addr.        */
         /* trans_len has not be set, so we continue to read unit we   */
         /* hit the end of file.                                       */
         while (!feof(stdin)) {
             /* read the data from stdin */
             num_wrote = fread(buf_p, sizeof(char), buf_len, stdin);
             if (num_wrote < 0) {
                 perror("fread failed. ");
                 main_ret = EXIT_FAILURE;
                 break; /* while loop */
	     } else if (num_wrote == 0) {
		 perror("fread read only eof in its final read");
		 break;
             }

             /* write data to addr */
             status = bt_write(btd, buf_p, remaddr, num_wrote, &xfer_done);
             if (BT_SUCCESS != status) {
                 bt_perror(btd, status, "Could not write to the device");
                 main_ret = EXIT_FAILURE;
                 break; /* while loop */
             }

             remaddr += xfer_done;
         }  /* end while not end of file loop */

     } /* end else direction reading from stdin and writing to addr */

     free(buf_p);

     /* Close the logical device. */
     status = bt_close(btd);
     if (BT_SUCCESS != status) {
          /* this is safe to do */
          bt_perror(btd, status, "Could not close the device");
          return EXIT_FAILURE;
     }

     /* Exit the program */
     return main_ret;

} /* end bt_cat() */


/*****************************************************************************
**
**        Function:    usage()
**
**        Purpose:     Print a help message for the program.
**
**        Args:        name_p        Name of the program- usually argv[0]
**
**        Returns:     void
**
*****************************************************************************/
static void usage(const char * name_p) {
     printf ("%s is an example program demonstrating the use of the\n", name_p);
     puts    ("bt_read() and bt_write() functions of the SBS Bit 3 MIRROR API.");
     puts    ("For more information, please consult the manual.\n");
     printf  ("usage: \n%s -l <trans> [options]\n", name_p);
     puts    ("\tRead <trans> bytes from the addr and write to");
     puts    ("\tstandard out.");
     printf  ("%s [options]\n", name_p);
     puts    ("\tRead from the standard in until an end of file marker is");
     puts    ("\treached, writing the data to the addr.");
     puts    ("Options are:");
     puts    ("\t-\t\tDisplays this message and exits");
     puts    ("\t-a <addr>\tAddress to read from or write to.");
     printf  ("\t\t\tDefault is address 0x%x.\n", DEF_ADDR);
     puts    ("\t-b <buflen>\tSets the buffer size.  Transfers are done in");
     puts    ("\t\t\tblocks of bytes of the buffer size whenever possible.");
     puts    ("\t-l <trans>\tIf this argument is given, bt_cat reads trans");
     puts    ("\t\t\tbytes from the addr and writes them to stdout.");
     puts    ("\t\t\tIf this argument is not given, bt_cat reads from stdin");
     puts    ("\t\t\tuntil end of file, and writes the data to the addr.");
     puts    ("\t-t <logdev>\tLogical device. (BT_DEV_MEM, BT_DEV_IO,");
     puts    ("\t\t\tBT_DEV_DEFAULT, etc.)");
     puts    ("\t\t\tDefault is to BT_DEV_MEM");
     puts    ("\t-u <unit>\tUnit Number to open.");
     printf  ("\t\t\tDefault is unit %d.\n", DEF_UNIT);
       
} /* end usage() */
