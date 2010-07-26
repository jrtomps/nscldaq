/******************************************************************************
**
**      Filename:       bt_bind.c
**
**      Purpose:        IRIX, Solaris & Linux implementation of bind
**                      for NanoBus hardware
**
**      Functions:      bt_bind(), bt_unbind(), bt_hw_bind(), bt_hw_unbind()
**
**      $Revision: 1.11 $
**
******************************************************************************/
/*****************************************************************************
**
**              Copyright (c) 2000 by SBS Technologies, Inc.
**                        All Rights Reserved.
**                 License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.11 $" __DATE__;
#endif /* LINT */

#include    "btapi.h"
#include    "btpiflib.h"


/*****************************************************************************
**
**      Name:           bt_bind
**
**      Purpose:        Implement BIOC_BIND function which Maps an application
**                      supplied buffer onto the remote bus.
**      Args:
**          btd         Device Descriptor
**          bind_p      Pointer to the bind handle to init.
**          offset_p    Pointer to offset into remote mem where buf is bound
**          buf_p       Pointer to buffer to bind
**          buf_len     Length of buffer
**          flags       Access rights requested on the bind
**          swapping    Swapping methond to use on remote accesses to the buffer
**          
**      Returns:
**          BT_ENOSUP   Always
**
**      Notes:
**      Hardware does not require any bind.
**
*****************************************************************************/

bt_error_t bt_bind(
    bt_desc_t       btd,
    bt_binddesc_t  *bind_p,
    bt_devaddr_t   *offset_p,
    void           *buf_p,
    size_t          buf_len,
    bt_accessflag_t flags,
    bt_swap_t       swapping)
{
    bt_error_t  retval = BT_SUCCESS;
    bt_bind_t   bind;
#ifndef FCTACH
    bt_devdata_t save_swap;
#endif  /* FCTACH */
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_bind:bad descriptor.");
        retval = BT_EDESC;
        goto bt_bind_end;
    }

    /*
    ** Verify parameters
    */
    if ((bind_p == NULL) || 
        (buf_p == NULL) ||
        (offset_p == NULL)) {
        DBG_STR("bt_bind:Null pointer passed in");
        DBG_MSG((stderr, "bindp=%x buf_p=%x offset_p=%x\n", 
            (unsigned int)bind_p, (unsigned int)buf_p, (unsigned int)offset_p));
        retval = BT_EINVAL;
        goto bt_bind_end;
    }
        
    /*
    ** Verify parameters
    */
    if (buf_len == 0) {
        DBG_STR("bt_bind:zero length buffer to bind");
        retval = BT_EINVAL;
        goto bt_bind_end;
    }
 
     /* 
    ** verify R/W permissions against the open 
    */
    if (((flags & BT_WR) && !(btd->access_flags & BT_WR))  ||
        ((flags & BT_RD) && !(btd->access_flags & BT_RD))) {
        DBG_STR("bt_bind:Read/Write permissions missmatch.");
        retval = BT_EACCESS;
        goto bt_bind_end;
    }

    /*
    ** Setup ioctl structure
    */
    bind.host_addr = (bt_data64_t) ((unsigned long) buf_p);
    bind.length = buf_len;
    bind.nowait = FALSE;
    bind.phys_addr = (bt_data32_t) (*offset_p);

#ifdef FCTACH
    bind.type = BT_BIND_USERVM;
    bind.offset = 0;
#else   /* FCTACH */
    /*
    ** Set special swapping if requested
    */
    if (swapping != BT_SWAP_DEFAULT) {
        retval = bt_get_info(btd, BT_INFO_SWAP, &save_swap);
        if (retval != BT_SUCCESS) {
            DBG_STR("bt_bind:Couldn't get BT_INFO_SWAP info.");
            goto bt_bind_end;
        }
        retval = bt_set_info(btd, BT_INFO_SWAP, swapping);
        if (retval != BT_SUCCESS) {
            (void) bt_ctrl(btd, BIOC_UNBIND, &bind);
            DBG_STR("bt_bind:Couldn't set swapping to users value.");
            goto bt_bind_end;
        }
    }
#endif  /* FCTACH */

    /*
    ** Do the actual bind
    */
    if ((retval = bt_ctrl(btd, BIOC_BIND, &bind)) != BT_SUCCESS) {
        DBG_STR("bt_bind:BIOC_BIND failed.");
        goto bt_bind_end;
    }

#ifndef FCTACH
    /*
    ** Restore special swapping if required
    */
    if (swapping != BT_SWAP_DEFAULT) {
        retval = bt_set_info(btd, BT_INFO_SWAP, save_swap);
        if (retval != BT_SUCCESS) {
            (void) bt_ctrl(btd, BIOC_UNBIND, &bind);
            DBG_STR("bt_bind:Couldn't restore swapping info.");
            goto bt_bind_end;
        }
    }
#endif  /* FCTACH */
    *offset_p = (bt_devdata_t) bind.phys_addr;
    *bind_p = (bt_binddesc_t) bind.sysinfo_p;
     DBG_MSG((stderr, "bind done bind_p=%x offset_p=%x\n", 
         (unsigned int)*bind_p, (unsigned int)*offset_p));

bt_bind_end:
    return (retval);
}


/*****************************************************************************
**
**      Name:           bt_unbind
**
**      Purpose:        Implement BIOC_UNBIND function
**
**      Args:
**          btd         Device Descriptor
**          desc        Bind descriptor returned from bt_bind()
**          
**      Returns:
**          BT_ENOSUP   Always
**
**      Notes:
**      Hardware does not require any bind.
**
*****************************************************************************/

bt_error_t bt_unbind(
    bt_desc_t     btd,
    bt_binddesc_t desc)
{
    bt_error_t  retval = BT_SUCCESS;
    bt_bind_t   unbind;
   
    /*
    ** Check for bad descriptor
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_unbind:bad descriptor.");
        retval = BT_EDESC;
        goto bt_unbind_end;
    }

    /*
    ** Verify parameters
    */
    if (desc == (bt_binddesc_t) NULL){
        DBG_STR("bt_unbind:Null bind descriptor passed in");
        retval = BT_EINVAL;
        goto bt_unbind_end;
    }

    /*
    ** Setup ioctl structure
    */
    unbind.sysinfo_p = (bt_devaddr_t) desc;

    /*
    ** Do the actual unbind
    */
    if ((retval = bt_ctrl(btd, BIOC_UNBIND, &unbind)) != BT_SUCCESS) {
        DBG_STR("bt_unbind:unbind failed");
        goto bt_unbind_end;
    }

bt_unbind_end:
    return (retval);
}

/*****************************************************************************
**
**      Name:           bt_hw_bind
**
**      Purpose:        Implement BIOC_HW_BIND function which Maps a physical
**                      bus address onto the remote bus.
**      Args:
**          btd         Device Descriptor
**          bind_p      Pointer to the bind handle to init.
**          offset_p    Pointer to offset into remote mem where buf is bound
**          loc_addr    Physical bus address to bind
**          buf_len     Length of buffer
**          flags       Access rights requested on the bind
**          swapping    Swapping methond to use on remote accesses to the buffer
**          
**      Returns:
**          BT_ENOSUP   Always
**
**      Notes:
**      Hardware does not require any bind.
**
*****************************************************************************/

bt_error_t bt_hw_bind(
    bt_desc_t       btd,
    bt_binddesc_t  *bind_p,
    bt_devaddr_t   *offset_p,
    bt_devaddr_t    loc_addr,
    size_t          buf_len,
    bt_accessflag_t flags,
    bt_swap_t       swapping)
{
    bt_error_t  retval = BT_SUCCESS;
    bt_bind_t   bind;
#ifndef FCTACH
    bt_devdata_t save_swap;
#endif  /* FCTACH */
   
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_hw_bind:bad descriptor");
        retval = BT_EDESC;
        goto bt_hw_bind_end;
    }

    /*
    ** Verify parameters
    */
    if ((bind_p == NULL) || (offset_p == NULL)) {
        DBG_STR("bt_hw_bind:Null pointer passed in.");
        retval = BT_EINVAL;
        goto bt_hw_bind_end;
    }
    /*
    ** Verify parameters
    */
    if (buf_len == 0) {
        DBG_STR("bt_hw_bind:zero length buffer to bind");
        retval = BT_EINVAL;
        goto bt_hw_bind_end;
    }
         
    /* 
    ** verify R/W permissions against the open 
    */
    if (((flags & BT_WR) && !(btd->access_flags & BT_WR))  ||
        ((flags & BT_RD) && !(btd->access_flags & BT_RD))) {
        DBG_STR("bt_hw_bind:Read/Write permissions don't match open.");
        retval = BT_EACCESS;
        goto bt_hw_bind_end;
    }

    /*
    ** Setup ioctl structure
    */
    bind.host_addr = loc_addr;
    bind.length = buf_len;
    bind.nowait = FALSE;
    bind.phys_addr = *offset_p;

#ifdef FCTACH
    bind.type = BT_BIND_USERVM;
    bind.offset = 0;
#else   /* FCTACH */
    /*
    ** Set special swapping if requested
    */
    if (swapping != BT_SWAP_DEFAULT) {
        retval = bt_get_info(btd, BT_INFO_SWAP, &save_swap);
        if (retval != BT_SUCCESS) {
            DBG_STR("bt_hw_bind:Couldn't get default swap info.");
            goto bt_hw_bind_end;
        }
    }
#endif   /* FCTACH */

    /*
    ** Do the actual bind
    */
    if ((retval = bt_ctrl(btd, BIOC_HW_BIND, &bind)) != BT_SUCCESS) {
        DBG_STR("bt_hw_bind:hw bind call failed.");
        goto bt_hw_bind_end;
    }

#ifndef FCTACH
    /*
    ** Restore special swapping if required
    */
    if (swapping != BT_SWAP_DEFAULT) {
        retval = bt_set_info(btd, BT_INFO_SWAP, save_swap);
        if (retval != BT_SUCCESS) {
            (void) bt_ctrl(btd, BIOC_HW_UNBIND, &bind);
            DBG_STR("bt_hw_bind:Couldn't restore default swapping.");
            goto bt_hw_bind_end;
        }
    }
#endif   /* FCTACH */
    *offset_p = bind.phys_addr;
    *bind_p = (bt_binddesc_t) bind.sysinfo_p;

bt_hw_bind_end:
    return (retval);
}


/*****************************************************************************
**
**      Name:           bt_hw_unbind
**
**      Purpose:        Implement BIOC_HW_UNBIND function
**
**      Args:
**          btd         Device Descriptor
**          desc        Bind descriptor returned from bt_bind()
**          
**      Returns:
**          BT_ENOSUP   Always
**
**      Notes:
**      Hardware does not require any bind.
**
*****************************************************************************/

bt_error_t bt_hw_unbind(
    bt_desc_t     btd,
    bt_binddesc_t desc)
{
    bt_error_t  retval = BT_SUCCESS;
    bt_bind_t   unbind;
   
    /*
    ** Check for bad descriptor
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_hw_unbind:bad descriptor.");
        retval = BT_EDESC;
        goto bt_hw_unbind_end;
    }

    /*
    ** Verify parameters
    */
    if (desc == (bt_binddesc_t) NULL){
        DBG_STR("bt_hw_unbind:Null bind descriptor passed in");
        retval = BT_EINVAL;
        goto bt_hw_unbind_end;
    }

    /*
    ** Setup ioctl structure
    */
    unbind.sysinfo_p = (bt_devaddr_t) desc;

    /*
    ** Do the actual unbind
    */
    if ((retval = bt_ctrl(btd, BIOC_HW_UNBIND, &unbind)) != BT_SUCCESS) {
        DBG_STR("bt_hw_unbind:hw unbind call failed");
        goto bt_hw_unbind_end;
    }

bt_hw_unbind_end:
    return (retval);
}

#if FCTACH
/*****************************************************************************
**
**      Name:           bt_hw_bind
**
**      Purpose:        Implement BIOC_HW_BIND function which Maps a physical
**                      bus address onto the remote bus.
**      Args:
**          btd         Device Descriptor
**          bind_p      Pointer to the bind handle to init.
**          mapped_phys_addr    (OUT) Pointer to the physical address of the virtual mapped
**          offset_p    Pointer to offset into remote mem where buf is bound
**          loc_addr    Physical bus address to bind
**          buf_len     Length of buffer
**          flags       Access rights requested on the bind
**          swapping    Swapping methond to use on remote accesses to the buffer
**          type        virtual buffer vs physical buffer type
**          
**      Returns:
**          pass-fail
**
**      Notes:
**
*****************************************************************************/
bt_error_t bt_fcbind(
    bt_desc_t       btd,
    bt_binddesc_t  *bind_p,
    bt_devaddr_t   *mapped_phys_addr,
    bt_devaddr_t    offset,
    bt_devaddr_t    buf_p,
    size_t          buf_len,
    bt_accessflag_t flags,
    unsigned char   type)
{
bt_bind_t   bind;

    bt_error_t  retval = BT_SUCCESS;
    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        DBG_STR("bt_fcbind:bad descriptor");
        retval = BT_EDESC;
        goto bt_fcbind_end;
    }

    /*
    ** Verify parameters
    */
    if ((bind_p == NULL) || (mapped_phys_addr == NULL)) {
        DBG_STR("bt_fcbind:Null pointer passed in.");
        retval = BT_EINVAL;
        goto bt_fcbind_end;
    }
        
    /*
    ** Verify parameters
    */
    if (buf_len == 0) {
        DBG_STR("bt_bind:zero length buffer to bind");
        retval = BT_EINVAL;
        goto bt_fcbind_end;
    }
 
     /* 
    ** verify R/W permissions against the open 
    */
    if (((flags & BT_WR) && !(btd->access_flags & BT_WR))  ||
        ((flags & BT_RD) && !(btd->access_flags & BT_RD))) {
        DBG_STR("bt_fcbind:Read/Write permissions don't match open.");
        retval = BT_EACCESS;
        goto bt_fcbind_end;
    }

    /*
    ** Setup ioctl structure
    */
    bind.offset = offset;
    bind.host_addr = buf_p;
    bind.length = buf_len;
    bind.nowait = FALSE;
    bind.type = type;

    /*
    ** Do the actual bind
    */
    if ((retval = bt_ctrl(btd, BIOC_HW_BIND, &bind)) != BT_SUCCESS) {
        DBG_STR("bt_fcbind:hw bind call failed.");
        goto bt_fcbind_end;
    }

    *mapped_phys_addr = bind.phys_addr;
    *bind_p = (bt_binddesc_t) bind.sysinfo_p;

bt_fcbind_end:
    return (retval);
}

/*****************************************************************************
**
**      Name:           bt_fcvbind
**
**      Purpose:        Implement BIOC_VBIND function which shows the binding
**                      table entry desired.
**      Args:
**          btd         Device Descriptor
**          offset      (OUT) Pointer to offset into remote mem where buf is bound
**          mapped_phys_addr    (OUT) Pointer to the physical address of the virtual mapped
**          length      (OUT) Length of buffer
**          type        (OUT) virtual buffer vs physical buffer type
**          
**      Returns:
**          pass-fail
**
**      Notes:
**
*****************************************************************************/
bt_error_t bt_fcvbind(
    bt_desc_t       btd,
    bt_data16_t     index,
    bt_devaddr_t   *offset,
    bt_devaddr_t   *length,
    bt_devaddr_t   *mapped_phys_addr,
    bt_data8_t     *type)
{
bt_vbind_t   vbind;
bt_error_t  retval;
 
    /*
    ** Verify parameters
    */
    if ((offset == NULL) || (mapped_phys_addr == NULL) ||
        (length == NULL) || (type == NULL)) {
        DBG_STR("bt_fcvbind:Null pointer passed in.");
        retval = BT_EINVAL;
        goto bt_fcvbind_end;
    }
     /*
    ** Do the actual view bind ioctl
    */
    vbind.entryno = index;

    if ((retval = bt_ctrl(btd, BIOC_VBIND, &vbind)) != BT_SUCCESS) {
        DBG_STR("bt_fcvbind:fcvbind call failed.");
    } else {
        *offset = vbind.offset;
        *length = vbind.length;
        *mapped_phys_addr = vbind.phys_addr;
        *type = vbind.type;
    }
bt_fcvbind_end:
    return(retval);
}


#endif  /* FCTACH */

