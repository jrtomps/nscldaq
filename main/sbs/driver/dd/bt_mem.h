/*****************************************************************************
**      Filename:   bt_mem.h
**
**      Purpose:    Generic header for bt_mem.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_mem.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.6 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by SBS Technologies, Inc.     
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_MEM_H)
#define _BT_MEM_H

/*****************************************************************************
**
**  Include Files 
**
*****************************************************************************/

#if     defined(__sun)
#include        <sys/kmem.h>

#elif	defined(__linux__)

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include	<linux/slab.h>
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
#include	<linux/malloc.h>
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
            LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

#elif defined(__vxworks)
#include <stdio.h>
#include <vxworks.h>
#include <string.h> /* for bzero() */
#if defined (FCTACH)
#include <btdef.h>
#endif /* defined (FCTACH) */

#endif  /* defined(__sun) */

/*****************************************************************************
**
**  Data Structures 
**
*****************************************************************************/

/* Global used to track memory usage */

extern volatile int btk_alloc_total_g;

/* Constants for btk_alloc() flags parameter */

#define BTK_ALLOC_SWAPPABLE     (1u<<0) /* Allow memory to be swapped out */

/*****************************************************************************
**
**  Protoypes
**
*****************************************************************************/


/* Prototypes for routines in bt_mem.c file */

int btk_mem_init(void);

void btk_mem_fini(void);

void *btk_mem_alloc(size_t size, bt_data32_t flags);

void btk_mem_free(void * kmem_p, size_t size);

/*****************************************************************************
**
**      Definition of BTK_BZERO
**
*****************************************************************************/
#if defined (_AIX)
#define BTK_BZERO(buf_p, size)     bzero((buf_p), (size))

#elif defined (BT_NTDRIVER)
#define BTK_BZERO(buf_p, size)   RtlZeroMemory((buf_p), (size))
#define BTK_BCOPY(src_p, dest_p, size)  RtlCopyMemory((dest_p), (src_p), (size))

#elif defined (__hpux)
#define BTK_BZERO(buf_p, size)     bzero((buf_p), (size))

#elif defined (__sgi)
#define BTK_BZERO(buf_p, size)          bzero((buf_p), (size))
#define BTK_BCOPY(src_p, dest_p, size)  bcopy((src_p), (dest_p), (size))

#elif defined (__sun)
#define BTK_BZERO(buf_p, size)          bzero((buf_p), (size))
#define BTK_BCOPY(src_p, dest_p, size)  bcopy((src_p), (dest_p), (size))

#elif defined (__vxworks)
#define BTK_BZERO(buf_p, size)          bzero((buf_p), (size))
#define BTK_BCOPY(src_p, dest_p, size)  bcopy((src_p), (dest_p), (size))

#elif defined (BT_uCOS)
#define BTK_BZERO(buf_p, size)     memset((buf_p), '\0', (size))

#elif defined (__linux__)
#define BTK_BZERO(buf_p, size)     memset((buf_p), '\0', (size))
#define BTK_BCOPY(src_p, dest_p, size)  memcpy((dest_p), (src_p), (size))

#elif defined (__lynxos)
#define BTK_BZERO(buf_p, size)     memset((buf_p), '\0', (size))
#define BTK_BCOPY(src_p, dest_p, size)  memcpy((dest_p), (src_p), (size))

#else
#error unknown operating system
#endif /* operating system */


#endif /* !defined(_BT_MEM_H) */
