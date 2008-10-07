/******************************************************************************
**
**      Filename:       btp_mmap.c
**
**      Purpose:        Device memory-mapping entry points for Linux driver.
**
**      Functions:      btp_mmap(), btp_lm_mmap()
**
**      $Revision: 1320 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1320 $" __DATE__;
#endif  /* LINT */

#include <asm/io.h>
#include <linux/mm.h>

#include "btdd.h"

#include <linux/module.h>       /* Must be after btdd.h */
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
#include <linux/sched.h> 
#endif

#if     !defined(MAP_NR)
#define MAP_NR(addr) (__pa(addr) >> PAGE_SHIFT)
#endif  /* !defined(MAP_NR) */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define vm_offset vm_pgoff*PAGE_SIZE
#endif  /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
static inline
int remap_page_range(struct vm_area_struct *vma, unsigned long uvaddr,
		     unsigned long paddr, unsigned long size, pgprot_t prot)
{
  return remap_pfn_range(vma, uvaddr, paddr >> PAGE_SHIFT, size, prot);
}

#endif

/*
** Local function prototypes
*/
void bt_vmopen(struct vm_area_struct *vma_p);
void bt_vmclose(struct vm_area_struct *vma_p);
unsigned long bt_vmnopage(struct vm_area_struct *vma_p, unsigned long address, int write_access);

/*
**  Structure to search mmap requests for
*/
typedef struct {
    bt_dev_t        axs_type;
    unsigned long   vm_boffset;
    unsigned long   vm_length;
    caddr_t         vm_p;
} search_t;

/* 
** Structure defining the valid operations 
*/
struct vm_operations_struct btp_vm_ops = {
#if     defined(__GCC__)        /* Initialize by element name */
    /* This form is supposed to be supported in C9X as well... */
    .open =     bt_vmopen,
    .close=     bt_vmclose,
    .nopage=    bt_vmnopage
#else   /* defined(__GCC__) */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    open:	bt_vmopen,
    close:	bt_vmclose,
    nopage:     (void *)bt_vmnopage,
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
    bt_vmopen,          /* open */
    bt_vmclose,         /* close */
    NULL,               /* unmap */
    NULL,               /* protect */
    NULL,               /* sync */
    NULL,               /* advise */
    bt_vmnopage,        /* nopage */
    NULL,               /* wppage */
    NULL,               /* swapout */
    NULL                /* swapin */
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
            LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

#endif  /* defined(__GCC__) */
};

/*
** Static function prototypes
*/
static int mmap_search(void *current_p, void *find_p);
static void retrieve_init_mm_ptr(void);

static struct mm_struct *init_mm_ptr;

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_MMAP_C);


/*****************************************************************************
**
**      Name:           btp_mmap()
**
**      Purpose:        Creates memory mapped entries from user space to the
**                      physical address of our device.
**
**      Args:
**          inode_p     Pointer to inode structure
**          file_p      Pointer to file structure
**          vma_p       Pointer to virtual memory area requested
**          
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          <0          Error number
**          Otherwise   Undefined
**
**      Notes:
**
*****************************************************************************/

int btp_mmap(
    struct file *file_p,
    struct vm_area_struct *vma_p
    )
{
    FUNCTION("btp_mmap");
    LOG_DEVID(file_p);
    
    int                 ret_val = 0;
    unsigned int        start, need = 1;
    bt_error_t          retval = BT_SUCCESS;
    bt_unit_t           *unit_p = GET_UNIT_PTR(file_p);
    unsigned            length = vma_p->vm_end - vma_p->vm_start;
    bt_dev_t            type = GET_LDEV_TYPE(file_p);
    unsigned long       dest_addr = vma_p->vm_offset;
    bt_data32_t         mreg_value;
    unsigned long       pci_addr;
    bt_mtrack_t         *new_mmap_p = NULL;
    btk_llist_elem_t    *element_p;

    FENTRY;
    TRC_MSG(BT_TRC_MMAP, (LOG_FMT "Mapping to device %d offset 0x%lx.\n",
        LOG_ARG, type, dest_addr));
    /* 
    **  Verify logical device even supports unmapping 
    */
    if (IS_CLR(unit_p->logstat[type], STAT_ONLINE)) {
        INFO_STR("Logical device not on-line");
        ret_val = -ENXIO;
        goto btp_mmap_end;
    }
    if (IS_CLR(unit_p->logstat[type], STAT_MMAP)) {
        INFO_STR("Logical device does not support unmapping");
        ret_val = -EINVAL;
        goto btp_mmap_end;
    }
    
    /*
    ** Check for page alignment
    */
    TRC_MSG(BT_TRC_MMAP | BT_TRC_DETAIL,
	(LOG_FMT "dest_addr = 0x%lx; vma_p->vm_start = 0x%lx\n",
        LOG_ARG, dest_addr, vma_p->vm_start));

    if (0 != (dest_addr & ~PAGE_MASK)) {
        /* They must give us a page-aligned offset */
        INFO_STR("Offset is not page aligned.\n");
        ret_val = -ENXIO;
        goto btp_mmap_end;
    }

    /*
    ** Make sure the power is on to map remote dual port
    */
    if ((type == BT_DEV_RDP) &&
        IS_SET(unit_p->bt_status, BT_POWER)) {
        INFO_STR("Power must be on before remote dual port can be mmaped");
        ret_val = -EINVAL;
        goto btp_mmap_end;
    }

#if EAS_BIND_CODE 
/* EAS BIND CODE */
    /* check if mmap for kernel allocated bind buffer */
    if ((dest_addr) && (dest_addr == (unsigned long)unit_p->bt_kmalloc_buf)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
        unsigned long offset = vma->vm_offset;
#else
    	unsigned long offset = vma_p->vm_pgoff<<PAGE_SHIFT;
#endif
        unsigned long size = vma_p->vm_end - vma_p->vm_start;
        
        if (offset & ~PAGE_MASK)
        {
            TRC_MSG(BT_TRC_BIND, 
                   (LOG_FMT "mmap offset not aligned: %ld\n",
                   LOG_ARG, offset));
            ret_val = -EINVAL;
            goto btp_mmap_end;
        }
        
        if (size > unit_p->bt_kmalloc_size)
        {
            TRC_MSG(BT_TRC_BIND, 
                   (LOG_FMT "mmap size too big: size=0x%x kmalloc_size=0x%x\n",
                   LOG_ARG, (unsigned int)size, (unsigned int)unit_p->bt_kmalloc_size));
            ret_val = -EINVAL;
            goto btp_mmap_end;
        }
        
	    /* we only support shared mappings. Copy on write mappings are
	       rejected here. A shared mapping that is writeable must have the
	       shared flag set.
	    */
	    if ((vma_p->vm_flags & VM_WRITE) && !(vma_p->vm_flags & VM_SHARED))
	    {
            INFO_STR("mmap writeable mappings must be shared, rejecting\n");
            ret_val = -EINVAL;
            goto btp_mmap_end;
	    }

	    /* we do not want to have this area swapped out, lock it */
	    vma_p->vm_flags |= VM_LOCKED;
        
        /* for the virtual contiguous memory area (kmalloc)
           we create a mapping between the physical pages and the virtual
           addresses of the application with remap_page_range.
        */
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
        if (remap_page_range(vma_p, vma_p->vm_start,
#else
        if (remap_page_range(vma_p->vm_start,
#endif
                             virt_to_phys((void*)((unsigned long)unit_p->bt_kmalloc_buf)),
                             size,
                             PAGE_SHARED))
        {
            INFO_STR("mmap remap page range failed\n");
            ret_val = -EINVAL;
            goto btp_mmap_end;
        }
        goto btp_mmap_end;
    }
#endif  /* EAS_BIND_CODE  */


    
    /* 
    ** Reset the VM operations to our routines 
    */
    vma_p->vm_ops = &btp_vm_ops;
    vma_p->vm_file = file_p;

    /*
    ** Retrieve the mm_struct pointer for process 0 (idle process)
    ** this is needed by the nopage routine so it can get 
    ** the physical address of the vmalloc pages.
    */
    retrieve_init_mm_ptr();

    /*
    ** If we are mapping driver memory, i.e. local memory
    ** or interrupt queues, we don't do anything more
    ** we let our nopage routine handle it all.
    */
    if (type == BT_DEV_LM) {
        goto btp_mmap_end;
    } else if ((type == BT_DEV_DIAG) &&
               (dest_addr >= BT_DIAG_ISR_Q_OFFSET)) {
        goto btp_mmap_end;
    }

    /*
    ** Now we handle the case of its a hardware address
    ** but no mapping registers are involved.
    */
    if (type == BT_DEV_LDP) {
        pci_addr = unit_p->mr_phys_addr + BT_CMR_DP_OFFSET;
        need = 0;
    } else if ((type == BT_DEV_RDP) && IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
        pci_addr = unit_p->mr_phys_addr + BT_CMR_DP_OFFSET + BT_CMR_REMOTE_OFFSET;
        need = 0;
    } else if (type == BT_DEV_DIAG) {
        if ((dest_addr >= BT_DIAG_MREG_OFFSET) &&
            (dest_addr < BT_DIAG_NIO_OFFSET)) {
            pci_addr = unit_p->mr_phys_addr + (dest_addr - BT_DIAG_MREG_OFFSET);
        } else if ((dest_addr >= BT_DIAG_NIO_OFFSET) &&
                   (dest_addr < BT_DIAG_ISR_Q_OFFSET)) {
            pci_addr = unit_p->csr_phys_addr + (dest_addr - BT_DIAG_NIO_OFFSET);
        } else  {
            INFO_STR("Invalid diagnostic mapping request.");
            ret_val = -ENXIO;
            goto btp_mmap_end;
        }
        need = 0;
        
    /* 
    ** Allocate and program the mapping regs
    */
    } else {

        /* 
        ** Allocate the mapping RAM
        */
        need = (length / BT_PAGE_SIZE) + ((length % BT_PAGE_SIZE) ? 1 : 0);
        btk_mutex_enter(unit_p, &(unit_p->mreg_mutex));
        retval = btk_bit_alloc(unit_p, unit_p->mmap_aval_p, need, 1, &start);
        btk_mutex_exit(unit_p, &(unit_p->mreg_mutex));
        if (BT_SUCCESS != retval) {
            TRC_STR(BT_TRC_INFO|BT_TRC_MMAP, 
                "Could not mmap area: Not enough mapping registers.\n");
            ret_val = -ENOMEM;
            goto btp_mmap_end;
        }
    
        /*
        ** Load map regs - setup vme address, address modifier, and function code
        */
        mreg_value = dest_addr & BT_MREG_ADDR_MASK;
        btk_setup_mreg(unit_p, type, &mreg_value, TRUE);
        btk_put_mreg_range(unit_p, start, need, BT_LMREG_PCI_2_CABLE, mreg_value);

        /*
        ** Calculate the PCI address
        */
        pci_addr = unit_p->rr_phys_addr + start * BT_PAGE_SIZE;
    }
    TRC_MSG(BT_TRC_MMAP, (LOG_FMT "Mapping device %d offset 0x%lx, start %d, need %d\n",
            LOG_ARG, type, dest_addr, start, need));

    /* 
    ** Now manipulate the user VMA to point directly to our device 
    */
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
    if (remap_page_range(vma_p, vma_p->vm_start,
#else
    if (remap_page_range(vma_p->vm_start, 
#endif
        pci_addr, length, vma_p->vm_page_prot)) {
        TRC_STR(BT_TRC_INFO|BT_TRC_MMAP, "Call to remap_page_range() failed.\n");
        if (need != 0) {
            btk_put_mreg_range(unit_p, start, need, BT_LMREG_PCI_2_CABLE, BT_MREG_INVALID);
            btk_mutex_enter(unit_p, &(unit_p->mreg_mutex));
            (void) btk_bit_free(unit_p, unit_p->mmap_aval_p, start, need);
            btk_mutex_exit(unit_p, &(unit_p->mreg_mutex));
        }
        ret_val = -ENOMEM;
        goto btp_mmap_end;
    }

    /*
    ** Need to track resources so they can be released
    */
    if (need != 0) {

        /* 
        **  Allocate a new mmap requested list item
        */
        element_p = btk_llist_elem_create(sizeof(bt_mtrack_t), 0);
        if (element_p == (btk_llist_elem_t *) NULL) {
            INFO_STR("Not enough resources to track mmap request");
            btk_put_mreg_range(unit_p, start, need, BT_LMREG_PCI_2_CABLE, BT_MREG_INVALID);
            btk_mutex_enter(unit_p, &(unit_p->mreg_mutex));
            (void) btk_bit_free(unit_p, unit_p->mmap_aval_p, start, need);
            btk_mutex_exit(unit_p, &(unit_p->mreg_mutex));
            ret_val = -ENOMEM;
            goto btp_mmap_end;
        }
        new_mmap_p = (bt_mtrack_t *) btk_llist_elem_data(element_p);
         
        /*
        **  Initialize fields of mmap structure
        */
        new_mmap_p->start_mreg = start;
        new_mmap_p->num_mreg   = need;
        new_mmap_p->axs_type = type;
        new_mmap_p->vm_boffset = vma_p->vm_offset;
        new_mmap_p->vm_length = vma_p->vm_end - vma_p->vm_start;
        new_mmap_p->vm_p = (caddr_t) vma_p;

        /*
        **  Record the information for munmap
        **  Note clink_un/lock are not used since the interrpt routine
        **  does not touch the queue.
        */
        btk_mutex_enter(unit_p, &unit_p->llist_mutex);
        btk_llist_insert_first(&unit_p->qh_mmap_requests, element_p);
        btk_mutex_exit(unit_p, &unit_p->llist_mutex);
    }
    
    if (ret_val >= 0) {
        bt_vmopen(vma_p);
    }
btp_mmap_end:
    FEXIT(ret_val);
    return ret_val;
}


/*****************************************************************************
**
**      Name:           bt_vmopen()
**
**      Purpose:        Virtual memory operation for mmap() area. Called
**                      when the same Virtual Memory Area (vma) is openned.
**                      I think this may occur when a process is cloned.
**
**      Args:
**          vma_p       Pointer to the virtual memory area.
**          
**      Modifies:
**          None
**          
**      Returns:        Void
**
**      Notes:
**      vma_p->vm_file should always point to the file descriptor that
**      opened the device.
**
*****************************************************************************/

void bt_vmopen(
    struct vm_area_struct *vma_p
    )
{
    FUNCTION("bt_vmopen");
    LOG_DEVID(vma_p->vm_file);

    FENTRY;

#if     LINUX_VERSION_CODE >= (KERNEL_VERSION(2,5,0))
    (void)try_module_get(THIS_MODULE);
#else
    MOD_INC_USE_COUNT;
#endif
    TRC_STR(BT_TRC_MMAP|BT_TRC_DETAIL, "Incrementing use count.\n");

    FEXIT(0);
    return;
}


/*****************************************************************************
**
**      Name:           bt_vmclose()
**
**      Purpose:        Virtual memory operation for mmap() area. Called
**                      when the Virtual Memory Area (vma) is closed by
**                      munmap().
**                      I think this can happen multiple times with cloned
**                      processes.
**
**      Args:
**          vma_p       Pointer to the virtual memory area.
**          
**      Modifies:
**          None
**          
**      Returns:        Void
**
**      Notes:
**      vma_p->vm_file should always point to the file descriptor that
**      opened the device.
**
*****************************************************************************/

void bt_vmclose(
    struct vm_area_struct *vma_p
    )
{
    FUNCTION("bt_vmclose");
    LOG_DEVID(vma_p->vm_file);
    bt_unit_t           *unit_p = GET_UNIT_PTR(vma_p->vm_file);
    bt_dev_t            type = GET_LDEV_TYPE(vma_p->vm_file);
    bt_mtrack_t         *mmap_p = NULL;
    btk_llist_elem_t    *element_p;
    search_t            search_val;
    unsigned long       dest_addr = vma_p->vm_offset;

    FENTRY;

#if EAS_BIND_CODE 
/* EAS BIND CODE */
    /* check if mmap for kernel allocated bind buffer */
    if ((dest_addr) && (dest_addr == (unsigned long)unit_p->bt_kmalloc_buf)) {
	    vma_p->vm_flags &= ~VM_LOCKED;
        /* nothing more to do here */
        goto bt_vmclose_end;
    }
#endif /* EAS_BIND_CODE */




    /*
    ** If we are mapping driver memory or we did not have to use
    ** mapping regs we are done execpt for the count
    */
    if (type == BT_DEV_LM) {
        goto bt_vmclose_end;
    } else if (type == BT_DEV_DIAG) {
        goto bt_vmclose_end;
    } else if (type == BT_DEV_LDP) {
        goto bt_vmclose_end;
    } else if ((type == BT_DEV_RDP) && 
               IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
        goto bt_vmclose_end;
    }

    /*
    ** Try to find the original mmap request  
    */
    search_val.axs_type = type;
    search_val.vm_boffset = vma_p->vm_offset;
    search_val.vm_length = vma_p->vm_end - vma_p->vm_start;
    search_val.vm_p = (caddr_t) vma_p;
    btk_mutex_enter(unit_p, &unit_p->llist_mutex);
    element_p = btk_llist_find_first(&unit_p->qh_mmap_requests, mmap_search, &search_val);
    if (element_p != (btk_llist_elem_t *) NULL) {

        /*
        ** Found it
        */
        btk_llist_remove(element_p);
        btk_mutex_exit(unit_p, &unit_p->llist_mutex);
        mmap_p = (bt_mtrack_t *) btk_llist_elem_data(element_p);
        TRC_MSG((BT_TRC_MMAP | BT_TRC_DETAIL), 
                (LOG_FMT "mmap existed: start %d; need %d\n", 
                LOG_ARG, mmap_p->start_mreg, mmap_p->num_mreg));

        /*
        **  Invalidate the mapping regs and relese bit map bits
        */
        btk_put_mreg_range(unit_p, mmap_p->start_mreg, mmap_p->num_mreg, BT_LMREG_PCI_2_CABLE, BT_MREG_INVALID);
        btk_mutex_enter(unit_p, &(unit_p->mreg_mutex));
        (void) btk_bit_free(unit_p, unit_p->mmap_aval_p, mmap_p->start_mreg, mmap_p->num_mreg);
        btk_mutex_exit(unit_p, &(unit_p->mreg_mutex));

        /*
        **  Release system resources
        */
        btk_llist_elem_destroy(element_p, sizeof(bt_mtrack_t));

    /*
    ** Did not find it, nothing to do though
    */
    } else {
        btk_mutex_exit(unit_p, &unit_p->llist_mutex);
        TRC_STR((BT_TRC_MMAP | BT_TRC_DETAIL), "Did not find original mmap request");
    }

bt_vmclose_end:
#if     LINUX_VERSION_CODE >= (KERNEL_VERSION(2,5,0))
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif
    TRC_STR(BT_TRC_MMAP|BT_TRC_DETAIL, "Decrementing use count.\n");

    FEXIT(0);
    return;
}


/*****************************************************************************
**
**      Name:           bt_vmnopage()
**
**      Purpose:        Virtual memory operation for mmap() area. Called
**                      when the Virtual Memory Area (vma) is expanded.
**
**      Args:
**          vma_p           Pointer to the virtual memory area.
**          address         ???
**          write_access    ???
**          
**      Modifies:
**          None
**          
**      Returns:        0 causing a SIGBUS or
**                      physical address of driver memory
**
**      Notes:
**
*****************************************************************************/

unsigned long bt_vmnopage(
    struct vm_area_struct *vma_p,
    unsigned long address, 
    int write_access
    )
{
    FUNCTION("bt_vmnopage");
    LOG_DEVID(vma_p->vm_file);
    bt_unit_t           *unit_p = GET_UNIT_PTR(vma_p->vm_file);
    bt_dev_t            type = GET_LDEV_TYPE(vma_p->vm_file);
    unsigned long       dest_addr = address - vma_p->vm_start + vma_p->vm_offset;
    unsigned long       page;
    pgd_t               *pgd_p;
    pmd_t               *pmd_p;
    pte_t               *pte_p;
    caddr_t             kmem_p;

    FENTRY;

    /*
    ** This function only does something for maps that try to access 
    ** driver memory like the local memory device.  Otherwise
    ** we return the zero page
    */
    if (type == BT_DEV_LM) {
        kmem_p = unit_p->lm_kaddr + dest_addr;
    } else if ((type == BT_DEV_DIAG) &&
               (dest_addr >= BT_DIAG_ISR_Q_OFFSET)) {
        kmem_p = (caddr_t) BTK_Q_USE + (dest_addr - BT_DIAG_ISR_Q_OFFSET);
    } else {
        FEXIT(0);
        return 0;
    }

    /*
    ** We need to increment the page count and return the
    ** physical address of the page.  See book Linux Device Drivers by
    ** Alessandro Rubini, Chapter 13 on mmap.
    */
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    page = (unsigned long)kmem_p;
#else
    page = VMALLOC_VMADDR(kmem_p);
#endif

#if EAS_BIND_CODE
/* EAS TMP CODE */
    if (type == BT_DEV_LM)
    {
        TRC_MSG((BT_TRC_BIND | BT_TRC_DETAIL), 
                (LOG_FMT "mmap kernel buf: kernel virt=0x%x kernel phys=0x%x\n",
                LOG_ARG, (unsigned int)kmem_p, (unsigned int)page));
    }
#endif /* EAS_BIND_CODE */

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    spin_lock(&init_mm.page_table_lock);
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) */

    pgd_p = pgd_offset(&init_mm, page);
    pmd_p = pmd_offset(pgd_p, page);
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
    pte_p = pte_offset_kernel(pmd_p, page);
#else
    pte_p = pte_offset(pmd_p, page);
#endif
    page = (unsigned long)pte_page(*pte_p);

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    spin_unlock(&init_mm.page_table_lock);
    get_page((struct page *)page);
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
    atomic_inc(&mem_map[MAP_NR(page)].count);
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
            LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

    FEXIT(page);
    return page;
}
static void retrieve_init_mm_ptr(void)
{
    struct task_struct *t_p;


#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
    for (t_p = current; (t_p = next_task(t_p)) != current; ) {
#else
    for (t_p = current; (t_p = t_p->next_task) != current; ) {
#endif
        if (t_p->pid == 0) {
            break;
        }
    }
    init_mm_ptr = t_p->mm;
}

/******************************************************************************
**
**  Function:   mmap_search()
**
**  Purpose:    Compares the current queue item to the values passed
**              with the compare item and returns 0 if it was a match.
**
**  Args:       current_p   Pointer to current data element
**              find_p      Pointer to data element we are looking for.
**
**  Returns:    0           Match.
**              Otherwise   No match.
**
**  Notes:      Should be used with btk_llist_?() routines.
**
******************************************************************************/
/*ARGSUSED*/
static int mmap_search(
  void *current_p, 
  void *find_p)
{
    FUNCTION("mmap_search");
    LOG_UNKNOWN_UNIT;

    bt_mtrack_t   *c_p = (bt_mtrack_t *) current_p;
    search_t      *m_p = (search_t *) find_p;
     int           found = 1;

    FENTRY;
    
    if ((c_p->axs_type == m_p->axs_type) &&
        (c_p->vm_boffset == m_p->vm_boffset) &&
        (c_p->vm_length == m_p->vm_length) &&
        (c_p->vm_p == m_p->vm_p)) {
        found = 0;
    }

    FEXIT(found);
    return(found);
}

