/******************************************************************************
**
**  Filename:   bt_bit.c
**
**  Purpose:    Kernel bit map manipulation routines. Used to track allocation
**              and deallocation bit maps.
**
**  Functions:  btk_bit_init(), btk_bit_reset(), btk_bit_fini(),
**              btk_bit_alloc(), btk_bit_free(), btk_bit_set(), 
**              btk_bit_clr(), btk_bit_chk(), btk_bit_max(),
**              btk_map_half(), btk_map_retore()
**
**  Calls:      btk_mem_alloc(), btk_mem_free()
**
**      $Revision: 2330 $
**
******************************************************************************/
/******************************************************************************
**
**     Copyright (c) 1997-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
******************************************************************************/

#if !defined(LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif /* !LINT */


/*
**  Include files
*/
#include "btdd.h"

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

/*
** Internal structure used for tracking bit map
**
** Bits are numbered 0 (on the farthest left) to BPW (on the farthest
** right) so as to look natural in a hex dump.
**
** 1 -> allocated, 0 -> free
*/

typedef struct {
    /*
    ** There are two sizes here because for PCI to PCI only 16 Mbytes of
    ** mapping register are useable.  However, for PCI to VMEbus 32 Mbytes
    ** are useable.  Since this dynamic we use a half function to reduce
    ** it but still need to free the alloc_size
    */
    unsigned int   size;
    unsigned int   alloc_size;
    unsigned int   first_avail;

    /*
    ** Need this word here to protect the backtracking code. 
    ** It is always marked as 'allocated' so the backtracking will
    ** stop without needing an additional bounds check.
    */
    WORD_T      reserved;

    /*
    ** The bit array is really dynamically sized. It is done this way
    ** so that we can do the btk_mem_alloc() and have a field in the struct
    ** for the bitmap.
    */
    WORD_T      bits[2];

    /* Allocates at most two extra words.
    ** The structure includes two words, which takes care of the case when
    ** the truncation from division would spill some into next word. The
    ** other simplifies the search loop so we don't have to check for end
    ** of array.
    */

#define BT_BITMAP_SIZE(x)  ((x)*sizeof(WORD_T)/BPW + sizeof(bit_tracker_t))

} bit_tracker_t;

BT_FILE_NUMBER(TRACE_BT_BIT_C);

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


/******************************************************************************
**
**              Local routines
**
*****************************************************************************/

/******************************************************************************
**
**      Function:   find_first_one()
**
**      Purpose:    Finds the first one in a word.
**
**      Args:
**          word        Word to search for a one bit.
**
**      Returns:
**          Bit number (0-BPW) of first zero bit. Bit 0 is the left most
**          bit in this case.
**
******************************************************************************/

static unsigned int find_first_one(
    WORD_T word
    )
{
    WORD_T          mask;
    unsigned int    bit_num;
    unsigned int    shift;

    if (0 == word) {
        return BPW;
    }

    bit_num = 0;
    shift = BPW/2;                              /* Shift half the bits */
    mask = WBIT_MASK << shift;     /* Lower half of bits are set */

    do {
        if (word & mask) {
           /* First one is in this section, so clear the other bits */
           word &= mask;
        } else {
           /* First one is in upper section */
           bit_num += shift;
        }
        shift >>= 1;
        mask ^= mask >> shift;
    } while (shift != 0);

    return bit_num;
}

/******************************************************************************
**
**      Function:   find_first_zero()
**
**      Purpose:    Finds the first zero in a word.
**
**      Args:
**          word        Word to search for a zero bit.
**
**      Returns:
**          Bit number (0-BPW) of first one bit. Bit 0 is the left most
**          bit in this case.
**
******************************************************************************/
static unsigned int find_first_zero(
    WORD_T word
    )
{
    if (WBIT_MASK == word) {
        return BPW;
    }
    return find_first_one(~word);
}



/******************************************************************************
**
**              External routines
**
*****************************************************************************/

/******************************************************************************
**
**      Function:   btk_bit_init()
**
**      Purpose:    Creates the bit map structure for use.
**
**      Args:
**          size        Number of elements that must be tracked
**          bitmap_p    Pointer to pointer to hold bit map structure.
**
**      Returns:
**          BT_SUCCESS  Everything is just swell
**          BT_ENOMEM   Couldn't allocation enough memory for the bit map
**                      structure.
**
******************************************************************************/

bt_error_t btk_bit_init(
    unsigned int size,
    void **bitmap_p
    )
{
    bit_tracker_t *map_p;

    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */

    FUNCTION("btk_bit_init");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if (NULL == bitmap_p) {
        INFO_STR("Pointer to pointer to bit map structure is NULL.");
        retval = BT_EINVAL;
        goto btk_bit_init_end;
    }

    map_p = btk_mem_alloc(BT_BITMAP_SIZE(size), 0);
    if (NULL == map_p) {
        INFO_STR("Could not allocate space for allocation structure.");
        retval = BT_ENOMEM;
        goto btk_bit_init_end;
    }

    BTK_BZERO((void *) map_p, BT_BITMAP_SIZE(size));
    map_p->alloc_size = size;
    map_p->size = size;
    map_p->first_avail = 0;
    map_p->reserved = WBIT_MASK;

    *bitmap_p = (void *) map_p;

btk_bit_init_end:
    FEXIT(retval);
    return retval;
}


/******************************************************************************
**
**      Function:   btk_bit_reset()
**
**      Purpose:    Resets a bit map structure to intial state
**
**      Args:
**          bitmap_p    Pointer to hold bit map structure.
**
**      Returns:
**          BT_SUCCESS  Everything is just swell
**
******************************************************************************/

bt_error_t btk_bit_reset(
    void *bitmap_p
    )
{
    unsigned int    size;

    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;

    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */

    FUNCTION("btk_bit_reset");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if (NULL == map_p) {
        INFO_STR("Pointer to bit map structure is NULL.");
        retval = BT_EINVAL;
        goto btk_bit_reset_end;
    }

    size = map_p->alloc_size;

    BTK_BZERO((void *) map_p, BT_BITMAP_SIZE(size));
    map_p->size = size;
    map_p->alloc_size = size;
    map_p->first_avail = 0;
    map_p->reserved = WBIT_MASK;

btk_bit_reset_end:
    FEXIT(retval);
    return retval;
}

/******************************************************************************
**
**      Function:   btk_bit_fini()
**
**      Purpose:    Destroys the bit map structure.
**
**      Args:
**          map_p       Pointer to hold bit map structure.
**
**      Returns:
**          BT_SUCCESS  Everything is just swell
**
******************************************************************************/

bt_error_t btk_bit_fini(
    void *bitmap_p
    )
{
    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */
    bit_tracker_t *map_p = bitmap_p;
    size_t      free_size;

    FUNCTION("btk_bit_fini");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if (NULL == bitmap_p) {
        INFO_STR("Pointer to bit map structure is NULL.");
        retval = BT_EINVAL;
        goto btk_bit_fini_end;
    }

    free_size = BT_BITMAP_SIZE(map_p->alloc_size);

    btk_mem_free(bitmap_p, free_size);

btk_bit_fini_end:
    FEXIT(retval);
    return retval;
}

/******************************************************************************
**
**      Function:   btk_map_half()
**
**      Purpose:    Reduces the bit map in half.
**
**      Args:
**          bitmap_p    Pointer to the bit map structure.
**
**      Returns:
**          BT_SUCCESS  Everything is just swell
**
******************************************************************************/

bt_error_t btk_map_half(
    void *bitmap_p
    )
{
    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */
    bit_tracker_t *map_p = bitmap_p;

    FUNCTION("btk_map_half");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    map_p->size = map_p->alloc_size / 2;

    FEXIT(retval);
    return retval;
}

/******************************************************************************
**
**      Function:   btk_map_restore()
**
**      Purpose:    Restores the bit map to its original size.
**
**      Args:
**          bitmap_p    Pointer to the bit map structure.
**
**      Returns:
**          BT_SUCCESS  Everything is just swell
**
******************************************************************************/

bt_error_t btk_map_restore(
    void *bitmap_p
    )
{
    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */
    bit_tracker_t *map_p = bitmap_p;

    FUNCTION("btk_map_restore");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    map_p->size = map_p->alloc_size;

    FEXIT(retval);
    return retval;
}

/******************************************************************************
**
**  Function:   btk_bit_free()
**
**  Purpose:    Frees the specified bit maps.
**
**  Args:       
**              bitmap_p    Pointer to bit map tracking structure
**              start       First in block to release
**              num_entries Number to release
**
**  Returns:    NA
**
**  Notes:      Should only be called with the unit_p locked to 
**              serialize access to the shared mmap_aval_p array.
**
******************************************************************************/

bt_error_t btk_bit_free(
    void *bitmap_p,
    unsigned int start, 
    unsigned int num_entries)
{
    bt_error_t  retval = BT_SUCCESS;    /* Assume everything works */

    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;

    unsigned int    bit_num;
    int             index;

    WORD_T      mask;

    FUNCTION("btk_bit_free");
    LOG_UNKNOWN_UNIT;
    FENTRY;
  
    TRC_MSG(BT_TRC_ALLOC, (LOG_FMT "start = %d, num_entries = %d\n",
          LOG_ARG, start, num_entries));

    /* Figure out starting index into bit map array and which bit number */
    index = start / BPW;
    bit_num = start % BPW;


    /* If will have to cross a word boundary */
    if (num_entries + bit_num >= BPW) {
        if (bit_num != 0) {     /* Clear all bits to end of word */
            mask = WBIT_MASK << (BPW - bit_num);
            num_entries -= BPW - bit_num;
            map_p->bits[index] &= mask;
            /* Position is now the start of next word */
            index++; bit_num = 0;
        }

        /* Clear whole words now */
        while (num_entries >= BPW) {
            map_p->bits[index++] = 0;
            num_entries -= BPW;
        }
    }

    /* Have less than one word left now */
    if (num_entries != 0) {
        mask = WBIT_MASK >> num_entries; /* Create 'num_entries' zero bits */
        mask = RIGHT_ROTATE(mask, bit_num);     /* Move them to correct bit */
        map_p->bits[index] &= mask;
        num_entries = 0;
    }

    /* If this is earlier than previous allocations, update start */
    if (start < map_p->first_avail) {
        map_p->first_avail = start;
    }

    FEXIT(retval);
    return retval;
}



/******************************************************************************
**
**      Function:   btk_bit_alloc()
**
**      Purpose:    Locate a section of consecutive allocation bits
**              large enough to accomidate the request.
**
**      Args:
**          bitmap_p    Pointer to bit map tracking structure
**          needed      Number of bits needed
**          align       Bit boundary request must be aligned to
**          first_p     Pointer to location to store location of first bit
**
**
**      Modifies:
**          first_p     Return value; starting location of the block of
**                      requested allocation bits. Zero is the first.
**                      
**      Returns:
**          BT_SUCCESS  Everything is just swell
**          BT_ENOMEM   If not enough space open
**
**  Notes:      Should only be called with the unit_p locked to 
**              serialize access to the shared bitmap_p.
**
**              This routine always assumes that the searches should
**              start at first avail pointer and continue until the
**              end of the array is reached.  
**
******************************************************************************/

bt_error_t btk_bit_alloc(
    void *bitmap_p,
    unsigned int needed,
    unsigned int align,
    unsigned int *first_p
    )
{
    bt_error_t  retval = BT_SUCCESS;    /* Assume we can find enough */

    unsigned int    start;          /* Start of section to check */
    int             found;          /* Number of consecutive bits found */
    int             requested = (int) needed; /* Signed version of needed */

    unsigned int    adjust;         /* Used as temp to adjust found and bit_num */

    WORD_T          search_mask;    /* Current search mask */
    unsigned int    search_size;    /* How many bits we search for each time */
    unsigned int    shift;          /* How far do we shift each time we search */

    int             search_index;   /* Index into array for search */
    unsigned int    search_bit;     /* Which bit we are currently searching at */

    WORD_T          mask;           /* Temporary bit mask */
    int             index;          /* Index into search array */
    unsigned int    bit_num;        /* Which bit we are currently checking */

    WORD_T          word;           /* Temporary copy of current word in array */

    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;

    FUNCTION("btk_bit_alloc");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** When we want to find a large consecutive block of available bits,
    ** we can speed up the search by looking for a block of half that
    ** size, and then backtracking to the first available bit.
    **
    ** To simplify the code (so we don't have to split masks over two
    ** locations), we only deal with search masks that are a power of
    ** two.
    */

    BTK_ASSERT(align != 0);
    BTK_ASSERT(WBIT_MASK == map_p->reserved);
    BTK_ASSERT(align < map_p->size);

    if (1 == needed) {          /* If only looking for a single open area */
        search_size = 1u;
        search_mask = BIT_ZERO;
        if (1 == align) {
            shift = 1u;
        } else {
            shift = BPW * 2;
            while (shift > align) {
                shift /= 2;
            }
        }
    } else {                    /* Need a large block of bits */
        shift = BPW * 2;
        search_size = BPW;
        search_mask = WBIT_MASK;

        /* Find largest power-of-two search pattern we can use */
        while ((needed < shift) && (shift > align)) {
            shift /= 2;
            search_size /= 2;
            search_mask <<= search_size;
        }
    }

    if (shift > BPW) {  /* Limit the amount of backtracking required */
        shift = BPW;
    }

    /* Find first location with open bits */
    start = map_p->first_avail;
    search_index = start / BPW;
    search_bit = start % BPW;

    /* Make sure that this bit is available */
    if ((word = map_p->bits[search_index]) & (BIT_ZERO>>search_bit)) {
        /* Better start from the beginning. */
        search_index = 0;
        word = map_p->bits[search_index];

        /* Need to search for first available bit */
        while (WBIT_MASK == word) {
            word = map_p->bits[++search_index];
        }
        search_bit = find_first_zero(word);
        start = map_p->first_avail = search_index * BPW + search_bit;
    }

    /* Make starting location a multiple of our shift */
    if ( (shift > 1) && (search_bit % shift) ){
        search_bit += shift - search_bit % shift;
    }

    /* Back up one position, so start of each loop advances the search */
    if (search_bit < shift) {
        search_bit = BPW - shift;
        search_index--;
    } else {
        search_bit -= shift;
    }

    if (search_bit != 0) {
        search_mask = RIGHT_ROTATE(search_mask, search_bit);
    }

    found = 0;
    do {        /* Search for available bits */

        /* Loop always increments index at the start */

        if (search_size >= BPW) {
            /* Find first full word with no bits set */
            do {
                search_index++;
            } while (map_p->bits[search_index]);
            /* Array has one extra zero'ed word to terminate loop */

        } else {
            word = map_p->bits[search_index];
            do {
                search_bit += shift;
                search_mask = RIGHT_ROTATE(search_mask, shift);
                if (search_bit >= BPW) {
                    search_index++;
                    search_bit = 0;
                    word = map_p->bits[search_index];
                }
            } while (search_mask & word);
        }
        start = search_index * BPW + search_bit;
        found = search_size;

        if (shift > 1) {
            /* Now back track to first available bit */
            index = search_index;
            bit_num = search_bit;

            if (0 == search_bit) {
                /*
                ** Note: An index of -1 will work because it will access the
                ** reserved word. The researved word is marked with all bits
                ** allocated, terminating the search without additional logic.
                */
                bit_num = BPW;
                index--;
            }
            bit_num--;

            /* Don't ever have to backtrack across a word boundary */
            word = map_p->bits[index];
            /* While zeros preceeding */
            while ( !( word & (BIT_ZERO >> bit_num)) ) {
                found++;
                bit_num--;
                start--;
            }
        }

        /* Align starting position */
        if ((align > 1) && (0 != (start % align))) {
            /* Compute how far to next boundary */
            adjust = align - (start % align);

            found -= adjust;
            start += adjust;
        }

        /* Fix case where start is now outside search area */
        if (found < 0) {
            found = 0;
        }

        /* Check if we are done */
        if ( found >= requested ) {
            /* We have a winner! */
            break;
        }

        if (start + needed > map_p->size) {
            /* Not enough room left to fulfill the request */
            break;
        }

        /* Search forward for remaining bits */

        index = (start + found) / BPW;
        bit_num = (start + found) % BPW;
        word = map_p->bits[index];

        /* Check if all we need can be found within this word */
        if (bit_num + needed - found <= BPW) {
            mask = (WBIT_MASK << (BPW + found - needed)) >> bit_num;
            if (!(word & mask)) {
                /* We have a winner! */
                found = needed;
                break;
            }

            /* Need to continue searching, not enough bits available */
            continue;
        }
        
        /* Will need more after this word */
        mask = WBIT_MASK >> bit_num;

        if (word & mask) {
            /* Need to continue searching, not enough bits available */
            continue;
        }

        found += BPW - bit_num;
        index++;
        bit_num = 0;

        /*
        ** Either we will succeed in finding enough space, or we should
        ** restart the search one shift into the next word.
        */
        search_mask <<= search_bit;
        search_bit = 0;

        /* Check whole words now */
        while ((found + ((int) BPW) <= requested) && !(map_p->bits[index])) {
            found+= BPW;
            index++;
            /* Just in case we need to continue the search */
            search_index++;
        }

        /* Have enough where we don't need a full word any longer */
        word = map_p->bits[index];
        while ( (found < requested) && !(word & (BIT_ZERO >> bit_num)) ) {
            found++;
            bit_num++;
        }

    } while ((found < requested) && (search_index * BPW + shift < map_p->size));

    /* If went beyond the end of the bitmap */
    if (start + needed > map_p->size) {
        found = 0;
    }

    if (found < requested) {
        /* Not enough out there, sorry. */
        retval = BT_ENOMEM;

        /* Make sure no one mistakes this for a good return */
        *first_p = map_p->size + 1;
    } else {
        /* Tell them where to find the resources */

        *first_p = start;

        found = needed;         /* Might have found more than needed */

        /* Now start counting down the number found, marking them as in use */
        index = start / BPW;
        bit_num = start % BPW;

        /* If have multiple words to allocate */
        if (found + bit_num >= BPW) {
            mask = WBIT_MASK >> bit_num;
            map_p->bits[index] |= mask;
            found -= BPW - bit_num;

            /* Take care of big chunks in the middle */
            index++;
            bit_num = 0;
            while (found >= BPW) {
                map_p->bits[index++] = WBIT_MASK;
                found -= BPW;
            }
        }

        /* Take care of last word */
        if (1 == found) {
            /* Only have one bit to allocate */
            map_p->bits[index] |= BIT_ZERO >> bit_num;

            /* Don't bother updating found */

        } else if (found > 0) {
            /* Allocate remaining bits from this word */
            mask = (WBIT_MASK >> found) ^ WBIT_MASK;

            /* They may not be word aligned though */
            mask >>= bit_num;
            map_p->bits[index] |= mask;

            /* Don't bother updating found */
        }


        /* Update the first location to start looking next time */
        if (start == map_p->first_avail) {
            map_p->first_avail = start + needed;
        }
    }

    TRC_MSG(BT_TRC_ALLOC, 
        (LOG_FMT "Needed %d, Start %d, Found %d, Return %d.\n",
        LOG_ARG, needed, start, found, retval));

    FEXIT(retval);
    return(retval);
}


/******************************************************************************
**
**      Function:   btk_bit_specify()
**
**      Purpose:    Allocate the given region of the bit map or return error.
**
**      Args:
**          bitmap_p    Pointer to bit map tracking structure
**          start       Starting bit of region to allocate
**          needed      Number of bits needed
**
**      Modifies:
**          NA
**                      
**      Returns:
**          BT_SUCCESS  Everything is just swell
**          BT_ENOMEM   Region allready used
**
**  Notes:      Should only be called with the unit_p locked to 
**              serialize access to the shared bitmap_p.
**
******************************************************************************/

bt_error_t btk_bit_specify(
    void *bitmap_p,
    unsigned int start,
    unsigned int needed
    )
{
    bt_error_t      retval = BT_SUCCESS;    /* Assume we can find enough */

    int             search_index;   /* Index into array for search */
    unsigned int    search_bit;     /* Which bit we are currently searching at */
    unsigned int    bit_num;        /* Which bit we are currently checking */

    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;

    FUNCTION("btk_bit_specify");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    BTK_ASSERT(WBIT_MASK == map_p->reserved);

    /*
    ** Sequently look and make sure each bit is clear
    */
    for (bit_num = start; bit_num < (start + needed); bit_num++) {
        search_index = bit_num / BPW;
        search_bit = bit_num % BPW;
        if (IS_SET(map_p->bits[search_index], (BIT_ZERO>>search_bit))) {
            retval = BT_ENOMEM;
            break;
        }
    }


    /*
    ** If no error, set each bit to indicate it is allocated
    */
    if (retval == BT_SUCCESS) {
        for (bit_num = start; bit_num < (start + needed); bit_num++) {
            search_index = bit_num / BPW;
            search_bit = bit_num % BPW;
            SET_BIT(map_p->bits[search_index], (BIT_ZERO>>search_bit));
        }
    }

    TRC_MSG(BT_TRC_ALLOC, 
        (LOG_FMT "Needed %d, Start %d, Return %d.\n",
        LOG_ARG, needed, start, retval));

    FEXIT(retval);
    return(retval);
}

/****************************************************************************
 **
 **    Function:        btk_bit_set()
 **
 **    Purpose:         Force a specific bit to be set (1).
 **
 **    Args:
 **        bitmap_p
 **                     Bitmap pointer
 **        bitnum
 **                     Bit number to force to 1
 **
 **    Returns:         int
 **                         0 if the bit was previously unset (0)
 **                         1 if the bit was previously set (1)
 **                         A negative number on error
 **                         -1 if the bitnum is beyond the end of the bitmap
 **
 ***************************************************************************/
int btk_bit_set(
    void * bitmap_p,
    unsigned int bitnum)
{
    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;
    int           retval;
    WORD_T        mask;
    unsigned int  idx;

    FUNCTION("btk_bit_set");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if ((map_p == NULL) || (bitnum >= map_p->size)) {
        /* Off the end */
        retval = -1;
    } else {
        idx = (bitnum / BPW);
        mask = BIT_ZERO >> (bitnum % BPW);
        retval = ((map_p->bits[idx] & mask) != 0);
        map_p->bits[idx] |= mask;
    }

    FEXIT(retval);
    return retval;
}


/****************************************************************************
 **
 **    Function:        btk_bit_clr()
 **
 **    Purpose:         Force a specific bit to be clear (0).
 **
 **    Args:
 **        bitmap_p
 **                     Bitmap pointer
 **        bitnum
 **                     Bit number to force to 0
 **
 **    Returns:         int
 **                         0 if the bit was previously unset (0)
 **                         1 if the bit was previously set (1)
 **                         A negative number on error
 **                         -1 if the bitnum is beyond the end of the bitmap
 **
 ***************************************************************************/
int btk_bit_clr(
    void * bitmap_p,
    unsigned int bitnum)
{
    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;
    int           retval;
    WORD_T        mask;
    unsigned int  idx;

    FUNCTION("btk_bit_clr");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if ((map_p == NULL) || (bitnum >= map_p->size)) {
        /* Off the end */
        retval = -1;
    } else {
        idx = (bitnum / BPW);
        mask = BIT_ZERO >> (bitnum % BPW);
        retval = ((map_p->bits[idx] & mask) != 0);
        map_p->bits[idx] &= ~mask;
        if (bitnum < map_p->first_avail) {
            map_p->first_avail = bitnum;
        }
    }

    FEXIT(retval);
    return retval;
}

/****************************************************************************
 **
 **    Function:        btk_bit_chk()
 **
 **    Purpose:         Return the value of a given bit.
 **
 **    Args:
 **        bitmap_p
 **                     Bitmap pointer
 **        bitnum
 **                     Index of the bit to check
 **
 **    Returns:         int
 **                         0 if the bit is clear/free (0)
 **                         1 if the bit is set/allocated (1)
 **                         -1 on error
 **
 ***************************************************************************/
int btk_bit_chk(
    void * bitmap_p,
    unsigned int bitnum)
{
    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;
    int           retval;
    WORD_T        mask;
    unsigned int  idx;

    FUNCTION("btk_bit_chk");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    if ((map_p == NULL) || (bitnum >= map_p->size)) {
        /* Off the end */
        retval = -1;
    } else {
        idx = (bitnum / BPW);
        mask = BIT_ZERO >> (bitnum % BPW);
        retval = ((map_p->bits[idx] & mask) != 0);
    }

    FEXIT(retval);
    return retval;
}


/******************************************************************************
**
**      Function:   btk_bit_max()
**
**      Purpose:    Locate a large section of consecutive allocation bits.
**
**      Args:
**          bitmap_p    Pointer to bit map tracking structure
**          align       Bit boundary request must be aligned to
**          size_p      Pointer to location to store size of large section.
**
**
**      Modifies:
**          size_p      Return value; size of largest available section found.
**                      
**      Returns:
**          BT_SUCCESS  Everything is just swell
**          BT_ENOMEM   If not enough space open
**
**  Notes:      Should only be called with the unit_p locked to 
**              serialize access to the shared bitmap_p.
**
**              This routine always assumes that the searches should
**              start at first avail pointer and continue until the
**              end of the array is reached.  
**
**              This routine doesn't guarentee that it finds the absolute
**              largest section. Could be off by up to BPW*2-2 bits.
**
******************************************************************************/

bt_error_t btk_bit_max(
    void *bitmap_p,
    unsigned int align,
    unsigned int *size_p
    )
{
    bt_error_t      retval = BT_SUCCESS;    /* Assume we can find enough */

    unsigned int    start;          /* Start of section to check */
    unsigned int    found;          /* Number of consecutive bits found */
    unsigned int    biggest = 0;    /* Biggest section found so far */

    unsigned int    adjust;         /* Used as temp to adjust found and bit_num */

    WORD_T          search_mask;    /* Current search mask */
    unsigned int    search_size;    /* How many bits we search for each time */
    unsigned int    shift;          /* How far do we shift each time we search */

    int             search_index;   /* Index into array for search */
    unsigned int    search_bit;     /* Which bit we are currently searching at */

    WORD_T          mask;           /* Temporary bit mask */
    int             index;          /* Index into search array */
    unsigned int    bit_num;        /* Which bit we are currently checking */

    WORD_T          word;           /* Temporary copy of current word in array */

    bit_tracker_t *map_p = (bit_tracker_t *) bitmap_p;

    FUNCTION("btk_bit_max");
    LOG_UNKNOWN_UNIT;
    FENTRY;

    /*
    ** When we want to find a large consecutive block of available bits,
    ** we can speed up the search by looking for a block of half that
    ** size, and then backtracking to the first available bit.
    **
    ** To simplify the code (so we don't have to split masks over two
    ** locations), we only deal with search masks that are a power of
    ** two.
    */

    BTK_ASSERT(align != 0);
    BTK_ASSERT(WBIT_MASK == map_p->reserved);
    BTK_ASSERT(align < map_p->size);

    search_size = 1u;
    search_mask = BIT_ZERO;
    if (1 == align) {
	shift = 1u;
    } else {
        shift = BPW * 2;
	while (shift > align) {
	    shift /= 2;
	}
    }

    if (shift > BPW) {
        shift = BPW;
    }
    
    /* Find first location with open bits */
    start = map_p->first_avail;
    search_index = start / BPW;
    search_bit = start % BPW;

    /* Make sure that this bit is available */
    if ((word = map_p->bits[search_index]) & (BIT_ZERO>>search_bit)) {
        /* Better start from the beginning. */
        search_index = 0;
        word = map_p->bits[search_index];

        /* Need to search for first available bit */
        while (WBIT_MASK == word) {
            word = map_p->bits[++search_index];
        }
        search_bit = find_first_zero(word);
        start = map_p->first_avail = search_index * BPW + search_bit;
    }

    /* Make starting location a multiple of our shift */
    if ( (shift > 1) && (search_bit % shift) ){
        search_bit += shift - search_bit % shift;
    }

    /* Back up one position, so start of each loop advances the search */
    if (search_bit < shift) {
        search_bit = BPW - shift;
        search_index--;
    } else {
        search_bit -= shift;
    }

    if (search_bit != 0) {
        search_mask = RIGHT_ROTATE(search_mask, search_bit);
    }

    found = 0;
    do {        /* Search for available bits */

        /* Adjust search if already found a large area */
        if ( (1u == search_size) && (biggest >= BPW*2) ) {
	    shift = BPW*2;
	    search_size = BPW;
	    search_mask = WBIT_MASK;
	    while (shift > align) {
	        shift /= 2;
		search_size /= 2;
		search_mask <<= search_size;
	    }
	    if (shift > BPW) {
	        shift = BPW;
	    }
	}

        /* Loop always increments index at the start */

        if (search_size >= BPW) {
            /* Find first full word with no bits set */
            do {
                search_index++;
            } while (map_p->bits[search_index]);
            /* Array has one extra zero'ed word to terminate loop */

        } else {
            word = map_p->bits[search_index];
            do {
                search_bit += shift;
                search_mask = RIGHT_ROTATE(search_mask, shift);
                if (search_bit >= BPW) {
                    search_index++;
                    search_bit = 0;
                    word = map_p->bits[search_index];
                }
            } while (search_mask & word);
        }
        start = search_index * BPW + search_bit;
        found = search_size;

        if (shift > 1) {
            /* Now back track to first available bit */
            index = search_index;
            bit_num = search_bit;

            if (0 == search_bit) {
                /*
                ** Note: An index of -1 will work because it will access the
                ** reserved word. The researved word is marked with all bits
                ** allocated, terminating the search without additional logic.
                */
                bit_num = BPW;
                index--;
            }
            bit_num--;

            /* Don't ever have to backtrack across a word boundary */
            word = map_p->bits[index];
            /* While zeros preceeding */
            while ( !( word & (BIT_ZERO >> bit_num)) ) {
                found++;
                bit_num--;
                start--;
            }
        }

        /* Align starting position */
        if ((align > 1) && (0 != (start % align))) {
            /* Compute how far to next boundary */
            adjust = align - (start % align);

            if (found > adjust) {
                found -= adjust;
            } else {
                found = 0;
            }
            start += adjust;
        }

        /* Check if smaller than previous finds */

        if (map_p->size - start <= biggest) {
            /* Not enough room left to be larger than previous finds */
            break;
        }

        /* Search forward for remaining bits */

        index = (start + found) / BPW;
        bit_num = (start + found) % BPW;
        word = map_p->bits[index];

        /* Find out if rest of this word is available */
        mask = WBIT_MASK >> bit_num;

	if (0 != (word & mask)) {
	     /* Less than whole word available */
	     if (biggest > found + BPW - bit_num) {
	         /* Already found something larger than this */
	         continue;
	     }

	     mask = BIT_ZERO >> bit_num;
	     adjust = find_first_one(word & mask);
             if (adjust < BPW) {
		 found += adjust;
		 bit_num += adjust;
	     }

	     if (found > biggest) {
	         biggest = found;
	     }
	     continue;
	}

        found += BPW - bit_num;
        index++;
        bit_num = 0;

        /*
        ** Either we will succeed in finding enough space, or we should
        ** restart the search one shift into the next word.
        */
        search_mask <<= search_bit;
        search_bit = 0;

        /* Check whole words now */
        while (0 == (map_p->bits[index])) {
            found+= BPW;
            index++;
            /* Just in case we need to continue the search */
            search_index++;
        }

        /* Check if this can be bigger than previous values */
	if (biggest < found + BPW - bit_num) {
	    word = map_p->bits[index];
	    adjust = find_first_one(word);
	    if (adjust < BPW) {
		found += adjust;
		bit_num += adjust;
	    }
	}

        /* It is possible to go past the end of the map */
        if ((bit_num + index * BPW) > map_p->size) {
            found -= bit_num + index * BPW - map_p->size;
        }

	if (found > biggest) {
	    biggest = found;
	}

    } while (search_index * BPW + shift < map_p->size);

    *size_p = biggest;

    TRC_MSG(BT_TRC_ALLOC, 
        (LOG_FMT "Biggest %d, Return %d.\n", LOG_ARG, biggest, retval));

    FEXIT(retval);
    return(retval);
}
