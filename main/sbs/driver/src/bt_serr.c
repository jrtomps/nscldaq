/******************************************************************************
**
**    Filename:    bt_serr.c
**
**    Purpose:    Shared version of bt_strerror() routine.
**
**    Functions:    bt_strerror()
**
**    $Revision: 1.7 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef    LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.7 $" __DATE__;
#endif    /* LINT */

#include <string.h>
#include <stdio.h>

#include "btapi.h"
#include "btpiflib.h"

 

/*****************************************************************************
**
**    Name:         bt_strerror
**
**    Purpose:      Writes an error message for the given bt_error_t into
**                  a buffer.
**
**    Args:
**        btd_p     Pointer to Bit 3 descriptor
**        status    Bit 3 Mirror API error value, type bt_error_t.
**        msg_p     Prefix to put in the message.
**        buf_p     Buffer to copy error message to.
**        buf_len   The length of the buffer pointed to by buf_p
**
**    Modifies:
**        buf_p     Puts as much of the message string as fits in the buffer 
**                  with a terminating NULL.
**
**    Returns:
**        NULL      Couldn't fit all of the message in buffer or couldn't 
**                  interprete error code.
**        otherwise Start of message, always same as value of buf_p.
**
*****************************************************************************/

#if defined (_WIN32) /* The __stdcall calling convention is used to call Win32 API functions.*/
BT_EXTERN char * _stdcall bt_strerror(
#else
char * bt_strerror(
#endif

    bt_desc_t btd, 
    bt_error_t status, 
    const char * msg_p, 
    char * buf_p, 
    size_t buf_len
    )
{
    char        *curr_str = NULL;
    int         curr_str_len;
    size_t      buf_remaining = buf_len - 1;

    bt_status_t bt_stat;	/* Encoded error status value */

    memset(buf_p, '\0', buf_len);

#define    ADD_MSG(message_p) { \
    curr_str = message_p; curr_str_len = strlen(curr_str) ; \
    (void) strncat(buf_p, curr_str, buf_remaining); \
    if (curr_str_len >= (signed)buf_remaining) { \
        return NULL; \
    } \
    buf_remaining -= curr_str_len; \
}

    if (NULL != msg_p) {
        ADD_MSG((char *) msg_p);
        ADD_MSG(": ");
    }

    if (BT_ESTATUS == status) {     /* Status register tells what error */

        ADD_MSG("BT_ESTATUS: NanoBus hardware status error.");

        if (BT_SUCCESS != bt_ctrl(btd, BIOC_STATUS, &bt_stat)) {
            ADD_MSG("\nFatal condition: Could not query NanoBus status!");
        } else {  /* if (BT_SUCCESS != ...) */

	    /*
	    ** Decode each of the error status bits in the return value
	    ** after shifting down.
	    */
	    bt_stat = bt_stat >> BT_INTR_ERR_SHFT;
            if (bt_stat & LSR_NO_CONNECT) {
                ADD_MSG("\nRemote powered off or cable disconnected");
                return buf_p;
            }
    
            if (bt_stat & LSR_REMBUS_ERR) {
                ADD_MSG("\nRemote System Bus Error");
            }

            if (bt_stat & LSR_PARITY_ERR) { 
                ADD_MSG("\nInterface parity error");
            }

            if (bt_stat & LSR_TIMEOUT_ERR) {
                ADD_MSG("\nInterface timeout");
            }

            if (bt_stat & LSR_LRC_ERR) {
                ADD_MSG("\nLongitudinal Redundancy Check (long parity) error");
            }
        } /* end else bt_ctrl() worked */

        return buf_p;

    } /* end if BT_ESTATUS == status */

    /* Not a status error, can handle in one big switch statement */
    switch (status) {

      /* Mirror API bt_error_t types */

      case BT_EABORT:
          curr_str = "BT_EABORT: Action was aborted";
          break;

      case BT_EACCESS:
          curr_str = "BT_EACCESS: Requested access not permitted";
          break;

      case BT_EBUSY:
          curr_str = "BT_EBUSY: Time expired before a resource became available";
          break;

      case BT_EDESC:
          curr_str = "BT_EDESC: Invalid descriptor";
          break;

      case BT_EFAIL:
          curr_str = "BT_EFAIL: Operation failed";
          break;

      case BT_EINVAL:
          curr_str = "BT_EINVAL: Invalid argument passed to Mirror API function";
          break;

      case BT_EIO:
          curr_str = "BT_EIO: I/O Error";
          break;

      case BT_ENXIO:
          curr_str = "BT_ENODEV: Device does not exist";
          break;

      case BT_ENOMEM:
          curr_str = "BT_ENOMEM: Insufficient resources";
          break;

      case BT_ENOPWR:
          curr_str = "BT_ENOPWR: Power is off or cable is disconnected";
          break;

      case BT_ENOSUP:
          curr_str = "BT_ENOSUP: Not supported in this implementation";
          break;

      case BT_ESTATUS: /* Handled above. Here to avoid compiler warning! */
          break;

      case BT_ELCARD:
          curr_str = "BT_ELCARD: Local card failed diagnostics - replace local card";
          break;

      case BT_ERCARD:
          curr_str = "BT_ERCARD: Remote card failed diagnostics - replace remote card";
          break;

      case BT_ECABLE:
          curr_str = "BT_ECABLE: Interface cable failed diagnostics - replace cable";
          break;

      case BT_EPAIR:
          curr_str = "BT_EPAIR: Adapter card pair failed diagnostics - replace both cards";
          break;

      case BT_SUCCESS:
          curr_str = "BT_SUCCESS: No error occurred";
          break;
    
      /* End of common Mirror API bt_error_t types */

      default:
          return NULL;

    } /* end switch */

#undef    ADD_MSG

    curr_str_len = strlen(curr_str);
    (void) strncat(buf_p, curr_str, buf_remaining);
    if (curr_str_len >= (signed) buf_remaining) {
        return NULL;
    }

    return buf_p;

} /* end bt_strerror */
