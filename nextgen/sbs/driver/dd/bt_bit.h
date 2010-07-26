/*****************************************************************************
**      Filename:   bt_bit.h
**
**      Purpose:    Bit 3 Generic header for bt_bit.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_bit.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.2 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2004 by Bit 3 Computer Corporation.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_BIT_H)
#define _BT_BIT_H

/*
**  Include files
*/

#if     defined(FCTACH)
#if     defined(__vxworks)
#include    <string.h>      /* remains in header to resolve "sizeof" */
#endif  /* __vxworks */
#endif  /* defined(FCTACH) */



/*****************************************************************************
**
**      Local macros and structures
**
*****************************************************************************/

#define BPW     (32u)           /* Bits per word */
#define WORD_T  bt_data32_t     /* Type of word, must be unsigned */
#define WBIT_MASK 0xffffffff    /* Word with all bits set */


#define BIT_ZERO        (1u<<(BPW-1))   /* Word with only leftmost bit set */

/* Safe to rotate any amount */
#define RIGHT_ROTATE(value, shift)      \
        ( ( (value) >> ((shift)%BPW) ) | ( (value) << ((BPW - (shift))%BPW) ) )


#ifdef BT13908
static unsigned int find_first_one(WORD_T word);
static unsigned int find_first_zero(WORD_T word);
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, find_first_one)
#pragma alloc_text(PAGE, find_first_zero)
#pragma alloc_text(PAGE, btk_bit_init)
#pragma alloc_text(PAGE, btk_bit_fini)
#pragma alloc_text(PAGE, btk_bit_reset)
#pragma alloc_text(PAGE, btk_bit_free)
#pragma alloc_text(PAGE, btk_bit_alloc)
#pragma alloc_text(PAGE, btk_bit_set)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */



/*****************************************************************************
**
**  Protoypes and defineitions for bt_bit.c
**
*****************************************************************************/

bt_error_t btk_bit_init(
        unsigned size, 
        void **bitmap_p);

bt_error_t btk_bit_reset(
        void *bitmap_p);

bt_error_t btk_bit_fini(
        void *bitmap_p);

bt_error_t btk_map_half(
        void *bitmap_p);

bt_error_t btk_map_restore(
        void *bitmap_p);

bt_error_t btk_bit_free(
        void *bitmap_p, 
        unsigned start, 
        unsigned num_entries);

bt_error_t btk_bit_alloc(
        void *bitmap_p, 
        unsigned needed,
        unsigned align, 
        unsigned *first_p);

bt_error_t btk_bit_specify(
        void *bitmap_p, 
        unsigned start, 
        unsigned needed);

int btk_bit_set(
        void * bitmap_p, 
        unsigned bitnum);

int btk_bit_clr(
        void * bitmap_p, 
        unsigned bitnum);

int btk_bit_chk(
        void * bitmap_p, 
        unsigned bitnum);

bt_error_t btk_bit_max(
        void * bit_map_p, 
        unsigned align, 
        unsigned *size);

#define  SET_BIT(base, bits)  ((base) |= (bits))
#define  CLR_BIT(base, bits)  ((base) &= ~(bits))
#define  TST_BIT(base, bits)  (((base) & (bits)) != 0)
#define  IS_CLR(base, bits)   (((base) & (bits)) == 0)
#define  IS_SET(base, bits)   TST_BIT(base, bits)


#endif /* !defined(_BT_BIT_H) */
