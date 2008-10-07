/*****************************************************************************
**
**      Filename:   btpapi.h
**
**      Purpose:    NanoBus UNIX Mirror API include file for Linux
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BTPAPI_H)
#define _BTPAPI_H

#if     !defined(_BTAPI_H)
#include        "btapi.h"
#endif  /* !defined(_BTAPI_H) */

#include        "btpdef.h"
#include        "btngpci.h"
#include        <stdio.h>


/******************************************************************************
**
**      Constants and typedef's of general use with NanoBus
**
******************************************************************************/
#define BT_MAX_DEV_NAME FILENAME_MAX

typedef void *bt_binddesc_t;
typedef struct bt_lib_data_s * bt_desc_t;


/*****************************************************************************
**
**      Bit 3 Library Function prototypes specific to NanoBus
**
*****************************************************************************/
extern const char * bt_reg2str(bt_reg_t reg);

extern bt_error_t bt_get_io(bt_desc_t btd, bt_reg_t reg, bt_data32_t *value_p);

extern bt_error_t bt_put_io(bt_desc_t btd, bt_reg_t reg, bt_data32_t value);

extern  bt_error_t bt_status(bt_desc_t btd, bt_data32_t *status_p);

extern  bt_error_t bt_reset(bt_desc_t btd);

extern  bt_error_t bt_cas(bt_desc_t btd, bt_devaddr_t rem_off, 
                          bt_data32_t cmpval, bt_data32_t swapval, 
                          bt_data32_t * prevval_p, size_t data_size);

extern  bt_error_t bt_tas(bt_desc_t btd, bt_devaddr_t rem_off, 
                          bt_data8_t * prevval_p);

extern  bt_error_t bt_send_irq(bt_desc_t btd);

extern  bt_error_t bt_hw_bind(bt_desc_t btd, bt_binddesc_t *bind_p, bt_devaddr_t *offset_p,
                              bt_devaddr_t loc_addr, size_t buf_len, bt_accessflag_t flags,
                              bt_swap_t swapping);
                              
extern  bt_error_t bt_hw_unbind(bt_desc_t btd, bt_binddesc_t desc);

extern  bt_error_t bt_hw_write(bt_desc_t btd, bt_devaddr_t bus_addr, bt_devaddr_t xfer_addr, size_t req_len, size_t *xfer_len_p);

extern  bt_error_t bt_hw_read(bt_desc_t btd, bt_devaddr_t bus_addr, bt_devaddr_t xfer_addr, size_t req_len, size_t *xfer_len_p);

extern  bt_error_t bt_take_sema(bt_desc_t btd, bt_reg_t sema, bt_msec_t timeout);

extern  bt_error_t bt_give_sema(bt_desc_t btd, bt_reg_t sema);

extern bt_error_t bt_lcard_diag(bt_desc_t btd, char *rev_info_p, int *line_p);

extern bt_error_t bt_cable_diag(bt_desc_t btd, char *rev_info_p, int *line_p);

extern bt_error_t bt_rcard_diag(bt_desc_t btd, bt_dev_t rldev, bt_devaddr_t raddr, bt_data32_t rlen, char *rev_info_p, int *line_p);

extern bt_error_t bt_pair_diag(bt_desc_t btd, bt_dev_t rldev, bt_devaddr_t raddr, bt_data32_t rlen, char *rev_info_p, int *line_p);

extern bt_error_t bt_driver_version(bt_desc_t btd, char *rev_info_p, int *line_p);

/******************************************************************************
**
** Macros to insert and extract fields
**
** All of the fields are defined with two macros,
**      fieldname_SHFT is the amount to shift the header right by
**      fieldname_MASK is the mask for the size of the field 
**
** The macros EXTRACT and INSERT use a shorthand notation, where only
** the field name is specified.  The macro concatenates the _SHFT and _MASk
** portion of the names and then calls the MASK_INSERT or MASK_EXTRACT
** macros to do the actual work.
**
** As long as ALL fields have both the _SHFT and _MASK values defined,
** this should work well.
******************************************************************************/

#define BT_CONCAT(a,b)  a ## b

#define BT_EXTRACT(data, field) \
        BT_MASK_EXTRACT(data, BT_CONCAT(field, _SHFT), BT_CONCAT(field, _MASK))

#define BT_INSERT(data, field, value) \
  BT_MASK_INSERT(data, BT_CONCAT(field, _SHFT), BT_CONCAT(field, _MASK), value)

#define BT_MASK_EXTRACT(data, shift, mask) \
        (((data) >> (shift)) & (mask))
#define BT_MASK_INSERT(data, shift, mask, value) \
        (((data) & ~((mask)<<(shift))) | ((value) << (shift)))

#endif /* !defined(_BTPAPI_H) */

