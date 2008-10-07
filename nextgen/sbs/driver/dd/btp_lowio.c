/*****************************************************************************
**
**      Filename:   btp_lowio.c
**
**      Purpose:    Bit 3 PCI adaptor device driver module.
**                  Contains low level adaptor register access routines.
**
**      Functions:  bt_restore_state(), bt_save_state(), bt_gtioreg(),
**                  bt_stioreg(), bt_orioreg()
**
**      $Revision: 742 $
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/
#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $ "__DATE__;
#endif  /* !LINT */

/* 
** Include files 
*/
#include <asm/io.h>
#include "btdd.h"

/*
**  Static variables
*/
#if defined(DEBUG)
BT_FILE_NUMBER(TRACE_BTP_LOWIO_C);
#endif

/*****************************************************************************
**
**      Name:           btp_get_d8
**
**      Purpose:        Read a 8-bit data location from the local bus.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**
**      Modifies:
**          None
**          
**      Returns:
**          Data read.
**
**      Notes:
**
*****************************************************************************/

bt_data8_t btp_get_d8(
    bt_unit_t   *unit_p, 
    void        *addr_p)
{
    bt_data8_t  value;

#if defined(DEBUG)
    FUNCTION("btp_get_d8");
    LOG_UNIT(unit_p);
#endif /* defined(DEBUG) */

    /*
    ** Do the access
    */
    value = PIO_READ_8(addr_p);

#if defined(DEBUG)
    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    return value;
}

/*****************************************************************************
**
**      Name:           btp_put_d8
**
**      Purpose:        Write a 8-bit data location.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**          value       Value to write.
**
**      Modifies:
**          None
**          
**      Returns:
**          void
**
**      Notes:
**
*****************************************************************************/

void btp_put_d8(
    bt_unit_t   *unit_p, 
    void        *addr_p,
    bt_data8_t  value)
{
    
#if defined(DEBUG)
    FUNCTION("btp_put_d8");
    LOG_UNIT(unit_p);

    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    /*
    ** Do the access
    */
    PIO_WRITE_8(value, addr_p);
    return;
}

/*****************************************************************************
**
**      Name:           btp_get_d16
**
**      Purpose:        Read a 16-bit data location.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**
**      Modifies:
**          None
**          
**          
**      Returns:
**          Data read.
**
**      Notes:
**
*****************************************************************************/

bt_data16_t btp_get_d16(
    bt_unit_t   *unit_p, 
    void        *addr_p)
{
    bt_data16_t value;

#if defined(DEBUG)
    FUNCTION("btp_get_d16");
    LOG_UNIT(unit_p);
#endif /* defined(DEBUG) */
    
    /*
    ** Make the access
    */
    value = PIO_READ_16(addr_p);
#if defined(DEBUG)
    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    return value;
}

/*****************************************************************************
**
**      Name:           btp_put_d16
**
**      Purpose:        Write a 16-bit data location.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**          value       Value to write.
**
**      Modifies:
**          None
**          
**      Returns:
**          void
**
**      Notes:
**
*****************************************************************************/

void btp_put_d16(
    bt_unit_t   *unit_p, 
    void        *addr_p,
    bt_data16_t value)
{

#if defined(DEBUG)
    FUNCTION("btp_put_d16");
    LOG_UNIT(unit_p);

    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    /*
    ** Make the access
    */
    PIO_WRITE_16(value, addr_p);
    return;
}

/*****************************************************************************
**
**      Name:           btp_get_d32
**
**      Purpose:        Read a 32-bit data location.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**
**      Modifies:
**          None
**          
**          
**      Returns:
**          Data read.
**
**      Notes:
**          Put any required swapping in this routine.
**
*****************************************************************************/

bt_data32_t btp_get_d32(
    bt_unit_t   *unit_p, 
    void        *addr_p)
{

    bt_data32_t value;

#if defined(DEBUG)
    FUNCTION("btp_get_d32");
    LOG_UNIT(unit_p);
#endif /* defined(DEBUG) */

    value = PIO_READ_32(addr_p);
#if defined(DEBUG)
    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    return value;
}

/*****************************************************************************
**
**      Name:           btp_put_d32
**
**      Purpose:        Write a 32-bit data location.
**
**      Args:
**          unit_p      Unit pointer.
**          addr_p      Pointer to actual bus address that should be read.
**          value       Value to write.
**
**      Modifies:
**          None
**          
**      Returns:
**          void
**
**      Notes:
**          Put any required swapping in this routine.
**
*****************************************************************************/

void btp_put_d32(
    bt_unit_t   *unit_p, 
    void        *addr_p,
    bt_data32_t value)
{
#if defined(DEBUG)
    FUNCTION("btp_put_d32");
    LOG_UNIT(unit_p);

    TRC_MSG(BT_TRC_RAWIO, (LOG_FMT "Bus Address 0x%lx, value 0x%x.\n",
            LOG_ARG, (unsigned long) addr_p, value));
#endif /* defined(DEBUG) */

    PIO_WRITE_32(value, addr_p);
    return;
}

