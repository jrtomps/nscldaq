/*****************************************************************************
**
**      Filename:   btdd.h
**
**      Purpose:    Shared driver private header file
**
**      Macros, Prototypes & Functions: 
**      BTK_BZERO, BTK_ASSERT, btk_tck_get, btk_tck2msec, btk_msec2tck, 
**      BTK_GET_PROC, BTK_REL_PROC
**
**      From bt_mem.c:
**      btk_mem_init, btk_mem_fini, btk_mem_alloc, btk_mem_free
**
**      From bt_bit.c:
**      btk_bit_init, btk_bit_fini, btk_bit_reset, btk_bit_alloc, btk_bit_free,
**      btk_bit_set, btk_bit_clr, btk_bit_chk, btk_bit_max, btk_bit_specify,
**      btk_map_half, btk_map_restore
**
**      From bt_mutex.c:
**      btk_mutex_init, btk_mutex_fini, btk_mutex_enter, btk_mutex_exit
**
**      From bt_rwlck.c:
**      btk_rw_lock_init, btk_rw_lock_fini, btk_rwlock_wr_enter, 
**      btk_rwlock_rd_enter, btk_rwlock_wr_exit, btk_rwlock_rd_exit
**
**      From bt_queue.c:
**      btk_queue_init(), btk_queue_fini(), btk_queue_item_create(),
**      btk_queue_item_destroy(), btk_queue_push(), btk_queue_pull(),
**      btk_queue_count()
**
**      From bt_slist.c:
**      btk_slist_init(), btk_slist_fini(), btk_slist_item_create(),
**      btk_slist_item_destroy(), btk_slist_insert(), btk_slist_find(),
**      btk_slist_remove(), btk_slist_count(), btk_slist_step()
**
**      From bt_llist.c:
**      btk_llist_init(), btk_llist_next(), btk_llist_prev(), 
**      btk_llist_first(), btk_llist_last(), btk_llist_elem_init(), 
**      btk_llist_insert_first(), btk_llist_insert_last(), 
**      btk_llist_insert_after(), btk_llist_insert_before(), 
**      btk_llist_remove(), btk_llist_find_first(), btk_llist_find_last(), 
**      btk_llist_find_next(), btk_llist_find_prev(), btk_llist_onall()
**      btk_llist_count()
**
**      From bt_crc16.c:
**      BT_CRC16(), bt_crc16_buffer(), bt_crc16_buffer_swap()
**
**      From bt_fifo.c:
**      btk_fifo_init(), btk_fifo_head_init(), btk_fifo_fini(), 
**      btk_fifo_head_fini(), btk_fifo_insert(), btk_fifo_remove(), 
**      btk_fifo_length(), btk_fifo_is_overflowing(), btk_fifo_is_empty()
**      
**	From bt_delay.c
**	btk_delay()
**      btk_sleep()
**      btk_timeout()
**      btk_untimeout()
**
**      $Revision$
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1997 - 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef _BTDD_H
#define _BTDD_H

#define EAS_BIND_CODE   1
#define EAS_A64_CODE    1

/*
** Note:        This file includes the header file (btxdd.h) for the specific
**              driver after defining some data structures and macros shared
**              by several drivers.
*/

/*****************************************************************************
**
**      Header files included
**
*****************************************************************************/

#include <stddef.h>     /* Needed for size_t */

#if	defined(__linux__)

#include        <linux/types.h>
#include	<asm/semaphore.h>
#include	<linux/timer.h>
#include	<linux/version.h>

#elif     defined(__unix) || defined(__vxworks)
#include        <sys/types.h>
#endif  /* defined(__unix) || defined(__vxworks) */

#if defined (BT_NTDRIVER)

#if defined(BT_WDM_DRIVER)
    /* We're a new WDM driver and not an old NT driver */
#include <wdm.h>
#else /* BT_WDM_DRIVER */
    /* We're an old NT driver */
#include <ntddk.h>
#endif /* BT_WDM_DRIVER */

#elif defined (__sun)
#include <sys/debug.h>  /* For ASSERT macro */

#elif defined (__vxworks)
#include <assert.h>
#include <semLib.h>      /* For mutex and event code */
#include <wdlib.h>        
#include <syslib.h>      /* for sysClkRateGet */
#include <logLib.h>      /* for logMsg() */
#include <stdio.h>       /* needed for fprintf() calls */
#include <string.h>      /* for bzero */


#elif defined (BT_uCOS)
#include <string.h>		/* For BTK_BZERO() macro, memset() */
#include <assert.h>		/* For BTK_ASSERT() macro, assert() */
#include <../sys/ucos/includes.h>

#endif  /* OS dependent header file inclusions */

#if   defined(BT984)
#define MIN_SIZE_MAPPING_BAR_0          0x200  // Min Size of BAR0 required mapping
#endif


/*****************************************************************************
**  Generic macros that may be useful to all device driver platforms.
*****************************************************************************/

/* BT_BCD4 converts value to a four digit Binary Coded Decimal value which is */
/* quite useful when invoked as BT_BCD4(__LINE__) in hardware tracing macros. */
#define BT_BCD4(value)  ((value) / 1000 % 10 << 12 \
                        | (value) / 100 % 10 << 8 \
                        | (value) / 10 % 10 << 4 \
                        | (value) % 10)

/*****************************************************************************
******************************************************************************
**
**      Documentation on kernel interface routines
**
******************************************************************************
******************************************************************************

This documents various decisions made while creating a library of kernel code 
that we could share.


In general, all the kernel shared routines start with btk_ in the function 
name, and bt_ in the file name. Unless there is a reason not to, the unit 
structure is passed to these routines.


There are twelve sections:
1: Allocation/deallocation of kernel memory
2: Macro to clear memory
3: Mutual exclusion
4: Event notification
5: Assertion checking
6: Bit map allocation/deallocation
7: Reader/writer lock
8: Singly linked list queue (documentation section not written yet)
9: Sorted list  (documentation section not written yet)
10: Unordered doubly linked list (documentation section not written yet)
11: CCITT CRC-16 Checksums
12: Circular queue of fixed-sized data structures (fifo)

The prototypes for all these routines are in a single header file: 
/share/include/btdd.h This replaces the symbolic link to btxdd.h that
we used in the past.


---
1: Allocation/deallocation of kernel memory
File: /share/kernel/bt_mem.c

Functions:
int btk_mem_init(void);
void btk_mem_fini(void);
void * btk_mem_alloc(size_t size, bt_data32_t flags);
void btk_mem_free(void * kmem_p, size_t size);

These routines do not take the unit pointer as a parameter because they may 
be used to allocate the unit pointer.


btk_mem_init() 
Initializes any state information needed by the kernel memory routines. It 
must be called exactly once, before the first btk_alloc() call, when the 
driver is loaded. Do not call it once per unit. Return value of 0 indicates 
success, all others are error value.

The btk_mem_init() routine calls btk_mutex_init() with NULL as the unit 
parameter. This allows us to use the normal btk_mutex_enter() and 
btk_mutex_exit() routines to ensure that the tracking of resources works on 
multiprocessor systems.


btk_mem_fini()
Does the inverse of btk_mem_init(), releasing any resources in preparation
for the driver unloading. This includes a call to btk_mutex_fini() with NULL
as the unit paramter. It also must be called exactly once, after all kernel
memory has been returned just before the driver unloads.


btk_mem_alloc()
Allocates a block of memory 'size' bytes in length from kernel space. Passing 
zero for flags should always result in safe behavior for that system. It 
returns either NULL (if not enough memory) or a pointer to the allocated 
memory block.

The flags parameter gives additional OPTIONAL permissions:
BTK_ALLOC_SWAPPABLE     It is allowable to allocate memory that can be 
                        swapped (paged) out to disk.


btk_mem_free()
Releases memory previously allocated by btk_mem_alloc(). Size parameter must 
be identical value to what was passed in on original btk_mem_alloc() request. 
Not all implementations will need to use the size parameter.


---
2: Macro to clear memory
File: /share/include/btdd.h

Rather than calling a routine, this is a macro in this header file.

It calls the appropriate routine for that operating environment. It only has 
to work for kernel memory.

This does the equivilent to memset(buf_p, '\0', size). We need this because 
not every system allows access to full Standard C library from kernel.

---

3: Mutual exclusion handling
File: /share/kernel/bt_mutex.c

int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p, ...);
void btk_mutex_fini(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

void btk_mutex_enter(bt_unit_t *unit_p, bt_mutex_t *mutex_p);
void btk_mutex_exit(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

The bt_mutex_t is an opaque type to be defined for each operating system.

All of the mutex routines need to accept NULL for the unit pointer. This will 
most likely only affect tracing.


btk_mutex_init()
Initializes the mutex. The parameters for this routine will vary depending on 
the operating system. Each operating system is expected to need slightly 
different information including the highest interrupt level or process 
priority level it has to protect against.

That the parameters change depending on which OS is used is not seen as 
a problem because this is done in the driver configuration routine. That 
routine is already very driver and operating system specific, there is 
little or no code that could be shared between modules.

If the operating system differeniates between spin locks and sleepable 
mutexs, the btk_mutex_init() routine will have to decide which is used 
at the time that the mutex is created.

When creating a mutex, if you expect it to busy wait it would be a good 
idea to select a name that indicates this. For example, a mutex that blocks 
out our interrupt service routien may be called isr_spin.


btk_mutex_fini()
Destroys the mutex, releasing any resources allocated when the mutex was 
created.


void btk_mutex_enter()
Waits for the mutex. Depending on how the mutex was initialized, this could 
be either a busy wait or it could cause the thread to sleep and allow other 
processes to run.

The routine will wait forever for the resource. There is no timeout. 

The mutex can not be nested.


void bt_mutex_exit()
Releases the mutex, allows other processes to acquire it.


To review, these mutual exclusion routines:
1: Wait forever for the resource
2: Can't be nested.
3: Can either spin lock (busy wait) or task switch (sleep) depending on the 
parameters passed into btk_mutex_init().
4: With the exception of the creation routine, they do not return any result 
code. They are assumed to always complete correctly.


---

4: Event notification
File: /share/kernel/bt_event.c

bt_tck_t btk_msec2tck(bt_msec_t time);
bt_msec_t btk_tck2msec(bt_tck_t time);

int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state, ...);
void btk_event_fini(bt_unit_t *unit_p, bt_event_t *event_p);
bt_error_t btk_event_wait(bt_unit_t *unit_p, bt_event_t *event_p, bt_tck_t 
timeout)
void btk_event_set(bt_unit_t *unit_p, bt_event_t *event_p);


The bt_event_t is an opaque type to be defined for each operating system.

All of the event routines need to accept NULL for the unit pointer. This will 
most likely only affect tracing.

The basic idea is to wait for the occurance of something, with a timeout 
interval. Three examples of uses include creating a locking mechanism, waiting 
for a DMA Done interrupt, and waiting for a remote system reset to complete.



btk_msec2tck()
Converts from milliseconds to 'bt_tck_t', which should be the same format as 
the native operating system needs for it's timer. Can use a macro or static 
function in btdd.h for this routine.


btk_tck2msec()
Converts from native operating system time format to milliseconds. Inverse of 
btk_tck2msec() routine. Can use a macro or static function in btdd.h for this 
routine.


btk_event_init()
Creates the object. The 'state' parameter is the initial state of the 
condition, TRUE (full) or FALSE (empty). The parameters for this routine 
will vary depending on the operating system. Some operating systems may 
need additional information in order to create the object.


btk_event_fini()
Gets rid of object, flushing any threads waiting, and releases resources
used by the object.


btk_event_wait()
Waits until event becomes available. Goes to sleep if empty. The object is 
empty when this routine returns with SUCCESS.

There are only three valid return values from this routine:
BT_SUCCESS      Got the object, everything is just swell.
BT_EBUSY        Timeout expired before the object became available.
BT_EABORT       A user action caused the operation to abort.


It is assumed that there will not be any other error cases in production 
drivers. The library routine may include assertions to this effect.

The timeout value is in a format native to the operating system. Use 
btk_msec2tck() to convert a bt_msec_t value to this value.


btk_event_set()
Forces the object set (full). If there are any threads waiting on the object, 
this will cause EXACTLY ONE of those sleeping threads to wake.


---

5: Assertion checking, the assert() macro from Standard C in a kernel version
File: /share/include/btdd.h

BTK_ASSERT(expression)

Assertion checking, verifies that the expression is true. This is a rough 
equivilent to the Standard C assert() checking, and always evaluates to TRUE 
if NDEBUG is defined.

This provides a debugging check for things that should NEVER happen.

Do not use it for external parameter validation. If a parameter is passed in 
from the user, we need the validation checks in the production version of the 
driver.

It is meant to check assumptions made by the code. This would include 
assumptions about the relative order of some enum values, the size of a 
structure, or possibly that a required value has already been initialized.

Production code should always define NDEBUG, resulting in all the assertion 
checking being compiled out of the production code.

The actual implementation should print out the assertion that failed and then 
most likely execute a breakpoint in the driver. Each OS should choose an 
appropriate response that makes it reasonably easy to identify what 
assumption failed and where.


---
6: Bit map allocation/deallocation
File: /share/kernel/bt_bit.c


bt_error_t btk_bit_init(bt_unit_t *unit_p, unsigned size, void **bitmap_p)
bt_error_t btk_bit_reset(bt_unit_t *unit_p, void *bitmap_p)
bt_error_t btk_bit_fini(bt_unit_t *unit_p, void *bit_map_p)

bt_error_t btk_bit_free(bt_unit_t *unit_p, void *bitmap_p, unsigned start, 
unsigned num_entries)

bt_error_t btk_bit_alloc(bt_unit_t *unit_p, void *bitmap_p, unsigned needed, 
unsigned align, unsigned *first_p)

int btk_bit_set(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum)
int btk_bit_clr(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum)
int btk_bit_chk(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum)

bt_error_t btk_bit_max(bt_unit_t *unit_p, void * bit_map_p, unsigned align, unsigned *size);

bt_error_t btk_bit_specify(bt_unit_t *unit_p, void *bitmap_p, unsigned start, unsigned needed);

Routines that manipulate a bit map allocation structure. One example would
be using them in order to track mapping register usage.

These are not general bitset manipulation routines, but instead are targeted
at allocating and freeing a set of resources (page registers, etc.).


btk_bit_init()
Creates the bit map structure for use. The 'size' parameter is how many bits 
are needed for the bitmap.

btk_bit_reset()
Resets a bit map structure to intial state, with all bits unallocated. Allows 
you to reset it, rather than requiring that the bit map be destroyed and 
created all over again.

btk_bit_fini()
Releases resources for given bit map structure.

btk_bit_free()
Releases previously allocated portion of bit map.

btk_map_half()
Temporary reduces a bit map to half of its original size.  Useful for PCI to
PCI mode where only half of the 32 Mbytes of mapping regs are used.

btk_map_restore()
Restores a halfed bit map to its original size.  

btk_bit_alloc()
Allocates a portion of bit map. The 'needed' parameter is how many bits are 
required, the 'align' specifies the alignment required, and 'first_p' is the 
index (starting from zero) of the first available entry.

If there are not enough bits available to fulfill the request, the 'first_p' 
is set to a value greater than the maximum bit number in the structure.

btk_bit_specify()
Allocates a given portion of bit map if it is free. The 'start' parameter is 
the starting bit of the region.  The 'needed' parameter is the number of
bits the region will cover.  If the entire region is free it is changed to
allocated, otherwise it is untouched and an error is returned.

btk_bit_set()
Forces a single bit to be set i.e. allocated.  Returns the previous value
of the bit or -1 on error.

btk_bit_clr()
Forces a single bit to clear i.e. freed.  Like btk_bit_set() it returns
the previous value of the bit.

btk_bit_chk()
Returns the current value of the bit- 0 if cleared/freed, 1 if set/allocated,
or -1 on error.

btk_bit_max()
Find the largest section of consecutive free bits available. Useful for finding
out how big an allocation can succeed.

---
7: Reader/writer lock
File: /share/kernel/bt_rwlock.c

int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_fini(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_wr_enter(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_rd_enter(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_wr_exit(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_rd_exit(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
#define BTK_RWLOCK_REF_COUNT(rwlock_p) (rwlock_p)->ref_count


btk_rwlock_init()
Initializes a kernel reader/writer lock object.


btk_rwlock_fini()
Releases any resources allocated by btk_rwlock_init().


btk_rwlock_wr_enter()
Acquire the reader/writer lock for writer (exclusive) access.

Preference is given to write requests.  If the lock is held by one
or more readers when btk_rwlock_wr_enter() is called, all subsequent
calls to btk_rwlock_rd_enter() will block until the pending write
request has been acquired and released.  This is done to avoid starving
the write requests.

Reader/write lock operations are not recursive.  If the same thread
calls either btk_rwlock_wr_enter() or btk_rwlock_rd_enter() after
acquiring either the reader or writer lock, a deadlock will result.


btk_rwlock_rd_enter()
Acquire the reader/writer lock for reader (shared) access.

An arbitrary number of threads may hold the reader lock simultaneously.

Preference is given to write requests.  If the lock is held by one
or more readers when btk_rwlock_wr_enter() is called, all subsequent
calls to btk_rwlock_rd_enter() will block until the pending write
request has been acquired and released.  This is done to avoid starving
the write requests.

Reader/write lock operations are not recursive.  If the same thread
calls either btk_rwlock_wr_enter() or btk_rwlock_rd_enter() after
acquiring either the reader or writer lock, a deadlock will result.


btk_rwlock_wr_exit()
Releases a kernel writer lock.

The caller must be running in the context of the same thread that
acquired the lock.


btk_rwlock_rd_exit()
Releases a kernel reader lock.

The caller must be running in the context of the same thread that
acquired the lock.


BTK_RWLOCK_REF_COUNT(rwlock_p
Macro to get the reader/writer lock reference count.  This is the number
of readers currently holding the lock.  As written this assumes that all
implementations of bt_rwlock_t have a ref_count member.

--

11: CCITT CRC-16 Checksum calculation
File: /share/kernel/bt_crc16.c

There are two ways we can do CRC calculation: the efficient way, which
uses a 512-byte table, and the expensive way, which doesn't need the table.
I'm providing both.  To not use the table, #define NO_CRC_TABLE.

Note that the CRC functions do no tracing.

Functions:
    BT_CRC16()
    bt_crc16_buffer()
    bt_crc16_buffer_swap()

bt_data16_t BT_CRC16(bt_data16_t initial_crc, bt_data8_t data)
    Calculates a crc value for a single byte.  This macro either expands
to a call to the function bt_crc16_calc() if NO_CRC_TABLE is defined, or to
an inline expression using the external table bt_crc16_table[] if NO_CRC_TABLE
is not defined.  If it is a macro, old_crc is referenced multiple times, but
it's value is not modified.  The normal way this macro is used is:
        for (idx = 0; idx < len; ++idx) {
            crc = BT_CRC16(crc, buf_p[idx]);
        }

bt_data16_t bt_crc16_buffer(bt_data16_t initial_crc, void * buf_p, size_t len)
    Calculates the crc for an entire buffer.  Basically, it has already
implemented the example for loop just above.

bt_data16_t bt_crc16_buffer_swap(bt_data16_t initial_crc, void * buf_p, 
                               size_t len, size_t siz)
    Like bt_crc16_buffer, but instead of iterating through the buffer
directly, it does an implicit endianess swap.  siz is one of 8, 4, 2,
or 1- the maximum word-size to byte swap.  The function works like the
following loop:
        for (idx = 0; idx < len; ++idx) {
            crc = BT_CRC16(crc, buf_p[idx ^ (siz - 1)]);
        }
With one difference, that lengths which are not a even multiple of siz
are handled by decreasing siz.  

---

12: Circular queue of fixed-size data stuctures
File: bt_fifo.c

As both "circular buffer" and "queue" are defined, I'm calling this function
a fifo.

These functions handle a circular queue of fixed-sized data structures,
where the size of the data structures is small enough that using linked
lists would greatly increase the memory utilization, and where the cost
of copying the structure is negligable.  The obvious use for this code
is IRQ handling- where the structure is only two words long, the IRQ
type and the vector.  Adding two pointers to the structure would double
the memory use, but copying two words is cheap.  Also, the routines do
not require allocating memory except at initialization time, or handling
free lists.

I seperated out the head pointer from the rest of the fifo structure-
the normal case is that there is only one "inserter" (the ISR) and
multiple "removers" (one per registered ICBR).  By having the head pointer
seperate, each "remover" can have it's own head pointer, and a more
complex API can be avoided.

I attempted to write the functions such that, in the normal case, no
mutual exclusion is needed.  If there is only one ISR running at a time,
and it is the only one inserting elements, and each ICBR has it's own
head pointer, then no mutual exclusion should be required.  The function 
comments (in bt_fifo.c) define what mutual exclusion is needed in more
complex cases.  These routines do not do mutual exclusion themselves.

Functions:
    btk_fifo_init()
    btk_fifo_head_init()
    btk_fifo_fini()
    btk_fifo_head_fini()
    btk_fifo_insert()
    btk_fifo_remove()
    btk_fifo_length()
    btk_fifo_is_overflowing()
    btk_fifo_is_empty()

btk_fifo_init()
	Initializes a fifo structure.  Note that you have to allocate in
the internal buffer it uses before calling this function.  You also set
the size of the objects handled by the queue here.

btk_fifo_head_init()
	Initialize a head structure.  This should be called after 
btk_fifo_init() is called, but it can be called anytime thereafter (even
after objects have already been added and removed from the queue).  The
head pointer is initialized such that the fifo is currently empty.

btk_fifo_fini()
	Finalize the fifo structure.  Note that this does not free the 
internal buffer (that was passed in to btk_fifo_init), so an implementation
will have to arrange to free that seperately after btk_fifo_fini() is called.

btk_fifo_head_fini()
	Finalized a head pointer structure.  Mainly there for symmetery.

btk_fifo_insert()
	Add an object into the queue.  The code is optimized for adds- it is 
assumed that inserts are taking place in some sort of interrupt or signal
context, and that the less time spent there the better.  Note the size of
the data structure being added was set in btk_fifo_init().  Also note that
the data is copied out of the passed in structure and into the internal
buffer.

btk_fifo_remove()
	Remove an object from the queue.  This code is more complex and
time consuming than insert because it's assumed to be running in a "normal"
context where the extra computation is not a problem.  So this is where
overflows are detected, for example.

btk_fifo_length()
	Returns the number of items in the queue.  Note that if the
queue is overflowing, this number can be larger than the size of the internal
buffer.

btk_fifo_is_empty() and btk_fifo_is_overflowing()
	Are simple functions to query the state of the fifo.


--
13. 

btk_delay()
    Provides a busy-wait delay function for SMALL time delays.
Usually only used for fast polling operations.

btk_sleep()
    Provides a non-busy wait sleep function.  Try to use this function
as little as possible in shared code since most operating systems have
restriction on what locks can be held across this call.  Instead use
events to wait for a situtation to occur.  Return value may
not be implementable on all systems, some may allways return BT_SUCCESS.

btk_timeout()
    Schedules the given fuction to be called useconds later.  The fuction 
is passed the given argument pointer.

btk_untimeout()
    Cancels a timeout function scheduled with btk_timeout.


******************************************************************************
******************************************************************************
**
**      End of documentation
**
******************************************************************************
*****************************************************************************/



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

#else
#error unknown operating system
#endif /* operating system */

/*****************************************************************************
**
**      Definition of BTK_ASSERT
**
*****************************************************************************/
/* NT needs NDEBUG define before we can start the assert definitions */
#if defined (BT_NTDRIVER)
#if DBG  
#undef NDEBUG
#else   /* !DBG */
#ifndef NDEBUG
#define NDEBUG
#endif  /* !NDEBUG */
#endif  /* !DBG */
#endif /* end BT_NTDRIVER */


#if     defined(NDEBUG)
#define BTK_ASSERT(X)   (TRUE)

#else   /* NDEBUG */

#if defined (_AIX)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}


#elif defined (BT_NTDRIVER)
#define BTK_ASSERT(EX)  if (!(EX)) { \
    FATAL_STR( #EX ); \
    DbgPrint(" Assertion failed -\n     " __FILE__ " LN " LINE_STR); \
    DbgBreakPoint(); \
}

#elif defined (__hpux)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__sgi)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__sun)

#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); debug_enter("Assertion failed:" __FILE__ LINE_STR);}

#elif defined (__vxworks)
#define BTK_ASSERT(EX)  assert((EX))

#elif defined (BT_uCOS)
#define BTK_ASSERT(EX)  assert((EX))

#elif defined (__linux__)
#define BTK_ASSERT(EX)  if (!(EX)) \
        { FATAL_STR( #EX ); panic("Assertion failed:" __FILE__ LINE_STR);}

#else
#error unknown operating system
#endif /* operating system */

#endif /* NDEBUG */

/*****************************************************************************
**
**      Definition of BTK_LOCK_ISR & BTK_UNLOCK_ISR
**
** These macros are meant to provide a means of locking out the driver's 
** interrupt service rouitne.  Currently they provide a system defined return
** type BTK_LOCK_RETURN_T that is required on vxWorks and sgi.  As  
** platforms are implemented, if a return value is not necessary 
** BT_LOCK_RETURN_T should be defined as int and not used during the 
** BTK_LOCK/UNLOCK_ISR macros.  It has not been determined that the 
** BTK_LOCK_RETURN_T will meet the needs of all systems, therefore, this 
** type and these macros may need to be modified in the future, but this 
** is a start for now.
**
*****************************************************************************/
#if defined (_AIX)
#error ISR locking not defined for AIX

#elif defined (__hpux)
#error ISR locking not defined for HP-UX

#elif defined (__sgi)
#define BTK_LOCK_RETURN_T               int
#define BTK_LOCK_ISR(unit_p, curr_pl)   curr_pl = (LOCK(&(unit_p->isr_lock),plhi))
#define BTK_UNLOCK_ISR(unit_p, new_pl)  (UNLOCK(&(unit_p->isr_lock), new_pl))

#elif defined (__sun)
#define BTK_LOCK_RETURN_T               int
#define BTK_LOCK_ISR(unit_p, curr_pl)   mutex_enter(&unit_p->isr_lock);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  mutex_exit(&unit_p->isr_lock);

#elif defined (__vxworks)

/*
** Locking interrupts is a more efficient way of protecting a critical 
** section that task locking since interurpt lockint just involves hardare 
** manipulation and task unlocking requires a pass through the rescheduling 
** code.  But interrupt locking is dangerous. If you make a mistake with 
** interrupt locked, you hang your processor. 
*/
#define BTK_LOCK_RETURN_T                int
#define BTK_LOCK_ISR(unit_p, lock_key)   lock_key = (intLock())
#define BTK_UNLOCK_ISR(unit_p, lock_key) (intUnlock((lock_key)))

#elif defined (BT_uCOS)
#error ISR locking not defined for UCOS

#elif defined (__linux__)
#define BTK_LOCK_RETURN_T               int
#define BTK_LOCK_ISR(unit_p, curr_pl)   curr_pl = 0; btk_mutex_enter(unit_p, &unit_p->isr_lock);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  btk_mutex_exit(unit_p, &unit_p->isr_lock);

#elif defined(BT_NTDRIVER)
#define BTK_LOCK_RETURN_T               KIRQL
#define BTK_LOCK_ISR(unit_p, curr_pl)   KeAcquireSpinLock(&unit_p->isr_lock, &curr_pl);
#define BTK_UNLOCK_ISR(unit_p, new_pl)  KeReleaseSpinLock(&unit_p->isr_lock, new_pl);

#else
#error unknown operating system
#endif /* operating system */

/*****************************************************************************
**
**  Data structures for bt_queue.c routines
**
*****************************************************************************/

typedef unsigned long btk_list_type_t;

typedef struct btk_queue_item {
    struct btk_queue_item  *next_p;     /* Next (forward) pointer             */
    btk_list_type_t         queue_type; /* [non-]interrupt / [non-]swappable. */
    void                   *data_p;     /* Pointer to the data section.       */
    size_t                  data_size;  /* Data section's size in bytes.      */
} btk_queue_item_t;

typedef struct btk_queue {
    struct btk_queue_item  *push_p;     /* Pointer to last queue item pushed. */
    struct btk_queue_item  *pull_p;     /* Pointer to next queue item pulled. */
    btk_list_type_t         queue_type; /* [non-]interrupt / [non-]swappable. */
    unsigned long           count;      /* Current count of items in queue.   */
} btk_queue_t;

/*****************************************************************************
**
**  Data structures for bt_slist.c routines
**
*****************************************************************************/

typedef struct btk_slist_item { /* Depends on implementation structure used.  */
    struct btk_slist_item  *next_p;     /* Next (forward) pointer             */
    struct btk_slist_item  *prev_p;     /* Previous (backward) pointer        */
    btk_list_type_t         slist_type; /* [non-]interrupt / [non-]swappable. */
    void                   *data_p;     /* Pointer to the item's data section */
    size_t                  data_size;  /* Size in bytes of the data section. */
} btk_slist_item_t;

typedef struct btk_slist {
    struct btk_slist_item  *root_p;     /* Pointer to the root (head) of slist*/
    btk_list_type_t         slist_type; /* [non-]interrupt / [non-]swappable. */
    unsigned long           count;      /* Current count of items in the slist*/
    int                   (*slist_compare_fn_p)(
                     void * compare, void *match);  /* Compare/find function. */
    int                     free_flag;  /* != 0 if init() allocated this 
                                           structure */
} btk_slist_t;


/*****************************************************************************
**
**  Data structures for bt_llist.c routines
**
*****************************************************************************/

typedef struct btk_llist_elem_s 
{
    struct btk_llist_elem_s * next_p;
    struct btk_llist_elem_s * prev_p;
    struct btk_llist_s * parent_p;
    void * data_p;
} btk_llist_elem_t;

typedef struct btk_llist_s 
{
    btk_llist_elem_t * first_p;
    btk_llist_elem_t * last_p;
} btk_llist_t;

/*
**      Now include the specific driver's definitions
*/

#if     defined(__sun)

/*****************************************************************************
**
**  Data structures: Solaris section
**
*****************************************************************************/

#include <sys/ksynch.h>

/* Mutex routines */

typedef kmutex_t bt_mutex_t;

/* Event routines */

#if     !defined(bool_t)
#define bool_t  int
#endif  /* !defined(bool_t) */

typedef struct {
    kmutex_t mutex;
    kcondvar_t condvar;
    volatile bool_t binary_sema;
} bt_event_t;

typedef krwlock_t bt_rwlock_t;

typedef clock_t bt_tck_t;       /* Type for driver internal time */

#if defined(SUNOS5_5) || defined (SUNOS5_6)
typedef int btk_timeout_t;
#else /* defined(SUNOS5_5) || defined (SUNOS5_6) */
typedef timeout_id_t btk_timeout_t;
#endif /* defined(SUNOS5_5) || defined (SUNOS5_6) */


#if     defined(BT18sol)

#include "btqdd.h"

#elif   defined(BT945)

#include "btpdd.h"

#elif   defined(BT946)

#include "btpdd.h"

#elif   defined(BT15906)

#include "btmdd.h"

#else

#error  "Didn't define which product."

#endif  /* BT18sol, BT945 */

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_usectohz(time * 1000));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    return (BT_FOREVER == time) ? (BT_FOREVER) :
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_hztousec(time) / 1000);
}

static bt_tck_t btk_tck_get(void)
{
    FUNCTION("btk_tck_get");
    LOG_UNKNOWN_UNIT;

    unsigned long current_time = 0;
    
    if (0 != drv_getparm(LBOLT, &current_time)) {
        current_time = 0;
    }
    return (bt_tck_t) current_time;
}



#elif   defined(__sgi)
/*****************************************************************************
**
**  Data structures: SGI section
**
*****************************************************************************/

#include <sys/types.h>
#include <sys/ksynch.h>
#include <sys/ddi.h>

/* Process identification routines */
typedef u_long bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC (-1)
#define	BTK_GET_PROC(lval) { if (drv_getparm(PPID, &lval)) lval = -1; }
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

/* Mutex routines */
typedef mutex_t bt_mutex_t;

/* Event semaphores */
typedef sema_t bt_event_t;

/* Type for driver internal time */
typedef int bt_tck_t;

/* Type for timeout id, return from btk_timeout */
typedef toid_t btk_timeout_t;

/* Reader/writer lock routines */
typedef rwlock_t  bt_rwlock_t;

#if     defined(BT15904)
#include "btmdd.h"
#elif defined(BT965)
#include "btpdd.h"
#else
#error  "Didn't define which product."
#endif  /* BT15904 */

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert microsecods to clock ticks */
    /* time = number of microseconds to convert to clock ticks */
    /* No error value is returned.  If the number of ticks equivalent */
    /* to the number of microseconds is too large to represent, then */
    /* the max value is returned. */
    return (bt_tck_t) ((BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_usectohz((clock_t) time * 1000)));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to microseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the microsecond time is too large to   */
    /* bt represented, then the max clock_t value is returned. */
    return (bt_msec_t) ((BT_FOREVER == time) ? (BT_FOREVER) :
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : drv_hztousec((clock_t) time) / 1000));
}

static bt_tck_t btk_tck_get(void)
{
    FUNCTION("btk_tck_get");
    LOG_UNKNOWN_UNIT;

    unsigned long current_ticks = 0;
    int ret_val;

    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  
    ret_val = drv_getparm(LBOLT, &current_ticks);

    /* if couldn't read ticks return zero to caller */
    if (ret_val == -1) {
        current_ticks = 0;
    }
    
    return((bt_tck_t)current_ticks);
}



#elif   defined(__vxworks)

/*****************************************************************************
**
**  Data structures: VxWorks section
**
*****************************************************************************/


/* Process identification routines */
typedef int bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC (-1)
#define	BTK_GET_PROC(lval) { (lval) = taskIdSelf();}
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

/* Mutex routines */

typedef SEM_ID  bt_mutex_t;     /* Mutual Exclusion semaphore */

/* Event routines */

typedef SEM_ID  bt_event_t;     /* Binary Semaphore */

/* Type for timeout id, return from btk_timeout */
typedef WDOG_ID btk_timeout_t;

typedef int bt_tck_t; /* Type for driver internal time */

/* Reader/writer lock routines */

typedef struct
{
    SEM_ID spin_lock;   /* protects ref_count */
    SEM_ID fast_mutex;
    SEM_ID event;
    long int ref_count;     /* number of readers holding the lock */
} bt_rwlock_t;

#if     defined(BT18901)

#include "btvdd.h"

#elif   defined(BT993)

#include "btpdd.h"

#else   /* !defined(BT18901) || !defined(BT993) */

#error  "Didn't define which product."

#endif  /* defined(BT18901), defined(BT993) */

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/


static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    return (BT_FOREVER == time) ? (WAIT_FOREVER) : 
        ((BT_NO_WAIT == time) ? NO_WAIT : time * sysClkRateGet() / 1000);
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    return (WAIT_FOREVER == time) ? (BT_FOREVER) :
        ((NO_WAIT == time) ? BT_NO_WAIT : time * 1000 / sysClkRateGet() );
}

static bt_tck_t btk_tck_get(void)
{
    return (bt_tck_t) tickGet();
}



#elif   defined(BT_NTDRIVER)
/*****************************************************************************
**
**  Data structures: NT section
**
*****************************************************************************/

/* Mutex routines */
typedef enum {
    BT_SPIN_LOCK,
    BT_FAST_MUTEX
} bt_mutex_type_t;

typedef struct {
    bt_mutex_type_t mutex_type;
    union {
        KSPIN_LOCK spin_lock;
        FAST_MUTEX fast_mutex;
        KMUTEX mutex;
    } mutex_obj;
    KIRQL old_irql;     /* storage for spinlock's old IRQL */
} bt_mutex_t;

/* Event routines */
typedef KEVENT bt_event_t;

/* Driver internal time in 100nS units. */
typedef LONGLONG bt_tck_t;

/* Type for timeout id, return from btk_timeout */
typedef PKTIMER btk_timeout_t;

/* Reader/writer lock routines */
typedef struct
{
    KSPIN_LOCK spin_lock;   /* protects ref_count */
    FAST_MUTEX fast_mutex;
    KEVENT event;
    long int ref_count;     /* number of readers holding the lock */
} bt_rwlock_t;

#if     defined(BT15901)

#include "btmdd.h"

#elif   defined(BT13908)

#include "btfdd.h"

#elif   defined(BT13908)

#include "btfdd.h"

#elif   defined(BT_NTDRIVER)

#include "btwdd.h"

#else   /* BT15901 || BT13908 || BT_NTDRIVER */

#error  "Didn't define which product."

#endif  /* BT15901 || BT13908 || BT_NTDRIVER */

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

#if defined(BT13908)
static bt_tck_t btk_msec2tck(bt_msec_t time);
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_msec2tck)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    return (BT_FOREVER == time) ? ((bt_tck_t) MAXLONGLONG) : 
        ((BT_NO_WAIT == time) ? ((bt_tck_t) 0) : (((bt_tck_t) time) * 10000));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    return (bt_msec_t)((MAXLONGLONG == time) ? (BT_FOREVER) :
        ((0 == time) ? BT_NO_WAIT : time / 10000));
}

static bt_tck_t btk_tck_get(void)
{
    LARGE_INTEGER current_time;

    KeQuerySystemTime(&current_time);
    return current_time.QuadPart;
}




#elif   defined(BT_uCOS)
/*****************************************************************************
**
**  Data structures: uC OS section
**
*****************************************************************************/

/* Mutex routines */
typedef OS_EVENT * bt_mutex_t;

/* Event semaphores */
typedef OS_EVENT * bt_event_t;

typedef ULONG bt_tck_t; /* Type for driver internal time */

typedef void bt_rwlock_t;

#if     defined(BT2345)

#include "btfdd.h"

#else

#error  "Didn't define which product."

#endif  /* BT2345 */

/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter three times.
*/

static bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert microsecods to clock ticks */
    /* time = number of microseconds to convert to clock ticks */

    /* No error value is returned.  */
    /* Assume TICK_HZ <= 1000, so no chance of overflow */

    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ((BT_NO_WAIT == time) ? BT_NO_WAIT : (time / (1000/ TICK_HZ) ));
}

static bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to microseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the microsecond time is too large to   */
    /* be represented, then the max clock_t value is returned. */

    bt_msec_t msec;

    if ((BT_FOREVER == time) || (BT_NO_WAIT == time)) {
        msec = time;
    } else if (time < (BT_D32_MASK-1) / (1000 / TICK_HZ)) {
        /* If no overflow.  Above check assumes that TICK_HZ <= 1000 */
        msec = time * 1000 / TICK_HZ  ;

    } else {
        /* If it would overflow */
        msec = BT_D32_MASK - 1;
    } 

    return (msec);
}

static bt_tck_t btk_tck_get(void)
{
    ULONG	current_ticks;

    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  
    current_ticks = OSTimeGet();

    return((bt_tck_t)current_ticks);
}



/*****************************************************************************
**
**  Data structures: Linux section
**
*****************************************************************************/

#elif   defined(__linux__)

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/spinlock.h>
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
#include "asm/spinlock.h"
#endif /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
           LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

typedef pid_t bt_proc_t;	/* Process Identifier */
#define	BTK_NULL_PROC ((pid_t) -1)

#define	BTK_GET_PROC(lval) { (lval) = current->pid; }
#define	BTK_REL_PROC(lval) { ; }	/* Needed by other operating systems */
#define BTK_PROC_FMT "%d"

typedef int bt_cookie_t;	/* == 0 if not used from IRQ level */

/* Mutex routines */

typedef struct {
    int		irq_excl;	/* == 0 if not used from IRQ level */
    spinlock_t  lock;
    unsigned long flags;
} bt_mutex_t;

/* Event routines */

typedef struct {
    int			irq_excl;	/* == 0 if not used from IRQ */
    struct semaphore 	sem;		/* Semaphore */
    unsigned int	timed_out;	/* Flag: Have we timed out */

    struct timer_list 	timer;		/* Used for timeout function */
} bt_event_t;

typedef unsigned long bt_tck_t; /* Type for driver internal time */

typedef int btk_timeout_t;              /* Dummy value just used to compile. */

/* 
** Reader/writer lock routines 
**
** Not properly implemented yet, need to look into support
** for read/write locks in Linux kernel patches.  Current code
** only does a simple lock that is not callable from interrupt
** context.
*/
typedef struct {
    spinlock_t  rw_lock;
    int         lock;
} bt_rwlock_t;

#if defined(BT1003) /* Select product model */

#include "btpdd.h"

#else /* Select product model */

#error  "Didn't define which product."

#endif /* Select product model */


/*
**      Time conversion routines
**
** Rather than using macros, I've used static functions for time conversion.
** If I used macros, I would have to evaluate the parameter multiple times.
*/

static __inline__ bt_tck_t btk_msec2tck(bt_msec_t time) 
{
    /* convert millisecods to clock ticks */
    /* time = number of milliseconds to convert to clock ticks */

    /* No error value is returned.  */

    return (BT_FOREVER == time) ? (BT_FOREVER) : 
        ( (BT_NO_WAIT == time) ? BT_NO_WAIT : 
	  (time > ULONG_MAX/1000 -1) ? ULONG_MAX-1 : (time * HZ / 1000) );
}

static __inline__ bt_msec_t btk_tck2msec(bt_tck_t time)
{
    /* convert clock ticks to milliseconds */
    /* time = the number of clock ticks to convert to equivalen microseconds */
    /* No error value is returned. If the millisecond time is too large to   */
    /* be represented, then the max value is returned. */

    bt_msec_t msec;

    if ((BT_FOREVER == time) || (BT_NO_WAIT == time)) {
        msec = time;
    } else if (time < (ULONG_MAX-1) / (1000 / HZ)) {
        msec = time * 1000 / HZ  ;

    } else {
        /* If it would overflow */
        msec = ULONG_MAX - 1;
    } 

    return (msec);
}

static __inline__ bt_tck_t btk_tck_get(void)
{
    /* Read the number of clock ticks since the last system reboot. */
    /* The difference between the values returned from successive */
    /* calls to this function is the elapsed time between the calls */
    /* in system clock ticks. */  

    return((bt_tck_t) jiffies);
}


/*****************************************************************************
**
**  Data structures: ALL other operating systems
**
*****************************************************************************/

#else   /* Haven't written them yet */

#error  Code not written yet

#endif  /* Define data structures for each operating system */




/*****************************************************************************
**
**      Prototypes and definitions for bt_mem.c 
**
*****************************************************************************/

/* Global used to track memory usage */

extern volatile int btk_alloc_total_g;

/* Prototypes for routines in bt_mem.c file */

int btk_mem_init(void);
void btk_mem_fini(void);

void *btk_mem_alloc(size_t size, bt_data32_t flags);
void btk_mem_free(void * kmem_p, size_t size);

/* Constants for btk_alloc() flags parameter */

#define BTK_ALLOC_SWAPPABLE     (1u<<0) /* Allow memory to be swapped out */


/*****************************************************************************
**
**      Prototypes and definitions for bt_bit.c 
**
*****************************************************************************/

/* Prototypes for routines in bt_bit.c file */

bt_error_t btk_bit_init(bt_unit_t *unit_p, unsigned size, void **bitmap_p);
bt_error_t btk_bit_reset(bt_unit_t *unit_p, void *bitmap_p);
bt_error_t btk_bit_fini(bt_unit_t *unit_p, void *bitmap_p);
bt_error_t btk_map_half(bt_unit_t *unit_p, void *bitmap_p);
bt_error_t btk_map_restore(bt_unit_t *unit_p, void *bitmap_p);
bt_error_t btk_bit_free(bt_unit_t *unit_p, void *bitmap_p, unsigned start, 
    unsigned num_entries);
bt_error_t btk_bit_alloc(bt_unit_t *unit_p, void *bitmap_p, unsigned needed,
    unsigned align, unsigned *first_p);
bt_error_t btk_bit_specify(bt_unit_t *unit_p, void *bitmap_p, unsigned start, unsigned needed);
int btk_bit_set(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum);
int btk_bit_clr(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum);
int btk_bit_chk(bt_unit_t * unit_p, void * bitmap_p, unsigned bitnum);
bt_error_t btk_bit_max(bt_unit_t *unit_p, void * bit_map_p, unsigned align, unsigned *size);

/*****************************************************************************
**
**      Prototypes and definitions for bt_mutex.c 
**
*****************************************************************************/

/* Prototype for btk_mutex_init() includes OS dependent parameters */
#if defined (_AIX)

#elif defined (BT_NTDRIVER)
int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p, bt_mutex_type_t mutex_type);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p, char *name_p);

#elif defined (__sun)
int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p, char *name_p, ddi_iblock_cookie_t *cookie_p); 

#elif defined (__vxworks)
int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

#elif defined (BT_uCOS)
int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

#elif defined (__linux__)

int btk_mutex_init(bt_unit_t *unit_p, bt_mutex_t *mutex_p, bt_cookie_t irq_cookie);

#else
#error unknown operating system
#endif /* operating system */

void btk_mutex_fini(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

void btk_mutex_enter(bt_unit_t *unit_p, bt_mutex_t *mutex_p);

void btk_mutex_exit(bt_unit_t *unit_p, bt_mutex_t *mutex_p);


/*****************************************************************************
**
**      Prototypes and definitions for bt_event.c 
**
*****************************************************************************/

bt_tck_t btk_msec2tck(bt_msec_t time);
bt_msec_t btk_tck2msec(bt_tck_t time);

/* Prototype for btk_event_init() includes OS dependent parameters */
#if defined (_AIX)

#elif defined (BT_NTDRIVER)
int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state);

#elif defined (__sun)
int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state, char *name_p, ddi_iblock_cookie_t *cookie_p); 

#elif defined (__vxworks)
int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state);

#elif defined (BT_uCOS)
int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state);

#elif defined (__linux__)

int btk_event_init(bt_unit_t *unit_p, bt_event_t *event_p, bool_t state, bt_cookie_t irq_cookie);

#else
#error unknown operating system
#endif /* operating system */

void btk_event_fini(bt_unit_t *unit_p, bt_event_t *event_p);
bt_error_t btk_event_wait(bt_unit_t *unit_p, bt_event_t *event_p, bt_tck_t 
timeout);
void btk_event_set(bt_unit_t *unit_p, bt_event_t *event_p);


/*****************************************************************************
**
**      Prototypes and definitions for bt_rwlck.c
**
*****************************************************************************/

/* Prototype for btk_rwlock_init() includes OS dependent parameters */
#if defined (_AIX)

#elif defined (BT_NTDRIVER)

int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p, char *name_p);

#elif defined (__sun)

#elif defined (__vxworks)
int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);

#elif defined (BT_uCOS)
int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
   
#elif defined (__linux)

int btk_rwlock_init(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);

#else
#error unknown operating system
#endif /* operating system */


void btk_rwlock_fini(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_wr_enter(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_rd_enter(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_wr_exit(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);
void btk_rwlock_rd_exit(bt_unit_t *unit_p, bt_rwlock_t *rwlock_p);

/* Macro to get the reader/writer lock reference count.  This is the number
   of readers currently holding the lock.  As written this assumes that all
   implementations of bt_rwlock_t have a ref_count member. */
#define BTK_RWLOCK_REF_COUNT(rwlock_p) (rwlock_p)->ref_count


/*****************************************************************************
**
**      Prototypes and definitions for bt_queue.c
**
*****************************************************************************/


btk_queue_t * btk_queue_init( bt_unit_t *unit_p, btk_queue_t **queue_p, btk_list_type_t queue_type);

bt_error_t btk_queue_fini( bt_unit_t *unit_p, btk_queue_t *queue_p);

btk_queue_item_t * btk_queue_item_create( bt_unit_t *unit_p, btk_queue_item_t **queue_item_p, btk_list_type_t queue_type, size_t data_size);

bt_error_t btk_queue_item_destroy( bt_unit_t *unit_p, btk_queue_item_t *queue_item_p);

bt_error_t btk_queue_push( bt_unit_t *unit_p, btk_queue_t *queue_p, btk_queue_item_t *queue_item_p);

btk_queue_item_t * btk_queue_pull( bt_unit_t *unit_p, btk_queue_t *queue_p, btk_queue_item_t **queue_item_p);

unsigned long btk_queue_count( bt_unit_t *unit_p, btk_queue_t *queue_p, unsigned long *count_p);



/*****************************************************************************
**
**      Prototypes and definitions for bt_slist.c
**
*****************************************************************************/

btk_slist_t * btk_slist_init( bt_unit_t *unit_p, btk_slist_t *slist_p, btk_list_type_t slist_type, int (*slist_compare_fn_p)( void *value_p, void  *match_p));

bt_error_t btk_slist_fini( bt_unit_t *unit_p, btk_slist_t *slist_p);

btk_slist_item_t * btk_slist_item_create( bt_unit_t *unit_p, btk_list_type_t slist_type, size_t data_size);

bt_error_t btk_slist_item_destroy( bt_unit_t *unit_p, btk_slist_item_t *slist_p);

bt_error_t btk_slist_insert( bt_unit_t *unit_p, btk_slist_t *slist_p, btk_slist_item_t *slist_item_p);

btk_slist_item_t * btk_slist_find( bt_unit_t *unit_p, btk_slist_t *slist_p, btk_slist_item_t *slist_item_p, void *data_p);

btk_slist_item_t * btk_slist_remove( bt_unit_t *unit_p, btk_slist_t *slist_p, btk_slist_item_t *slist_item_p);

unsigned long btk_slist_count( bt_unit_t *unit_p, btk_slist_t *slist_p, unsigned long *count_p);

btk_slist_item_t * btk_slist_step( bt_unit_t *unit_p, btk_slist_t *slist_p, btk_slist_item_t *slist_item_p);

/*****************************************************************************
**
**      Prototypes and definitions for bt_llist.c
**
*****************************************************************************/

extern void btk_llist_init(btk_llist_t * list_p);
extern void btk_llist_elem_init(btk_llist_elem_t * new_p, void * data_p);
extern void * btk_llist_elem_data(btk_llist_elem_t * elem_p);
extern btk_llist_elem_t * btk_llist_elem_create(size_t data_size, bt_data32_t alloc_flags);
extern void btk_llist_elem_destroy(btk_llist_elem_t * elem_p, size_t data_size);
extern void btk_llist_remove(btk_llist_elem_t * elem_p);
extern btk_llist_elem_t * btk_llist_next(btk_llist_elem_t * curr_p);
extern btk_llist_elem_t * btk_llist_prev(btk_llist_elem_t * curr_p);
extern btk_llist_elem_t * btk_llist_first(btk_llist_t * list_p);
extern btk_llist_elem_t * btk_llist_last(btk_llist_t * list_p);
extern btk_llist_elem_t * btk_llist_find_first(btk_llist_t * list_p, int(*func_p)(void *, void *), void * second_arg);
extern btk_llist_elem_t * btk_llist_find_last(btk_llist_t * list_p, int(*func_p)(void *, void *), void * second_arg);
extern btk_llist_elem_t * btk_llist_find_next(btk_llist_elem_t * curr_p, int(*func_p)(void *, void *), void * second_arg);
extern btk_llist_elem_t * btk_llist_find_prev(btk_llist_elem_t * curr_p, int(*func_p)(void *, void *), void * second_arg);
extern void btk_llist_insert_first(btk_llist_t * list_p, btk_llist_elem_t * new_p);
extern void btk_llist_insert_last(btk_llist_t * list_p, btk_llist_elem_t * new_p);
extern void btk_llist_insert_after(btk_llist_elem_t * old_p, btk_llist_elem_t * new_p);
extern void btk_llist_insert_before(btk_llist_elem_t * old_p, btk_llist_elem_t * new_p);
extern void * btk_llist_first_data(btk_llist_t * list_p);
extern void * btk_llist_prev_data(btk_llist_elem_t * curr_p);
extern void * btk_llist_next_data(btk_llist_elem_t * curr_p);
extern void * btk_llist_last_data(btk_llist_t * list_p);
extern void * btk_llist_find_first_data(btk_llist_t * list_p, int(*func_p)(void *, void *), void * second_arg);
extern void * btk_llist_find_last_data(btk_llist_t * list_p, int(*func_p)(void *, void *), void * second_arg);
extern void * btk_llist_find_next_data(btk_llist_elem_t * curr_p, int(*func_p)(void *, void *), void * second_arg);
extern void * btk_llist_find_prev_data(btk_llist_elem_t * curr_p, int(*func_p)(void *, void *), void * second_arg);
extern void btk_llist_onall(btk_llist_t * list_p, void(*func_p)(void *, void *), void * second_arg);
extern int btk_llist_count(btk_llist_t * list_p);

/*****************************************************************************
**
**      Prototypes and definitions for bt_crc16.c 
**
*****************************************************************************/
#if defined(NO_CRC_TABLE)

extern bt_data16_t bt_crc16_calc(bt_data16_t crc, bt_data8_t val);
#define BT_CRC16(crc, val) (bt_crc16_calc((crc), (val)))

#else /* NO_CRC_TABLE */

extern bt_data16_t bt_crc16_table[0x100];
#define BT_CRC16(crc, val) ((bt_data16_t) ((((crc) << 8) ^ bt_crc16_table[(((crc) >> 8) ^ (val)) & 0xFFu]) & 0xFFFFu))

#endif /* NO_CRC_TABLE */

extern bt_data16_t bt_crc16_buffer(bt_data16_t crc, void * buf_p, size_t len);
extern bt_data16_t bt_crc16_buffer_swap(bt_data16_t crc, void * buf_p, size_t len, size_t siz);

/*****************************************************************************
**
**      Prototypes and definitions for bt_fifo.c 
**
*****************************************************************************/

typedef struct bt_fifo_s {
    char * buf_p;                /* internal buffer */
    size_t elem_size;            /* size of elements being handled */
    unsigned long num_elems;     /* number of elements the internal buffer 
                                    can hold */
    volatile unsigned long tail; /* Next location to insert at */
} bt_fifo_t;

typedef unsigned long bt_fifo_head_t;  /* Next location to remove */

extern void btk_fifo_init(bt_fifo_t * fifo_p, size_t elem_size, unsigned long num_elems, void * buf_p);
extern void btk_fifo_head_init(bt_fifo_t * fifo_p, bt_fifo_head_t * head_p);
extern void btk_fifo_fini(bt_fifo_t * fifo_p);
extern void btk_fifo_head_fini(bt_fifo_head_t * head_p);
extern void btk_fifo_insert(bt_fifo_t * fifo_p, void * elem_p);
extern int btk_fifo_remove(bt_fifo_t * fifo_p, bt_fifo_head_t * head_p, void * elem_p);
extern unsigned long btk_fifo_length(bt_fifo_t * fifo_p, bt_fifo_head_t * head_p);
extern int btk_fifo_is_overflowing(bt_fifo_t * fifo_p, bt_fifo_head_t * head_p);
extern int btk_fifo_is_empty(bt_fifo_t * fifo_p, bt_fifo_head_t * head_p);


/*****************************************************************************
**
**      Prototypes and definitions for bt_delay.c 
**
*****************************************************************************/

extern void btk_delay(int usec);
extern bt_error_t btk_sleep(int usec);
extern btk_timeout_t btk_timeout(void (*f_time_expire)(caddr_t), caddr_t arg_p, long usec);
extern bt_error_t btk_untimeout(void (*f_time_expire)(caddr_t), caddr_t arg_p, btk_timeout_t timeout_id);

#endif /* !defined(_BTDD_H) */
