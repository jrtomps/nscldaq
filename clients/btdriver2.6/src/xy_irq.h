/*****************************************************************************
**
**  Filename:   xy_irq.h
**
**  Purpose:    Declare prototypes for the functions in xy_irq.c
**
**    Declares prototype for start_interrupts
**
**
**  Copyright (c) 1996-1997 by Bit 3 Computer Corporation.
**  All rights reserved.
**  License governs use and distribution.
**
**  $Revision$
**
*****************************************************************************/


#ifndef _XY_IRQ_H
#define _XY_IRQ_H


/*****************************************************************************
**
**  Define external functions start_interrupts() & stop_interrupts()
**
*****************************************************************************/

extern bt_error_t start_interrupts(bt_desc_t btd, void * xycom_base_p, 
	unsigned long hertz, bt_data8_t vme_level, bt_data8_t vector);
extern bt_data16_t stop_interrupts(bt_desc_t btd);
  

#endif /* _XY_IRQ_H */

