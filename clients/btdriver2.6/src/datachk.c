/****************************************************************************
**
**      Filename:    datachk.c
**
**      Purpose:     Perform a data pattern transfer and verify the data.
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
    
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <string.h>
#include    <errno.h>
#include    <time.h>
#include    "btapi.h"

/* Macros */
#define DEF_UNIT        0            /* default unit number */
#define DEF_ADDR        0x00010000   /* default address */
#define DEF_LENGTH      0xf0         /* default length */
#define DEF_COUNT       1            /* default count */
#define DEF_PWIDTH      BT_WIDTH_D32 /* default width */
#define ALLOC_ALIGN     (2048)       /* Size to align buffers to */
#define string(expr)    s2(expr)
#define s2(literal) #literal

#define ALIGN(value, align) ((ptrdiff_t) (((unsigned long) value) & ((align) -1)))

#define IS_ALIGNED(value,align) (0 == ALIGN(value,align))

/* Types */
typedef enum patterns_d {
    PAT_INC,            /* incrementing pattern */
    PAT_ALT,            /* 1's complament       */
    PAT_ROL,            /* Rotated Left on bit  */
    PAT_RAND            /* Random data pattern */
} patterns_t;

/*  Function prototypes */
static void dmpbuf(char buf[], int max_len, int start);
static void pat_rol(char *buffer, size_t access, size_t size, unsigned long seed);
static void pat_alt(char *buffer, size_t access, size_t size, unsigned long seed);
static void pat_inc(char *buffer, size_t access, size_t size, unsigned long seed);
static void pat_rand(char *buffer, size_t access, size_t size, unsigned long seed);
static void pat_xor(char *buffer, size_t access, size_t size);
static void usage(char *prg);



/*****************************************************************************
**
**      Program:    datachk
**
**      Purpose:    Reads and writes from device using a specific pattern
**                  and verifies that no data or status errors occured.
**
**      Args:
**          argc    ANSI C
**          argv    ANSI C
**
**      Modifies:
**   
**      Returns:
**          EXIT_SUCCESS    Completed successfully;
**          EXIT_FAILURE    Failed. 
**
*****************************************************************************/
int main(
    int argc,
    char **argv)
{
    char            *prog_p         = argv[0];
    bt_dev_t        type            = BT_DEV_DEFAULT;
    int             unit            = DEF_UNIT;
    unsigned long   remote_address  = DEF_ADDR;
    size_t          xfer_length     = DEF_LENGTH;
    int             xfer_count      = DEF_COUNT;
    size_t          xfer_misalign   = 0;
    patterns_t      pattern         = PAT_INC;
    size_t          pwidth          = DEF_PWIDTH;
    unsigned long   seed            = 0;
    int                retval          = EXIT_SUCCESS;

    char            *kern_addr      = NULL;    /* Fixed Kernel Address for buffer */
    char            *alloc_buf_p    = NULL;    /* Buffer from malloc() call */
    char            *wrbuf          = NULL;    /* Address of buffer to write */
    char            *rdbuf          = NULL;    /* Address of buffer to read */
    char            *c1_p           = NULL;
    char            *c2_p           = NULL;
    bt_desc_t       btd;
    char            devname[BT_MAX_DEV_NAME];
    int             pass;
    bt_error_t      status;
    size_t          xfer_done;
    int             rwflag = 1;             /* w & r = 1 r/o = 0 */

    /* Parse the command line */
    argc--;
    while (argv++, argc--) {

        if ('-' == **argv) {

            switch (*(*argv+1)) {

                case 'r': /* read only */
                    rwflag = 0;
                    break;

                case 'a': /* address location */
                    argv++;
                    argc--;
                    remote_address = strtoul(*argv, NULL, 0);
                    break;

                case 'c':  /* transfer count */
                    argv++;
                    argc--;
                    xfer_count = (int) strtoul(*argv, NULL, 0);
                    break;

                /* Not all devices supprot kernal address so use with caution. */
                case 'k':   /* kernal address */
                    argv++;
                    argc--;
                    kern_addr = (char *) strtoul(*argv, NULL, 0);
                    break;

                case 'l':  /* length */
                    argv++;
                    argc--;
                    xfer_length = strtoul(*argv, NULL, 0 );
                    break;
        
                case 'm':   /* miss align */
                    argv++;
                    argc--;
                    xfer_misalign = (size_t) strtoul(*argv, NULL, 0);
                    break;

                case 'p':  /* pattern */
                    argv++;
                    argc--;
                    pattern = (patterns_t) strtoul(*argv, NULL, 0);
                    break;

                case 's':  /* seed */
                    argv++;
                    argc--;
                    seed = strtoul(*argv, NULL, 0);
                    break;

                case 't':  /* logical type */
                    argc--;
                    argv++;
                    if ((type = bt_str2dev(*argv)) >= BT_MAX_DEV) {
                        fprintf(stderr, "Invalid access type: %s\n", *argv);
                        usage(prog_p);
                        return EXIT_FAILURE;
                    }
                    break;

                case 'u':  /* unit number */
                    argv++;
                    argc--;
                    unit = (int) strtoul(*argv, NULL, 0);
                    break;

                case 'w':  /* width */
                    argv++;
                    argc--;
                    pwidth = (size_t) strtoul(*argv, NULL, 0);
                    break;

                default:
                    usage(prog_p);
                    return EXIT_FAILURE;

            } /* end switch on argument type */

        } else {
            usage(prog_p);
            return EXIT_FAILURE;
        } /* end else bad arguments */

    } /* end while more arguments */

    xfer_misalign %= ALLOC_ALIGN;

    /*  Initialize the memory buffer that data is read into */
    if (NULL == kern_addr) {
        alloc_buf_p = malloc((xfer_length + ALLOC_ALIGN) *2 + xfer_misalign);
        if (NULL == alloc_buf_p) {
            perror("Malloc failure. ");
            return errno;
        }

        rdbuf = alloc_buf_p;
        if (!IS_ALIGNED(rdbuf, ALLOC_ALIGN)) {
            rdbuf = rdbuf + ALLOC_ALIGN - ALIGN(rdbuf, ALLOC_ALIGN);
        }

    } else {

        /* Assume they gave us a properly aligned address */
        rdbuf = kern_addr;
        alloc_buf_p = NULL;    /* Makes it safe to free(alloc_buf_p) */

    } /* end else kern address defined by user */

    wrbuf = rdbuf + xfer_length + ALLOC_ALIGN - ALIGN(xfer_length, ALLOC_ALIGN);

    rdbuf += xfer_misalign;
    wrbuf += xfer_misalign;

    /* Open the device and logical unit */
    status = bt_open(&btd, 
        bt_gen_name(unit,type,devname,BT_MAX_DEV_NAME), BT_RD | BT_WR);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status,"Could not open the device");
        free(alloc_buf_p);
        return EXIT_FAILURE;
    }

    /* Clear any errors on interface. */
    status = bt_clrerr(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status,"Could not clear errors");
        (void) bt_close(btd);
        free(alloc_buf_p);
        return EXIT_FAILURE;
    }

    /* Init the output buffer with the requested pattern  */
    switch ( pattern ) {

      case PAT_ALT:
          pat_alt(wrbuf, pwidth, xfer_length, seed);
          break;

      case PAT_ROL:
          pat_rol(wrbuf, pwidth, xfer_length, seed);
          break;

      case PAT_RAND:
          pat_rand(wrbuf, pwidth, xfer_length, seed);
	  break;

      case PAT_INC:
      default:
          pat_inc(wrbuf, pwidth, xfer_length, seed);
          break;

    } /* end switch pattern */

    /* Do xfer_count write/read passes */
    for (pass = 0; ((0 == xfer_count) || (pass < xfer_count)); ) {

    /*
    ** Increment the pass count.
    **
    ** Do not put this in the for() statement! Doing so causes
    ** it to report performing one too many transfers.
    */
        pass++;


        if (rwflag) {
            /* Write the data to the logical device */
            status = bt_write(btd,(void *) wrbuf,remote_address,xfer_length,&xfer_done);
            if (BT_SUCCESS != status) {

               fprintf(stderr,"Write failed pass %d status %d \n",pass,status);

            if (xfer_done != xfer_length) {
            fprintf(stderr, "bytes requested = 0x%1x, bytes written = 0x%1x status %d retval %d\n",
                (unsigned int) xfer_length, (unsigned int) xfer_done, 
		(unsigned int) status, (unsigned int)BT_EIO);
            }
       
                bt_perror(btd, status, __FILE__ " line " string(__LINE__) ":" );
                retval = EXIT_FAILURE;
                break;
            } 
        }


        /* Read the data from the logical device */
        memset((void *) rdbuf, 0, xfer_length);
        status = bt_read(btd,(void *) rdbuf,remote_address,xfer_length,&xfer_done);
        if (BT_SUCCESS != status) {
            fprintf(stderr,"Read failed pass %d\n",pass);

            if (xfer_done != xfer_length) {
                fprintf(stderr, "bytes requested = 0x%1x, bytes written = 0x%1x\n",
                (unsigned int) xfer_length, (unsigned int) xfer_done);
            }

            bt_perror(btd, status, __FILE__ " line " string(__LINE__) ":" );
            retval = EXIT_FAILURE;
            break;
        }


        /* Compare the two buffers and see if they match */
        if (memcmp(wrbuf, rdbuf, xfer_length) != 0) {
            fprintf(stderr,"Buffer Mismatch: Transfer Error, Pass %d\n",pass);

            /* Calculate offset of error and display */
            c1_p = wrbuf;
            c2_p = rdbuf;
            xfer_done = 0;
            while (xfer_done < xfer_length) {

                if (*c1_p != *c2_p) {
                    fprintf(stderr,"First mismatch at offset 0x%08lX\nSent:\n", 
                            (unsigned long) xfer_done);
                    dmpbuf(c1_p,xfer_length,(int) xfer_done);
                    fprintf(stderr,"Recv:\n");
                    dmpbuf(c2_p,xfer_length,(int) xfer_done);
                    break;
                } /* end if read buffer does not equal write buffer */

                c1_p++;
                c2_p++;
                xfer_done++;
            }  /* end while xter_done */

            /* Exit the bt_read()/bt_write() loop */
            retval = EXIT_FAILURE;
            break;

        } /* end if memcmp */

        /* Give some sort of status if running forever */
        if ((0==xfer_count) && (0 == (pass % 100))) {
            printf("Pass %d.\n", pass);
        }

        /* Invert the pattern buffer for next pass */
        pat_xor(wrbuf, pwidth, xfer_length);
    } /* end for pass loop */

    /* Free the data buffers */
    free(alloc_buf_p);

    /* Close the unit and print the summary */
    status = bt_close(btd);
    if (BT_SUCCESS != status) {
        bt_perror(btd, status, "Could not close the device");
        return EXIT_FAILURE;
    }

    printf(" %s Summary:\n", prog_p);
    printf("Buffer Size      = 0x%08x  %d\n", xfer_length, xfer_length);
    printf("Total Passes     = 0x%08x  %d\n", pass,  pass);

    if (EXIT_SUCCESS != retval) {
        printf("Failed to complete all passes.\n");
    }

    return retval;
} /* end datachk() */



/*****************************************************************************
**
**      Function:   pat_inc()
**
**      Purpose:    Fills buffer with an incrementing pattern.
**
**      Args:
**          buffer  Pointer to the buffer to fill.
**          access  Size of the pattern values (1, 2, or 4 bytes).
**          size    Number of bytes to fill with pattern.
**          seed    Starting pattern.
** 
**      Modifies:
**
**      Returns:
** 
*****************************************************************************/
static void pat_inc (
    char *buffer, 
    size_t access, 
    size_t size, 
    unsigned long seed
    )
{
    bt_data8_t    *d8_p = NULL;
    bt_data8_t    d8_val;
    bt_data16_t   *d16_p = NULL;
    bt_data16_t   d16_val;
    bt_data32_t   *d32_p = NULL;
    bt_data32_t   d32_val;

    d32_p = (bt_data32_t *) buffer;
    d16_p = (bt_data16_t *) buffer;
    d8_p  = (bt_data8_t  *) buffer;

    d32_val = seed;
    d16_val = (bt_data16_t) seed;
    d8_val = (bt_data8_t) seed;

    memset(buffer, 0, size);
    while (size / access > 0) {

        switch ( access ) {

          case BT_WIDTH_D32:
              *d32_p++ = d32_val++;
              break;

          case BT_WIDTH_D16:
              *d16_p++ = d16_val++;
              break;

          case BT_WIDTH_D8:
          default:
              *d8_p++ = d8_val++;
              break;

        } /* end switch */

        size -= access;
    } /* end while loop */
} /* end pat_inc() */

/*****************************************************************************
**
**      Function:   pat_alt()
**
**      Purpose:    Fills buffer with an alternating pattern.
**
**      Args:
**          buffer  Pointer to the buffer to fil.
**          access  Size of the pattern values (1, 2, or 4 bytes).
**          size    Number of bytes to fill with pattern.
**          seed    Starting pattern.
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void pat_alt(
    char *buffer, 
    size_t access, 
    size_t size, 
    unsigned long seed)
{

    bt_data8_t    *d8_p = NULL;
    bt_data8_t    d8_val;
    bt_data16_t   *d16_p = NULL;
    bt_data16_t   d16_val;
    bt_data32_t   *d32_p = NULL;
    bt_data32_t   d32_val;

    d32_p = (bt_data32_t *) buffer;
    d16_p = (bt_data16_t *) buffer;
    d8_p  = (bt_data8_t  *) buffer;

    d32_val = seed;
    d16_val = (bt_data16_t) seed;
    d8_val = (bt_data8_t) seed;

    memset(buffer, 0, size);

    while (size / access > 0) {

        switch ( access ) {

          case BT_WIDTH_D32:
              *d32_p++ = d32_val;
              d32_val  = ~d32_val;
              break;

          case BT_WIDTH_D16:
              *d16_p++ = d16_val;
              d16_val  = ~d16_val;
              break;

          case BT_WIDTH_D8:
          default:
              *d8_p++ = d8_val++;
              d8_val  = ~d8_val;
              break;

        } /* end switch */

        size -= access;

    } /* end while loop */
} /* end pat_alt() */

/*****************************************************************************
**
**      Function:   pat_rol()
**
**      Purpose:    Fills buffer with an alternating pattern.
**
**      Args:
**          buffer  Pointer to the buffer to fil.
**          access  Size of the pattern values (1, 2, or 4 bytes).
**          size    Number of bytes to fill with pattern.
**          seed    Starting pattern.
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void pat_rol(
    char *buffer, 
    size_t access, 
    size_t size, 
    unsigned long seed)
{
    bt_data8_t    *d8_p = NULL;
    bt_data8_t    d8_val;
    bt_data16_t   *d16_p = NULL;
    bt_data16_t   d16_val;
    bt_data32_t   *d32_p = NULL;
    bt_data32_t   d32_val;

    d32_p = (bt_data32_t *) buffer;
    d16_p = (bt_data16_t *) buffer;
    d8_p  = (bt_data8_t  *) buffer;

    d32_val = seed;
    d16_val = (bt_data16_t) seed;
    d8_val = (bt_data8_t) seed;

    memset(buffer, 0, size);

    while (size / access > 0) {

        switch (access) {

          case BT_WIDTH_D32:
              *d32_p++ = d32_val;
              if ((d32_val & 0x80000000) != 0 ) {
                  d32_val  <<= 1;
                  d32_val   |= 1;
              } else {
                  d32_val  <<= 1;
              }
              break;

          case BT_WIDTH_D16:
              *d16_p++ = d16_val;
              if ((d16_val & 0x8000) != 0 ) {
                  d16_val  <<= 1;
                  d16_val   |= 1;
              } else {
                  d16_val  <<= 1;
              }
              break;

          case BT_WIDTH_D8:
          default:
              *d8_p++ = d8_val++;
              if ((d8_val & 0x80) != 0 ) {
                  d8_val  <<= 1;
                  d8_val |= 1;
              } else {
                  d8_val <<= 1;
              }
              break;

        } /* end switch */
        size -= access;

    } /* end while loop */
} /* end pat_rol() */


/*****************************************************************************
**
**      Function:   pat_rand()
**
**      Purpose:    Puts random data into buffer.
**
**      Args:
**          buffer  Pointer to the buffer to fil.
**          access  Size of the pattern values (1, 2, or 4 bytes).
**          size    Number of bytes to fill with pattern.
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void pat_rand(
    char *buffer, 
    size_t access, 
    size_t size,
    unsigned long seed
    )
{
    int rand_seed;

    unsigned char  *c_p = NULL;
    unsigned short *s_p = NULL;
    unsigned long  *l_p = NULL;

    l_p   = (unsigned long  *) buffer;
    s_p   = (unsigned short *) buffer;
    c_p   = (unsigned char  *) buffer;

    if (0 == seed) {
        rand_seed = (int) clock;
	if (-1 != rand_seed) {
	    srand(rand_seed);
	    printf("Random seed used: %d.\n", rand_seed);
	}
    } else {
        srand((int) seed);
    }

    switch ( access ) {
      case BT_WIDTH_D32:
          while (size / access > 0) {
              *l_p++ = (((unsigned int) rand() & BT_D16_MASK) << BT_D16_SHFT) | 
	      		((unsigned int) rand() & BT_D16_MASK);
              size -= access;
          }
          break;

      case BT_WIDTH_D16:
          while (size / access > 0) {
              *s_p++ = ((unsigned int) rand() & BT_D16_MASK);
              size -= access;
          }
          break;

    case BT_WIDTH_D8:
    default:
        while (size / access > 0) {
            *c_p++ = ((unsigned int) rand() & BT_D8_MASK);
            size -= access;
        }
        break;

    } /* end switch */
} /* end pat_xor() */

/*****************************************************************************
**
**      Function:   pat_xor()
**
**      Purpose:    Inverts pattern currently in the buffer.
**
**      Args:
**          buffer  Pointer to the buffer to fil.
**          access  Size of the pattern values (1, 2, or 4 bytes).
**          size    Number of bytes to fill with pattern.
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void pat_xor(
    char *buffer, 
    size_t access, 
    size_t size
    )
{
    unsigned char  *c_p = NULL;
    unsigned short *s_p = NULL;
    unsigned long  *l_p = NULL;

    l_p   = (unsigned long  *) buffer;
    s_p   = (unsigned short *) buffer;
    c_p   = (unsigned char  *) buffer;

    switch ( access ) {
      case BT_WIDTH_D32:
          while (size / access > 0) {
              *l_p++ ^= BT_D32_MASK;
              size -= access;
          }
          break;

      case BT_WIDTH_D16:
          while (size / access > 0) {
              *s_p++ ^= BT_D16_MASK;
              size -= access;
          }
          break;

    case BT_WIDTH_D8:
    default:
        while (size / access > 0) {
            *c_p++ ^= BT_D8_MASK;
            size -= access;
        }
        break;

    } /* end switch */
} /* end pat_xor() */

/*****************************************************************************
**
**      Function:   dmpbuf()
**
**      Purpose:    Prints a range of data from the given buffer.
**
**      Args:
**          buf     Pointer to the first data item to print in the buffer.
**          max_len Size of the buffer in bytes.
**          start   Number of bytes from start of original buffer. 
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void dmpbuf(
    char buf[], 
    int max_len, 
    int start
    )
{

#define MAX_PRINT 32

    char  hex[80];
    char  ascii[80];
    int   tot_print = 0;
    int   curr_pos  = start;
    int   inx;

    unsigned char    curr_ch;
  
    /* Print 32 bytes starting at buf in hex and ascii, can't go past   */
    /*  end of buffer (max_len).                                        */
    hex[0] = ascii[0] = '\0';
    while ((curr_pos < max_len) && (tot_print < MAX_PRINT)) {
  
        /*  Print the starting offset for the line */
        sprintf((char *) &hex[0], "%08x  ", (unsigned) curr_pos);
        memset(ascii, '\0', sizeof(ascii));

        /* Print the hex and ascii value for each value */
        for (inx = 0; 
            (inx < 16) && (curr_pos < max_len) && (tot_print < MAX_PRINT); 
            inx++, curr_pos++, tot_print++) {

            curr_ch = buf[tot_print];
            sprintf((char *) &hex[strlen(hex)], "%02x ", curr_ch);
            ascii[strlen(ascii)] = (isprint(curr_ch) ? curr_ch : '.');
        }

        /* Fill remaining line with blanks */
        while (inx < 16) {
            strcat(hex, "   ");
            inx++;
        }

        strcat(ascii, "  ");
        printf("%s  %s\n", hex, ascii); 
        hex[0] = ascii[0] = '\0';
    } /* end while char to print */
} /* end dmpbuf() */

/*****************************************************************************
**
**      Function:   usage()
**
**      Purpose:    Prints information about the proper way to
**                  invoke the command.
**
**      Args:
**          prog_p  Pointer to program name.
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/
static void usage(
    char *prog_p
    )
{
    fprintf(stderr, "usage: %s -[talcuwps]\n", prog_p);
    fprintf(stderr, "         -a <numb>  Address to Access   (default = 0x%08x)\n",
            DEF_ADDR);
    fprintf(stderr, "         -c <numb>  Count of Transfers  (default = %d)\n",
            DEF_COUNT);
    fprintf(stderr, "         -l <numb>  Length of transfer  (default = 0x%08x)\n",
            DEF_LENGTH);
    fprintf(stderr, "         -m <misalign> Miss align (default = 0)\n");
    fprintf(stderr, "         -p <numb>  Fill Pattern: 0 = inc, 1 = 1's comp, 2 = ROL\n");
    fprintf(stderr, "         -r         Read only flag (Don't write first)\n");
    fprintf(stderr, "         -s <value> Value to seed pattern with\n");
    fprintf(stderr, "         -t         Logical device type (default = %s)\n",
            bt_dev2str(BT_DEV_DEFAULT));
    fprintf(stderr, "         -u <numb>  Unit Number         (default = %d)\n",
            DEF_UNIT);
    fprintf(stderr, "         -w <numb>  Width of Pattern    (default = %d bytes)\n",
            (int) DEF_PWIDTH);
} /* end usage() */
