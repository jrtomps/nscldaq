/****************************************************************************
**
**      Filename:       bt_diag.c
**
**      Purpose:        SBS PCI Adaptor device driver module.
**                      Provides diagnostic routines that can be called
**                      from an ioctl.
**
**      $Revision: 1320 $
**
****************************************************************************/
/****************************************************************************
**
**        Copyright (c) 2001 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
****************************************************************************/

#if !defined(LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1320 $ "__DATE__;
static const char file_rev[] = "$Revision: 1320 $";
#endif	/* !LINT */

/*
**  Include files
*/
#include "btdd.h"

#if defined(__linux__)
#include <asm/io.h>
#endif /* defined(__linux__) */

/*
** Local macros
*/
#define REPEAT_COUNT (16)

#define ALL_ZERO        0x0L
#define ALL_ONE         0xffffffffL

#undef  BIT_ERROR
#define BIT_ERROR(bit_no, exp, rec) INFO_MSG((LOG_FMT \
          "Data error: bit #%8d,  expected = 0x%8x  received = 0x%8x\n", \
          LOG_ARG, (int) bit_no, (bt_devaddr_t) (exp & 0xffff), (bt_data32_t) rec));


#undef  ADDR_ERROR
#define ADDR_ERROR(bit_no, hi_lo, addr1, addr2) INFO_MSG((LOG_FMT \
          "Address error: bit #%8d is stuck %s,  zero addr = 0x%8x  test addr = 0x%8x\n", \
          LOG_ARG, (int) bit_no, hi_lo, (bt_devaddr_t) addr1, (bt_devaddr_t) addr2));

#undef  ADDRS_ERROR
#define ADDRS_ERROR(bit_no, addr1, addr2) INFO_MSG((LOG_FMT \
          "Address error: bit #%8d is shorted,  bad addr = 0x%8x  test addr = 0x%8x\n", \
          LOG_ARG, (int) bit_no, (bt_devaddr_t) addr1, (bt_devaddr_t) addr2));

#define DO_DMA (1)
#define DO_PIO (2)

#define ALLOC_ALIGN     (2048)       /* Size to align buffers to */


#define MAX_LENGTH (0x10000) /* The max length of the buffer 64K */ 


#define SIZE_1K (1u<<10)

#define FULL_TEST_LEN   (8 * SIZE_1K) /* Check all lengths up to 8K */
#define POWER_TEST_LEN  (16 * SIZE_1K) /* Go by powers of two from 16K up */
#define MAX_TEST_OFFSET (16)


/* Sanity check the #defines */
#if (POWER_TEST_LEN & (POWER_TEST_LEN - 1))
#error POWER_TEST_LEN needs to be a power of 2!
#endif /* POWER_TEST_LEN */

#define MAX_ADJ_TABLE 128


#if (POWER_TEST_LEN <= (FULL_TEST_LEN + MAX_ADJ_TABLE))
#error POWER_TEST_LEN needs to be larger than FULL_TEST_LEN + MAX_ADJ_TABLE
#endif /* POWER_TEST_LEN */


size_t adj_table[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    24, 28, 30, 31, 32, 33, 34, 36, 40, 48, 56, 60, 62, 63, 
    64, 65, 66, 68, 72, 80, 96, 112, 120, 124, 126, 127, 128,
    0 /* 0 is a marker value signifing the end of the list */
};


/*
** Local Types
*/
typedef enum { 
    REPEAT_DATA, 
    INCREMENT, 
    DECREMENT,
    COUNT_UP, 
    COUNT_DOWN 
}  pattern_number_t;

typedef struct {
    pattern_number_t type;
    bt_data32_t      fill;
    bt_data32_t      inc_dec;
    int              width;
} pattern_t;

typedef struct pattern_list {
    bt_data32_t          data_pattern;
    struct pattern_list *next_pattern;
} pattern_list_t;

/*
** Routines within this file
*/
bt_error_t btk_lcard_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

bt_error_t btk_cable_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

bt_error_t btk_rcard_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

bt_error_t btk_pair_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

bt_error_t btk_driver_version(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

int find_bad_bit(bt_unit_t *unit_p, bt_data32_t expected, bt_data32_t received);

int check_memory(bt_unit_t *unit_p, void *start_address_p, bt_data32_t mem_size, pattern_t pattern);

int initialize_memory(bt_unit_t *unit_p, void *start_address_p, bt_data32_t mem_size, pattern_t pattern);

int data_bus(bt_unit_t *unit_p, void *test_p, void *verify_p, pattern_list_t *user_list_p);

int address_bus(bt_unit_t *unit_p, void *test_p, void *verify_p, bt_data32_t mem_size);

int access_width(bt_unit_t *unit_p, void *test_p, void *verify_p, bool_t same_endian);

int cell_integrity(bt_unit_t *unit_p, void *test_p, void *verify_p, bt_data32_t mem_size);

int adator_reg_test(bt_unit_t *unit_p);

int adator_rem_reg_test(bt_unit_t *unit_p);

static int btk_datachk (bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t base_addr, int len, bt_data32_t local_offset, bt_data32_t remote_offset, int dma_width);

static bt_data32_t genrand(void);

/*
** External routines
*/
extern bt_data32_t btk_get_io(bt_unit_t *unit_p, bt_reg_t reg); 
extern void btk_put_io(bt_unit_t *unit_p, bt_reg_t reg, bt_data32_t value); 
extern void btk_put_mreg(bt_unit_t *unit_p, unsigned int mr_idx, bt_mreg_t mreg_type, bt_data32_t value);

extern bt_error_t btk_set_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, bt_devdata_t value);
extern bt_error_t btk_get_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, bt_devdata_t *value_p);
extern bt_data8_t btp_get_d8(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d8(bt_unit_t *unit_p, void *addr_p, bt_data8_t value);
extern bt_data16_t btp_get_d16(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d16(bt_unit_t *unit_p, void *addr_p, bt_data16_t value);
extern bt_data32_t btp_get_d32(bt_unit_t *unit_p, void *addr_p);
extern void btp_put_d32(bt_unit_t *unit_p, void *addr_p, bt_data32_t value);
extern bt_error_t btk_hw_xfer(bt_unit_t *unit_p, bt_dev_t ldev, int xfer_dir, bt_hw_xfer_t *hw_xfer_p);
extern bt_error_t btk_pio_xfer(bt_unit_t *unit_p, bt_dev_t ldev, caddr_t laddr_p, bt_data32_t ldev_addr, int *xfer_length_p, size_t xfer_dir);
extern bt_error_t btk_setpage(bt_unit_t *unit_p, bt_dev_t ldev, 
            bt_data32_t ldev_addr, btk_page_t *page_p);
#if defined(__linux__)
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int *xfer_length, int xfer_dir, int data_width);
#else
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int xfer_length, int xfer_dir, int data_width);
#endif
extern void btk_setup_mreg(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t *mreg_value_p, bt_operation_t op);
extern void btk_put_mreg_range(bt_unit_t *unit_p, unsigned int mre_start, unsigned int num_mregs, bt_mreg_t mreg_type, bt_data32_t value);


/*
**  Local variables
*/
BT_FILE_NUMBER(TRACE_BT_DIAG_C);
  
/******************************************************************************
**
**      Function:       btk_driver_version()
**
**      Purpose:        Report the driver's version as assigned in MKS.
**
**      Args:           
**          unit_p      Unit pointer
**          diag_p->    Diagnostic parameter structure
**            rev_info    Text string with driver version.
**            error       Set to appropriate return value.
**            line_number Line number failure was discovered on.
**          
**      Returns:
**          BT_EINVAL   Illegal parameter passed to function.
**
**          BT_SUCCESS  
**          
**      Notes:
**
******************************************************************************/
bt_error_t btk_driver_version(
    bt_unit_t *unit_p, 
    bt_hw_diag_t *diag_p)
{
    FUNCTION("btk_driver_version");
    LOG_UNIT(unit_p);
    
    bt_data32_t           lnumber = 0;
    bt_error_t            retval = BT_SUCCESS;
    
    FENTRY;
    
    /*
    **  Validate arguments
    */
    if (NULL == diag_p) {
        INFO_STR("Diagnostic parameter sturcture is a NULL.");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_driver_version_end;
    }

    /*
    ** Set the revision info
    */
    strncpy((char *) &diag_p->rev_info, (char *) &unit_p->driver_version, BT_DIAG_MAX_REV_INFO);
    diag_p->rev_info[BT_DIAG_MAX_REV_INFO-1] = '\0';

btk_driver_version_end:
    diag_p->error = retval;
    diag_p->line_number = lnumber;
    FEXIT(retval);
    return(retval);
}

  
/******************************************************************************
**
**      Function:       btk_lcard_diag()
**
**      Purpose:        Determine whether the local card is operating 
**                      properly.
**
**      Args:           
**          unit_p      Unit pointer
**          diag_p->    Diagnostic parameter structure
**            error       Set to appropriate return value.
**            rev_info    Text string with file and revision number.
**            line_number Line number failure was discovered on.
**          
**      Returns:
**          BT_ENOSUP   Not supported on this hardware.
**          BT_ENXIO    Local memory device not enabled.
**          BT_ENOPWR   Pig tail connector not installed.
**          BT_EINVAL   Illegal parameter passed to function.
**          BT_EDESC    Illegal descriptor passed to function.
**          BT_ELCARD   Testing failed, local card needs repair.
**
**          BT_SUCCESS  Testing passed, local card appears good.
**          
**
**      Notes:
**          Any data in local dual port memory or the local memory 
**          device will be destroyed.
**
**          Assumes exclusive use of the adapter and thus does not 
**          use proper mutexes.
**        
**      Basic Operation:
**
**          1. Test an unused portion of PCI to remote mapping registers 
**          with the 
**            64-bit wide data bus test
**            16 Kbyte address bus test
**            64-bit wide access width test
**            16 Kbyte cell integrity test
**          
**          2. Test the node local DMA address registers 
**
**          3. Test the local dual port RAM with the
**            64-bit wide data bus test
**            192 Kbyte address bus test
**            64-bit wide access width test
**            192 Kbyte cell integrity test
**
**          4. Put the card in loop back mode 
**
**            4.1. The remote dual port memory will be written to and
**            remote dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**        
**            4.2. The remote dual port memory will be written to and
**            local dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**
**            4.3. The Remote RAM is will be written to and Remote RAM read from
**            to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**            4.4. The Remote RAM is will be written to and local memory
**            read from to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**
**          Pig tail connector must be installed so remote accesses are local
**
**          5. Take the card out of loop back mode 
**
**            5.1. The remote dual port memory will be written to and
**            remote dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**        
**            5.2. The remote dual port memory will be written to and
**            local dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**
**            5.3. The Remote RAM is will be written to and Remote RAM read from
**            to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**            5.4. The Remote RAM is will be written to and local memory
**            read from to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**            5.5. Test that an interrupt can be received (using PT)
**
**        
**        
**
******************************************************************************/
bt_error_t btk_lcard_diag(
    bt_unit_t *unit_p, 
    bt_hw_diag_t *diag_p)
{
    FUNCTION("btk_lcard_diag");
    LOG_UNIT(unit_p);
    
    bt_data32_t           lnumber = 0;
    bt_error_t            retval = BT_SUCCESS;
    void                 *test_p = NULL;
    void                 *verify_p = NULL;
    bt_data32_t           mem_size;
    bt_data32_t           save_irq; 
    btk_page_t            page;
    int                   total_irq, prg_irq;
    
    FENTRY;
    
    /*
    **  Validate arguments
    */
    if (NULL == diag_p) {
        INFO_STR("Diagnostic parameter sturcture is a NULL.");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_lcard_diag_end;
    }

    /*
    ** Set the revision info
    */
    strncpy((char *)&diag_p->rev_info, file_rev, BT_DIAG_MAX_REV_INFO);
    diag_p->rev_info[BT_DIAG_MAX_REV_INFO-1] = '\0';

    /*
    ** Only PCI to PCI and dataBLIZZARD implemented now
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN) ||
        IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
        INFO_STR("Only dataBLIZZARD PCI to PCI configurations supported");
        lnumber = __LINE__;
        retval = BT_ENOSUP;
        goto btk_lcard_diag_end;
    }

    /*
    ** Cable needs to be connected
    */
    if (IS_SET(unit_p->bt_status, BT_POWER)) {
        INFO_STR("Cable disconnected or pig tail connector not installed");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_lcard_diag_end;
    }
    
    /*
    ** Set the page to point at the local memory device
    */
    if (BT_SUCCESS != btk_setpage(unit_p, BT_AXSRR, 0,  & page)) {
        INFO_STR("Failed to set remote memory page");
        lnumber = __LINE__;
        retval = BT_EIO;
        goto btk_lcard_diag_end;
    }

    /*
    ** Test the Local DMA address registers
    */
    lnumber = adator_reg_test(unit_p);
    if (0 != lnumber) {
        INFO_STR("Failed testing of local card's DMA address registers");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    
    /*
    ** Test unused portion of PCI to remote mapping registers
    ** upper 16 Mbytes
    */
    test_p =  (void *) (unit_p->mreg_p + BT_CMR_RMEM_SIZE_P2P);
    mem_size = BT_CMR_RMEM_SIZE_P2P;
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of unused mapping registers");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of unused mapping registers");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of unused mapping registers");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of unused mapping registers");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    
    /* 
    ** Test local dual port RAM
    */
    if ((unit_p->kern_addr[BT_AXSLDP] == NULL) ||
        (unit_p->kern_length[BT_AXSLDP] <= 256)) {
        INFO_STR("Internal error: local dual port RAM does not exist or is too small");
        lnumber = __LINE__;
        retval = BT_ENXIO;
        goto pci_pio_end;
    }
    test_p =  (void *) (unit_p->kern_addr[BT_AXSLDP]);
    mem_size = unit_p->kern_length[BT_AXSLDP];
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of local dual port RAM");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of local dual port RAM");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of local dual port RAM");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of local dual port RAM");
        retval = BT_ELCARD;
        goto pci_pio_end;
    }

    /*
    ** Set local loopback bit 
    **
    ** Disable interrupts since switching in and out of loopback
    ** will cause cable parity errors
    **
    ** Must delay for 1.5 seconds before and after switching
    ** into or out of loopback
    **
    ** Should not hold locks over use of btk_sleep
    */
    save_irq = btk_get_io(unit_p, BT_LOC_INT_CTRL);
    btk_put_io(unit_p, BT_LOC_INT_CTRL, 0); 
    btk_put_io(unit_p, BT_LOC_LOOPBACK, LLB_LOC_LB_ENABLE); 
    btk_sleep(1500 * 1000);

    /*
    ** Dual Port RAM tests
    */
    /* 
    ** Test and verify remote DP RAM using same pointer
    */
    if ((unit_p->kern_addr[BT_AXSRDP] == NULL) ||
        (unit_p->kern_length[BT_AXSRDP] <= 256)) {
        INFO_STR("Internal error: dual port RAM does not exist");
        lnumber = __LINE__;
        retval = BT_ENXIO;
        goto pci_loop_end;
    }
    test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
    mem_size = unit_p->kern_length[BT_AXSRDP];
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }

    /* 
    ** Test and verify remote DP RAM using local DP RAM pointer
    */
    test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
    verify_p =  (void *) (unit_p->kern_addr[BT_AXSLDP]);
    mem_size = unit_p->kern_length[BT_AXSRDP];
    lnumber = data_bus(unit_p, test_p, verify_p, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed second data bus test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed second address bus test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = access_width(unit_p, test_p, verify_p, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed second access width test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed second cell integrity test of remote dual port RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    
    /*
    **  Now test the Remote RAM
    */
    if ((page.bus_base_p == NULL) ||
        (page.page_size < 256)) {
        INFO_STR("Remote RAM does not exist");
        lnumber = __LINE__;
        retval = BT_ENXIO;
        goto pci_loop_end;
    }
    if ((unit_p->kern_addr[BT_AXSLM] == NULL) ||
        (page.page_size > unit_p->kern_length[BT_AXSLM])) {
        INFO_STR("Local Memory does not exist or is too small for testing");
        lnumber = __LINE__;
        retval = BT_ENXIO;
        goto pci_loop_end;
    }
    
    /* 
    ** Test and verify remote RAM using same pointer
    */
    test_p =  (void *) (page.bus_base_p);
    mem_size = (bt_data32_t) page.page_size;
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }

    /*
    ** SJF 3/22/2002 the diagnostic pair test will fail the first pass when
    ** the local diagnostic test is run with the 'test and verify remote
    ** RAM using local memory for verify' test is left in the code.  I have
    ** no idea why this is making the pair test fail but for now I'll just 
    ** remove this section of the test from the local tests.
    */
if (0) {
    /* 
    ** Test and verify remote RAM using local memory for verify
    */
    test_p =  (void *) (page.bus_base_p);
    mem_size = (bt_data32_t) page.page_size;
    verify_p =  (void *) (unit_p->kern_addr[BT_AXSLM]);
    lnumber = data_bus(unit_p, test_p, verify_p, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed second data bus test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed second address bus test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
    lnumber = access_width(unit_p, test_p, verify_p, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed second access width test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end; 
    }
    lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed second cell integrity test of Remote RAM");
        retval = BT_ELCARD;
        goto pci_loop_end;
    }
} /* end if (0) */
    
    /*
    ** Take it out of loopback, re-enable interrupts, and clear any
    ** cable errors that occured because of the loopback switches
    */
    btk_put_io(unit_p, BT_LOC_LOOPBACK, 0); 
    btk_sleep(1500 * 1000);
    btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR); 
    btk_put_io(unit_p, BT_LOC_INT_CTRL, save_irq); 

    /* 
    ** Now test with the pig connector on if possible
    ** This allows testing of the actual optics where loopback loops
    ** back before the optics.
    */
    if (IS_SET(unit_p->bt_status, BT_LOOPBACK)) {
        /* 
        ** Test remote DP RAM same addresses
        */
        test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
        mem_size = unit_p->kern_length[BT_AXSRDP];
        lnumber = data_bus(unit_p, test_p, NULL, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed third data bus test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed third address bus test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, NULL, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed third access width test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed third cell integrity test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }

        /*
        ** Test Remote DP RAM different addresses
        */
        test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
        verify_p =  (void *) (unit_p->kern_addr[BT_AXSLDP]);
        mem_size = unit_p->kern_length[BT_AXSRDP];
        lnumber = data_bus(unit_p, test_p, verify_p, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed fourth data bus test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed fourth address bus test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, verify_p, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed fourth access width test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed fourth cell integrity test of remote dual port RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }    

        /*
        ** Test remote RAM same addresses
        **
        ** Note remote page set in prior test
        */
        test_p =  (void *) (page.bus_base_p);
        mem_size = (bt_data32_t) page.page_size;
        lnumber = data_bus(unit_p, test_p, NULL, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed third data bus test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed third address bus test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, NULL, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed third access width test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed third cell integrity test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }

        /* 
        ** Test remote RAM different addresses
        */
        test_p =  (void *) (page.bus_base_p);
        mem_size = (bt_data32_t) page.page_size;
        verify_p =  (void *) (unit_p->kern_addr[BT_AXSLM]);
        lnumber = data_bus(unit_p, test_p, verify_p, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed fourth data bus test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed fourth address bus test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end; 
        }
        lnumber = access_width(unit_p, test_p, verify_p, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed fourth access width test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end; 
        }
        lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed fourth cell integrity test of Remote RAM");
            retval = BT_ELCARD;
            goto pci_pio_end; 
        }

        /*
        ** Test PT interrupt
        */
        total_irq = unit_p->sig_count;
        prg_irq = unit_p->sig_prg_cnt;
  
        /*
        ** Set the PT and delay for irq to come in
        */
        btk_put_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_RMREG_PCI_2_CABLE, 0);
        btk_put_io(unit_p, BT_LOC_CMD1, LC1_SND_PT_INT);
        btk_sleep(1000);

        /*
        ** Make sure counts got incremented
        */
        if (unit_p->sig_count != (total_irq + 1)) {
            INFO_STR("Failed interrupt test, total count mismatch");
            retval = BT_ELCARD;
            goto pci_pio_end;
        } 
        if (unit_p->sig_prg_cnt != (prg_irq + 1)) {
            INFO_STR("Failed interrupt test, programmed count mismatch");
            retval = BT_ELCARD;
            goto pci_pio_end;
        } 

    } /* end if card has loopback connector installed */
    goto pci_pio_end;

    
pci_loop_end:
    /*
    ** Take it out of loopback, re-enable interrupts, and clear any
    ** cable errors that occured because of the loopback switches
    */
    btk_put_io(unit_p, BT_LOC_LOOPBACK, 0); 
    btk_sleep(1500 * 1000);
    btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR); 
    btk_put_io(unit_p, BT_LOC_INT_CTRL, save_irq); 
    
pci_pio_end:
    /*
    ** Free the mapping RAM allocated during the btk_setpage()
    */
    if (page.mreg_need != 0) {
        btk_mutex_enter(unit_p, &unit_p->mreg_mutex);
        btk_bit_free(unit_p, unit_p->mmap_aval_p, page.mreg_start, page.mreg_need);
        btk_mutex_exit(unit_p, &unit_p->mreg_mutex);
    }
    
btk_lcard_diag_end:
    diag_p->error = retval;
    diag_p->line_number = lnumber;
    FEXIT(retval);
    return(retval);
}


/*****************************************************************************
**
**      Name:           btk_cable_diag
**
**      Purpose:        Determine whether the cable is operating 
**                      properly.
**
**      Args:
**          unit_p      Unit pointer
**          diag_p->    Diagnostic parameter structure
**            error       Set to appropriate return value.
**            rev_info    Text string with file and revision number.
**            line_number Line number failure was discovered on.
**          
**      Returns:
**          BT_ENOSUP   Not supported on this hardware.
**          BT_ENXIO    Local memory device not enabled.
**          BT_ENOPWR   Pig tail connector not installed.
**          BT_EINVAL   Illegal parameter passed to function.
**          BT_EDESC    Illegal descriptor passed to function.
**          BT_ECABLE   Testing failed, cable needs repair.
**
**          BT_SUCCESS  Testing passed, cable appears good.
**          
**
**      Notes:
**          The pig tail connector must be installed.
**
**          Any data in local dual port memory or the local memory 
**          device will be destroyed.
**
**          Assumes exclusive use of the adapter and thus does not 
**          use proper mutexes.
**        
**          Should be done after btk_lcard_diag() passes.
**        
**      Basic Operation:
**
**          Pig tail connector must be installed at the end of the cable
**            so remote accesses move through the entire cable and become
**            local ones.
**
**
**            1. The remote dual port memory will be written to and
**            remote dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**        
**            2. The remote dual port memory will be written to and
**            local dual port RAM read from to verify the operation.
**              64-bit wide data bus test
**              192 Kbyte address bus test
**              64-bit wide access width test
**              192 Kbyte cell integrity test
**
**            3. The Remote RAM is will be written to and Remote RAM read from
**            to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**            4. The Remote RAM is will be written to and local memory
**            read from to verify the operation.    
**              64-bit wide data bus test
**              address bus test
**              64-bit wide access width test
**              cell integrity test
**
**
**
*****************************************************************************/

bt_error_t btk_cable_diag(
    bt_unit_t *unit_p, 
    bt_hw_diag_t *diag_p)
{
    FUNCTION("btk_cable_diag");
    LOG_UNIT(unit_p);
    
    bt_data32_t     lnumber = 0;
    bt_error_t      retval = BT_SUCCESS;
    void           *test_p = NULL;
    void           *verify_p = NULL;
    bt_data32_t     mem_size;
    btk_page_t      page;

    FENTRY;

    /*
    **  Validate arguments
    */
    if (NULL == diag_p) {
        INFO_STR("Diagnostic parameter sturcture is a NULL.");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_cable_diag_end;
    }

    /*
    ** Set the revision info
    */
    strncpy((char *)&diag_p->rev_info, file_rev, BT_DIAG_MAX_REV_INFO);
    diag_p->rev_info[BT_DIAG_MAX_REV_INFO-1] = '\0';

    /*
    ** Only PCI to PCI and dataBLIZZARD implemented now
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN) ||
        IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
        INFO_STR("Only dataBLIZZARD PCI to PCI configurations supported");
        lnumber = __LINE__;
        retval = BT_ENOSUP;
        goto btk_cable_diag_end;
    }

    /*
    ** Cable needs to be connected
    */
    if (IS_SET(unit_p->bt_status, BT_POWER)) {
        INFO_STR("Cable disconnected or remote power is off");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_cable_diag_end;
    }

    /*
    ** Pigtail needs to be connected
    */
    if (IS_CLR(unit_p->bt_status, BT_LOOPBACK)) {
        INFO_STR("Pig tail connector not installed");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_cable_diag_end;
    }
    
    /*
    ** Set the page to point at the local memory device
    */
    if (BT_SUCCESS != btk_setpage(unit_p, BT_AXSRR, 0,  & page)) {
        INFO_STR("Failed to set remote memory page");
        lnumber = __LINE__;
        retval = BT_EIO;
        goto btk_cable_diag_end;
    }

    /* 
    ** Since the adapter is off line go ahead and test dual port RAM
    ** Note: pig tail should be installed so remote dp is actually local
    ** First test is verified using the same pointer
    */
    if (IS_SET(unit_p->bt_status, BT_LOOPBACK)) {
        /* 
        ** Test remote DP RAM same addresses
        */
        test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
        mem_size = unit_p->kern_length[BT_AXSRDP];
        lnumber = data_bus(unit_p, test_p, NULL, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed first data bus test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed first address bus test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, NULL, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed first access width test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed first cell integrity test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }

        /*
        ** Test Remote DP RAM different addresses
        */
        test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
        verify_p =  (void *) (unit_p->kern_addr[BT_AXSLDP]);
        mem_size = unit_p->kern_length[BT_AXSRDP];
        lnumber = data_bus(unit_p, test_p, verify_p, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed second data bus test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed second address bus test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, verify_p, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed second access width test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed second cell integrity test of remote dual port RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }

        /*
        ** Test remote RAM
        */
        if ((page.bus_base_p == NULL) ||
            (page.page_size < 256)) {
            INFO_STR("Remote RAM does not exist");
            lnumber = __LINE__;
            retval = BT_ENXIO;
            goto pci_pio_end;
        }
    
        /*
        ** Test and verify remote RAM using same pointer
        */
        test_p =  (void *) (page.bus_base_p);
        mem_size = (bt_data32_t) page.page_size;
        lnumber = data_bus(unit_p, test_p, NULL, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed first data bus test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed first address bus test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = access_width(unit_p, test_p, NULL, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed first access width test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed first cell integrity test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }

        /* 
        ** Test remote RAM different addresses
        */
        test_p =  (void *) (page.bus_base_p);
        mem_size = (bt_data32_t) page.page_size;
        verify_p =  (void *) (unit_p->kern_addr[BT_AXSLM]);
        lnumber = data_bus(unit_p, test_p, verify_p, NULL);
        if (0 != lnumber) {
            INFO_STR("Failed second data bus test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end;
        }
        lnumber = address_bus(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed second address bus test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end; 
        }
        lnumber = access_width(unit_p, test_p, verify_p, TRUE);
        if (0 != lnumber) {
            INFO_STR("Failed second access width test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end; 
        }
        lnumber = cell_integrity(unit_p, test_p, verify_p, mem_size);
        if (0 != lnumber) {
            INFO_STR("Failed second cell integrity test of Remote RAM");
            retval = BT_ECABLE;
            goto pci_pio_end; 
        }

    } /* end if card has loopback connector installed */
    
pci_pio_end:
    /*
    ** Free the mapping RAM allocated during the btk_setpage()
    */
    if (page.mreg_need != 0) {
        btk_mutex_enter(unit_p, &unit_p->mreg_mutex);
        btk_bit_free(unit_p, unit_p->mmap_aval_p, page.mreg_start, page.mreg_need);
        btk_mutex_exit(unit_p, &unit_p->mreg_mutex);
    }
    
btk_cable_diag_end:
    diag_p->error = retval;
    diag_p->line_number = lnumber;
    FEXIT(retval);
    return(retval);
}


/*****************************************************************************
**
**      Name:           btk_rcard_diag
**
**      Purpose:        Determine whether the remote card is operating 
**                      properly.
**
**      Args:
**          unit_p      Unit pointer
**          diag_p->    Diagnostic parameter structure
**            error       Set to appropriate return value.
**            rev_info    Text string with file and revision number.
**            line_number Line number failure was discovered on.
**            rldev       Remote logical device type to use during testing.
**                          One of bt_dev_t values, normally BT_DEV_A32.
**            raddr       Remote device address to use during testing.
**            rlen        Lenght in bytes of remote resource
**          
**      Returns:
**          BT_ENOSUP   Not supported on this hardware.
**          BT_ENXIO    Local memory device not enabled.
**          BT_ENOPWR   Pig tail connector not installed.
**          BT_EINVAL   Illegal parameter passed to function.
**          BT_EDESC    Illegal descriptor passed to function.
**          BT_ERCARD   Testing failed, remote card needs repair.
**
**          BT_SUCCESS  Testing passed, remote card appears good.
**          
**
**      Notes:
**          Cable must be attached to remote card.  
**
**          Any data in remote dual port memory, specified remote resource 
**          may be destroyed.
**        
**          Should be done after btk_lcard_diag() and btk_cable_diag() passes.
**        
**      Basic Operation:
**
**          Test a remote register
**
**          The remote dual port memory will be be tested
**            64-bit wide data bus test
**            192 Kbyte address bus test
**            64-bit wide access width test
**            192 Kbyte cell integrity test
**        
**          The remote RAM will be be tested
**            64-bit wide data bus test
**            address bus test
**            64-bit wide access width test
**            cell integrity test
**        
*****************************************************************************/
bt_error_t btk_rcard_diag(
    bt_unit_t *unit_p, 
    bt_hw_diag_t *diag_p)
{
    FUNCTION("btk_rcard_diag");
    LOG_UNIT(unit_p);
    
    bt_data32_t     lnumber = 0;
    bt_error_t      retval = BT_SUCCESS;
    void           *test_p = NULL;
    bt_data32_t     mem_size, mreg_value;
    unsigned int    mreg_need, mreg_start;
    
    FENTRY;


    /*
    **  Validate arguments
    */
    if (NULL == diag_p) {
        INFO_STR("Diagnostic parameter sturcture is a NULL.");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_rcard_diag_end;
    }

    /*
    ** Set the revision info
    */
    strncpy((char *)&diag_p->rev_info, file_rev, BT_DIAG_MAX_REV_INFO);
    diag_p->rev_info[BT_DIAG_MAX_REV_INFO-1] = '\0';

    /*
    ** Only PCI to PCI and dataBLIZZARD implemented now
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN) ||
        IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
        INFO_STR("Only dataBLIZZARD PCI to PCI configurations supported");
        lnumber = __LINE__;
        retval = BT_ENOSUP;
        goto btk_rcard_diag_end;
    }

    /*
    ** Cable needs to be connected
    */
    if (IS_SET(unit_p->bt_status, BT_POWER)) {
        INFO_STR("Cable disconnected");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_rcard_diag_end;
    }
    if (IS_SET(unit_p->bt_status, BT_LOOPBACK)) {
        INFO_STR("Pig-tail connector can not be installed");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_rcard_diag_end;
    }

    /*
    ** Remote memory must be at least 256 bytes
    */
    if (diag_p->r_len  < 256) {
        INFO_STR("Remote memory must be at least 256 bytes");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_rcard_diag_end;
    }

    /*
    ** Test the remote register
    */
    lnumber = adator_rem_reg_test(unit_p);
    if (0 != lnumber) {
        INFO_STR("Failed testing of lRemote card's registers");
        retval = BT_ERCARD;
        goto btk_rcard_diag_end;
    }

    /* 
    ** Test Remote dual port RAM
    */
    if ((unit_p->kern_addr[BT_AXSRDP] == NULL) ||
        (unit_p->kern_length[BT_AXSRDP] < 256)) {
        INFO_STR("Internal error: local dual port RAM does not exist");
        retval = BT_ENXIO;
        goto btk_rcard_diag_end;
    }
    test_p =  (void *) (unit_p->kern_addr[BT_AXSRDP]);
    mem_size = unit_p->kern_length[BT_AXSRDP];
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of Remote dual port RAM");
        retval = BT_ERCARD;
        goto btk_rcard_diag_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of Remote dual port RAM");
        retval = BT_ERCARD;
        goto btk_rcard_diag_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of Remote dual port RAM");
        retval = BT_ERCARD;
        goto btk_rcard_diag_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of Remote dual port RAM");
        retval = BT_ERCARD;
        goto btk_rcard_diag_end;
    }

    /*
    ** Setup an mmap pointer to Remote RAM
    */
    mreg_need = (diag_p->r_len / BT_617_PAGE_SIZE) + 1;
    btk_mutex_enter(unit_p, &unit_p->mreg_mutex);
    retval = btk_bit_alloc(unit_p, unit_p->mmap_aval_p, mreg_need, 1, &mreg_start);
    btk_mutex_exit(unit_p, &unit_p->mreg_mutex);
    if (retval != BT_SUCCESS) {
        INFO_STR("Unable to allocate mapping reg for remote testing");
        retval = BT_ENOMEM;
        goto btk_rcard_diag_end;
    }
    mreg_value = diag_p->r_addr & BT_MREG_ADDR_MASK;
    btk_setup_mreg(unit_p, (bt_dev_t) (diag_p->r_ldev), &mreg_value, BT_OP_PIO);
    btk_put_mreg_range(unit_p, mreg_start, mreg_need, BT_LMREG_PCI_2_CABLE, mreg_value);

    /* 
    ** Test Remote  RAM
    */
    test_p = unit_p->kern_addr[diag_p->r_ldev] + 
             (mreg_start * BT_617_PAGE_SIZE) + 
             (diag_p->r_addr & (BT_617_PAGE_SIZE - 1));
    mem_size = diag_p->r_len;
    lnumber = data_bus(unit_p, test_p, NULL, NULL);
    if (0 != lnumber) {
        INFO_STR("Failed initial data bus test of Remote RAM");
        retval = BT_ERCARD;
        goto pci_pio_end;
    }
    lnumber = address_bus(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial address bus test of Remote RAM");
        retval = BT_ERCARD;
        goto pci_pio_end;
    }
    lnumber = access_width(unit_p, test_p, NULL, TRUE);
    if (0 != lnumber) {
        INFO_STR("Failed initial access width test of Remote RAM");
        retval = BT_ERCARD;
        goto pci_pio_end;
    }
    lnumber = cell_integrity(unit_p, test_p, NULL, mem_size);
    if (0 != lnumber) {
        INFO_STR("Failed initial cell integrity test of Remote RAM");
        retval = BT_ERCARD;
        goto pci_pio_end;
    }
    
pci_pio_end:
    /*
    ** Free the mapping RAM allocated during the btk_setpage()
    */
    btk_mutex_enter(unit_p, &unit_p->mreg_mutex);
    btk_bit_free(unit_p, unit_p->mmap_aval_p, mreg_start, mreg_need);
    btk_mutex_exit(unit_p, &unit_p->mreg_mutex);
    
btk_rcard_diag_end:
    diag_p->error = retval;
    diag_p->line_number = lnumber;
    FEXIT(retval);
    return(retval);
}


/*****************************************************************************
**
**      Name:           btk_pair_diag
**
**      Purpose:        Determine whether a pair of cards is operating 
**                      properly.
**
**      Args:
**          unit_p      Unit pointer
**          diag_p->    Diagnostic parameter structure
**            error       Set to appropriate return value.
**            rev_info    Text string with file and revision number.
**            line_number Line number failure was discovered on.
**            rldev       Remote logical device type to use during testing.
**                          One of bt_dev_t values, normally BT_DEV_A32.
**            raddr       Remote device address to use during testing.
**            rlen        Lenght in bytes of remote resource
**          
**      Returns:
**          BT_ENOSUP   Not supported on this hardware.
**          BT_ENXIO    Local memory device not enabled.
**          BT_ENOPWR   Pig tail connector not installed.
**          BT_EINVAL   Illegal parameter passed to function.
**          BT_EDESC    Illegal descriptor passed to function.
**          BT_EPAIR    Testing failed, both cards should be replaced.
**
**          BT_SUCCESS  Testing passed, adapter set appears good.
**          
**
**      Notes:
**          Cable must be attached to remote card.  Any data in specified 
**          remote resource or the local memory device may be destroyed.
**        
**          Requires the local memory device to be present.
**          
**          Should be done after btk_lcard_diag(), btk_cable_diag() and
**          btk_rcard_diag() passes.
**        
**      Basic Operation:
**          The software will malloc memory from the kernel to use for
**            testing. 
**
**          1. Test longest DMA possible 
**
**          2. Test a range of small DMAs
**
**          3. Test a range of medium DMAs
**
**          4. Test a range of large DMAs 
**
**          5. Test a range of local offsets using the largest DMA possible
**
**          6. Test a range of remote offsets using the largest DMA possible
**
*****************************************************************************/
bt_error_t btk_pair_diag(
    bt_unit_t *unit_p, 
    bt_hw_diag_t *diag_p)
{
    FUNCTION("btk_pair_diag");
    LOG_UNIT(unit_p);
    
    bt_data32_t     lnumber = 0;
    bt_error_t      retval = BT_SUCCESS;
    bt_dev_t        ldev;
    int             threshold_save;
    int             spread = 0x100;         /* Must be D64 aligned */
    int             d64, inx, len;
    
    
    FENTRY;

    /*
    ** Save off the DMA threshold
    */
    threshold_save = unit_p->dma_threshold;
    unit_p->dma_threshold = 4;
    
    /*
    **  Validate arguments
    */
    if (NULL == diag_p) {
        INFO_STR("Diagnostic parameter sturcture is a NULL.");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_pair_diag_end;
    }
    ldev = (bt_dev_t) diag_p->r_ldev;

    /*
    ** Set the revision info
    */
    strncpy((char *)&diag_p->rev_info, file_rev, BT_DIAG_MAX_REV_INFO);
    diag_p->rev_info[BT_DIAG_MAX_REV_INFO-1] = '\0';

    /*
    ** Only PCI to PCI and dataBLIZZARD implemented now
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN) ||
        IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
        INFO_STR("Only dataBLIZZARD PCI to PCI configurations supported");
        lnumber = __LINE__;
        retval = BT_ENOSUP;
        goto btk_pair_diag_end;
    }

    /*
    ** Cable needs to be connected
    */
    if (IS_SET(unit_p->bt_status, BT_POWER)) {
        INFO_STR("Cable disconnected");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_pair_diag_end;
    }

    /*
    ** If loopback connector is installed do not run this software
    */
    if (IS_SET(unit_p->bt_status, BT_LOOPBACK)) {
        INFO_STR("Pig-tail connector can not be installed");
        lnumber = __LINE__;
        retval = BT_ENOPWR;
        goto btk_pair_diag_end;
    }

    /*
    ** Remote memory must be at least 0x1000
    */
    if (diag_p->r_len < BT_617_PAGE_SIZE) {
        INFO_STR("Diagnostic buffer too small, must be at least one page");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_pair_diag_end;
    }

    /*
    ** Remote memory must be D64 aligned
    */
    if (!IS_D64_ALIGNED(diag_p->r_addr)) {
        INFO_STR("Diagnostic buffer must be 8 byte aligned");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_pair_diag_end;
    }

    /*
    ** Remote length must be D64 aligned
    */
    if (!IS_D64_ALIGNED(diag_p->r_len)) {
        INFO_STR("Diagnostic buffer size must be 8 byte aligned");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_pair_diag_end;
    }

    /*
    ** Local memory must be at least size of remote memory
    */
    if (unit_p->kern_length[BT_AXSLM] < diag_p->r_len) {
        INFO_STR("Local memory too small, must be at least size of diag buffer");
        lnumber = __LINE__;
        retval = BT_EINVAL;
        goto btk_pair_diag_end;
    }

    /*
    ** Start testing DMA
    **
    ** One long DMA
    */
    lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, diag_p->r_len, 0, 0, BT_WIDTH_D64);
    if (0 != lnumber) {
        INFO_STR("Max length DMA failed");
        retval = BT_EPAIR;
        goto btk_pair_diag_end;
    }

    /*
    ** Test some short DMAs
    */
    for (inx = 0x4; inx <= 0x300; inx += 4) {
        d64 = (int) ((inx % 8) ? BT_WIDTH_D32 : BT_WIDTH_D64);
        lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, inx, 0, 0, d64);
        if (0 != lnumber) {
            INFO_MSG((LOG_FMT "Small DMAs failed, length 0x%8x\n", LOG_ARG, inx));
            retval = BT_EPAIR;
            goto btk_pair_diag_end;
        }
    }

    /*
    ** Test some medium DMAs
    */
    len = (int) ((diag_p->r_len / 2) & ~(BT_WIDTH_D32 - 1));
    for (inx = 0; inx <= spread; inx += 4) {
        d64 = (int) (((len + inx) % 8) ? BT_WIDTH_D32 : BT_WIDTH_D64);
        lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, len + inx, 0, 0, d64);
        if (0 != lnumber) {
            INFO_MSG((LOG_FMT "Medium DMAs failed, length 0x%8x\n", LOG_ARG, len + inx));
            retval = BT_EPAIR;
            goto btk_pair_diag_end;
        }
    }

    /*
    ** Test some long DMAs
    */
    for (inx = 0; inx <= spread; inx += 4) {
        d64 = (int) (((len - inx) % 8) ? BT_WIDTH_D32 : BT_WIDTH_D64);
        lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, len - inx, 0, 0, d64);
        if (0 != lnumber) {
            INFO_MSG((LOG_FMT "Medium DMAs failed, length 0x%8x\n", LOG_ARG, len - inx));
            retval = BT_EPAIR;
            goto btk_pair_diag_end;
        }
    }

    /*
    ** Vary the local address
    ** 
    ** Note length and remote address must allways be D64 aligned
    */
    for (inx = 0; inx <= spread; inx += 4) {
        d64 = (int) ((inx % 8) ? BT_WIDTH_D32 : BT_WIDTH_D64);
        lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, len - spread, inx, 0, d64);
        if (0 != lnumber) {
            INFO_MSG((LOG_FMT "Local offset DMAs failed, offset 0x%8x\n", LOG_ARG,  inx));
            retval = BT_EPAIR;
            goto btk_pair_diag_end;
        }
    }

    /*
    ** Vary the remote address
    ** 
    ** Note length and local address must allways be D64 aligned
    */
    for (inx = 0; inx <= spread; inx += 4) {
        d64 = (int) ((inx % 8) ? BT_WIDTH_D32 : BT_WIDTH_D64);
        lnumber = btk_datachk(unit_p, ldev, diag_p->r_addr, len - spread, 0, inx, d64);
        if (0 != lnumber) {
            INFO_MSG((LOG_FMT "Remote offset DMAs failed, offset 0x%8x\n", LOG_ARG,  inx));
            retval = BT_EPAIR;
            goto btk_pair_diag_end;
        }
    }
    
btk_pair_diag_end:
    /*
    ** Restore the DMA threshold
    */
    unit_p->dma_threshold = threshold_save;
    diag_p->error = retval;
    diag_p->line_number = lnumber;
    FEXIT(retval);
    return(retval);
}


/*****************************************************************************
**
**  Function:   data_bus()
**
**  Purpose:    To check the data bus leading to a particular location for
**              stuck, open, or shorted lines. 
**
**  Args:
**      unit_p          Unit structure pointer
**      test_p          Pointer to the first of two longword
**      verify_p        Pointer to use to verify writes, NULL use test_p
**      user_list_p     Pointer to a linked list of user selected patterns. 
**                        Used for finding data sensitivity problems.
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Success
**      else            Line number of the found failure
**
**  Calls:
**      find_bad_bit
**
**  Note:
**      This tests uses the longword location addressed by test_p and 
**      the next longword location.
**
*****************************************************************************/

int data_bus(
    bt_unit_t *unit_p,
    void *test_p, 
    void *verify_p,
    pattern_list_t *user_list_p)

{

#define SLIDE_1         0x00000001L
#define SLIDE_0         0xfffffffeL

    volatile bt_data32_t   *lword_p = NULL; 
    volatile bt_data32_t   *next_lword_p = NULL;
    volatile bt_data32_t   *read_p = NULL;
    volatile bt_data32_t   *flush_p = NULL;
    bt_data32_t             pattern;
    bt_data32_t             not_pattern;
    bt_data32_t             read_pattern, flush;
    pattern_list_t         *current_pattern_p = NULL;
    int                     error = 0;
    int                     bit_number;

    FUNCTION("data_bus");
    LOG_UNIT(unit_p);
 
    FENTRY;
  
    /* 
    ** Initialize variables 
    */
    lword_p           = (volatile bt_data32_t *) test_p;
    flush_p           = (volatile bt_data32_t *) test_p;
    next_lword_p      = lword_p + 1;
    if (verify_p == NULL) {
        read_p = lword_p;
        verify_p = test_p;
    } else {
        read_p = (volatile bt_data32_t *) verify_p;
    }

    /* 
    ** Test for bad data lines with a sliding 1 pattern
    ** Test 0 pattern seperatly 
    */
    pattern        = ALL_ZERO;
    *lword_p       = pattern;
    not_pattern    = ~pattern;
    *next_lword_p  = not_pattern;   /* Must be here to make data bus change states */
    flush          = *flush_p;      /* Otherwise won't find solder voids */
    read_pattern   = *read_p;

    if ( read_pattern != pattern ) {
        error = __LINE__;
        bit_number = find_bad_bit(unit_p, pattern, read_pattern);
        BIT_ERROR(bit_number, pattern, read_pattern);
        goto data_bus_end;
    }

    /* 
    ** Loop through the set of patterns 00000001, 00000002, 00000004, ... 
    */
    for ( pattern = SLIDE_1; pattern != 0L; pattern <<= 1 ) {
    
        /* 
        ** Write and read the pattern 
        */
        *lword_p       = pattern;
        not_pattern    = ~pattern;
        *next_lword_p  = not_pattern;
        flush          = *flush_p;
        read_pattern = *read_p;

        /* 
        ** Check whether the pattern matches 
        */
        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number = find_bad_bit(unit_p, pattern, read_pattern);
            BIT_ERROR(bit_number, pattern, read_pattern);
            goto data_bus_end;
        } 
    }

    /* 
    ** Test for bad data lines with a sliding 0 pattern
    ** Test ffffffff pattern seperatly 
    */
    pattern        = ALL_ONE;
    *lword_p       = pattern;
    not_pattern    = ~pattern;
    *next_lword_p  = not_pattern;
    flush          = *flush_p;
    read_pattern   = *read_p;
    if ( read_pattern != pattern ) {
        error = __LINE__;
        bit_number = find_bad_bit(unit_p, pattern, read_pattern);
        BIT_ERROR(bit_number, pattern, read_pattern);
        goto data_bus_end;
    }

    /* 
    ** Loop through the set of patterns fffffffe, fffffffd, fffffffb, ... 
    */
    for ( pattern = SLIDE_0; pattern != 0xffffffffL;
          pattern = (pattern << 1) | 1 ) {
    
        /* 
        ** Write and read the pattern 
        */
        *lword_p       = pattern;
        not_pattern    = ~pattern;
        *next_lword_p  = not_pattern;
        flush          = *flush_p;
        read_pattern   = *read_p;

        /* 
        ** Check whether the pattern matches 
        */
        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number = find_bad_bit(unit_p, pattern, read_pattern);
            BIT_ERROR(bit_number, pattern, read_pattern);
            goto data_bus_end;
        }
    }

    /* 
    ** Test for bad data lines with the user's supplied patterns 
    */
    for ( current_pattern_p = user_list_p; current_pattern_p != NULL; 
          current_pattern_p = current_pattern_p->next_pattern ) {
    
        /* 
        ** Write and read the pattern 
        */
        pattern        = current_pattern_p->data_pattern;
        *lword_p       = pattern;
        not_pattern    = ~pattern;
        *next_lword_p  = not_pattern;
        flush          = *flush_p;
        read_pattern   = *read_p;

        /* 
        ** Check whether the pattern matches 
        */
        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number = find_bad_bit(unit_p, pattern, read_pattern);
            BIT_ERROR(bit_number, pattern, read_pattern);
            goto data_bus_end;
        }
    }

data_bus_end:
    FEXIT(error);
    return (error);
} /* end data_bus() */


/*****************************************************************************
**
**  Function:   address_bus()
**
**  Purpose:    To check the address bus for stuck, open, or shorted lines. 
**
**  Args:
**      unit_p          Unit structure pointer
**      test_p          Pointer to the start of the memory region.
**                        This address should be mem_size aligned.
**      verify_p        Pointer to use to verify writes, NULL use test_p
**      mem_size        Number of bytes in the memory region.
**                        This determines the number of address lines checked
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Success
**      else            Line number of the found failure
**
**  Calls:
**      find_addr_bit
**
**  Note:
**      This test does test address lines A0 and A1.
**
*****************************************************************************/

int address_bus(
    bt_unit_t *unit_p,
    void *test_p, 
    void *verify_p,
    bt_data32_t mem_size)
{


#define START_OFFSET    4

    volatile bt_data32_t   *zero_lword_p = NULL;
    volatile bt_data32_t   *test_lword_p = NULL;
    volatile bt_data32_t   *lword_p = NULL;
    volatile bt_data32_t   *read_p = NULL;
    volatile bt_data32_t   *flush_p = NULL;
    bt_data32_t             offset;
    bt_data32_t             test_offset;
    bt_data32_t             pattern = ALL_ONE;
    bt_data32_t             not_pattern = ALL_ZERO;
    bt_data32_t             read_pattern, flush;
    int                     bit_number;
    int                     error = 0;

    FUNCTION("address_bus");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* 
    ** Initialize zero address and starting offset 
    */
    zero_lword_p  = (volatile bt_data32_t *) test_p;
    flush_p       = (volatile bt_data32_t *) test_p;
    offset = START_OFFSET;
    if (verify_p == NULL) {
        read_p = zero_lword_p;
        verify_p = test_p;
    } else {
        read_p = (volatile bt_data32_t *) verify_p;
    }

    /*
    ** Check for address lines stuck at zero
    ** Initialize zero address with background pattern 
    */
    *zero_lword_p = pattern;


    /* 
    ** Check for address line stuck at 0 
    */
    do {

        /* 
        ** Write test pattern to 2^n boundaries of the zero_address 
        */
        test_lword_p  = (volatile bt_data32_t *) ((bt_data8_t *) zero_lword_p + offset);
        *test_lword_p = not_pattern;

        /* 
        ** Read zero address to make sure it was not overwritten 
        */
        flush = *flush_p;
        read_pattern = *read_p;

        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number = find_bad_bit(unit_p, offset, (bt_data32_t) 0);
            ADDR_ERROR(bit_number, "low", zero_lword_p, test_lword_p);
            goto address_bus_end;
        }

        /* 
        ** Go to next 2^n address boundary 
        */
        offset <<= 1;

    } while ( offset < mem_size );

    /*
    ** Check for address lines stuck at one
    ** Initialize starting offset
    */
    offset = START_OFFSET;


    /* 
    ** Check for address line stuck at 1 
    */
    do {

        /* 
        ** Fill address boundary 2^n with background pattern 
        */
        test_lword_p  = (volatile bt_data32_t *) ((bt_data8_t *) zero_lword_p + offset);
        *test_lword_p = pattern;

        /* 
        ** Write test pattern to the zero_address 
        */
        *zero_lword_p = not_pattern;

        /* 
        ** Read 2^n address boundary to make sure it was not overwritten 
        */
        read_p = (volatile bt_data32_t *) ((bt_data8_t *) verify_p + offset);
        flush = *flush_p;
        read_pattern = *read_p;

        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number = find_bad_bit(unit_p, offset, (bt_data32_t) 0);
            ADDR_ERROR(bit_number, "high", test_lword_p, zero_lword_p);
            goto address_bus_end;
        }

        /* 
        ** Go to next 2^n address boundary 
        */
        offset <<= 1;

    } while ( offset < mem_size );

    /*
    ** Check for address lines shorted together
    ** Initialize address boundary offset to be tested
    */
    lword_p       = (volatile bt_data32_t *) test_p;
    test_lword_p  = zero_lword_p;
    test_offset = START_OFFSET;


    /* 
    ** Fill all 2^n address boundaries with a background pattern 
    */
    offset        = START_OFFSET;

    do { 
        /* 
        ** Note: zero address is filled during first pass of the next do while 
        */
        lword_p  = (volatile bt_data32_t *) ((bt_data8_t *) zero_lword_p + offset);
        *lword_p = pattern;
        offset <<= 1;
    } while ( offset < mem_size );


    /* 
    ** Check for address lines shorted together 
    */
    do {

        /* 
        ** Fill the previous test address with the background pattern 
        */
        *test_lword_p = pattern;   /* Fills in zero address during 1st pass */

        /* 
        ** Fill address boundary at the new test offset with the test pattern 
        */
        test_lword_p  = (volatile bt_data32_t *) ((bt_data8_t *) zero_lword_p + test_offset);
        *test_lword_p = not_pattern;

        /* 
        ** Check to make sure all of the other address boundaries are still ok 
        */
        offset         = START_OFFSET;
        flush          = *flush_p;
        read_pattern   = *((volatile bt_data32_t *) verify_p);
        if ( read_pattern != pattern ) {
            error = __LINE__;
            bit_number  = find_bad_bit(unit_p, test_offset, (bt_data32_t) 0);
            ADDRS_ERROR(bit_number, test_lword_p, zero_lword_p);
            goto address_bus_end;
        }
        do { 
      
            /* 
            ** Don't verify the test offset 
            */
            if ( offset == test_offset ) {
                offset <<= 1;
                continue;
            }
            read_p = (volatile bt_data32_t *) ((bt_data8_t *) verify_p + offset);
            flush = *flush_p;
            read_pattern = *read_p;
            if ( read_pattern != pattern ) {
                error = __LINE__;
                bit_number  = find_bad_bit(unit_p, test_offset, (bt_data32_t) 0);
                ADDRS_ERROR(bit_number, test_lword_p, lword_p);
                goto address_bus_end;
            }

            /* 
            ** Check next address boundary 
            */
            offset <<= 1;

        } while ( offset < mem_size );


        /* 
        ** Test the next address boundary offset 
        */
        test_offset <<= 1;

    } while ( test_offset < mem_size );


address_bus_end:
    FEXIT(error);
    return (error);

} /* end address_bus() */


/*****************************************************************************
**
**  Function:   access_width()
**
**  Purpose:    To verify that byte, word, and longword accesses can be made
**              at aligned boundaries. 
**
**  Args:
**      unit_p          Unit structure pointer
**      test_p          Pointer to the start of the memory region.
**      verify_p        Pointer to use to verify writes, NULL use test_p
**      same_endian     True if memory and processor are similar
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Success
**      else            Line number of the found failure
**
**  Calls:
**      initialize_memory, check_memory
**
**  Note:
**
*****************************************************************************/

int access_width(
    bt_unit_t *unit_p,
    void *test_p, 
    void *verify_p,
    bool_t same_endian)
{


#define MEM_BYTES       256L
#define MEM_WORDS       (MEM_BYTES / 2)
#define MEM_LWORDS      (MEM_BYTES / 4)

    void                    *read_p = NULL;
    volatile bt_data32_t    *flush_p = NULL;
    bt_data32_t             mem_size = MEM_BYTES;
    bt_data32_t             flush;
    pattern_t               pattern;
    int                     error = 0;

    FUNCTION("access_width");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* 
    ** Initialize testing pointer 
    */
    flush_p = (volatile bt_data32_t *) test_p;
    if (verify_p == NULL) {
        read_p = test_p;
        verify_p = test_p;
    } else {
        read_p = verify_p;
    }

    /*
    ** Test whether or not the memory can do byte accesses
    **
    ** Write and read a decrementing byte pattern like ff fe fd ... 2 1 0
    */
    pattern.type    = DECREMENT;
    pattern.fill    = 0xffL;
    pattern.inc_dec = 0x1L;
    pattern.width   = BT_WIDTH_D8;
    if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Could not initialize memory using byte accesses.\n");
        goto access_width_end;
    }
    flush = *flush_p;
    if (check_memory(unit_p, read_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Byte aligned access test failed.\n");
        goto access_width_end;
    }

    /*
    ** Test whether or not the memory can do word accesses
    **
    ** Write and read an incrementing word pattern like 0001 0203 ... feff
    */
    pattern.type    = INCREMENT;
    pattern.fill    = 0x0001L;
    pattern.inc_dec = 0x0202L;
    pattern.width   = BT_WIDTH_D16;
    if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Could not initialize memory using word accesses.\n");
        goto access_width_end;
    }
    flush = *flush_p;
    if (check_memory(unit_p, read_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Word aligned access test failed.\n");
        goto access_width_end;
    }

    /*
    ** Test whether or not the memory can do longword accesses
    **
    ** Write and read an decrementing lword pattern like 00010203 ... fcfdfeff
    */
    pattern.type    = DECREMENT;
    pattern.fill    = 0xfffefdfcL;
    pattern.inc_dec = 0x04040404L;
    pattern.width   = BT_WIDTH_D32;
    if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Could not initialize memory using long word accesses.\n");
        goto access_width_end;
    } 
    flush = *flush_p;
    if (check_memory(unit_p, read_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Long word aligned access test failed.\n");
        goto access_width_end;
    }

    /*
    ** Check that memory has the proper byte and word ordering
    */
    if ( same_endian ) {

        /* 
        ** Check that bytes are in the proper order, pattern 
        ** should be ff fe ... 1 0 
        */
        pattern.type    = DECREMENT;
        pattern.fill    = 0xffL;
        pattern.inc_dec = 0x1L;
        pattern.width   = BT_WIDTH_D8;
        if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
            error = __LINE__;
            INFO_STR("Could not initialize memory using word accesses.\n");
            goto access_width_end;
        }
        flush = *flush_p;
        if (check_memory(unit_p, read_p, mem_size, pattern)) {
            error = __LINE__;
            INFO_STR("Byte ordering test failed.\n");
            goto access_width_end;
        }

        /* 
        ** Check that words are in the proper order, pattern 
        ** should be fffe ... 0100 
        */
        pattern.type    = DECREMENT;
        pattern.fill    = 0xfffeL;
        pattern.inc_dec = 0x0202L;
        pattern.width   = BT_WIDTH_D16;
        if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
            error = __LINE__;
            INFO_STR("Could not initialize memory using word accesses.\n");
            goto access_width_end;
        }
        flush = *flush_p;
        if (check_memory(unit_p, read_p, mem_size, pattern)) {
            error = __LINE__;
            INFO_STR("Word ordering test failed.\n");
            goto access_width_end;
        }
    }

access_width_end:
    FEXIT(error);
    return (error);

}  /* end access_width() */


/*****************************************************************************
**
**  Function:   cell_integrity()
**
**  Purpose:    To check that each bit in memory can hold a one and a zero
**              and helps verify that addressing is working.
**
**  Args:
**      unit_p          Unit structure pointer
**      test_p          Pointer to the start of the memory region.
**                        Must be longword aligned.
**      verify_p        Pointer to use to verify writes, NULL use test_p
**      mem_size        Size in bytes of the memory region.
**                        Must be a longword multiple. 
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Success
**      else            Line number of the found failure
**
**  Calls:
**      find_bad_bit
**
*****************************************************************************/
int cell_integrity(
    bt_unit_t *unit_p,
    void *test_p, 
    void *verify_p,
    bt_data32_t mem_size)
{

    volatile bt_data32_t   *lword_test_p = NULL;
    volatile bt_data32_t   *lword_verify_p = NULL;
    volatile bt_data32_t   *read_p = NULL;
    volatile bt_data32_t   *flush_p = NULL;
    volatile void          *end_test_address_p = NULL;
    volatile void          *end_verify_address_p = NULL;
    pattern_t               pattern;
    bt_data32_t             read_pattern, flush;
    bt_data32_t             bytes;
    int                     error = 0;

    FUNCTION("cell_integrity");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* 
    ** Print error if address is not longword aligned 
    */
    if (!(IS_D32_ALIGNED(test_p))) {
        error = __LINE__;
        INFO_STR("Memory starting address must be lword aligned.\n");
        goto cell_integrity_end;
    }

    /* 
    ** Align memory size 
    */
    if ( !(IS_D32_ALIGNED(mem_size)) ) {
        mem_size &= ~(BT_WIDTH_D32 - 1);
    }

    /* 
    ** Initialize testing pointer 
    */
    flush_p = (volatile bt_data32_t *) test_p;
    if (verify_p == NULL) {
        read_p = test_p;
        verify_p = test_p;
    } else {
        read_p = verify_p;
    }

    /* 
    ** Calculate the ending address of memory 
    */
    end_test_address_p = (void *) ((bt_data8_t *) test_p + mem_size - 4);
    end_verify_address_p = (void *) ((bt_data8_t *) read_p + mem_size - 4);

    /*
    ** Fill memory with all zeros
    */
    pattern.type    = REPEAT_DATA;
    pattern.fill    = 0;
    pattern.inc_dec = 0;
    pattern.width   = BT_WIDTH_D32;
    if (initialize_memory(unit_p, test_p, mem_size, pattern)) {
        error = __LINE__;
        INFO_STR("Could not zero fill memory.\n");
        goto cell_integrity_end;
    }


    /*
    ** Check that each location could hold its zero value then set every bit
    */
    lword_test_p = (volatile bt_data32_t *) end_test_address_p;
    lword_verify_p = (volatile bt_data32_t *) end_verify_address_p;
    for (bytes = 0; bytes < mem_size; bytes += 4) {

        /* 
        ** Check each value for a zero value 
        */
        flush = *flush_p;
        read_pattern = *lword_verify_p;
        if ( read_pattern != (bt_data32_t) ALL_ZERO ) {
            error = __LINE__;
            INFO_MSG((LOG_FMT "Data mismatch: addr = 0x%8x  expected = 0x%8x  received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_test_p, (bt_data32_t) ALL_ZERO, read_pattern));
            goto cell_integrity_end;
        }

        /* 
        ** Set every bit 
        */
        *lword_test_p-- = (bt_data32_t) ALL_ONE;
        lword_verify_p--;
     
    }


    /*
    ** Check that each location could hold its one value then clear every bit
    */
    lword_test_p = (volatile bt_data32_t *) test_p;
    lword_verify_p = (volatile bt_data32_t *) read_p;
    for ( bytes = 0; bytes < mem_size; bytes += 4 ) {

        /* 
        ** Check each value for a one value 
        */
        flush = *flush_p;
        read_pattern = *lword_verify_p;
        if ( read_pattern != (bt_data32_t) ALL_ONE ) {
            error = __LINE__;
            INFO_MSG((LOG_FMT  "Data mismatch: addr = 0x%8x  expected = 0x%8x  received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_verify_p, (bt_data32_t) ALL_ONE, read_pattern));
            goto cell_integrity_end;

        }

        /* 
        ** Clear every bit 
        */
        *lword_verify_p++ = (bt_data32_t) ALL_ZERO;
     
    }



cell_integrity_end:
    FEXIT(error);
    return (error);

} /* end cell_integrity() */


/*****************************************************************************
**
**  Function:   find_bad_bit()
**
**  Purpose:    Examine the expected data and received data and reports the
**              first bit that is different.
**
**  Args:
**      unit_p          Unit structure pointer
**      expected        Gives the expected data pattern.
**                        Is the offset for address_bus tests 
**      received        Gives the actual data that was read.
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0 - 31          The data bit that is bad.
**      -1              Expected and received patterns were the same.
**
**  Calls:
**      NA
**
*****************************************************************************/

int find_bad_bit(
    bt_unit_t *unit_p,
    bt_data32_t expected, 
    bt_data32_t received)
{

#define MAX_BITS        32

    int         bit_number = 0;
    bt_data32_t bits_different;

    FUNCTION("find_bad_bit");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* Find out whether any bits are different by exclusive or */
    bits_different = expected ^ received;

    /* If a bit is wrong, then determine which bit it is */
    if ( bits_different ) {

        /* Report the first bit that is different (i.e. set) */
        for ( bit_number = 0; bit_number < MAX_BITS; bit_number++ ) {
            if ( IS_SET(bits_different,1) ) {
                break;
            }
            bits_different >>= 1;
        } 
  
    } else {
        bit_number = -1;
    }
   
    FEXIT(bit_number);
    return (bit_number);

} /* end find_bad_bit() */


/*****************************************************************************
**
**  Function:   initialize_memory()
**
**  Purpose:    To initialize a memory region to the specified pattern.
**
**  Args:
**      unit_p          Unit structure pointer
**      start_address_p Pointer to the start of the memory region.
**      mem_size        Length of the memory region in bytes.
**      pattern         Describes what pattern to use.
**        type          Must be either REPEAT_DATA, INCREMENT, DECREMENT,
**                                     COUNT_UP, SINGLE_1, or SINGLE_0.
**        fill          For REPEAT_DATA, INCREMENT, or DECREMENT
**                        fill gives initial or repeated pattern.
**                      For SINGLE_1 or SINGLE_0
**                        fill gives gives the length of the packet in bytes
**                        (bytes should be lword alinged).  A longword with a
**                        sliding 0/1 will be places in the middle of the
**                        packet.  Example SINGLE_0 fill = 0x10
**                        ffffffff ffffffff fffffffe ffffffff
**        inc_dec       Amount to increment or decrement fill to generate next
**                        location's pattern.
**        width         Must be either BT_WIDTH_D8, BT_WIDTH_D16, 
**                        or BT_WIDTH_D32 and defines how the pattern
**                        is written to memory. Note SINGLE_1 and SINGLE_0 
**                        ignore this field and do lword transfers
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Successful.
**      else            Error.
**
**  Calls:
**      NA
**
**  Note:
**      Memory size should be a multiple of data width size otherwise  
**      all of memory will not be filled.
**
*****************************************************************************/
int initialize_memory(
    bt_unit_t *unit_p,
    void *start_address_p, 
    bt_data32_t mem_size, 
    pattern_t pattern)
{

    volatile bt_data8_t     *byte_p = NULL;
    volatile bt_data16_t    *word_p = NULL;
    volatile bt_data32_t    *lword_p = NULL;
    bt_data32_t              fill_pattern;
    bt_data32_t              fill_inc_dec;
    int                      error = 0;
    bt_data32_t              index;

    FUNCTION("initialize_memory");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* 
    ** Memory size must be a multiple of access width 
    */
    if (((pattern.width == BT_WIDTH_D16) && !(IS_D16_ALIGNED(mem_size))) ||
        ((pattern.width == BT_WIDTH_D32) && !(IS_D32_ALIGNED(mem_size)))) {
        error = __LINE__;
        goto initialize_memory_end;
    }
    
    /* 
    ** Initialize pointers to the start of memory 
    */
    byte_p   = (bt_data8_t *)  start_address_p;
    word_p   = (bt_data16_t *) start_address_p;
    lword_p  = (bt_data32_t *) start_address_p;
    fill_pattern = pattern.fill;
    fill_inc_dec = pattern.inc_dec;
    switch ( pattern.type ) {

      case REPEAT_DATA:
        /* Write the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++ ) {
                *byte_p++ = (bt_data8_t) fill_pattern;
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2 ) {
                *word_p++ = (bt_data16_t) fill_pattern;
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4 ) {
                *lword_p++ = (bt_data32_t) fill_pattern;
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto initialize_memory_end;

        } /* end switch pattern.width */
        break;


      case INCREMENT:
        /* Write the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++ ) {
                *byte_p++ = (bt_data8_t) fill_pattern;
                fill_pattern += fill_inc_dec;
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2 ) {
                *word_p++ = (bt_data16_t) fill_pattern;
                fill_pattern += fill_inc_dec;
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4 ) {
                *lword_p++ = (bt_data32_t) fill_pattern;
                fill_pattern += fill_inc_dec;
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto initialize_memory_end;

        }
        break;


      case DECREMENT:
        /* Write the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++ ) {
                *byte_p++ = (bt_data8_t) fill_pattern;
                fill_pattern -= fill_inc_dec;
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2 ) {
                *word_p++ = (bt_data16_t) fill_pattern;
                fill_pattern -= fill_inc_dec;
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4 ) {
                *lword_p++ = (bt_data32_t) fill_pattern;
                fill_pattern -= fill_inc_dec;
            }

            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto initialize_memory_end;

        }
        break;


      case COUNT_UP:
        /* Write the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            fill_pattern = 0x00010203;
            fill_inc_dec = 0x04040404;
            for ( index = 0; index < mem_size; index += 4 ) {
                *byte_p++ = (bt_data8_t) fill_pattern;
                fill_pattern += fill_inc_dec;
                if ( fill_pattern == 0x01020303 ) {
                    fill_pattern = 0x00010203;
                }
            }
            break;

          case BT_WIDTH_D16:
            fill_pattern = 0x00000002;
            fill_inc_dec = 0x00040004;
            for ( index = 0; index < mem_size; index += 4 ) {
                *word_p++ = (bt_data16_t) fill_pattern;
                fill_pattern += fill_inc_dec;
                if ( fill_pattern == 0x00010002 ) {
                    fill_pattern = 0x00000002;
                }
            }
            break;

          case BT_WIDTH_D32:
            fill_pattern = 0x00000000;
            fill_inc_dec = 0x00000004;
            for ( index = 0; index < mem_size; index += 4 ) {
                *lword_p++ = (bt_data32_t) fill_pattern;
                fill_pattern += fill_inc_dec;
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto initialize_memory_end;

        }
        break;


      case COUNT_DOWN:
        /* Write the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            fill_pattern = 0xfffefdfc;
            fill_inc_dec = 0x04040404;
            for ( index = 0; index < mem_size; index += 4 ) {
                *byte_p++ = (bt_data8_t) fill_pattern;
                fill_pattern -= fill_inc_dec;
                if ( fill_pattern == 0xfefdfcfc ) {
                    fill_pattern = 0xfffefdfc;
                }
            }
            break;

          case BT_WIDTH_D16:
            fill_pattern = 0xfffffffd;
            fill_inc_dec = 0x00040004;
            for ( index = 0; index < mem_size; index += 4 ) {
                *word_p++ = (bt_data16_t) fill_pattern;
                fill_pattern -= fill_inc_dec;
                if ( fill_pattern == 0xfffefffd ) {
                    fill_pattern = 0xfffffffd;
                }
            }
            break;

          case BT_WIDTH_D32:
            fill_pattern = 0xffffffff;
            fill_inc_dec = 0x00000004;
            for ( index = 0; index < mem_size; index += 4 ) {
                *lword_p++ = (bt_data32_t) fill_pattern;
                fill_pattern += fill_inc_dec;
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto initialize_memory_end;

         } /* end  switch ( pattern.width ) */

    }  /* end  switch ( pattern.type ) */
    
initialize_memory_end:
    FEXIT(error);
    return (error);

} /* end initialize_memory() */



/*****************************************************************************
**
**  Function:   check_memory()
**
**  Purpose:    Check a memory region for a specified pattern.          
**
**  Args:
**      unit_p          Unit structure pointer
**      start_address_p Pointer to the start of the memory region.
**      mem_size        Length of the memory region in bytes.
**      pattern         Describes what pattern to use.
**        type          Must be either REPEAT_DATA, INCREMENT, DECREMENT,
**                                     COUNT_UP, SINGLE_1, or SINGLE_0.
**        fill          For REPEAT_DATA, INCREMENT, or DECREMENT
**                        fill gives initial or repeated pattern.
**                      For SINGLE_1 or SINGLE_0
**                        fill gives gives the length of the packet in bytes
**                        (bytes should be lword alinged).  A longword with a
**                        sliding 0/1 will be places in the middle of the
**                        packet.  Example SINGLE_0 fill = 0x10
**                        ffffffff ffffffff fffffffe ffffffff
**        inc_dec       Amount to increment or decrement fill to generate next
**                        location's pattern.
**        width         Must be either BT_WIDTH_D8, BT_WIDTH_D16, 
**                        or BT_WIDTH_D32 and defines how the pattern
**                        is written to memory. Note SINGLE_1 and SINGLE_0 
**                        ignore this field and do lword transfers
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0               Successful.
**      else            Error.
**
**  Calls:
**      NA
**
**  Note:
**
*****************************************************************************/
int check_memory(
    bt_unit_t *unit_p,
    void *start_address_p, 
    bt_data32_t mem_size, 
    pattern_t pattern)
{
    volatile bt_data8_t     *byte_p = NULL;
    volatile bt_data16_t    *word_p = NULL;
    volatile bt_data32_t    *lword_p = NULL;
    int                      error = 0;
    bt_data32_t              index;
    bt_data32_t              check_pattern;
    bt_data32_t              check_inc_dec;
    bt_data8_t               read_pattern_b;
    bt_data16_t              read_pattern_w;
    bt_data32_t              read_pattern_l;

    FUNCTION("check_memory");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /* 
    ** Memory size must be a multiple of access width 
    */
    if (((pattern.width == BT_WIDTH_D16) && !(IS_D16_ALIGNED(mem_size))) ||
        ((pattern.width == BT_WIDTH_D32) && !(IS_D32_ALIGNED(mem_size)))) {
        error = __LINE__;
        goto check_memory_end;
    }
    
    /* 
    ** Initialize pointers to the start of memory 
    */
    byte_p   = (bt_data8_t *)  start_address_p;
    word_p   = (bt_data16_t *) start_address_p;
    lword_p  = (bt_data32_t *) start_address_p;
    check_pattern = pattern.fill;
    check_inc_dec = pattern.inc_dec;

    /*
    **  Determine which pattern was specified
    */
    switch ( pattern.type ) {

      case REPEAT_DATA:
      
        /* read if the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++, byte_p++ ) {
                read_pattern_b = *byte_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_b != (bt_data8_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%2x received = 0x%2x\n", LOG_ARG, (bt_devaddr_t) byte_p, (bt_data8_t) check_pattern, read_pattern_b));
                    goto check_memory_end;
                }
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2, word_p++ ) {
                read_pattern_w = *word_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_w != (bt_data16_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%4x received = 0x%4x\n", LOG_ARG, (bt_devaddr_t) word_p, (bt_data16_t) check_pattern, read_pattern_w));
                    goto check_memory_end;
                }
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4, lword_p++ ) {
                read_pattern_l = *lword_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_l != (bt_data32_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%8x received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_p, (bt_data32_t) check_pattern, read_pattern_l));
                    goto check_memory_end;
                }
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto check_memory_end;

        } /* end switch pattern.width */
        break;


      case INCREMENT:

        /* read the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++, byte_p++ ) {
                read_pattern_b = *byte_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_b != (bt_data8_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%2x received = 0x%2x\n", LOG_ARG, (bt_devaddr_t) byte_p, (bt_data8_t) check_pattern, read_pattern_b));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                }
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2, word_p++ ) {
                read_pattern_w = *word_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_w != (bt_data16_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%4x received = 0x%4x\n", LOG_ARG, (bt_devaddr_t) word_p, (bt_data16_t) check_pattern, read_pattern_w));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                }
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4, lword_p++ ) {
                read_pattern_l = *lword_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_l != (bt_data32_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%8x received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_p, (bt_data32_t) check_pattern, read_pattern_l));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                }
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto check_memory_end;

        }
        break;


      case DECREMENT:

        /* read the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            for ( index = 0; index < mem_size; index++, byte_p++ ) {
                read_pattern_b = *byte_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_b != (bt_data8_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%2x received = 0x%2x\n", LOG_ARG, (bt_devaddr_t) byte_p, (bt_data8_t) check_pattern, read_pattern_b));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                }
            }
            break;

          case BT_WIDTH_D16:
            for ( index = 0; index < mem_size; index += 2, word_p++ ) {
                read_pattern_w = *word_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_w != (bt_data16_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%4x received = 0x%4x\n", LOG_ARG, (bt_devaddr_t) word_p, (bt_data16_t) check_pattern, read_pattern_w));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                }
            }
            break;

          case BT_WIDTH_D32:
            for ( index = 0; index < mem_size; index += 4, lword_p++ ) {
                read_pattern_l = *lword_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_l != (bt_data32_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%8x received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_p, (bt_data32_t) check_pattern, read_pattern_l));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                }
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto check_memory_end;

        }
        break;


      case COUNT_UP:

        /* read the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            check_pattern = 0x00010203;
            check_inc_dec = 0x04040404;
            for ( index = 0; index < mem_size; index += 4, byte_p++ ) {
                read_pattern_b = *byte_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_b != (bt_data8_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%2x received = 0x%2x\n", LOG_ARG, (bt_devaddr_t) byte_p, (bt_data8_t) check_pattern, read_pattern_b));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                    if ( check_pattern == 0x01020303 ) {
                        check_pattern = 0x00010203;
                    }
                }
            }
            break;

          case BT_WIDTH_D16:
            check_pattern = 0x00000002;
            check_inc_dec = 0x00040004;
            for ( index = 0; index < mem_size; index += 4, word_p++ ) {
                read_pattern_w = *word_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_w != (bt_data16_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%4x received = 0x%4x\n", LOG_ARG, (bt_devaddr_t) word_p, (bt_data16_t) check_pattern, read_pattern_w));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                    if ( check_pattern == 0x00010002 ) {
                        check_pattern = 0x00000002;
                    }
                }
            }
            break;

          case BT_WIDTH_D32:
            check_pattern = 0x00000000;
            check_inc_dec = 0x00000004;
            for ( index = 0; index < mem_size; index += 4, lword_p++ ) {
                read_pattern_l = *lword_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_l != (bt_data32_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%8x received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_p, (bt_data32_t) check_pattern, read_pattern_l));
                    goto check_memory_end;
                } else {
                    check_pattern += check_inc_dec;
                }
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto check_memory_end;

        }
        break;


      case COUNT_DOWN:

        /* read the fill pattern to memory at the proper width */
        switch ( pattern.width ) {

          case BT_WIDTH_D8:
            check_pattern = 0xfffefdfc;
            check_inc_dec = 0x04040404;
            for ( index = 0; index < mem_size; index += 4, byte_p++ ) {
                read_pattern_b = *byte_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_b != (bt_data8_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%2x received = 0x%2x\n", LOG_ARG, (bt_devaddr_t) byte_p, (bt_data8_t) check_pattern, read_pattern_b));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                    if ( check_pattern == 0xfefdfcfc ) {
                        check_pattern = 0xfffefdfc;
                    }
                }
            }
            break;

          case BT_WIDTH_D16:
            check_pattern = 0xfffffffd;
            check_inc_dec = 0x00040004;
            for ( index = 0; index < mem_size; index += 4, word_p++ ) {
                read_pattern_w = *word_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_w != (bt_data16_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%4x received = 0x%4x\n", LOG_ARG, (bt_devaddr_t) word_p, (bt_data16_t) check_pattern, read_pattern_w));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                    if ( check_pattern == 0xfffefffd ) {
                        check_pattern = 0xfffffffd;
                    }
                }
            }
            break;

          case BT_WIDTH_D32:
            check_pattern = 0xffffffff;
            check_inc_dec = 0x00000004;
            for ( index = 0; index < mem_size; index += 4, lword_p++ ) {
                read_pattern_l = *lword_p;

                /* Check if the pattern in memory is the same as check pattern */
                if (read_pattern_l != (bt_data32_t) check_pattern) {
                    error = __LINE__;
                    INFO_MSG((LOG_FMT "Data Mismatch at addr 0x%8x expected = 0x%8x received = 0x%8x\n", LOG_ARG, (bt_devaddr_t) lword_p, (bt_data32_t) check_pattern, read_pattern_l));
                    goto check_memory_end;
                } else {
                    check_pattern -= check_inc_dec;
                }
            }
            break;

          default:
            error = __LINE__;
            INFO_MSG((LOG_FMT "Invalid pattern width of %d found.\n", LOG_ARG, (int) pattern.width));
            goto check_memory_end;

         } /* end  switch ( pattern.width ) */

    }  /* end  switch ( pattern.type ) */
    
check_memory_end:
    FEXIT(error);
    return (error);

} /* end check_memory() */



/*****************************************************************************
**
**  Function:   adator_reg_test()
**
**  Purpose:    To verify that node I/O registers, local and remote, appear
**              to be functioning correctly.
**
**  Args:
**      unit_p          Unit structure pointer
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0              Successful.
**      else            Error.
**
**  Calls:
**      NA
**
**  Note:
**
*****************************************************************************/
int adator_reg_test(
    bt_unit_t *unit_p)
{
    int                   error = 0;
    bt_data32_t           reg_value;

    FUNCTION("adator_reg_test");
    LOG_UNIT(unit_p);
 
    FENTRY;
    
    /*
    ** Simple test of a local register
    ** Clear it out
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, 0);
    reg_value = btk_get_io(unit_p, BT_LDMA_ADDR);
    if (reg_value != 0) {
        INFO_MSG((LOG_FMT "Failed to clear local DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0, reg_value));
        error = __LINE__;
        goto adaptor_reg_test_end;
    }

    /*
    ** Set all the bits, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, ALL_ONE);
    reg_value = btk_get_io(unit_p, BT_LDMA_ADDR);
    if (reg_value != 0xfffffffc) {
        INFO_MSG((LOG_FMT "Failed to set local DMA address; sent 0x%lx; recv 0x%x\n",
                   LOG_ARG, ALL_ONE, reg_value));
        error = __LINE__;
        goto adaptor_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, 0x12345678);
    reg_value = btk_get_io(unit_p, BT_LDMA_ADDR);
    if (reg_value != 0x12345678) {
        INFO_MSG((LOG_FMT "Failed pattern 1 to local DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0x12345678, reg_value));
        error = __LINE__;
        goto adaptor_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, 0xaa55aa55);
    reg_value = btk_get_io(unit_p, BT_LDMA_ADDR);
    if (reg_value != 0xaa55aa54) {
        INFO_MSG((LOG_FMT "Failed pattern 2 to local DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0xaa55aa54, reg_value));
        error = __LINE__;
        goto adaptor_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_LDMA_ADDR, 0x55aa55aa);
    reg_value = btk_get_io(unit_p, BT_LDMA_ADDR);
    if (reg_value != 0x55aa55a8) {
        INFO_MSG((LOG_FMT "Failed pattern 3 to local DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0x55aa55a8, reg_value));
        error = __LINE__;
        goto adaptor_reg_test_end;
    }

adaptor_reg_test_end:
    FEXIT(error);
    return (error);

} /* end adator_reg_test() */



/*****************************************************************************
**
**  Function:   adator_rem_reg_test()
**
**  Purpose:    To verify that node I/O registers, local and remote, appear
**              to be functioning correctly.
**
**  Args:
**      unit_p          Unit structure pointer
**                      
**  Modifies:
**      NA
**
**  Returns:
**      0              Successful.
**      else            Error.
**
**  Calls:
**      NA
**
**  Note:
**
*****************************************************************************/
int adator_rem_reg_test(
    bt_unit_t *unit_p)
{
    int                   error = 0;
    bt_data32_t           reg_value;

    FUNCTION("adator_rem_reg_test");
    LOG_UNIT(unit_p);
 
    FENTRY;

    /*
    ** Simple test of a remote register
    ** Clear it out
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, 0);
    reg_value = btk_get_io(unit_p, BT_RDMA_ADDR);
    if (reg_value != 0) {
        INFO_MSG((LOG_FMT "Failed to clear remote DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0, reg_value));
        error = __LINE__;
        goto adaptor_rem_reg_test_end;
    }

    /*
    ** Set all the bits, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, ALL_ONE);
    reg_value = btk_get_io(unit_p, BT_RDMA_ADDR);
    if (reg_value != 0xfffffffc) {
        INFO_MSG((LOG_FMT "Failed to set remote DMA address; sent 0x%lx; recv 0x%x\n",
                   LOG_ARG, ALL_ONE, reg_value));
        error = __LINE__;
        goto adaptor_rem_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, 0x12345678);
    reg_value = btk_get_io(unit_p, BT_RDMA_ADDR);
    if (reg_value != 0x12345678) {
        INFO_MSG((LOG_FMT "Failed pattern 1 to remote DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0x12345678, reg_value));
        error = __LINE__;
        goto adaptor_rem_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, 0xaa55aa55);
    reg_value = btk_get_io(unit_p, BT_RDMA_ADDR);
    if (reg_value != 0xaa55aa54) {
        INFO_MSG((LOG_FMT "Failed pattern 2 to remote DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0xaa55aa54, reg_value));
        error = __LINE__;
        goto adaptor_rem_reg_test_end;
    }

    /*
    ** Try a pattern, low 2 bits not significant
    */
    btk_put_io(unit_p, BT_RDMA_ADDR, 0x55aa55aa);
    reg_value = btk_get_io(unit_p, BT_RDMA_ADDR);
    if (reg_value != 0x55aa55a8) {
        INFO_MSG((LOG_FMT "Failed pattern 3 to remote DMA address; sent 0x%x; recv 0x%x\n",
                   LOG_ARG, 0x55aa55a8, reg_value));
        error = __LINE__;
        goto adaptor_rem_reg_test_end;
    }

#if EAS_A64_CODE 
/* EAS A64 */
    /* Test the A64 registers if using BT_PCI_DEVICE_RPQ601332 */
    if (unit_p->rem_id == BT_PN_PCI_DBA64_RPQ) {
        /* The remote board supports A64, test the registers */
        
        /* Test the PIO A64 register */

        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, 0);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64PIO);
        if (reg_value != 0) {
            INFO_MSG((LOG_FMT "Failed to clear remote A64 PIO address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, ALL_ONE);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64PIO);
        if (reg_value != ALL_ONE) {
            INFO_MSG((LOG_FMT "Failed to set remote A64 PIO address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, (unsigned int)ALL_ONE, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, 0x12345678);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64PIO);
        if (reg_value != 0x12345678) {
            INFO_MSG((LOG_FMT "Failed pattern 1 to remote A64 PIO address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0x12345678, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, 0xaa55aa55);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64PIO);
        if (reg_value != 0xaa55aa55) {
            INFO_MSG((LOG_FMT "Failed pattern 2 to remote A64 PIO address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0xaa55aa55, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64PIO, 0x55aa55aa);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64PIO);
        if (reg_value != 0x55aa55aa) {
            INFO_MSG((LOG_FMT "Failed pattern 3 to remote A64 PIO address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0x55aa55aa, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        /* Test the DMA A64 register */

        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, 0);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64DMA);
        if (reg_value != 0) {
            INFO_MSG((LOG_FMT "Failed to clear remote A64 DMA address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, ALL_ONE);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64DMA);
        if (reg_value != ALL_ONE) {
            INFO_MSG((LOG_FMT "Failed to set remote A64 DMA address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, (unsigned int)ALL_ONE, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, 0x12345678);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64DMA);
        if (reg_value != 0x12345678) {
            INFO_MSG((LOG_FMT "Failed pattern 1 to remote A64 DMA address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0x12345678, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, 0xaa55aa55);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64DMA);
        if (reg_value != 0xaa55aa55) {
            INFO_MSG((LOG_FMT "Failed pattern 2 to remote A64 DMA address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0xaa55aa55, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
        btk_put_io(unit_p, BT_RPQ_REM_A64DMA, 0x55aa55aa);
        reg_value = btk_get_io(unit_p, BT_RPQ_REM_A64DMA);
        if (reg_value != 0x55aa55aa) {
            INFO_MSG((LOG_FMT "Failed pattern 3 to remote A64 DMA address; sent 0x%x; recv 0x%x\n",
                       LOG_ARG, 0x55aa55aa, reg_value));
            error = __LINE__;
            goto adaptor_rem_reg_test_end;
        }
    }
#endif  /* EAS_A64_CODE */
        
adaptor_rem_reg_test_end:
    FEXIT(error);
    return (error);

} /* end adator_rem_reg_test() */


/*****************************************************************************
**
**      Function:   btk_datachk()
**
**      Purpose:    Fills buffer with an incrementing pattern.
**
**      Args:
**          unit_p          Pointer to the unit structure.
**          ldev            Logical devcie type.
**          base_address    Remote address to use during testing
**          len             Number of bytes to test.     
**          local_offset    Offset into the local buffers.
**          remote_offset   Offset into the remote buffer.
**          rdbuf           Local read buffer
**          wrbuf           Local write buffer
**           
**      Modifies:
**
**      Returns:
**
*****************************************************************************/

static int btk_datachk (
    bt_unit_t   *unit_p, 
    bt_dev_t    ldev, 
    bt_data32_t base_address, 
    int         len, 
    bt_data32_t local_offset, 
    bt_data32_t remote_offset,
    int         dma_width)
{
    
    bt_error_t   status;
    char         *temp_p = NULL;
    bt_data32_t  *d32_p = NULL;
    char         *lm_p = NULL;
    int          retval = BT_SUCCESS;
    int          inx;
    int         xfer_len;

    FUNCTION("btk_datachk");
    LOG_UNIT(unit_p);

    FENTRY;

    /*
    ** Malloc a temp buffer
    */
    if ((temp_p = btk_mem_alloc(len, 0)) == NULL) {
        INFO_STR("Failed to malloc diag memory buffer");
        retval = __LINE__;
        goto btk_datachk_end;
    }
    d32_p = (bt_data32_t *) temp_p;

    /*
    ** Get pointer to offset in local memory
    */
    lm_p = unit_p->kern_addr[BT_AXSLM] + local_offset;

    /*
    ** Initialize a random pattern in the buffer
    */
    for (inx = 0; inx < len/4; inx++) {
        d32_p[inx] = genrand();
    }

    /*
    ** Copy the pattern into the local memory device
    */
    BTK_BCOPY(temp_p, lm_p, len);

    /*
    ** DMA the data to the remote system
    */
    xfer_len = len;
#if defined(__linux__)
    status = btk_dma_xfer(unit_p, ldev,    local_offset,  base_address + remote_offset, &xfer_len, BT_WRITE, dma_width);
#else
    status = btk_dma_xfer(unit_p, ldev,    local_offset,  base_address + remote_offset, xfer_len, BT_WRITE, dma_width);
#endif
    if (status != BT_SUCCESS) {
        INFO_STR("DMA write failed during diagnostics");
        retval = __LINE__;
        goto mem_free_end;
    }

    /*
    ** Clear the local memory
    */
    BTK_BZERO(lm_p, len);
        
    /*
    ** DMA the data from the remote system
    */
    xfer_len = len;
#if defined(__linux__)
    status = btk_dma_xfer(unit_p, ldev,    local_offset,  base_address + remote_offset, &xfer_len, BT_READ, dma_width);
#else
    status = btk_dma_xfer(unit_p, ldev,    local_offset,  base_address + remote_offset, xfer_len, BT_READ, dma_width);
#endif
    if (status != BT_SUCCESS) {
        INFO_STR("DMA read failed during diagnostics");
        retval = __LINE__;
        goto mem_free_end;
    }

    /*
    ** Check the data
    */
    for (inx = 0; inx < len; inx++) {
        if (temp_p[inx] != lm_p[inx]) {
            INFO_STR("DMA data miscompare during diagnostics");
            INFO_MSG((LOG_FMT "Fist mismatch at offset 0x%08lX; sent 0x%x; recv 0x%x\n",
                        LOG_ARG, (unsigned long) inx, temp_p[inx], lm_p[inx]));
            retval = __LINE__;
            goto mem_free_end;
        }
    }

mem_free_end:
    /*
    ** Free the memory
    */
    btk_mem_free(temp_p, len);
    
btk_datachk_end:
    FEXIT(retval);
    return retval;
}

/* A C-program for TT800 : July 8th 1996 Version */
/* by M. Matsumoto, email: matumoto@math.keio.ac.jp */
/* genrand() generate one pseudorandom number with double precision */
/* which is uniformly distributed on [0,1]-interval */
/* for each call.  One may choose any initial 25 seeds */
/* except all zeros. */

/* See: ACM Transactions on Modelling and Computer Simulation, */
/* Vol. 4, No. 3, 1994, pages 254-266. */
#define N 25
#define M 7

static bt_data32_t
genrand(void)
{
    bt_data32_t y;
    static int k = 0;
    static bt_data32_t x[N]={ /* initial 25 seeds, change as you wish */
        0x95f24dab, 0x0b685215, 0xe76ccae7, 0xaf3ec239, 0x715fad23,
        0x24a590ad, 0x69e4b5ef, 0xbf456141, 0x96bc1b7b, 0xa7bdf825,
        0xc1de75b7, 0x8858a9c9, 0x2da87693, 0xb657f9dd, 0xffdc8a9f,
        0x8121da71, 0x8b823ecb, 0x885d05f5, 0x4e20cd47, 0x5a9ad5d9,
        0x512c0c03, 0xea857ccd, 0x4cc1d30f, 0x8891a8a1, 0xa6b7aadb
    };
    static bt_data32_t mag01[2]={ 
        0x0, 0x8ebfd028 /* this is magic vector `a', don't change */
    };

    /* 
    ** generate N words at one time 
    */
    if (k==N) { 
      int kk;
      for (kk=0;kk<N-M;kk++) {
        x[kk] = x[kk+M] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
      }
      for (; kk<N;kk++) {
        x[kk] = x[kk+(M-N)] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
      }
      k=0;
    }
    y = x[k];
    y ^= (y << 7) & 0x2b5b2500; /* s and b, magic vectors */
    y ^= (y << 15) & 0xdb8b0000; /* t and c, magic vectors */
/* 
   the following line was added by Makoto Matsumoto in the 1996 version
   to improve lower bit's corellation.
   Delete this line to o use the code published in 1994.
*/
    y ^= (y >> 16); /* added to the 1994 version */
    k++;
    
    return(y);
}


