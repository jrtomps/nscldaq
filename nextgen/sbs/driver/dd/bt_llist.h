/*****************************************************************************
**      Filename:   bt_llist.h
**
**      Purpose:    Generic header for bt_llist.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_llist.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.3 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _BT_LLIST_H
#define _BT_LLIST_H

/*****************************************************************************
**
**  Include Files 
**
*****************************************************************************/

#if defined (__vxworks)
#include <stdio.h>
#include <vxworks.h>
#include <string.h>  /* for bzero() call */
#endif /* defined (__vxworks) */

/* This is a convience. */
#ifndef RETURN
#define RETURN(x) FEXIT(x); return (x)
#endif /* RETURN */

#if defined(BT13908)
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_llist_init)
#pragma alloc_text(PAGE, btk_llist_next)
#pragma alloc_text(PAGE, btk_llist_prev)
#pragma alloc_text(PAGE, btk_llist_first)
#pragma alloc_text(PAGE, btk_llist_last)
#pragma alloc_text(PAGE, btk_llist_elem_init)
#pragma alloc_text(PAGE, btk_llist_insert_first)
#pragma alloc_text(PAGE, btk_llist_insert_last)
#pragma alloc_text(PAGE, btk_llist_insert_after)
#pragma alloc_text(PAGE, btk_llist_insert_before)
#pragma alloc_text(PAGE, btk_llist_remove)
#pragma alloc_text(PAGE, btk_llist_find_first)
#pragma alloc_text(PAGE, btk_llist_find_last)
#pragma alloc_text(PAGE, btk_llist_find_next)
#pragma alloc_text(PAGE, btk_llist_find_prev)
#pragma alloc_text(PAGE, btk_llist_onall)
#pragma alloc_text(PAGE, btk_llist_count)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

/*****************************************************************************
**
**  Data Structures 
**
*****************************************************************************/

typedef struct btk_llist_elem_s 
{
    struct btk_llist_elem_s *next_p;
    struct btk_llist_elem_s *prev_p;
    struct btk_llist_s *parent_p;
    void *data_p;
} btk_llist_elem_t;

typedef struct btk_llist_s 
{
    btk_llist_elem_t *first_p;
    btk_llist_elem_t *last_p;
} btk_llist_t;


/*****************************************************************************
**
**  Protoypes
**
*******************************************************************/

extern void btk_llist_init(btk_llist_t * list_p);

extern void btk_llist_elem_init(
        btk_llist_elem_t *new_p, 
        void * data_p);

extern void * btk_llist_elem_data(btk_llist_elem_t *elem_p);

extern btk_llist_elem_t *btk_llist_elem_create(
        size_t data_size, 
        bt_data32_t alloc_flags);

extern void btk_llist_elem_destroy(
        btk_llist_elem_t *elem_p, 
        size_t data_size);

extern void btk_llist_remove(btk_llist_elem_t *elem_p);

extern btk_llist_elem_t *btk_llist_next(btk_llist_elem_t *curr_p);

extern btk_llist_elem_t *btk_llist_prev(btk_llist_elem_t *curr_p);

extern btk_llist_elem_t *btk_llist_first(btk_llist_t *list_p);

extern btk_llist_elem_t *btk_llist_last(btk_llist_t *list_p);

extern btk_llist_elem_t *btk_llist_find_first(
        btk_llist_t *list_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern btk_llist_elem_t *btk_llist_find_last(
        btk_llist_t * list_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern btk_llist_elem_t *btk_llist_find_next(
        btk_llist_elem_t *curr_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern btk_llist_elem_t *btk_llist_find_prev(
        btk_llist_elem_t *curr_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern void btk_llist_insert_first(
        btk_llist_t *list_p, 
        btk_llist_elem_t *new_p);

extern void btk_llist_insert_last(
        btk_llist_t *list_p, 
        btk_llist_elem_t *new_p);

extern void btk_llist_insert_after(
        btk_llist_elem_t *old_p, 
        btk_llist_elem_t *new_p);

extern void btk_llist_insert_before(
        btk_llist_elem_t *old_p, 
        btk_llist_elem_t *new_p);

extern void *btk_llist_first_data(btk_llist_t *list_p);

extern void *btk_llist_prev_data(btk_llist_elem_t *curr_p);

extern void *btk_llist_next_data(btk_llist_elem_t *curr_p);

extern void *btk_llist_last_data(btk_llist_t *list_p);

extern void * btk_llist_find_first_data(
        btk_llist_t *list_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern void *btk_llist_find_last_data(
        btk_llist_t * list_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern void *btk_llist_find_next_data(
        btk_llist_elem_t *curr_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern void *btk_llist_find_prev_data(
        btk_llist_elem_t *curr_p, 
        int(*func_p)(void *, void *), 
        void *second_arg);

extern void btk_llist_onall(
        btk_llist_t *list_p, 
        void(*func_p)(void *, void *), 
        void *second_arg);

extern int btk_llist_count(btk_llist_t *list_p);


#endif /* !defined(_BT_LLIST_H) */
