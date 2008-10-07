/****************************************************************************
 **
 **    Filename:        btk_llist.c
 **
 **    Purpose:         Linked list routines- unlike slist, these routines
 **                     do not impose an order on the list.
 **
 **    Functions:       None
 **
 **    $Revision: 742 $
 **
 ***************************************************************************/
/****************************************************************************
 **
 **               Copyright (c) 1999 by SBS Technologies, Inc.
 **                          All Rights Reserved.
 **                 License governs use and distribution.
 **
 ***************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__" $Revision: 742 $"__DATE__;
#endif /* LINT */

/*
**                         A Statement of Philosophy
**
**     Why were these routines written when the slist routines.  In 
** attempting to use them, I have found numerous situations where the slist
** routines did not apply- generally because the slist routines impose an
** order on the list.  In situations like LIFOs, FIFOs, or lists in which
** there is no obvious order, the slist routines are at best clumsy to use
** and at worst impossible to use.  Also, the slist routines do not handle
** lists which must be searched in multiple ways, or data structures which
** can be in multiple lists at the same time, or which can move from one
** list to a different list (say, an allocated list and a free list).
**
**     Thus the impeteous for these routines (plus the excuse that "they
** were easy"- total initial coding time was less than 30 minutes not
** counting comments).  The philosophy in writting them was:
**     1) Impose no order on the list.
**     2) Do not take over allocation/deallocation for the programmer-
**        do not assume any memory relationship between the linked list
**        element structure, the data structure being managed, and the
**        linked list structure.
**     3) Do things in the simple, obvious way.  Don't get clever.
**     4) Provide a minimum set so that any needed functionality could be
**        built ontop of these routines.
**     5) Don't do locking.  Any locking required should be done by the
**        programmer before calling these functions.
**
**     An example of how to use this code is at the end of this file.
**
** There are two sets of routines for most of the getting/finding.
** The first is btk_llist_??? which returns the linked list element.
** The second is btk_llist_???_data which returns the void *data 
** structure associated with the link list element.
**
*/

#include "btdd.h"

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
/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_LLIST_C);

/****************************************************************************
 **
 **    Function:        btk_llist_init()
 **
 **    Purpose:         Initial a linked list structure.
 **
 **    Args:
 **        list_p
 **                     Pointer to the linked list structure to initialize.
 **
 **    Returns:         void
 **
 **    Notes:
 **
 **        The programmer is responsible for actually allocating the 
 **        structure, before calling this function.
 **
 ***************************************************************************/
void btk_llist_init(
    btk_llist_t * list_p) 
{
    FUNCTION("btk_llist_init");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    list_p->first_p = NULL;
    list_p->last_p = NULL;

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_elem_init()
 **
 **    Purpose:         Associate a data pointer with a linked list element.
 **
 **    Args:
 **        new_p
 **                     linked list element to associate data with.
 **        data_p
 **                     data pointer to associate with linked list element.
 **
 **    Returns:         void
 **
 ***************************************************************************/
void btk_llist_elem_init(
    btk_llist_elem_t * new_p, 
    void * data_p) 
{
    FUNCTION("btk_llist_elem_init");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    new_p->data_p = data_p;

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_elem_data()
 **
 **    Purpose:         Get a data pointer for given linked list element.
 **
 **    Args:
 **        elem_p
 **                     linked list element to get data from.
 **
 **    Returns:         void *
 **                         Pointer to the associated data on success
 **                         NULL on empty list.
 **
 ***************************************************************************/
void * btk_llist_elem_data(
    btk_llist_elem_t * elem_p) 
{
    FUNCTION("btk_llist_elem_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if (elem_p != NULL) {
        FEXIT(elem_p->data_p);
        return(elem_p->data_p);
    } else {
        FEXIT(NULL);
        return(NULL);
    }


}

/****************************************************************************
 **
 **    Function:        btk_llist_elem_create()
 **
 **    Purpose:         Create a linked list element along with the
 **                     associated data structure.
 **
 **    Args:
 **        new_p
 **                     linked list element to associate data with.
 **        data_p
 **                     data pointer to associate with linked list element.
 **
 **    Returns:         void
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_elem_create(
    size_t data_size,
    bt_data32_t alloc_flags) 
{
    FUNCTION("btk_llist_elem_create");
    LOG_UNKNOWN_UNIT;
    btk_llist_elem_t    *llist_element_p;

    FENTRY;

    /*
    ** Allocate a new list item
    */
    if ((llist_element_p = btk_mem_alloc(sizeof(btk_llist_elem_t), alloc_flags)) == NULL) {
        TRC_STR(BT_TRC_INFO|BT_TRC_DETAIL, "Could not allocate llist element.");
        FEXIT(NULL);
        return(NULL);
    }

    /*
    ** Allocate a new data item
    */
    if ((llist_element_p->data_p = btk_mem_alloc(data_size, alloc_flags)) == NULL) {
        btk_mem_free(llist_element_p, sizeof(btk_llist_elem_t));
        TRC_STR(BT_TRC_INFO|BT_TRC_DETAIL, "Could not allocate llist data element.");
        FEXIT(NULL);
        return(NULL);
    }
    BTK_BZERO(llist_element_p->data_p, sizeof(data_size));

    FEXIT(llist_element_p);
    return(llist_element_p);

}

/****************************************************************************
 **
 **    Function:        btk_llist_elem_destroy()
 **
 **    Purpose:         Destroy a linked list element along with the
 **                     associated data structure.
 **
 **    Args:
 **        new_p
 **                     linked list element to associate data with.
 **        data_p
 **                     data pointer to associate with linked list element.
 **
 **    Returns:         void
 **
 **    Notes:           Linked list element should be removed from the
 **                     list first with btk_llink_remove() before
 **                     calling this routine.
 **
 ***************************************************************************/
void btk_llist_elem_destroy(
    btk_llist_elem_t * elem_p, 
    size_t data_size)
{
    FUNCTION("btk_llist_elem_destroy");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    btk_mem_free(elem_p->data_p, data_size);
    btk_mem_free(elem_p, sizeof(btk_llist_elem_t));

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_remove()
 **
 **    Purpose:         Remove a linked list element from a linked list.
 **
 **    Args:
 **        elem_p
 **                     Linked list element to remove.
 **
 **    Returns:         void
 **
 ***************************************************************************/
void btk_llist_remove(
    btk_llist_elem_t * elem_p) 
{
    FUNCTION("btk_llist_remove");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if (elem_p->prev_p == NULL) 
    {
        BTK_ASSERT(elem_p->parent_p->first_p == elem_p);
        elem_p->parent_p->first_p = elem_p->next_p;
    } 
    else 
    {
        elem_p->prev_p->next_p = elem_p->next_p;
    }
    if (elem_p->next_p == NULL) 
    {
        BTK_ASSERT(elem_p->parent_p->last_p == elem_p);
        elem_p->parent_p->last_p = elem_p->prev_p;
    } 
    else 
    {
        elem_p->next_p->prev_p = elem_p->prev_p;
    }
    elem_p->parent_p = NULL;
    elem_p->next_p = NULL;
    elem_p->prev_p = NULL;

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_insert_first()
 **
 **    Purpose:         Insert a linked list element into a linked list as
 **                     the first element.
 **
 **    Args:
 **        list_p
 **                     List to insert the element into
 **        new_p
 **                     Element to insert
 **
 **    Returns:         void
 **
 ***************************************************************************/
void btk_llist_insert_first (
    btk_llist_t * list_p, 
    btk_llist_elem_t * new_p) 
{
    FUNCTION("btk_llist_insert_first");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    new_p->prev_p = NULL;
    new_p->next_p = list_p->first_p;
    new_p->parent_p = list_p;
    list_p->first_p = new_p;
    if (new_p->next_p == NULL) 
    {
        BTK_ASSERT(list_p->last_p == NULL);
        list_p->last_p = new_p;
    } 
    else 
    {
        new_p->next_p->prev_p = new_p;
    }

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_insert_last()
 **
 **    Purpose:         Insert a linked list element into a linked list as
 **                     the last element in the list.
 **
 **    Args:
 **        list_p
 **                     List to insert the element into
 **        new_p
 **                     Element to insert
 **
 **    Returns:         void
 **
 ***************************************************************************/
void btk_llist_insert_last (
    btk_llist_t * list_p, 
    btk_llist_elem_t * new_p)
{
    FUNCTION("btk_llist_insert_last");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    new_p->next_p = NULL;
    new_p->prev_p = list_p->last_p;
    new_p->parent_p = list_p;
    list_p->last_p = new_p;
    if (new_p->prev_p == NULL) 
    {
        BTK_ASSERT(list_p->first_p == NULL);
        list_p->first_p = new_p;
    } 
    else 
    {
        new_p->prev_p->next_p = new_p;
    }

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_insert_after()
 **
 **    Purpose:         Insert a linked list element (new_p) into a linked
 **                     list as the item immediately after a given item
 **                     (old_p).
 **
 **    Args:
 **        old_p
 **                     Element already in the linked list to insert new_p 
 **                     after
 **        new_p
 **                     Element to insert right after old_p.
 **
 **    Returns:         void
 **
 **    Notes:
 **
 **        new_p is not removed from it's list before being inserted.
 **        old_p is not checked to make sure it's in a list.
 **
 ***************************************************************************/
void btk_llist_insert_after (
    btk_llist_elem_t * old_p, 
    btk_llist_elem_t * new_p)
{
    FUNCTION("btk_llist_insert_after");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    new_p->prev_p = old_p;
    new_p->next_p = old_p->next_p;
    new_p->parent_p = old_p->parent_p;
    if (old_p->next_p == NULL) 
    {
        BTK_ASSERT(old_p->parent_p->last_p == old_p);
        old_p->parent_p->last_p = new_p;
    } 
    else 
    {
        old_p->next_p->prev_p = new_p;
    }
    old_p->next_p = new_p;

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_insert_before()
 **
 **    Purpose:         Insert a linked list element (new_p) into a linked
 **                     list as the element immediately preceding a given
 **                     linked list element already in the list (old_p).
 **
 **    Args:
 **        old_p
 **                     Element already in the linked list to insert new_p 
 **                     before
 **        new_p
 **                     Element to insert right before old_p.
 **
 **    Returns:         void
 **
 **    Notes:
 **
 **        new_p is not removed from it's list before being inserted.
 **        old_p is not checked to make sure it's in a list.
 **
 ***************************************************************************/
void btk_llist_insert_before (
    btk_llist_elem_t * old_p,
    btk_llist_elem_t * new_p)
{
    FUNCTION("btk_llist_insert_before");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    new_p->prev_p = old_p->prev_p;
    new_p->next_p = old_p;
    new_p->parent_p = old_p->parent_p;
    if (old_p->prev_p == NULL) 
    {
        BTK_ASSERT(old_p->parent_p->first_p == old_p);
        old_p->parent_p->first_p = new_p;
    } 
    else 
    {
        old_p->prev_p->next_p = new_p;
    }
    old_p->prev_p = new_p;

    FEXIT(0);

}

/****************************************************************************
 **
 **    Function:        btk_llist_next()
 **
 **    Purpose:         Return the next linked list element.
 **
 **    Args:
 **        curr_p
 **                     Pointer to the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         NULL on end of list.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_next(
    btk_llist_elem_t * curr_p) 
{
    FUNCTION("btk_llist_next");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((curr_p != NULL) && (curr_p->next_p != NULL)) 
    {
        RETURN(curr_p->next_p);
    } 
    else 
    {
        BTK_ASSERT((curr_p == NULL) || (curr_p->parent_p->last_p == curr_p));
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_prev()
 **
 **    Purpose:         Returns the previous linked linked list element.
 **
 **    Args:
 **        curr_p
 **                     Pointer to the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         NULL on begining of list.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_prev(
    btk_llist_elem_t * curr_p) 
{
    FUNCTION("btk_llist_prev");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((curr_p != NULL) && (curr_p->prev_p != NULL)) 
    {
        RETURN(curr_p->prev_p);
    } 
    else 
    {
        BTK_ASSERT((curr_p == NULL) || (curr_p->parent_p->first_p == curr_p));
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_first()
 **
 **    Purpose:         Returns the first element of the linked list.
 **
 **    Args:
 **        list_p
 **                     Pointer to the linked list structure.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         NULL on empty list.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_first(
    btk_llist_t * list_p) 
{
    FUNCTION("btk_llist_first");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((list_p != NULL) && (list_p->first_p != NULL)) 
    {
        RETURN(list_p->first_p);
    } 
    else 
    {
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_last()
 **
 **    Purpose:         Returns the last element of the linked list.
 **
 **    Args:
 **        list_p
 **                     Pointer to the linked list structure.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         NULL on empty list.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_last(
    btk_llist_t * list_p) 
{
    FUNCTION("btk_llist_last");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((list_p != NULL) && (list_p->last_p != NULL)) 
    {
        RETURN(list_p->last_p);
    } 
    else 
    {
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_first()
 **
 **    Purpose:         Find the first element in a list which satisfies
 **                     a given function.
 **
 **    Args:
 **        list_p
 **                     Linked list to search
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         which satisfies the function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_find_first(
    btk_llist_t * list_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    btk_llist_elem_t * curr_p;
    FUNCTION("btk_llist_find_first");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->first_p; curr_p != NULL; curr_p = curr_p->next_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_last()
 **
 **    Purpose:         Just like btk_llist_find_first(), except is searches
 **                     backwards through the list, starting at the end.
 **
 **    Args:
 **        list_p
 **                     Linked list to search
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         which satisfies the function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_find_last(
    btk_llist_t * list_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    btk_llist_elem_t * curr_p;
    FUNCTION("btk_llist_find_last");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->last_p; curr_p != NULL; curr_p = curr_p->prev_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_next()
 **
 **    Purpose:         Just like btk_llist_find_first, but instead of
 **                     starting at the begining of the list, it starts
 **                     it starts at the next element after a given
 **                     element.
 **
 **    Args:
 **        curr_p
 **                     Element to start just after.
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         which satisfies the function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_find_next(
    btk_llist_elem_t * curr_p, 
    int (*func_p)(void *, void *), 
    void * second_arg) 
{
    FUNCTION("btk_llist_find_next");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = curr_p->next_p; curr_p != NULL; curr_p = curr_p->next_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_prev()
 **
 **    Purpose:         Just like btk_llist_find_last, but instead of
 **                     starting at the end of the list, it starts
 **                     it starts at the previous element before a given
 **                     element.
 **
 **    Args:
 **        curr_p
 **                     Element to start just before.
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         btk_llist_elem_t *
 **                         Pointer to the list element data on success
 **                         which satisfies the function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
btk_llist_elem_t * btk_llist_find_prev(
    btk_llist_elem_t * curr_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    FUNCTION("btk_llist_find_prev");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = curr_p->prev_p; curr_p != NULL; curr_p = curr_p->prev_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_next_data()
 **
 **    Purpose:         Return the data associated with the next linked
 **                     list element.
 **
 **    Args:
 **        curr_p
 **                     Pointer to the current linked list element.
 **
 **    Returns:         void *
 **                         Pointer to the associated data on success
 **                         NULL on end of list.
 **
 ***************************************************************************/
void * btk_llist_next_data(
    btk_llist_elem_t * curr_p) 
{
    FUNCTION("btk_llist_next_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((curr_p != NULL) && (curr_p->next_p != NULL)) 
    {
        RETURN(curr_p->next_p->data_p);
    } 
    else 
    {
        BTK_ASSERT((curr_p == NULL) || (curr_p->parent_p->last_p == curr_p));
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_prev_data()
 **
 **    Purpose:         Returns the data associated with the previous linked
 **                     linked list element.
 **
 **    Args:
 **        curr_p
 **                     Pointer to the current linked list element.
 **
 **    Returns:         void *
 **                         Pointer to the associated data on success
 **                         NULL on begining of list.
 **
 ***************************************************************************/
void * btk_llist_prev_data(
    btk_llist_elem_t * curr_p) 
{
    FUNCTION("btk_llist_prev_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((curr_p != NULL) && (curr_p->prev_p != NULL)) 
    {
        RETURN(curr_p->prev_p->data_p);
    } 
    else 
    {
        BTK_ASSERT((curr_p == NULL) || (curr_p->parent_p->first_p == curr_p));
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_first_data()
 **
 **    Purpose:         Returns the data associated with the first element
 **                     of the linked list.
 **
 **    Args:
 **        list_p
 **                     Pointer to the linked list structure.
 **
 **    Returns:         void *
 **                         Pointer to the associated data on success
 **                         NULL on empty list.
 **
 ***************************************************************************/
void * btk_llist_first_data(
    btk_llist_t * list_p) 
{
    FUNCTION("btk_llist_first_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((list_p != NULL) && (list_p->first_p != NULL)) 
    {
        RETURN(list_p->first_p->data_p);
    } 
    else 
    {
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_last_data()
 **
 **    Purpose:         Returns the data associated with the last element
 **                     of the linked list.
 **
 **    Args:
 **        list_p
 **                     Pointer to the linked list structure.
 **
 **    Returns:         void *
 **                         Pointer to the associated data on success
 **                         NULL on empty list.
 **
 ***************************************************************************/
void * btk_llist_last_data(
    btk_llist_t * list_p) 
{
    FUNCTION("btk_llist_last_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    if ((list_p != NULL) && (list_p->last_p != NULL)) 
    {
        RETURN(list_p->last_p->data_p);
    } 
    else 
    {
        RETURN(NULL);
    }
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_first_data()
 **
 **    Purpose:         Find the first element in a list which satisfies
 **                     a given function.
 **
 **    Args:
 **        list_p
 **                     Linked list to search
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         void *
 **                         Associated data pointer which satisfies the
 **                         function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
void * btk_llist_find_first_data(
    btk_llist_t * list_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    btk_llist_elem_t * curr_p;
    FUNCTION("btk_llist_find_first_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->first_p; curr_p != NULL; curr_p = curr_p->next_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p->data_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_last_data()
 **
 **    Purpose:         Just like btk_llist_find_first(), except is searches
 **                     backwards through the list, starting at the end.
 **
 **    Args:
 **        list_p
 **                     Linked list to search
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         void *
 **                         Associated data pointer which satisfies the
 **                         function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
void * btk_llist_find_last_data(
    btk_llist_t * list_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    btk_llist_elem_t * curr_p;
    FUNCTION("btk_llist_find_last_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->last_p; curr_p != NULL; curr_p = curr_p->prev_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p->data_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_next_data()
 **
 **    Purpose:         Just like btk_llist_find_first, but instead of
 **                     starting at the begining of the list, it starts
 **                     it starts at the next element after a given
 **                     element.
 **
 **    Args:
 **        curr_p
 **                     Element to start just after.
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         void *
 **                         Associated data pointer which satisfies the
 **                         function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
void * btk_llist_find_next_data(
    btk_llist_elem_t * curr_p, 
    int (*func_p)(void *, void *), 
    void * second_arg) 
{
    FUNCTION("btk_llist_find_next_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = curr_p->next_p; curr_p != NULL; curr_p = curr_p->next_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p->data_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_find_prev_data()
 **
 **    Purpose:         Just like btk_llist_find_last, but instead of
 **                     starting at the end of the list, it starts
 **                     it starts at the previous element before a given
 **                     element.
 **
 **    Args:
 **        curr_p
 **                     Element to start just before.
 **        func_p
 **                     Function to satisify- function returns 0 if
 **                     satisfied, !0 otherwise.
 **        second_arg
 **                     Uninspected pointer passed to *func_p as it's
 **                     second argument (thus the name).  The first
 **                     argument is always the associated data pointer
 **                     of the current linked list element.
 **
 **    Returns:         void *
 **                         Associated data pointer which satisfies the
 **                         function on success
 **                         NULL if the function is never satisfied.
 **
 ***************************************************************************/
void * btk_llist_find_prev_data(
    btk_llist_elem_t * curr_p, 
    int (*func_p)(void *, void *),
    void * second_arg) 
{
    FUNCTION("btk_llist_find_prev_data");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = curr_p->prev_p; curr_p != NULL; curr_p = curr_p->prev_p) 
    {
        if ((*func_p)(curr_p->data_p, second_arg) == 0) 
        {
            RETURN(curr_p->data_p);
        }
    }
    RETURN(NULL);
}

/****************************************************************************
 **
 **    Function:        btk_llist_onall()
 **
 **    Purpose:         Call a given function on all elements of a given
 **                     list.
 **
 **    Args:
 **        list_p
 **                     Linked list to traverse.
 **        func_p
 **                     Pointer to the function to call.
 **        second_arg
 **                     Uninspected pointer to pass to *func_p as it's
 **                     second argument.  The first argument is always the
 **                     data pointer associated with the current linked list
 **                     element.
 **
 **    Returns:         void
 **
 **    Notes:
 **
 **        This function traverses the list from front to back.  No reason
 **        for needing other traversals could be thought of.
 **
 ***************************************************************************/
void btk_llist_onall(
    btk_llist_t * list_p, 
    void (*func_p)(void *, void *),
    void * second_arg)
{
    btk_llist_elem_t * curr_p;
    FUNCTION("btk_llist_onall");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->first_p; curr_p->next_p; curr_p = curr_p->next_p) 
    {
        (*func_p)(curr_p->data_p, second_arg);
    }

    FEXIT(0);

}


/****************************************************************************
 **
 **    Function:        btk_llist_count()
 **
 **    Purpose:         Count the number of elements on a given list
 **
 **    Args:
 **        list_p
 **                     Linked list to traverse.
 **
 **    Returns:         int	Number of elements
 **
 **    Notes:
 **
 **        This function traverses the list from front to back.  No reason
 **        for needing other traversals could be thought of.
 **
 ***************************************************************************/
int btk_llist_count(
    btk_llist_t * list_p) 
{
    btk_llist_elem_t * curr_p;
    int                count = 0;

    FUNCTION("btk_llist_count");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (curr_p = list_p->first_p; curr_p != NULL; curr_p = curr_p->next_p) 
    {
        count++;
    }

    FEXIT(count);
    return(count);

}

