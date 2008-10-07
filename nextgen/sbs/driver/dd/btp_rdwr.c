/******************************************************************************
**
**      Filename:       btp_rdwr.c
**
**      Purpose:        Device read and write entry points for Linux driver.
**
**      Functions:      btp_llseek(), btp_read(), btp_write(), 
**                      btp_lm_rd(), btp_lm_wr()
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $" __DATE__;
#endif  /* LINT */

#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/pagemap.h>      /* Needed for page_address() */
#include "btdd.h"

const char *no_unit_p = "Unit not initialized.\n";

/*
**  Function prototypes of external functions used within
*/
extern void btk_dma_pio(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr_p, bt_data32_t raddr, int *xfer_length_p, int *dma_flag_p, int *data_width_p, unsigned *start_p, unsigned *need_p);
extern bt_error_t btk_pio_xfer(bt_unit_t *unit_p, bt_dev_t ldev, caddr_t laddr_p, bt_data32_t ldev_addr, int *xfer_length_p, size_t xfer_dir);
#if defined(__linux__)
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int *xfer_length, int xfer_dir, int data_width);
#else
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int xfer_length, int xfer_dir, int data_width);
#endif
extern bt_error_t btk_take_drv_sema(bt_unit_t *unit_p);
extern void btk_give_drv_sema(bt_unit_t *unit_p);

/*
**  Function prototypes of static functions used within
*/
static int btp_xfer(bt_unit_t *unit_p, bt_dev_t type, bt_accessflag_t dir, void * data_p, unsigned long dest_addr, size_t length, size_t *xferred_bytes);
static ssize_t btp_lm_rd(struct file *file_p, char *data_p, size_t length, loff_t *new_fpos_p);
static ssize_t btp_lm_wr(struct file *file_p, const char *data_p, size_t length, loff_t *new_fpos_p);


/*
**  Function prototypes externally visible functions
*/
unsigned long bt_kvm2bus(void * vm_addr_p);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_RDWR_C);


/*****************************************************************************
**
**      Name:           btp_llseek()
**
**      Purpose:        Repositions read/write() position in device
**
**      Args:
**          file_p      Pointer to file structure
**          offset      File offset to position to
**          which       Method for positioning: SEEK_CUR, SEEK_END, SEEK_SET
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/

loff_t btp_llseek(
    struct file *file_p,
    loff_t offset,
    int which
    )
{
    FUNCTION("btp_llseek");
    LOG_DEVID(file_p);
    bt_dev_t type = GET_LDEV_TYPE(file_p);

    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    loff_t retval = 0;
    loff_t f_pos, end_pos;

    FENTRY;

    /* Determine where the end of the various logical devices are */
    switch(type) {
      case BT_DEV_RDP:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
            end_pos = 8 * SIZE_1MB;
        } else if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            end_pos = 192 * SIZE_1KB;
        } else {
            end_pos = 0;
        }
        break;
      case BT_DEV_LDP:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            end_pos = 0;
        } else {
            end_pos = 192 * SIZE_1KB;
        }
        break;
    case BT_DEV_GEO:
      case BT_DEV_A24:
        /* 24 address bits */
        end_pos = 16 * SIZE_1MB;
        break;

      case BT_DEV_IO:
        /* 16 address bits */
        end_pos = 64 * SIZE_1KB;
        break;
    case BT_DEV_A32:
    case BT_DEV_MCCTL:  
    case BT_DEV_CBLT:
        /* 32 address bits */
        end_pos = ((loff_t) 1)<<32;
        break;

      case BT_DEV_RE:
        /* 31 address bits, only provided for compatibility */
        end_pos = ((loff_t) 1)<<31;
        break;

      case BT_DEV_LM:
        /* User determined when device was loaded */
        end_pos = unit_p->lm_size;
        if (0 == end_pos) {
            INFO_STR("Local Memory device not enabled.\n");
            retval = -ENXIO;
            goto llseek_end;
        }
        break;

      default:
        TRC_MSG(BT_TRC_RD_WR, (LOG_FMT "Unrecognized device type: %d.\n",
            LOG_ARG, type));
        retval = -ENXIO;
        goto llseek_end;
    }

    /* Determine what the new address would be */
    switch(which) {
      case SEEK_SET:
        f_pos = offset;
        break;

      case SEEK_CUR:
        f_pos = file_p->f_pos + offset;
        break;

      case SEEK_END:
        f_pos = end_pos + offset;       /* Better be a negative offset */
        break;

      default:  /* Should never happen */
        INFO_STR("Invalid SEEK type.\n");
        retval = -EINVAL;
        goto llseek_end;
    }

    if (f_pos >= end_pos) {
        INFO_STR("Seek past end of logical device.\n");
        retval = -EINVAL;
        goto llseek_end;
    }

    file_p->f_pos = f_pos;
    retval = f_pos;

llseek_end:
    FEXIT(retval);
    return retval;
}


/*****************************************************************************
**
**      Name:           btp_read()
**
**      Purpose:        Read data from the specified logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data to
**          length      Number of bytes to transfer
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/

ssize_t btp_read(
    struct file *file_p,
    char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_read");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    int             ret_val;
    long            xferred_bytes = 0;
    unsigned long   dest_addr;
    size_t          amount_xferred;


    FENTRY;

    /*
    ** Handle local memory
    */
    if (type == BT_DEV_LM) {
        xferred_bytes = btp_lm_rd(file_p, data_p, length, new_fpos_p);
        goto btp_read_end;
    }

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /* 
    ** Check if this device supports reads
    */
    if ((unit_p->logstat[type] & STAT_READ) == 0) {
        INFO_STR("Logical device does not support reading");
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Read  logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Do the transfer by either PIO or DMA 
    */
    ret_val = btp_xfer(unit_p, type, BT_RD, data_p, dest_addr, length, &amount_xferred);
    if (amount_xferred > 0) {
        *new_fpos_p = dest_addr + amount_xferred;
        xferred_bytes = amount_xferred;
    } else {
        xferred_bytes = ret_val;
    }

btp_read_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_write()
**
**      Purpose:        Write data to the specified logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data from
**          length      Number of bytes to transfer
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/

ssize_t btp_write(
    struct file *file_p,
    const char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_write");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    int             ret_val;
    long            xferred_bytes = 0;
    unsigned long   dest_addr;
    size_t          amount_xferred;


    FENTRY;

    /*
    ** Handle local memory
    */
    if (type == BT_DEV_LM) {
        xferred_bytes = btp_lm_wr(file_p, data_p, length, new_fpos_p);
        goto btp_write_end;
    }

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /* 
    ** Check if this device supports writes
    */
    if ((unit_p->logstat[type] & STAT_WRITE) == 0) {
        INFO_STR("Logical device does not support writing");
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Write logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Do the transfer by either PIO or DMA 
    */
    ret_val = btp_xfer(unit_p, type, BT_WR, (char *) data_p, dest_addr, length, &amount_xferred);
    if (amount_xferred > 0) {
        *new_fpos_p = dest_addr + amount_xferred;
        xferred_bytes = amount_xferred;
    } else {
        xferred_bytes = ret_val;
    }

btp_write_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_lm_rd()
**
**      Purpose:        Read data from the local memory logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data to
**          length      Number of bytes to transfer
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/

static ssize_t btp_lm_rd(
    struct file *file_p,
    char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_lm_rd");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    long            xferred_bytes = 0; 
    unsigned long   dest_addr;
    char            *curr_buf_p;
    size_t          len;
    loff_t          f_pos = file_p->f_pos;

    FENTRY;

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /* 
    ** Check if this device supports reads
    */
    if ((unit_p->logstat[type] & STAT_READ) == 0) {
        INFO_STR("Logical device does not support reading");
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Read  logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Copy to user from kernel memory 
    */
    len = f_pos + length > unit_p->lm_size
	   ? unit_p->lm_size - f_pos : length;
    curr_buf_p = data_p;
    do {
        if (copy_to_user(curr_buf_p, unit_p->lm_kaddr + f_pos, len)) {
            if (0 == xferred_bytes) {
                xferred_bytes = -EFAULT;
            }
            break;
        }
        
        /* 
        ** Update counts and such 
        */
        xferred_bytes += len;
        curr_buf_p += len;
        f_pos += len;
        if (f_pos >= unit_p->lm_size) {
            f_pos = 0;
        }
        if (length - xferred_bytes > unit_p->lm_size) {
            len = unit_p->lm_size;
        } else {
            len = length - xferred_bytes;
        }
        *new_fpos_p = f_pos;
    } while ((xferred_bytes > 0) && (xferred_bytes < length));

btp_lm_rd_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_lm_wr()
**
**      Purpose:        Write data to the local memory logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data from
**          length      Number of bytes to transfer
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/

static ssize_t btp_lm_wr(
    struct file *file_p,
    const char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_lm_wr");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    char            *curr_buf_p;
    size_t          len;
    loff_t          f_pos = file_p->f_pos;
    long            xferred_bytes = 0; 
    unsigned long   dest_addr;

    FENTRY;

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /* 
    ** Check if this device supports writes
    */
    if ((unit_p->logstat[type] & STAT_WRITE) == 0) {
        INFO_STR("Logical device does not support writing");
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;

    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Write logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Copy to user from kernel memory 
    */
    len = f_pos + length > unit_p->lm_size
	   ? unit_p->lm_size - f_pos : length;
    curr_buf_p = (char *) data_p;
    do {
        if (copy_from_user(unit_p->lm_kaddr + f_pos, curr_buf_p, len)) {
            if (0 == xferred_bytes) {
                xferred_bytes = -EFAULT;
            }
            break;
        }
        /* Update counts and such */
        xferred_bytes += len;
        curr_buf_p += len;
        f_pos += len;
        if (f_pos >= unit_p->lm_size) {
            f_pos = 0;
        }
        if (length - xferred_bytes > unit_p->lm_size) {
            len = unit_p->lm_size;
        } else {
            len = length - xferred_bytes;
        }
        *new_fpos_p = f_pos;
    } while ((xferred_bytes > 0) && (xferred_bytes < length));


btp_lm_wr_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_xfer()
**
**      Purpose:        Transfers data using either PIO or DMA between user
**                      space and the device.
**
**      Args:
**          unit_p      Device unit pointer.
**          type        Logical device type.
**          dir         Direction: BT_RD or BT_WR.
**          usr_data_p  Address in user space of original data buffer.
**          dest_addr   Address to move data to on the device.
**          length      Number of bytes to tranfer.
**          xferred_bytes_p     Pointer to location holding number of
**                      bytes transferred.
**          
**      Modifies:
**          *xfer_byte_p Number of bytes transferred
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**      Currently uses an intermediate kernel buffer to copy data in/out of.
**
*****************************************************************************/

static int btp_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t type, 
    bt_accessflag_t dir, 
    void * usr_data_p, 
    unsigned long dest_addr, 
    size_t length,
    size_t *xferred_bytes_p
    )
{
    FUNCTION("btp_xfer");
    LOG_UNIT(unit_p);

    int             ret_val = 0;
    void            *data_p;
    int             dma_flag; 
    int             data_width; 
    size_t          length_remaining = length;
    caddr_t         kbuf_p;
    unsigned int    start, need;
    bt_data32_t     mreg_value;
    bt_error_t      retval = BT_SUCCESS;
    unsigned int    inx;
    unsigned long   pci_addr;
    bt_data32_t     ldma_addr;
	int	dummy_var;


    FENTRY;

    /* 
    ** Haven't transferred any data yet 
    */
    *xferred_bytes_p = 0;       

    /*
    **  Adjust for the extended remote ram window.
    */
    if (type == BT_AXSRE) {
        dest_addr |= RE_ADJUST;
    }

    /*
    ** Normally you would have the while loop in the read and write routines.
    **
    ** Since both would require the same loop, I've decided to just put it
    ** in this routine instead.
    */
    while ((length_remaining > 0) && (BT_SUCCESS == retval)) {
        size_t local_length = length_remaining;
        unsigned adjust = ((unsigned long) usr_data_p) % BT_WIDTH_D64;

        /*
        ** Make sure we don't try to transfer more than fits in our
        ** temporary buffer.
        */
        if (local_length > unit_p->dma_buf_size - adjust) {
            local_length = unit_p->dma_buf_size - adjust;
        }
        data_p = unit_p->dma_buf_p + adjust;

        /* 
        ** If writing, move data from user space to kernel 
        */
        if (BT_WR == dir) {
            dummy_var = copy_from_user(data_p, usr_data_p, local_length);
        }

        /*
        ** Calculate the length, data size and whether DMA is possible
        */
        btk_dma_pio(unit_p, type, (bt_data32_t) data_p, dest_addr, &local_length, &dma_flag, &data_width, &start, &need);
#define BTP_FREE_MREG  btk_mutex_enter(unit_p, &unit_p->mreg_mutex); \
                       (void) btk_bit_free(unit_p, unit_p->sdma_aval_p, start, need); \
                       btk_mutex_exit(unit_p, &unit_p->mreg_mutex);


        TRC_MSG(BT_TRC_RD_WR, 
                (LOG_FMT "Transferring %d bytes data to 0x%lx using %s.\n",
                LOG_ARG, local_length, dest_addr, ((dma_flag) ? "DMA" : "PIO")));
            
        if (dma_flag) {

            /*
            **  Setup vme address, address modifier, and function code
            */
	    mreg_value = 0;
            btk_setup_mreg(unit_p, type, &mreg_value, FALSE);

            /*
            ** Load the mapping registers
            */
            kbuf_p = (caddr_t) data_p;
            for (inx = start; inx < (start + need); inx++) {
                pci_addr = bt_kvm2bus(kbuf_p);
                if (0 == pci_addr) {
                    WARN_STR("Could not attach buffer to PCI bus.\n");
                    BTP_FREE_MREG;
                    btk_mutex_exit(unit_p, &unit_p->dma_mutex);
                    retval = BT_EIO;
                    goto btp_xfer_end;
                }
                mreg_value &= ~BT_MREG_ADDR_MASK;
                mreg_value |= (pci_addr & BT_MREG_ADDR_MASK);
                btk_put_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI, mreg_value);
                kbuf_p += BT_PAGE_SIZE;
            }

            /*
            ** Now we need to get the DMA semaphore 
            ** Note this routines does nothing in a single driver situtation
            */
            retval = btk_take_drv_sema(unit_p);
            if (retval != BT_SUCCESS) {
                BTP_FREE_MREG;
                btk_mutex_exit(unit_p, &unit_p->dma_mutex);
                goto btp_xfer_end;
            }

            /*
            ** If old Nanobus card, we must stop PIO from occuring
            */
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                btk_rwlock_wr_enter(unit_p, &unit_p->hw_rwlock);
            }
        
            /*
            ** Do the DMA
            */
            ldma_addr = (bt_data32_t) ((start * BT_PAGE_SIZE) + ((unsigned long) data_p & (BT_PAGE_SIZE - 1)));
#if defined(__linux__)
            retval = btk_dma_xfer(unit_p, type, ldma_addr, (bt_data32_t) dest_addr, &local_length, (dir == BT_RD) ? BT_READ : BT_WRITE, data_width);
#else
            retval = btk_dma_xfer(unit_p, type, ldma_addr, (bt_data32_t) dest_addr, local_length, (dir == BT_RD) ? BT_READ : BT_WRITE, data_width);
#endif
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                btk_rwlock_wr_exit(unit_p, &unit_p->hw_rwlock);
            }
            btk_give_drv_sema(unit_p);
            BTP_FREE_MREG;
            btk_mutex_exit(unit_p, &unit_p->dma_mutex);

        /*
        ** Do a PIO
        */
        } else {

            /* 
            ** Perform the proper direction PIO data transfer 
            */
            retval = btk_pio_xfer(unit_p, type, data_p, (unsigned long) dest_addr,
                                  &local_length, (dir == BT_RD) ? BT_READ : BT_WRITE);
        }
        TRC_MSG(BT_TRC_RD_WR, 
                (LOG_FMT "%s transfer done 0x%x bytes transferred\n",
                LOG_ARG, ((dma_flag) ? "DMA" : "PIO"), local_length));
  
        /*
        ** A DMA or PIO operation has been completed, check for errors
        */
        if (retval != BT_SUCCESS) {
            /* INFO message should be printed by failing routine */

#if defined(__linux__)
            /* update for partial transfer */
            length_remaining -= local_length;
            /* 
            ** If reading, move data from kernel to user space 
            */
            if (BT_RD == dir) {
                dummy_var = copy_to_user(usr_data_p, data_p, local_length);
            }
#endif
            break;

        /*
        ** This part may not be necessary any more ???
        ** This part may not be necessary any more ???
        */
        } else if (btk_get_io(unit_p, BT_LOC_STATUS) & LSR_CERROR_MASK) {
            INFO_STR("Adaptor error during read/write operation");
            retval = BT_EIO;

#if defined(__linux__)
            /* update for partial transfer */
            length_remaining -= local_length;
            /* 
            ** If reading, move data from kernel to user space 
            */
            if (BT_RD == dir) {
                dummy_var = copy_to_user(usr_data_p, data_p, local_length);
            }
#endif
            break;
        }

        /* 
        ** If reading, move data from kernel to user space 
        */
        if (BT_RD == dir) {
            dummy_var = copy_to_user(usr_data_p, data_p, local_length);
        }

        usr_data_p += local_length;
        dest_addr += local_length;
        length_remaining -= local_length;
    }

    /* 
    ** Need to convert from bt_error_t to an error number 
    */
btp_xfer_end:
    *xferred_bytes_p = length - length_remaining;
    switch (retval) {
      case BT_SUCCESS:
      case BT_EIO:
      case BT_ENXIO:
      case BT_ENOMEM:
      case BT_EINVAL:
      case BT_EACCESS:
      case BT_EDESC:
      case BT_ENOSUP:
        /* All of these have a POSIX equivilent number */
        ret_val = -retval;
        break;

      case BT_ENORD:
      case BT_ENOWR:
        ret_val = -EACCES;
        break;

      case BT_EBUSY:
        ret_val = -EAGAIN;
        break;

      case BT_EFAIL:
      case BT_ESTATUS:
      case BT_ENOPWR:
      case BT_EPWRCYC:
      default:
        ret_val = -EIO;
        break;
    }

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Name:           bt_kvm2bus
**
**      Purpose:        Converts from a vmalloc() address to the bus address.
**
**      Args:
**          addr        A vmalloc()ed address in kernel virtual address space.
**
**      Returns:
**          0           Failed to convert
**          Otherwise   PCI bus address of buffer
**
**      Notes:
**      Unfortunately, the virt_to_phys() routine doesn't correctly handle
**      addresses allocated via vmalloc(). This code is based on what I 
**      found in the drivers/char/bttv.c code.
**
*****************************************************************************/

unsigned long bt_kvm2bus(
    void * vm_addr_p
)
{
    FUNCTION("bt_kvm2bus");
    LOG_UNKNOWN_UNIT;

    unsigned long       ret_addr = 0;

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    unsigned long       addr = (unsigned long)vm_addr_p;
#else
    unsigned long       addr = VMALLOC_VMADDR(vm_addr_p);
#endif

    pgd_t       *pgd_p;         /* Page Global Descriptor */
    pmd_t       *pmd_p;         /* Page Middle Descriptor */
    pte_t       *pte_p;         /* Page Table Descriptor */

    FENTRY;

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) \
    ||  LINUX_VERSION_CODE <  KERNEL_VERSION(2,3,0)

    pgd_p = pgd_offset(current->mm, addr);

#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) \
    &&  LINUX_VERSION_CODE >=  KERNEL_VERSION(2,3,0) */

    pgd_p = pgd_offset_k(addr);

#endif  /* LINUX_VERSION_CODE */

    if (!pgd_none(*pgd_p)) {
        pmd_p = pmd_offset(pgd_p, addr);
        if (!pmd_none(*pmd_p)) {
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
            pte_p = pte_offset_kernel(pmd_p, addr);
#else
            pte_p = pte_offset(pmd_p, addr);
#endif

            if (pte_present(*pte_p)) {

#if     LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
                ret_addr = virt_to_bus((void *) pte_page(*pte_p)) +
                    (addr & (PAGE_SIZE-1));

#else   /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0) */
	/* This code must also be used by Linux 2.4.x >= KERNEL_VERSION(2,4,0) */

                /*
                ** Note: pte_page() definition changes based on OS rev.
                ** Ignore it if you get a compiler warning for page_address()
                ** about convertion between integer and pointer.
                */
                ret_addr = virt_to_bus(
                    (void *) page_address(pte_page(*pte_p)) ) +
                    (addr & (PAGE_SIZE-1) );

#endif  /* LINUX_VERSION_CODE */

            }

        }
    }

#if     defined(DEBUG)
    TRC_MSG(BT_TRC_DMA|BT_TRC_DETAIL, (LOG_FMT
        "Kernel VA %p = PCI addr 0x%lx.\n", LOG_ARG,
        vm_addr_p, ret_addr));
#endif  /* defined(DEBUG) */

    FEXIT(ret_addr);
    return ret_addr;
}

