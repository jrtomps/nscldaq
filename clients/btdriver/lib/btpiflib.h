/*****************************************************************************
**
**      Filename:   btqiflib.h
**
**      Purpose:    Irix NanoBus header file, private to interface library
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#include <errno.h>

#if !defined (__vxworks)
#include <pthread.h>
#endif /* !__vxworks */

#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>

#if	!defined(_BTQIFLIB_H)
#define _BTPIFLIB_H

#if	!defined(_BTIO_H)
#include	"btio.h"
#endif	/* !defined(_BTIO_H) */

#if	!defined(_BTAPI_H)
#include	"btapi.h"
#endif	/* !defined(_BTAPI_H) */

/* 
** ICBR list 
*/
typedef struct bt_icbr_list_s {
    /* Linked list processing depends on next_p being first field */
    struct bt_icbr_list_s   *next_p;    /* Next node on list */
    bt_icbr_t   *func_p;    /* ICBR Function to call */
    bt_irq_t	type;	    /* Type of interrupt registered for */
    bt_data32_t vector;     /* Vector to match */
                            /*   BT_VECTOR_ALL matches anything */
    void        *param_p;   /* Parameter to pass through to ICBR */
} bt_icbr_list_t;   

/* 
** Device descriptor
** Data structure for tracking each open descriptor 
*/
typedef struct bt_lib_data_s {
    /* Linked list processing depends on next_p being first field */
    struct bt_lib_data_s *next_p;	/* Next open descriptor */
    int			fd;		/* File descriptor for driver */
    unsigned long       access_flags;   /* access flags to bt_open call */

#if defined(__vxworks)
    SEM_ID              mutex;          /* Mutex for library code */
#else /* __vxworks */ 
    pthread_mutex_t	mutex;		/* Mutex for library code */
#endif /* __vxworks */

    /*
    ** Please note there will be one icbr thread for each open
    ** descriptor that has a icbr installed on it
    **
    ** It may be better to have one icbr per library instance
    ** but the code was written this way from the Nanolink SW
    */
    int                 fd_diag;        /* Diagnostic file descriptor */
#if defined(__vxworks)
    int                 icbr_thread;    /* Thread ID for ICBR dispatch */
#else /* __vxworks */ 
    pthread_t		icbr_thread;	/* Thread ID for ICBR dispatch */
#endif /* __vxworks */

#if defined(__vxworks)
    SEM_ID              icbr_started;   /* Condition var to wait till thread starts */
#else /* __vxworks */ 
    pthread_cond_t      icbr_started;   /* Condition var to wait till thread starts */
#endif /* __vxworks */
    int                 icbr_running;   /* != if thread stops or has error */
    bt_icbr_list_t      *icbr_head_p;	/* First ICBR tracking element */
    bt_data8_t          *icbr_mmap_p;   /* Thread q mmap pointer */
    size_t              icbr_mmap_len;  /* Thread q mmap length */
    bt_devaddr_t        thread_id;      /* Driver assigned thread id */
} bt_lib_data_t;

/*
** Error checking macros
*/
#define	BT_DESC_BAD(btd)	((NULL == btd) || (btd->fd < 0))
#define	BT_PTR_BAD(ptr,len)     ((NULL == ptr) || (len <= 0))

/*
** Error message macro use as follows
** DBG_STR("Message");
** DBG_MSG((stderr, "Message %d\n", value));
*/
#if defined(DEBUG)
#if defined(__vxworks)
#define DBG_STR(string)  logMsg("%d %s\n", __LINE__, (int)string, NULL, NULL, NULL, NULL);
#define DBG_MSG(string)  fprintf string;
#else /* defined __vxworks */
#define DBG_STR(string)    fprintf(stderr,"%s\n", string);
#define DBG_MSG(string)    fprintf string;
#endif /* defined __vxworks */
#else /* DEBUG */
#define DBG_STR(string)
#define DBG_MSG(string)
#endif /* DEBUG */

#endif /* !defined(_BTPIFLIB_H) */
