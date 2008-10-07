/******************************************************************************
**
**	Filename:	bt_string.c
**
**	Purpose:	Next Generation NanoBus Mirror API string routines
**
**	Functions:	bt_str2dev(), bt_dev2str(), bt_reg2str()
**
**      $Revision: 742 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef	LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 742 $" __DATE__;
#endif	/* LINT */


#include	<stddef.h>
#include	<ctype.h>
#include	<string.h>

#include	"btapi.h"


/*****************************************************************************
**
**	Name:		bt_dev2str
**
**	Purpose:	Converts from the bt_dev_t to an ASCII name.
**
**	Args:
**	    type	Access type to get name for.
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    NULL	Invalid device type passed in.
**	    Otherwise	String form of that logical device type.
**
**	Notes:
**
*****************************************************************************/

const char * bt_dev2str(
    bt_dev_t type
    )
{
    const char * retvalue;

    switch (type) {
      case BT_AXSIO:
	retvalue = "BT_DEV_NIO";
	break;

      case BT_AXSRDP:
        retvalue = "BT_DEV_RDP";
	break;

      case BT_AXSRI:
	retvalue = "BT_DEV_A16";
	break;

      case BT_AXSRR:
      case BT_AXSRE:
	retvalue = "BT_DEV_A32";
	break;

      case BT_AXS24:
	retvalue = "BT_DEV_A24";
	break;

      case BT_AXSLM:
	retvalue = "BT_DEV_LM";
	break;

      case BT_AXSLDP:
        retvalue = "BT_DEV_LDP";
	break;

      default:
	retvalue = NULL;
	break;
    }

    return retvalue;
}

/*****************************************************************************
**
**	Name:		bt_str2dev
**
**	Purpose:	Converts from an ASCII name to the bt_dev_t.
**
**	Args:
**	    name	Name to convert
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    BT_MAX_DEV	Invalid or unrecognized name.
**	    otherwise	access type corresponding to that name.
**
**	Notes:
**
*****************************************************************************/

bt_dev_t bt_str2dev(
    const char * name_p
    )
{
#define	LOGDEV_PREFIX	"BT_DEV_"
    bt_dev_t	retvalue = BT_MAX_DEV;

    if (strncmp(name_p, LOGDEV_PREFIX, strlen(LOGDEV_PREFIX)) == 0) {
        name_p += strlen(LOGDEV_PREFIX);
    }

    switch (toupper(*name_p)) {
      case 'A':
        switch (*(name_p + 1)) {
	  case '1':	/* A16 */
	    if ('6' == *(name_p + 2)
		&& '\0' == *(name_p + 3)) {
		retvalue = BT_DEV_A16;
	    }
	    break;

	  case '2':	/* A24 */
	    if ('4' == *(name_p + 2)
		&& '\0' == *(name_p + 3)) {
		retvalue = BT_DEV_A24;
	    }
	    break;

	  case '3':	/* A32 */
	    if ('2' == *(name_p + 2)
		&& '\0' == *(name_p + 3)) {
		retvalue = BT_DEV_A32;
	    }
	    break;

	  default:
	    break;
	}
	break;

      case 'D':
        /* Dual Port */
        if ('P' == toupper(*(name_p + 1))
	    && '\0' == *(name_p + 2)) {
	    retvalue = BT_DEV_DP;
        } else if ('I' == toupper(*(name_p + 1))
	    && 'A' == toupper(*(name_p + 2))
	    && 'G' == toupper(*(name_p + 3))
	    && '\0' == *(name_p + 4)) {
	    retvalue = BT_DEV_DIAG;
	/* NBUS DEFAULT is BT_DEV_RDP */
        } else if ('E' == toupper(*(name_p + 1))
	    && 'F' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_RDP;
        } else if ('E' == toupper(*(name_p + 1))
	    && 'F' == toupper(*(name_p + 2))
	    && 'A' == toupper(*(name_p + 3))
	    && 'U' == toupper(*(name_p + 4))
	    && 'L' == toupper(*(name_p + 5))
	    && 'T' == toupper(*(name_p + 6))
	    && '\0' == *(name_p + 7)) {
	    retvalue = BT_DEV_RDP;
	}
	break;

      case 'I':
        /* Remote Bus I/O */
	if ('O' == toupper(*(name_p + 1))
	    && '\0' == *(name_p + 2)) {
	    retvalue = BT_DEV_IO;
	}
	break;

      case 'L':
        /* Local memory */
	if ('M' == toupper(*(name_p + 1))
	    && '\0' == *(name_p + 2)) {
	    retvalue = BT_DEV_LM;
	} else if ('D' == toupper(*(name_p + 1))
	    && 'P' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_LDP;
	}
	break;

      case 'M':
        /* Default memory device */
	if ('E' == toupper(*(name_p + 1))
	    && 'M' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_MEM;
	}
	break;
	     
      case 'N':
        /* Node I/O (registers) device */
	if ('I' == toupper(*(name_p + 1))
	    && 'O' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_NIO;
	}
	break;
	     
      case 'O':
        /* Old Remote Extended (ORE) memory device for driver testing only! */
	/* The Mirror API must NOT have a Remote Extended (RE) device */
	if ('R' == toupper(*(name_p + 1))
	    && 'E' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_RE;       /* For driver testing only! */
	}
	break;

      case 'R':
        /* Remote memory */
	/* The Mirror API must NOT have a Remote Extended (RE) device */
	if ('R' == toupper(*(name_p + 1))
	    && '\0' == *(name_p + 2)) {
	    retvalue = BT_DEV_RR;
	} else if ('D' == toupper(*(name_p + 1))
	    && 'P' == toupper(*(name_p + 2))
	    && '\0' == *(name_p + 3)) {
	    retvalue = BT_DEV_RDP;
	}
	break;

      default:
        break;
    }

    return retvalue;
}

/*****************************************************************************
**
**	Name:		bt_reg2str
**
**	Purpose:	Converts from the bt_reg_t to full name.
**
**	Args:
**	    type	Link register to get name for.
**
**	Modifies:
**	    None
**	    
**	Returns:
**	    NULL	Unknown bt_reg_t value
**	    otherwise	String form of that register name.
**
**	Notes:
**
*****************************************************************************/

const char * bt_reg2str(
    bt_reg_t reg
    )
{
    const char * retvalue;

#define	BT_REG_NAME(x) \
      case x: {retvalue = #x ; break; }

    switch (reg) {
      BT_REG_NAME(BT_LOC_CMD1);
      BT_REG_NAME(BT_LOC_INT_CTRL);
      BT_REG_NAME(BT_LOC_INT_STATUS);
      BT_REG_NAME(BT_LOC_STATUS);
      BT_REG_NAME(BT_LOC_HANDSHAKE);
      BT_REG_NAME(BT_LOC_CONFIG_CTRL);
      BT_REG_NAME(BT_LOC_RCMD2);
      BT_REG_NAME(BT_LOC_RPAGE);
      BT_REG_NAME(BT_LOC_AMOD);
      BT_REG_NAME(BT_LDMA_CMD);
      BT_REG_NAME(BT_LDMA_RMD_CNT);
      BT_REG_NAME(BT_LDMA_ADDR);
      BT_REG_NAME(BT_LDMA_PKT_CNT);
      BT_REG_NAME(BT_LOC_TCSIZE);
      BT_REG_NAME(BT_LOC_256MB_PAGE);
      BT_REG_NAME(BT_LOC_RCMD1);
      BT_REG_NAME(BT_LOC_BUS_CTRL);
      BT_REG_NAME(BT_LOC_LOOPBACK);
      BT_REG_NAME(BT_LOC_MREG_CTRL);
      BT_REG_NAME(BT_LOC_SEMA0);
      BT_REG_NAME(BT_LOC_SEMA1);
      BT_REG_NAME(BT_LOC_SEMA2);
      BT_REG_NAME(BT_LOC_SEMA3);
      BT_REG_NAME(BT_LOC_SEMA4);
      BT_REG_NAME(BT_LOC_SEMA5);
      BT_REG_NAME(BT_LOC_SEMA6);
      BT_REG_NAME(BT_LOC_SEMA7);

      BT_REG_NAME(BT_REM_CMD1);
      BT_REG_NAME(BT_REM_STATUS);
      BT_REG_NAME(BT_REM_CMD2);
      BT_REG_NAME(BT_REM_PAGE);
      BT_REG_NAME(BT_REM_AMOD);
      BT_REG_NAME(BT_REM_IACK);
      BT_REG_NAME(BT_REM_CARD_ID);
      BT_REG_NAME(BT_REM_MREG_CTRL);
      BT_REG_NAME(BT_RDMA_ADDR);
      BT_REG_NAME(BT_RDMA_RMD_CNT);
      BT_REG_NAME(BT_REM_SLAVE_STATUS);
      BT_REG_NAME(BT_REM_SLAVE_CLRERR);
      BT_REG_NAME(BT_REM_SEMA0);
      BT_REG_NAME(BT_REM_SEMA1);
      BT_REG_NAME(BT_REM_SEMA2);
      BT_REG_NAME(BT_REM_SEMA3);
      BT_REG_NAME(BT_REM_SEMA4);
      BT_REG_NAME(BT_REM_SEMA5);
      BT_REG_NAME(BT_REM_SEMA6);
      BT_REG_NAME(BT_REM_SEMA7);

      BT_REG_NAME(BT_MAX_REGISTER);

      default:
        retvalue = "Unknown";
	break;
    }

#undef BT_REG_NAME

    return retvalue;
}

