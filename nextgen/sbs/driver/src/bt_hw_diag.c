/******************************************************************************
**
**      Filename:       bt_hw_diag.c
**
**      Purpose:        IRIX, Solaris & Linux implementation of Hardware
**                      diagnostic routines for dataBLIZZARD hardware
**
**      Functions:      bt_lcard_diag(), bt_cable_diag(), bt_rcard_diag(),
**                      bt_pair_diag(), bt_trace_init()  bt_driver_version()
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**              Copyright (c) 2000-2002 by SBS Technologies, Inc.
**                        All Rights Reserved.
**                 License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $" __DATE__;
#endif /* LINT */

#include <stdio.h>

#include  <string.h>   /* for strncp call */
#include "btapi.h"
#include "btio.h"
#include "btpiflib.h"



/*****************************************************************************
**
**      Name:           bt_driver_version
**
**      Purpose:        Returns the driver version as a text string
**
**      Args:
**          btd         Device Descriptor
**          ver_info_p  Text string with file and revision number.
**          line_p      Pointer to the line number failure was discovered.
**
**      Returns:
**          BT_ENOSUP   Not supported on this hardware.
**          BT_EINVAL   Illegal parameter passed to function.
**          BT_EDESC    Illegal descriptor passed to function.
**
**          BT_SUCCESS  Testing passed, local card appears good.
**          
**
**      Notes:
**
*****************************************************************************/

bt_error_t bt_driver_version(
    bt_desc_t       btd,
    char            *ver_info_p,
    int             *line_p)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_hw_diag_t    diag;
    char            model_version[BT_DIAG_MAX_REV_INFO];
    int             i;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_driver_version:bad descriptor.");
        retval = BT_EDESC;
        goto bt_driver_version_end;
    }

    for (i = 0; i < BT_DIAG_MAX_REV_INFO; i++) {
        model_version[i] = '\0';
    }

    /*
    ** Verify parameters
    */
#if defined (__vxworks)
    ver_info_p[BT_DIAG_MAX_REV_INFO] = NULL; 
#endif /* defined (__vxworks) */

    /*
    ** Setup ioctl structure
    */
    diag.error = (bt_data32_t) BT_SUCCESS;

    /*
    ** Retrieve the driver version number
    */
    if ((retval = bt_ctrl(btd, BIOC_DRIVER_VERSION, &diag)) != BT_SUCCESS) {
        DBG_STR("bt_lcard_diag:BIOC_DRIVER_VERSION failed.");
        if (diag.error != BT_SUCCESS) {
            retval = diag.error;
        }
        goto bt_driver_version_end;
    }

bt_driver_version_end:
    /*
    ** Report the line number and rev info if requested
    */
    if (ver_info_p != NULL) {
        sscanf((const char *) &diag.rev_info[0], "$Name: %s ", model_version);

#if defined (__vxworks)
        sprintf(ver_info_p, "%s", model_version);
#else
        snprintf(ver_info_p, BT_DIAG_MAX_REV_INFO, "%s", model_version);
#endif /* defined (__vxworks) */

    }

    if (line_p != NULL) {
        *line_p = diag.line_number;
    }
    
    return (retval);
}

/*****************************************************************************
**
**      Name:           bt_lcard_diag
**
**      Purpose:        Determine whether the local card is operating 
**                      properly.
**
**      Args:
**          btd         Device Descriptor
**          rev_info_p  Text string with file and revision number.
**          line_p      Pointer to the line number failure was discovered.
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
**          If the pig tail connector is installed on the local card 
**          and any data in local dual port memory or the local memory 
**          device may be destroyed.
**        
**          All diagnostics require the local memory device to be present.
**
**          Both line_p and rev_info_p can be null and no information will
**          be returned.
**
*****************************************************************************/

bt_error_t bt_lcard_diag(
    bt_desc_t       btd,
    char            *rev_info_p,
    int             *line_p)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_hw_diag_t    diag;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_lcard_diag:bad descriptor.");
        retval = BT_EDESC;
        goto bt_lcard_diag_end;
    }

    /*
    ** Verify parameters
    */

    /*
    ** Setup ioctl structure
    */
    diag.error = (bt_data32_t) BT_SUCCESS;

    /*
    ** Do the actual diagnostics
    */
    if ((retval = bt_ctrl(btd, BIOC_LCARD_DIAG, &diag)) != BT_SUCCESS) {
        DBG_STR("bt_lcard_diag:BIOC_LCARD_DIAG failed.");
        if (diag.error != BT_SUCCESS) {
            retval = diag.error;
        }
        goto bt_lcard_diag_end;
    }

bt_lcard_diag_end:
    /*
    ** Report the line number and rev info if requested
    */
    if (rev_info_p != NULL) {
        strncpy(rev_info_p, (const char *) &diag.rev_info[0], BT_DIAG_MAX_REV_INFO);
    }

    if (line_p != NULL) {
        *line_p = diag.line_number;
    }
    
    return (retval);
}


/*****************************************************************************
**
**      Name:           bt_cable_diag
**
**      Purpose:        Determine whether the cable is operating 
**                      properly.
**.
**      Args:
**          btd         Device Descriptor
**          rev_info_p  Text string with file and revision number.
**          line_p      Pointer to the line number failure was discovered.
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
**          If the pig tail connector is installed at the end of the 
**          cable and any data in local dual port memory or the local 
**          memory device may be destroyed.
**        
**          All diagnostics require the local memory device to be present.
**          
**          Should be done after bt_lcard_diag() passes.
**
*****************************************************************************/

bt_error_t bt_cable_diag(
    bt_desc_t       btd,
    char            *rev_info_p,
    int             *line_p)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_hw_diag_t    diag;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_cable_diag:bad descriptor.");
        retval = BT_EDESC;
        goto bt_cable_diag_end;
    }

    /*
    ** Verify parameters
    */

    /*
    ** Setup ioctl structure
    */
    diag.error = (bt_data32_t) BT_SUCCESS;

    /*
    ** Do the actual diagnostics
    */
    if ((retval = bt_ctrl(btd, BIOC_CABLE_DIAG, &diag)) != BT_SUCCESS) {
        DBG_STR("bt_cable_diag:BIOC_CABLE_DIAG failed.");
        if (diag.error != BT_SUCCESS) {
            retval = diag.error;
        }
        goto bt_cable_diag_end;
    }

bt_cable_diag_end:
    /*
    ** Report the line number and rev info if requested
    */
    if (rev_info_p != NULL) {
        strncpy(rev_info_p, (const char *) &diag.rev_info[0], BT_DIAG_MAX_REV_INFO);
    }
    if (line_p != NULL) {
        *line_p = diag.line_number;
    }
    
    return (retval);
}


/*****************************************************************************
**
**      Name:           bt_rcard_diag
**
**      Purpose:        Determine whether the remote card is operating 
**                      properly.
**.
**      Args:
**          btd         Device Descriptor
**          rldev       Remote logical device type to use during testing.
**                        One of bt_dev_t values, normally BT_DEV_A32.
**          raddr       Remote device address to use during testing.
**          rlen        Lenght in bytes of remote resource
**          rev_info_p  Text string with file and revision number.
**          line_p      Pointer to the line number failure was discovered.
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
**          Cable must be attached to remote card.  Any data in local dual 
**          port memory, remote dual port memory, specified remote resource
**          and the local memory device may be destroyed.
**        
**          All diagnostics require the local memory device to be present and
**          for the remote resource to be vaild.
**          
**          Should be done after bt_lcard_diag() and bt_cable_diag() passes.
**
*****************************************************************************/

bt_error_t bt_rcard_diag(
    bt_desc_t       btd,
    bt_dev_t        rldev,
    bt_devaddr_t    raddr,
    bt_data32_t     rlen,
    char            *rev_info_p,
    int             *line_p)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_hw_diag_t    diag;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_rcard_diag:bad descriptor.");
        retval = BT_EDESC;
        goto bt_rcard_diag_end;
    }

    /*
    ** Verify parameters
    */
    if ((rldev != BT_DEV_A32) &&
        (rldev != BT_DEV_A32) &&
        (rldev != BT_DEV_A16)) {
        DBG_STR("bt_rcard_diag: invalid remote logical device type.");
        retval = BT_EINVAL;
        goto bt_rcard_diag_end;
    }
    if (rlen == 0) {
        DBG_STR("bt_rcard_diag: zero remote length.");
        retval = BT_EINVAL;
        goto bt_rcard_diag_end;
    }

    /*
    ** Setup ioctl structure
    */
    diag.error = (bt_data32_t) BT_SUCCESS;
    diag.r_ldev = rldev;
    diag.r_addr = raddr;
    diag.r_len = rlen;

    /*
    ** Do the actual diagnostics
    */
    if ((retval = bt_ctrl(btd, BIOC_RCARD_DIAG, &diag)) != BT_SUCCESS) {
        DBG_STR("bt_rcard_diag:BIOC_RCARD_DIAG failed.");
        if (diag.error != BT_SUCCESS) {
            retval = diag.error;
        }
        goto bt_rcard_diag_end;
    }

bt_rcard_diag_end:

    /*
    ** Report the line number and rev info if requested
    */
    if (rev_info_p != NULL) {
        strncpy(rev_info_p, (const char *) &diag.rev_info[0], BT_DIAG_MAX_REV_INFO);
    }

    if (line_p != NULL) {
        *line_p = diag.line_number;
    }
    
    return (retval);
}


/*****************************************************************************
**
**      Name:           bt_pair_diag
**
**      Purpose:        Determine whether a pair of cards is operating 
**                      properly.
**.
**      Args:
**          btd         Device Descriptor
**          rldev       Remote logical device type to use during testing.
**                        One of bt_dev_t values, normally BT_DEV_A32.
**          raddr       Remote device address to use during testing.
**          rlen        Lenght in bytes of remote resource
**          rev_info_p  Text string with file and revision number.
**          line_p      Pointer to the line number failure was discovered.
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
**          Cable must be attached to remote card.  Any data in local dual 
**          port memory, remote dual port memory, specified remote resource
**          and the local memory device may be destroyed.
**        
**          All diagnostics require the local memory device to be present and
**          for the remote resource to be vaild.
**          
**          Should be done after bt_lcard_diag(), bt_cable_diag() and
**          bt_rcard_diag() passes.
**
*****************************************************************************/

bt_error_t bt_pair_diag(
    bt_desc_t       btd,
    bt_dev_t        rldev,
    bt_devaddr_t    raddr,
    bt_data32_t     rlen,
    char            *rev_info_p,
    int             *line_p)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_hw_diag_t    diag;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_pair_diag:bad descriptor.");
        retval = BT_EDESC;
        goto bt_pair_diag_end;
    }

    /*
    ** Verify parameters
    */
    if ((rldev != BT_DEV_A32) &&
        (rldev != BT_DEV_A32) &&
        (rldev != BT_DEV_A16)) {
        DBG_STR("bt_pair_diag: invalid remote logical device type.");
        retval = BT_EINVAL;
        goto bt_pair_diag_end;
    }
    if (rlen == 0) {
        DBG_STR("bt_pair_diag: zero remote length.");
        retval = BT_EINVAL;
        goto bt_pair_diag_end;
    }

    /*
    ** Setup ioctl structure
    */
    diag.error = (bt_data32_t) BT_SUCCESS;
    diag.r_ldev = rldev;
    diag.r_addr = raddr;
    diag.r_len = rlen;

    /*
    ** Do the actual diagnostics
    */
    if ((retval = bt_ctrl(btd, BIOC_PAIR_DIAG, &diag)) != BT_SUCCESS) {
        DBG_STR("bt_pair_diag:BIOC_PAIR_DIAG failed.");
        if (diag.error != BT_SUCCESS) {
            retval = diag.error;
        }
        goto bt_pair_diag_end;
    }

bt_pair_diag_end:
    /*
    ** Report the line number and rev info if requested
    */
    if (rev_info_p != NULL) {
        strncpy(rev_info_p, (const char *) &diag.rev_info[0], BT_DIAG_MAX_REV_INFO);
    }
    if (line_p != NULL) {
        *line_p = diag.line_number;
    }
    
    return (retval);
}


#if defined(__vxworks)


/*****************************************************************************
**
**      Name:           bt_trace_init
**
**      Purpose:        Setup the streams to log trace message too.ing 
**
**      Args:
**          btd         Device Descriptor
**          stream1     First stream to log trace message too.              
**          stream2     Second stream to log trace message too.              
**          
**      Returns:
**          BT_SUCCESS  If success, otherwise an error code.
**          
**
**      Notes:
**
*****************************************************************************/

bt_error_t bt_trace_init(
    bt_desc_t       btd,
    FILE           *stream1,
    FILE           *stream2)
{
    bt_error_t      retval = BT_SUCCESS;
    bt_trace_init_t trace_data;
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_pair_diag:bad descriptor.");
        retval = BT_EDESC;
        goto bt_trace_init_end;
    }

    /*
    ** Setup ioctl structure
    */
    trace_data.error = (bt_data32_t) BT_SUCCESS;
    trace_data.stream1 = (FILE *) stream1;
    trace_data.stream2 = (FILE *) stream2;

    /*
    ** Do the actual ioctl call
    */
    if ((retval = bt_ctrl(btd, BIOC_TRACE_INIT, &trace_data)) != BT_SUCCESS) {
        DBG_STR("bt_pair_diag:BIOC_TRACE_INIT failed.");
        if (trace_data.error != BT_SUCCESS) {
            retval = trace_data.error;
        }
        goto bt_trace_init_end;
    }

bt_trace_init_end:
    
    return (retval);
}

#endif /* defined(__vxworks) */


