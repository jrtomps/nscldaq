/******************************************************************************
**
**      Filename:   bt_info.c
**
**      Purpose:    Next Generation Mirror API info routines.
**
**      Functions:  bt_get_info(), bt_set_info()
**
**      $Revision$
**
******************************************************************************/
/******************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
******************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision$ "__DATE__;
#endif  /* !LINT */

/*
**  Include files
*/
#include <stddef.h>
#include <unistd.h>

#include "btio.h"
#include "btapi.h"
#include "btpiflib.h"



/******************************************************************************
**
**      Function:       bt_get_info()
**
**      Purpose:        Gets current setting of various information parameters.
**
**      Args:
**          btd         Unit descriptor
**          param       What parameter to set
**          value       Value to set parameter to 
**
**      Modifies:
**
**      Returns:
**          BT_SUCCESS  Successful.
**          else        error number.
**
******************************************************************************/

bt_error_t bt_get_info(
    bt_desc_t    btd, 
    bt_info_t    param, 
    bt_devdata_t * value_p
    )
{
    bt_param_t bt_info_block;

    bt_error_t retvalue;

    if (BT_DESC_BAD(btd)) {
	/* Bad Descriptor */
        DBG_STR("bt_get_info:bad descriptor.");
	retvalue = BT_EDESC;
	return retvalue;
    }

    bt_info_block.error = BT_SUCCESS;
    bt_info_block.param = param;                               

    do {	/* Allow us to use break for error exit */

	retvalue = bt_ctrl(btd, BIOC_DEV_ATTRIB, &bt_info_block);
    /*  retvalue = bt_ctrl(btd, BIOC_GET_INFO, &bt_info_block); CAR Solaris */
	if (BT_SUCCESS == retvalue) {
	    retvalue = bt_info_block.error;
	}
	if (BT_SUCCESS != retvalue) {
	    /* Exit with error */
	    *value_p = 0;
	    break;
	}

	/* Update the value */
	*value_p = bt_info_block.value;

    } while (FALSE);	/* Errors use break to get out */

    return retvalue;
}

/******************************************************************************
**
**      Function:       bt_set_info()
**
**      Purpose:        Sets current value of various information parameters.
**
**      Args:
**          btd         Unit descriptor
**          param       What parameter to set
**          value       Value to set parameter to 
**
**      Modifies:
**
**      Returns:
**          BT_SUCCESS  Successful.
**          else        error number.
**
******************************************************************************/

bt_error_t bt_set_info(
    bt_desc_t    btd, 
    bt_info_t    param, 
    bt_devdata_t value
    )
{
    bt_param_t bt_info_block;

    bt_error_t retvalue = BT_SUCCESS;

    if (BT_DESC_BAD(btd)) {
	/* Bad Descriptor */
        DBG_STR("bt_set_info:bad descriptor.");
	retvalue = BT_EDESC;
	return retvalue;
    }

    bt_info_block.error = BT_SUCCESS;
    bt_info_block.param = param;
    bt_info_block.value = value;
    retvalue = bt_ctrl(btd, BIOC_PARAM, &bt_info_block);
    /* retvalue = bt_ctrl(btd, BIOC_SET_INFO, &bt_info_block); CAR solaris */
    if (BT_SUCCESS == retvalue) {
        retvalue = bt_info_block.error;
    }

    return retvalue;
}

