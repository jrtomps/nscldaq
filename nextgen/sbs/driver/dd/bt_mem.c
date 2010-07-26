/*****************************************************************************
**
**      Filename:   bt_mem.c
**
**      Purpose:    Kernel memory allocation/deallocation routines.
**
**      Functions:      btk_mem_init(), btk_mem_fini(),
**                      btk_mem_alloc(), btk_mem_free()
**
**      $Revision: 2330 $
**
*****************************************************************************/
/*****************************************************************************
**
**            Copyright (c) 1997 - 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
#ifdef  __EPI__
__asm(".rdata");
__asm(".ascii\"@(#)"__FILE__"  $Revision: 2330 $ "__DATE__"\\0\"");
__asm(".align 3; .text");
#else
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ "__DATE__;
#endif  /* __EPI__*/
#endif  /* !LINT */

/*****************************************************************************
**
**      Include files
**
*****************************************************************************/

#include "btdd.h"


/*****************************************************************************
**
**      Global exported data
**
*****************************************************************************/

extern volatile int     btk_alloc_total_g;
volatile int     btk_alloc_total_g = 0;

/*
** Need a mutex to protect allocation/deallocation tracking.
*/

#if defined (__lynxos)

bt_mutex_t kmem_mutex = 0;     /* btk_mem() per driver (single) mutex */

#else

bt_mutex_t      kmem_mutex;     /* btk_mem() per driver (single) mutex */

#endif

#ifdef BT13908
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_mem_init)
#pragma alloc_text(PAGE, btk_mem_fini)
#pragma alloc_text(PAGE, btk_mem_alloc)
#pragma alloc_text(PAGE, btk_mem_free)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_MEM_C);

/*
** Macro so we can print pointers regardless of if kernel logging supports
** the %p format character. Adjust it for each operating system.
*/

#if	defined(__linux__)

#define	PTR_FMT	"%p"

#else	/* defined(__linux__) */

#define	PTR_FMT	"0x%4x"

#endif	/* defined(__linux__) */

/******************************************************************************
**
**      Function:       btk_mem_init()
**
**      Purpose:        Initializes any state information needed by the kernel
**                      memory routines.
**
**      Args:           Void
**          
**
**      Returns:
**              0               Success
**              Otherwise       error value.
**
**      Notes:
**      Must be called exactly once, before the first btk_mem_alloc() call, when
**      the driver is loaded. Do not call it once per unit.
**
**  NT Notes:
**      Caller must be running at IRQL <= DISPATCH_LEVEL.
**
**      These routines do not take the unit pointer as a parameter 
**      because they may be used to allocate the unit pointer.
**
**      btk_mem_init() 
**      Initializes any state information needed by the kernel memory 
**      routines. It must be called exactly once, before the first 
**      btk_alloc() call, when the driver is loaded. Do not call it 
**      once per unit. Return value of 0 indicates success, all others 
**      are error value.
**
******************************************************************************/

int btk_mem_init(
    void
    )
{
    int         retvalue = 0;   /* Assume success */

    FUNCTION("btk_mem_init");
    LOG_UNKNOWN_UNIT;

#if defined(__sgi)
    int    old_level = 0; /* old interrupt level */
#endif /* defined(__sgi) */

    FENTRY;

#if     defined(__sun)

    retvalue = btk_mutex_init(&kmem_mutex, "btk_mem mutex", NULL);

#elif defined(_NTDDK_)

    retvalue = btk_mutex_init(&kmem_mutex, BT_SPIN_LOCK);

#elif defined(__sgi)

    retvalue = btk_mutex_init(&kmem_mutex, "btk_mem_mutex");

#elif defined(BT_uCOS)

    retvalue = btk_mutex_init(&kmem_mutex);

#elif defined(__linux__)

    retvalue = btk_mutex_init(&kmem_mutex, 0);

#elif defined (__lynxos)

    retvalue = btk_mutex_init(&kmem_mutex);

#else

    retvalue = btk_mutex_init(&kmem_mutex);

#endif  /* __sun, _NTDDK_ */

    FEXIT(retvalue);
    return retvalue;
}


/******************************************************************************
**
**      Function:       btk_mem_fini()
**
**      Purpose:        Releases any resources allocated by btk_mem_init().
**
**      Args:           Void
**          
**
**      Returns:        Void
**
**      Notes:
**      Must be called exactly once, after the last btk_mem_alloc() call, when
**      the driver is unloaded. Do not call it once per unit.
**
**      These routines do not take the unit pointer as a parameter 
**      because they may be used to allocate the unit pointer.
**
**      btk_mem_fini()
**      Does the inverse of btk_mem_init(), releasing any resources 
**      in preparation for the driver unloading. It also 
**      must be called exactly once, after all kernel memory has been 
**      returned just before the driver unloads.
**
******************************************************************************/

void btk_mem_fini(
    void
    )
{
    FUNCTION("btk_mem_fini");
    LOG_UNKNOWN_UNIT;
    FENTRY;


    btk_mutex_fini(&kmem_mutex);

    FEXIT(0);

    return;
}



/******************************************************************************
**
**  Function:   btk_mem_alloc()
**
**  Purpose:    Allocate Kernel Memory
**
**  Args:
**              size    Size in bytes to allocate.
**              flags   Various optional flags. Not every system implements
**                      the flags. The following flags are currently defined:
**                      BTK_ALLOC_SWAPPABLE - OK to allocate memory that can be swapped out
**
**  Returns:    
**      NULL:       if not enough memory
**      otherwise:      pointer to allocated memory block
**
**  Notes:
**
**  NT Notes:
**      Callers must be running at IRQL PASSIVE_LEVEL if BTK_ALLOC_SWAPPABLE,
**      otherwise callers may run at IRQL <= DISPATCH_LEVEL.
**
**
**      These routines do not take the unit pointer as a parameter 
**      because they may be used to allocate the unit pointer.
**
******************************************************************************/

/* ARGSUSED */

void * btk_mem_alloc(
    size_t size,
    bt_data32_t flags
    )
{
    void * kmem_p = NULL;

    FUNCTION("btk_mem_alloc");
    LOG_UNKNOWN_UNIT;
    FENTRY;

#if     defined (_AIX)

    kmem_p = xmalloc(size, ALIGN_LONG, 
        (flags & BTK_ALLOC_SWAPPABLE) ? 0 : pinned_heap);

#elif   defined(__hpux)

    /*
    **  sys_memall() allocates a number of virtual memory pages (NBPG=4K).
    **  Since the driver calls btk_mem_alloc() to get each interrupt
    **  registration structure this is wasteful of lockable system memory.
    **  Eventually it would be a good idea to add another layer of memory
    **  management to clean this up.
    */
    kmem_p = sys_memall(size);

#elif   defined(__sun)

    kmem_p      = kmem_alloc( size, KM_NOSLEEP);

#elif   defined(__sgi)

    /*
    ** Does not request cache-aligned or physically contiguous memory.
    ** If the memory is going to be DMA'd in to or out of (i.e. write
    ** or read respectively) then kmem_alloc() must be called directly
    ** with the appropriate flag for cache-alignment.  In addition to
    ** cache-aligning the buffer, cache flushing issues must be taken
    ** into consideration and are unique for the platform in question.
    */

    kmem_p = kmem_alloc(size, 0);

#elif   defined(__vxworks)

    kmem_p = malloc(size);

#elif    defined(BT_uCOS)

    kmem_p = malloc(size);

#elif   defined(_NTDDK_)

    /* The requested memory is not cache aligned or physically contiguous.
       Don't use for DMA.  Also, don't use for a buffer to be shared with
       user space unless size < physical page size because the user virtual
       address won't be valid across non-contiguous pages.
    */
    kmem_p = ExAllocatePool(
        (flags & BTK_ALLOC_SWAPPABLE) ? PagedPool : NonPagedPool, size);

#elif	defined(__linux__)

    kmem_p = kmalloc(size, GFP_KERNEL);

#elif defined(__lynxos)

    kmem_p = sysbrk(size);

#endif  /* _AIX, __hpux, __sun, __sgi, __vxworks, _NTDDK_ */

    /* Protect btk_alloc_total_g accesses.  This mutex may not be held during
       the preceeding memory allocation to avoid doing the allocations at a
       raised interrupt level. */
    btk_mutex_enter(&kmem_mutex);

    btk_alloc_total_g += size;    /* running total of kmem */

    if (btk_alloc_total_g < 0) {
        FATAL_STR("Allocated kernel memory went negative.\n");
    }


    TRC_MSG((BT_TRC_ALLOC), 
            (LOG_FMT "ptr " PTR_FMT "; size %d; total %d.\n",
            LOG_ARG, kmem_p, (int) size, btk_alloc_total_g));

    btk_mutex_exit(&kmem_mutex);

    FEXIT(kmem_p);

    return(kmem_p);
}



/******************************************************************************
**
**      Function:   btk_mem_free()
**
**      Purpose:    Deallocate Kernel Memory
**
**      Args:       kmem_p  ptr to memory allocated with btk_mem_alloc().
**                  size    size of memory to be freed.
**
**      Returns:    Void
**
**  NT Notes:
**      Callers must be running at IRQL PASSIVE_LEVEL if memory was allocated
**      with BTK_ALLOC_SWAPPABLE, otherwise callers may run at IRQL <=
**      DISPATCH_LEVEL.
**
**
**      These routines do not take the unit pointer as a parameter 
**      because they may be used to allocate the unit pointer.
**
**      btk_mem_alloc()
**      Allocates a block of memory 'size' bytes in length from kernel 
**      space. Passing zero for flags should always result in safe 
**      behavior for that system. It returns either NULL (if not 
**      enough memory) or a pointer to the allocated memory block.
**
**      The flags parameter gives additional OPTIONAL permissions:
**      BTK_ALLOC_SWAPPABLE     It is allowable to allocate memory 
**                              that can be swapped (paged) out to disk.
**
**      btk_mem_free()
**      Releases memory previously allocated by btk_mem_alloc(). 
**      Size parameter must be identical value to what was passed 
**      in on original btk_mem_alloc() request.  Not all implementations 
**      will need to use the size parameter.
**
******************************************************************************/

void btk_mem_free(
    void * kmem_p, 
    size_t size
    )
{
    FUNCTION("btk_mem_free");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    TRC_MSG((BT_TRC_ALLOC | BT_TRC_DETAIL), 
            (LOG_FMT "ptr: " PTR_FMT "  size %d.\n",
            LOG_ARG, kmem_p, (int) size));


#if     defined (_AIX)
    {
        int     retvalue;

        retvalue = (int) xmfree(kmem_p, pinned_heap);
        if (o != retvalue) {
            INFO_MSG((LOG_FMT "Error %d from xmfree().\n"));
        }
    }

#elif   defined(__hpux)

    sys_memfree(kmem_p, size);

#elif   defined(__sun)

    kmem_free( kmem_p, size ); /* returns default value of retvalue */

#elif   defined(__sgi)

    kmem_free(kmem_p, size);

#elif   defined(__vxworks)

    free(kmem_p);

#elif   defined(_NTDDK_)

    ExFreePool(kmem_p);

#elif   defined(BT_uCOS)

    free(kmem_p);

#elif	defined(__linux__)

    kfree(kmem_p);

#endif  /* _AIX, __hpux, __sun, __sgi, __vxworks, _NTDDK_ */


    btk_mutex_enter(&kmem_mutex);

    btk_alloc_total_g -= size;    /* running total of kmem */
    if (btk_alloc_total_g < 0) {
        FATAL_STR("Allocated kernel memory went negative.\n");
    }

    TRC_MSG((BT_TRC_ALLOC), 
            (LOG_FMT "ptr " PTR_FMT "; size %d; total %d.\n",
            LOG_ARG, kmem_p, (int) size, btk_alloc_total_g));

    btk_mutex_exit(&kmem_mutex);

    FEXIT(0);
    return;
}

