/****************************************************************************
**
**      Filename:    bt_bind.c
**
**      Purpose:     Example program of binding a buffer to the remote bus.
**
**      $Revision$
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997,1998 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#if defined (BT1003)
#include <sys/types.h>
#include <asm/fcntl.h>
#include <asm/mman.h>
#include <scsi/sg.h>
#endif  /* BT1003 */

#include "btapi.h"

void print_buf (bt_devaddr_t addr, volatile char * buf_p, size_t plen);
/*****************************************************************************
**
**      Function prototypes
**
*****************************************************************************/

static void usage(const char *prog_p);


/*****************************************************************************
**
**      Program:    bt_bind
**
**      Purpose:    Binds a buffer to the remote bus, waits for input, and
**                  then prints the first 256 bytes to stdout.
**
**      Args:
**          -               Displays the command summary.
**          -t  <logdev>    Logical device. (BT_DEV_MEM, BT_DEV_IO, 
**                          BT_DEV_DEFAULT, etc.)
**                          Default is to BT_DEV_DEFAULT
**          -u  <unit>      Unit Number to open.  Default is unit 0.
**          -l  <len>       Length of the buffer to bind.
**                          default is one page.
**
*****************************************************************************/
int main (int argc, char ** argv) {
    bt_dev_t     logdev   = BT_DEV_DEFAULT; /* Logical device to open */
    int          unit     = 0;              /* Unit number to open */
    const char * name_p   = *argv;          /* Program name */
    void *       buf_p    = NULL;           /* buffer to bind - this
                                               gets allocated on the
                                               heap. */
    void *       bind_p   = NULL;          /* This is the aligned pointer
                                              that will actually be bound */
    size_t       buf_len  = 0;             /* original buffer size */
    size_t       bind_len = 0;             /* This is the aligned length-
                                              the amount that will actually
                                              be bound. */
    char *       temp_p;                   /* For detecting errors in
                                              strtol */
    bt_desc_t    btd;                      /* Device descriptor */
    char         devname[BT_MAX_DEV_NAME]; /* Device name */
    bt_devdata_t bind_align;               /* bind alignment restriction */
    bt_binddesc_t binddesc;                /* Bind descriptor */
    bt_devaddr_t bind_offset = BT_BIND_NO_CARE;  /* Bind offset */
    bt_error_t   status;                   /* SBS Bit 3 API return value */
    int          main_ret = EXIT_SUCCESS;  /* Return value for main() */
    int          chr;                      /* user input */
    bt_swap_t    swapping = BT_SWAP_DEFAULT;
                                           /* Swap values */

    /* try code vars */
    bt_devdata_t value;

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
              case 'l': /* set buffer length */
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
                  argc--; argv++;
                  break;
              case 's': /* set swap values */
                  if ((argc > 1) && (isdigit(argv[1][0]))) {
                      swapping = (bt_swap_t) strtoul(argv[1], &temp_p, 0);
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
    ** Query for required buffer alignment
    */
    status = bt_get_info(btd, BT_INFO_BIND_ALIGN, &bind_align);
    if (status != BT_SUCCESS) {
        bt_perror(btd, status, name_p);
        bt_close(btd);
        return EXIT_FAILURE;
    } else {

        printf ("Bind alignment is 0x%lX bytes\n", (unsigned long) bind_align);

        /*
        ** Align the buffer length
        */
        if ((buf_len % bind_align) != 0) {
            bind_len = buf_len  + bind_align - (buf_len % bind_align);
        } else {
            bind_len = buf_len;
        }

        printf ("Binding %lu (0x%lX) bytes\n", (unsigned long) bind_len,
            (unsigned long) bind_len);

        /*
        ** Allocate the buffer
        */
#if defined (BT1003)
        /* map a driver allocated kernel buffer to user space */

        /* set the kernel allocated buffer size */
        status = bt_set_info(btd, BT_INFO_KMALLOC_SIZ, buf_len);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not set kmalloc buffer size");
            bt_close(btd);
            return EXIT_FAILURE;
        }

        /* get a kernel allocated buffer */
        status = bt_get_info(btd, BT_INFO_KMALLOC_BUF, &value);
        printf ("Value of %s is 0x%lX status = %d\n", "BT_INFO_KMALLOC_BUF", (unsigned long) value, status);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not get kmalloc buffer");
            bt_close(btd);
            return EXIT_FAILURE;
        }            
        
        /* Do the mmap to map the kernel (kmalloc) buffer to user space */
        status = bt_mmap(btd, &buf_p, value, bind_len, BT_RD | BT_WR, BT_SWAP_DEFAULT);
        if (BT_SUCCESS != status) {
            bt_perror(btd, status, "Could not memory map the device");
            bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
            bt_close(btd);
            return EXIT_FAILURE;
        }
        if (buf_p == NULL) {
            fprintf(stderr, "%s: ERROR- could not allocate buffer.\n",
                    name_p);
            bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
            bt_close(btd);
            return EXIT_FAILURE;
        } else {
            printf ("Buffer init string = %s\n", (char *)buf_p);

            /* write a string into the buffer, read from remote with dumpdata, (comment out memset below!) to see this */
            sprintf((char *)((unsigned int)buf_p + 0x30), "User application set string\n");

            memset(buf_p, 0, buf_len);
            bind_p = (void *) buf_p;

            /* We now have a buffer (bind_p) and a length (bind_len) which
               fullfill bt_bind()'s alignment restrictions. */

            /*
            ** Bind the buffer
            */
            status = bt_bind(btd, &binddesc, &bind_offset, bind_p, bind_len,
                             BT_RDWR, swapping);
            if (status != BT_SUCCESS) {
                bt_perror(btd, status, name_p);
                bt_unmmap(btd, buf_p, bind_len);
                bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                bt_close(btd);
                return EXIT_FAILURE;
            } else {

                /*
                ** Lock the unit while any remote memory accesses are
                ** occuring.
                */
                status = bt_lock(btd, BT_FOREVER);
                if (status != BT_SUCCESS) {
                    bt_perror(btd, status, name_p);
                    bt_unmmap(btd, buf_p, bind_len);
                    bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                    bt_close(btd);
                    return EXIT_FAILURE;
                } else {
                    printf ("Buffer bound to remote memory offset 0x%08lX\n",
                            (unsigned long) bind_offset);

                    /*
                    ** Wait for user input to signal the remote accesses 
                    ** are complete.
                    */
                    chr = getchar();
                    while ((chr != 'q') && (chr != 'Q') && (chr != EOF)) {
                        chr = getchar();
                    }

                    /*
                    ** Unlock the unit
                    */
                    status = bt_unlock(btd);
                    if (status != BT_SUCCESS) {
                        bt_perror(btd, status, name_p);
                        bt_unmmap(btd, buf_p, bind_len);
                        bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                        bt_close(btd);
                        return EXIT_FAILURE;
                    }
                }

                /*
                ** Unbind the buffer
                */
                status = bt_unbind(btd, binddesc);
                if (status != BT_SUCCESS) {
                    bt_perror(btd, status, name_p);
                    bt_unmmap(btd, buf_p, bind_len);
                    bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                    bt_close(btd);
                    return EXIT_FAILURE;
                } else {
                    /*
                    ** Print buffer to stdout
                    */
                    print_buf(bind_offset, bind_p, 
                              (bind_len > 256)?256:bind_len);
                }

                /* Release the mmap resources */
                status = bt_unmmap(btd, buf_p, bind_len);
                if (BT_SUCCESS != status) {
                    bt_perror(btd, status, "Could not release the memory map");
                    bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                    (void) bt_close(btd);
                    return EXIT_FAILURE;
                }

                /* free the driver allocated kernel buffer */
                status = bt_set_info(btd, BT_INFO_KFREE_BUF, 0);
                if (BT_SUCCESS != status) {
                    bt_perror(btd, status, "Could not free the kmalloc buffer");
                    bt_close(btd);
                    return EXIT_FAILURE;
                }
            }
        }
#else   /* BT1003 */
        /* Note: we allocate an extra bind_align bytes to make sure we
           can align the starting address. */
        buf_p = malloc(bind_len + bind_align);
        if (buf_p == NULL) {
            fprintf(stderr, "%s: ERROR- could not allocate buffer.\n",
                    name_p);
            main_ret = EXIT_FAILURE;
        } else {
            memset(buf_p, 0, buf_len);

            /*
            ** Align the buffer
            */
            bind_p = (void *) (((bt_data8_t *) buf_p) + 
                               BT_ALIGN_PTR(buf_p, bind_align));

            /* We now have a buffer (bind_p) and a length (bind_len) which
               fullfill bt_bind()'s alignment restrictions. */

            /*
            ** Bind the buffer
            */
            status = bt_bind(btd, &binddesc, &bind_offset, bind_p, bind_len,
                             BT_RDWR, swapping);
            if (status != BT_SUCCESS) {
                bt_perror(btd, status, name_p);
                main_ret = EXIT_FAILURE;
            } else {

                /*
                ** Lock the unit while any remote memory accesses are
                ** occuring.
                */
                puts("Locking the unit.");
                status = bt_lock(btd, BT_FOREVER);
                if (status != BT_SUCCESS) {
                    bt_perror(btd, status, name_p);
                    main_ret = EXIT_FAILURE;
                } else {
                    printf ("Buffer bound to remote memory offset 0x%08lX\n",
                            (unsigned long) bind_offset);

                    /*
                    ** Wait for user input to signal the remote accesses 
                    ** are complete.
                    */
                    chr = getchar();
                    while ((chr != 'q') && (chr != 'Q') && (chr != EOF)) {
                        chr = getchar();
                    }

                    /*
                    ** Unlock the unit
                    */
                    status = bt_unlock(btd);
                    if (status != BT_SUCCESS) {
                        bt_perror(btd, status, name_p);
                        main_ret = EXIT_FAILURE;
                    }
                }

                /*
                ** Unbind the buffer
                */
                status = bt_unbind(btd, binddesc);
                if (status != BT_SUCCESS) {
                    bt_perror(btd, status, name_p);
                    main_ret = EXIT_FAILURE;
                } else {
                    /*
                    ** Print buffer to stdout
                    */
                    print_buf(bind_offset, bind_p, 
                              (bind_len > 256)?256:bind_len);
                }
            }
        }
#endif  /* BT1003 */
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
    printf ("%s is an example program demonstrating the use of the\n",
            name_p);
    puts   ("bt_read() and bt_write() functions of the SBS Bit 3 C API.");
    puts   ("For more information, please consult the manual.\n");
    printf ("usage: \n%s -l <numtrans> [options]\n", name_p);
    puts   ("\tRead <numtrans> bytes from the remote bus and write to");
    puts   ("\tstandard out.");
    printf ("%s [options]\n", name_p);
    puts   ("\tRead from the standard in until an end of file marker is");
    puts   ("\treached, writing the data to the remote bus.");
    puts   ("Options are:");
    puts   ("\t-\t\tDisplays this message and exits");
    puts   ("\t-t <logdev>\tLogical device. (BT_DEV_MEM, BT_DEV_IO,");
    puts   ("\t\t\tBT_DEV_DEFAULT, etc.)");
    puts   ("\t\t\tDefault is to BT_DEV_MEM");
    puts   ("\t-u <unit>\tUnit Number to open.  Default is unit 0.");
    puts   ("\t-l <buflen>\tSets the size of the buffer to bind.");
    puts   ("\t\t\tThe program may increase this size to fullfill");
    puts   ("\t\t\tbind alignment restrictions.");
    puts   ("\t-s <swapbits>\tSets the swap bits value for the call to");
    puts   ("\t\t\tbt_bind().  Note that the symbolic names are not");
    puts   ("\t\t\trecognized.");
}

/*****************************************************************************
**
**  Name:       print_buf
**
**  Purpose:    Prints out a buffer to stdout.
**
**  Args:
**      bt_devaddr_t addr       The address of the buffer.  This is used to
**                              format the output correctly, and is also
**                              printed as part of the output.
**      volatile char * buf_p   The buffer to print.  This may be a pointer
**                              into a memory mapped or bound area, or a
**                              pointer to standard application memory.
**      size_t plen             The length of the buffer to print.
**
**  Modifies:
**      Writes to stdout.
**      
**  Returns:
**      void        Output is not checked for errors.
**
**  Notes:
**      A standard line of output looks like:
**          0x80200000 0x65 0x66 ... 0x73 0x74 ABCDEFGHIJKLMNOP
**      It breaks down into three parts:
**          The address of the first byte of the line.  Lines are always
**          16-byte aligned.
**          The hexadecimal values of the bytes of the line.  Any bytes
**          which fall outside of the area requested to be printed are
**          replaced by spaces.
**          The character representations of the bytes of the line.  Any
**          bytes which fall outside of the area requested to be printed
**          are replaced by spaces.  Any byes which are not printable
**          characters (isprint() returns false for the character)
**          are replaced by periods ('.').
**
*****************************************************************************/
void print_buf (bt_devaddr_t addr, volatile char * buf_p, size_t plen) {
    unsigned int offset = (unsigned int) (addr & 15);
    char dbuf[17] = "                ";
    size_t idx;
    char data;


    /*
    **  The basic strategy we follow is to output the hexadecimal values
    **  as we read them in, and save their character representations in
    **  an array.  At the end of the line, we print the array, and (if
    **  there is going to be another line) the address for the next line.
    */

    /* Start the first line.  Note that we have to deal with situations
       where addr is not 16-byte aligned. */
    addr -= offset;
    printf ("0x%08lX: ", (unsigned long) addr);
    for (idx = 0; idx < offset; ++idx) {
        fputs ("   ", stdout);
    }

    /* Loop through reading the data and print the lines as nessecary. */
    for (idx = 0; idx < plen; ++idx) {
        data = *buf_p;
        buf_p++;
        if (isprint(data)) {
            dbuf[offset] = data;
        } else {
            dbuf[offset] = '.';
        }
        printf("%02X ", ((unsigned int) data) & 255);
        offset++;
        if (offset == 16) {
            fputc (' ', stdout);
            puts(dbuf);
            offset = 0;
            if (idx < (plen - 1)) {
                addr += 16;
                printf ("0x%08lX: ", (unsigned long) addr);
            }
        }
    }

    /* Finish off the last line.  Note that the final address might not
       be 16-byte aligned. */
    if (offset > 0) {
        memset (dbuf + offset, ' ', 16 - offset);
        while (offset < 16) {
            fputs("   ", stdout);
            offset++;
        }
        fputc(' ', stdout);
        puts(dbuf);
    }

}

