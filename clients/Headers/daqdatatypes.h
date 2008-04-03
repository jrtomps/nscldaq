/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/*
**++
**  FACILITY:
**
**      Data types
**
**  ABSTRACT:
**
**      Data types for the host processor.
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  CREATION DATE:     7-Oct-1987
**
**  MODIFICATION HISTORY:
**--
**/

#ifndef __DAQTYPES_H

#include <sys/types.h>

typedef    char		    INT8;
typedef    unsigned char    UINT8;
typedef    short	    INT16;
typedef    unsigned short   UINT16;
typedef    int		    INT32;
typedef    unsigned int     UINT32;


#define SHORT_SIGNATURE 0x0102	/*	 
				**  Will be byte flipped by buffer swab
				*/	 

#define LONG_SIGNATUREHI 0x0102 /* Our assumption is that everything is  */
#define LONG_SIGNATURELO 0x0304  /* Written little endian */

#define __DAQTYPES_H
#endif
